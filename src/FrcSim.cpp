#include <iostream>
#include <fstream>

#include <map>
#include <vector>
#include <algorithm>

#include <json/json.h>

#include <ghoul/GPtr.H>
#include <ghoul/GString.H>
#include <ghoul/ISingleton.H>
#include <ghoul/GString.H>
#include <ghoul/GPoint.H>
#include <ghoul/GRect.H>
#include <ghoul/GPair.H>
#include <ghoul/GTriple.H>
#include <ghoul/GFileName.H>
#include <ghoul/GMessage.H>
#include <ghoul/GCallback.H>
#include <ghoul/GException.H>

using namespace std;

#include "FrcSim.h"

#ifdef ANDROID
#include <android/log.h>
#define fprintf(a, ...) ((void)__android_log_print(ANDROID_LOG_INFO, "AerialAssist", __VA_ARGS__))
#endif // ANDROID

// Declare our game instance
AerialAssist game;

const GFileName AerialAssist::_kFieldBundle = "res/models/AerialAssistField.gpb";
const GFileName AerialAssist::_kRobotBundle = "res/models/AerialAssistRobot.gpb";

AerialAssist::AerialAssist() :
    _camera_h_node(NULL),
    _camera_v_node(NULL),
    _spotlight_node(NULL),
    _floor_node(NULL),
    _robot_node(NULL),
    _catapult_node(NULL),
    _camera(NULL),
    _scene(NULL),
    _light_node(NULL),
    _light(NULL),
    _spotlight(NULL),
    _font(NULL),
    _elapsedTime(0.0),
    _wireframe(false)
{
}

void AerialAssist::initialize()
{
    
	// Create the font and scene
    _font = Font::create("res/ui/arial.gpb");
    
    GFileName resPath = FileSystem::getResourcePath();
    GFileName textureMapFile = "res/data/TextureMap.json";
    GFileName AerialAssistField = _kFieldBundle;
    GString fullPath = resPath + textureMapFile;
    
    // Copy files from "res" directory to Android SD card
    FileSystem::createFileFromAsset(textureMapFile);
    
    loadTextureMap((const char*)fullPath);
    
#ifdef DEBUG
    fprintf(stderr, "[Debug] Loading field model from GPB \"%s\"\n", (const char*)AerialAssistField);
#endif // DEBUG
    
    // load the scene from the gameplay binary file
    Bundle* field_bundle_ptr = Bundle::create(AerialAssistField);

    // create scene
    _scene = field_bundle_ptr->loadScene();
    
    // set ambient light color
#ifdef DEBUG
    fprintf(stderr, "[Debug] Setting ambient color\n");
#endif // DEBUG
    _scene->setAmbientColor(0.18f, 0.18f, 0.18f);
    
    // load robot
#ifdef DEBUG
    fprintf(stderr, "[Debug] Loading robot model from GPB \"%s\"\n", (const char*)_kRobotBundle);
#endif // DEBUG
    Bundle *robot_bundle_ptr = Bundle::create(_kRobotBundle);
    Node* robot = robot_bundle_ptr->loadNode("AerialAssistRobot")->clone();
    if (robot)
    {
        _catapult_node = robot->findNode("Catapult");
        _robot_node = Node::create("Robot");
        _robot_node->addChild(robot);
        robot->setTranslation(0, 0, 21);
        _scene->addNode(_robot_node);
        _robot_node->setTranslation(Vector3(0, 0, 0));
#ifdef DEBUG
        fprintf(stderr, "[Debug] Loaded model (%f, %f, %f)\n", _robot_node->getTranslationX(), _robot_node->getTranslationY(), _robot_node->getTranslationZ());
#endif // DEBUG
        _scene->addNode(_robot_node);
    }
    
    SAFE_RELEASE(field_bundle_ptr);
    SAFE_RELEASE(robot_bundle_ptr);

    // Get the box model and initialize its material parameter values and bindings
//    Node* boxNode = _scene->findNode("box");
//    Model* boxModel = boxNode->getModel();
//    Material* boxMaterial = boxModel->getMaterial();
    
#ifdef DEBUG
    fprintf(stderr, "[Debug] Creating camera\n");
#endif // DEBUG
    
    // create camera
    _camera_h_node = Node::create("camera_h");
    _scene->addNode(_camera_h_node);
    _camera_v_node = Node::create("camera_v");
    _camera_h_node->addChild(_camera_v_node);
    _camera = Camera::createPerspective(50.0f, getAspectRatio(), 0.01f, 1000.0f);
    _camera_v_node->setCamera(_camera);
    _scene->setActiveCamera(_camera);
    
    _camera_h_node->setRotation(Vector3(0.0f, 1.0f, 0.0f), MATH_DEG_TO_RAD(180));
    _camera_h_node->setTranslation(0.0, 384.0, 58.0);
    _camera_v_node->setRotation(Vector3(1.0f, 0.0f, 0.0f), MATH_DEG_TO_RAD(-90));
    
#ifdef DEBUG
    fprintf(stderr, "[Debug] Creating light\n");
#endif // DEBUG
    
    // create light
    _light_node = Node::create("light");
    _camera_v_node->addChild(_light_node);
    _light = Light::createPoint(Vector3(1.0f, 1.0f, 1.0f), 1500.0f);
    _light_node->setCamera(_camera);
    
	// Create a spotlight and create a reference icon for the light
	_spotlight = Light::createSpot(Vector3::one(), 25000.0f, MATH_DEG_TO_RAD(0.01), MATH_DEG_TO_RAD(45.0));
	_spotlight_node = Node::create("spotLight");
	_spotlight_node->setLight(_spotlight);
    _spotlight_node->setRotation(Vector3(1.0f, 0.0f, 0.0f), MATH_DEG_TO_RAD(0));
	_spotlight_node->setTranslation(0.0f, 0.0f, 750.0f);
	_scene->addNode(_spotlight_node);
    
#ifdef DEBUG
    fprintf(stderr, "[Debug] Walking all scene nodes to set material\n");
#endif // DEBUG
    
    // Visit all the nodes in the scene to set the material
    _scene->visit(this, &AerialAssist::setSceneMaterial);
    
#ifdef DEBUG
    fprintf(stderr, "[Debug] Creating floor model\n");
#endif // DEBUG
    
    // Add a floor to the scene
//    createFloorModels();
}

void AerialAssist::setMaterial(Node* node_ptr, const char* diffuse_string_ptr, const char* normal_string_ptr, float specularity)
{
    if (node_ptr->getModel() == NULL)
    {
        return;
    }
#ifdef DEBUG
    fprintf(stderr, "[Debug]\tAssigning material for node \"%s\" to \"%s\"\n", node_ptr->getId(), diffuse_string_ptr);
#endif // DEBUG
    // create material
    bool material_set = false;
    Model* model = node_ptr->getModel();
//    Vector3 color;
//    if (model->getMaterial() != NULL)
//    {
//        material_set = true;
//        MaterialParameter *color_param = model->getMaterial()->getParameter("u_color");
//    }
//    if (GString(node_ptr->getId()).Left(17) != "AerialAssistField")
//    {
//        int count = model->getMesh()->getPartCount();
//        if (count > 0)
//        {
//            for (int i = 0; i < count; i++)
//            {
//                Material* old_material = model->getMaterial(i);
//                if (old_material != NULL)
//                {
//                    material_set = true;
//                    MaterialParameter *color_param = old_material->getParameter("u_color");
//                    if (color_param)
//                    {
//                        Texture::Sampler *samp = color_param->getSampler();
//                    }
//                }
//            }
//        }
//    }
    Material* material_ptr = NULL;
    if (!material_set)
    {
        material_ptr = model->setMaterial(VERT_SHADER, FRAG_SHADER, DEF_SHADER);
        Texture::Sampler* texture_sampler_ptr = material_ptr->getParameter("u_diffuseTexture")->setValue(diffuse_string_ptr, true);
        texture_sampler_ptr->setFilterMode(Texture::LINEAR_MIPMAP_LINEAR, Texture::LINEAR_MIPMAP_LINEAR);
        texture_sampler_ptr->setWrapMode(Texture::REPEAT, Texture::REPEAT);
    }
    else
    {
        material_ptr = model->setMaterial("res/shaders/colored.vert", "res/shaders/colored.frag", "SPOT_LIGHT_COUNT 1");
    }
    material_ptr->setParameterAutoBinding("u_worldViewMatrix", RenderState::WORLD_VIEW_MATRIX);
    material_ptr->setParameterAutoBinding("u_worldViewProjectionMatrix", RenderState::WORLD_VIEW_PROJECTION_MATRIX);
    material_ptr->setParameterAutoBinding("u_inverseTransposeWorldViewMatrix", RenderState::INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX);
#if BUMP_MAPPED == true
    Texture::Sampler* normal_sampler_ptr = material_ptr->getParameter("u_normalmapTexture")->setValue(normal_string_ptr, true);
    normal_sampler_ptr->setFilterMode(Texture::LINEAR_MIPMAP_LINEAR, Texture::LINEAR_MIPMAP_LINEAR);
    normal_sampler_ptr->setWrapMode(Texture::REPEAT, Texture::REPEAT);
#endif
    material_ptr->getStateBlock()->setCullFace(true);
    material_ptr->getStateBlock()->setDepthTest(true);
    material_ptr->getStateBlock()->setDepthWrite(true);
    material_ptr->getParameter("u_ambientColor")->bindValue(_scene, &Scene::getAmbientColor);

    // bind point light to material
//    material_ptr->getParameter("u_pointLightPosition[0]")->bindValue(_camera_h_node, &Node::getTranslationView);
//    material_ptr->getParameter("u_pointLightRangeInverse[0]")->bindValue(_light, &Light::getRangeInverse);
//    material_ptr->getParameter("u_pointLightColor[0]")->bindValue(_light, &Light::getColor);

    // bind spotlight to material
    material_ptr->getParameter("u_spotLightPosition[0]")->bindValue(_spotlight_node, &Node::getTranslationView);
    material_ptr->getParameter("u_spotLightRangeInverse[0]")->bindValue(_spotlight, &Light::getRangeInverse);
    material_ptr->getParameter("u_spotLightColor[0]")->bindValue(_spotlight, &Light::getColor);
    material_ptr->getParameter("u_spotLightDirection[0]")->bindValue(_spotlight_node, &Node::getForwardVectorView);
    material_ptr->getParameter("u_spotLightInnerAngleCos[0]")->bindValue(_spotlight, &Light::getInnerAngleCos);
    material_ptr->getParameter("u_spotLightOuterAngleCos[0]")->bindValue(_spotlight, &Light::getOuterAngleCos);
#if SPECULAR == true
//    if (specularity != 0.0f)
//    {
    material_ptr->getParameter("u_specularExponent")->setValue(specularity);
//    }
#endif
    int count = model->getMesh()->getPartCount();
    if (count > 0)
    {
        for (int i = 0; i < count; i++)
        {
            if (model->getMaterial(i) != NULL)
                model->setMaterial(model->getMaterial(), i);
        }
    }
}

void AerialAssist::finalize()
{
    SAFE_RELEASE(_robot_node);
    SAFE_RELEASE(_spotlight);
    SAFE_RELEASE(_spotlight_node);
    SAFE_RELEASE(_scene);
}

void AerialAssist::createFloorModels(void)
{
    float size = 1.0f;
    Mesh* tile_mesh_ptr = createFloorMesh();
    
    // create node
    _floor_node = Node::create("floor");
    _scene->addNode(_floor_node);
    _floor_node->release();
    
    Model* tile_model_ptr = Model::create(tile_mesh_ptr);
    _floor_node->setModel(tile_model_ptr);
    SAFE_RELEASE(tile_model_ptr);
    
    // create material
    setMaterial(_floor_node, "res/textures/tile_floor.png", "", 50.0f);
    Material* material_ptr = _floor_node->getModel()->getMaterial();
    
    // set position (converting ornament 2D space into model 3D space)
    float tile_x = 0;                // (3D) x =  x (2D)
    float tile_y = 0;                // (3D) y =  0 (2D)
    float tile_z = 0;                // (3D) z = -y (2D)
    _floor_node->setTranslation(Vector3(tile_x, tile_y, tile_z));

    SAFE_RELEASE(tile_mesh_ptr);
}

Mesh* AerialAssist::createFloorMesh(void)
{
    float vertices[] =
    {
        // bottom (-z)
        -500, -500, -0.01,   0,  1,  0,    0, 1,
        500,  -500, -0.01,   0,  1,  0,    1, 1,
        -500,  500, -0.01,   0,  1,  0,    0, 0,
        -500,  500, -0.01,   0,  1,  0,    0, 0,
        500,  -500, -0.01,   0,  1,  0,    1, 1,
        500,   500, -0.01,   0,  1,  0,    1, 0
    };
    unsigned int vertexCount = 6;
    unsigned int indexCount = 6;
    VertexFormat::Element elements[] =
    {
        VertexFormat::Element(VertexFormat::POSITION, 3),
        VertexFormat::Element(VertexFormat::NORMAL, 3),
        VertexFormat::Element(VertexFormat::TEXCOORD0, 2)
    };
    Mesh* mesh = Mesh::createMesh(VertexFormat(elements, 3), vertexCount, false);
    if (mesh == NULL)
    {
        GP_ERROR("Failed to create floor mesh.");
        return NULL;
    }
    mesh->setPrimitiveType(Mesh::TRIANGLES);
    mesh->setVertexData(vertices, 0, vertexCount);
    BoundingSphere boundingSphere(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
    mesh->setBoundingSphere(boundingSphere);
    return mesh;
}

void AerialAssist::loadTextureMap(const string &filename)
{
    std::string jsonInput;
    std::ifstream inFile;
    inFile.open(filename.c_str(), std::ios_base::in);
    if (inFile)
    {
        inFile.seekg(0, std::ios::end);
        long length = (long)inFile.tellg();
        jsonInput.resize(length);
        inFile.seekg(0, std::ios::beg);
        inFile.read(&jsonInput[0], jsonInput.size());
        inFile.close();
        
        Json::Value root;
        Json::Reader reader;
        if ( !reader.parse(jsonInput, root) )
        {
#if DEBUG
            fprintf(stderr, "[ERROR] File \"%s\" not parsed\n", filename.c_str());
#endif // DEBUG
            return;
        }
        Json::Value textureListArray = root["textureMapList"];
        if (textureListArray.isArray())
        {
            for (int i = 0; i < textureListArray.size(); i++)
            {
                Json::Value node = textureListArray[i];
                string id = node.get("node", "").asCString();
                string texture = node.get("texture", "").asCString();
                bool transparent = node.get("transparent", false).asBool();
#ifdef DEBUG
//                fprintf(stderr, "[Debug]\t\tReading texture \"%s\" for node \"%s\" (alpha %s)\n", texture.c_str(), id.c_str(), transparent?"true":"false");
#endif // DEBUG
                GPair<string, bool> pair(texture, transparent);
                textureList.insert(make_pair(id, pair));
            }
        }
    }
#if DEBUG
    else
    {
        fprintf(stderr, "[ERROR] File \"%s\" not opened\n", filename.c_str());
    }
#endif // DEBUG
}

void AerialAssist::update(float elapsedTime)
{
    _elapsedTime += elapsedTime;
#ifdef DEBUG
//    fprintf(stderr, "[Trace] elapsedTime=%f\n", _elapsedTime / 1000.0);
#endif // DEBUG
    _robot_node->setRotation(Vector3(0.0f, 0.0f, 1.0f), sin(_elapsedTime / 1000.0) * 2);
    if (_catapult_node)
    {
        _catapult_node->setRotation(Vector3(1.0f, 0.0f, 0.0f), abs(sin(_elapsedTime / 500) * 1.5));
    }
//    _spotlight_node->setTranslationZ(400.0 + sin(_elapsedTime / 1000.0) * 250);
//    _camera_h_node->rotateX(MATH_DEG_TO_RAD((float)elapsedTime / 10000.0f * 180.0f));
//    _camera_h_node->rotateY(MATH_DEG_TO_RAD((float)elapsedTime / 27500.0f * 180.0f));
}

void AerialAssist::render(float elapsedTime)
{
    // Clear the color and depth buffers
    clear(CLEAR_COLOR_DEPTH, Vector4::zero(), 1.0f, 0);

    
    // Visit all the nodes in the scene to build our render queues
    for (unsigned int i = 0; i < QUEUE_COUNT; ++i)
    {
        _renderQueues[i].clear();
    }
    _scene->visit(this, &AerialAssist::buildRenderQueues);
    
    // Iterate through each render queue and draw the nodes in them
    for (unsigned int i = 0; i < QUEUE_COUNT; ++i)
    {
        std::vector<Node*>& queue = _renderQueues[i];
        
#ifdef DEBUG
//        fprintf(stderr, "[Debug] Rendering %s queue with %lu nodes\n", i==0?"opaque":"transparent", queue.size());
#endif // DEBUG
        for (size_t j = 0, ncount = queue.size(); j < ncount; ++j)
        {
            queue[j]->getModel()->draw(_wireframe);
        }
    }
    
    // draw the frame rate
    drawFrameRate(_font, Vector4::one(), 5, 1, getFrameRate());
}

void AerialAssist::drawFrameRate(Font* font, const Vector4& color, unsigned int x, unsigned int y, unsigned int fps)
{
    char buffer[10];
    sprintf(buffer, "%u", fps);
    font->start();
    font->drawText(buffer, x, y, color, font->getSize());
    font->finish();
}

bool AerialAssist::buildRenderQueues(Node* node)
{
    Model* model = node->getModel();
    if (model)
    {
        // Perform view-frustum culling for this node
//        if (__viewFrustumCulling && !node->getBoundingSphere().intersects(_scene->getActiveCamera()->getFrustum()))
//        return true;
        
        // Determine which render queue to insert the node into
        std::vector<Node*>* queue;
        if (node->hasTag("transparent"))
        {
            queue = &_renderQueues[QUEUE_TRANSPARENT];
        }
        else
        {
            queue = &_renderQueues[QUEUE_OPAQUE];
        }
        queue->push_back(node);
    }
    return true;
}

bool AerialAssist::setSceneMaterial(Node* node)
{
    // If the node visited contains a model, draw it
    Model* model = node->getModel();
    if (model)
    {
        string id = node->getId();
        string texture = "res/textures/gray.png";
        bool transparent = false;
        map<string, GPair<string, bool> >::const_iterator it = textureList.find(id);
        if (it != textureList.end())
        {
            texture = it->second.first;
            transparent = it->second.second;
        }
#ifdef DEBUG
//        fprintf(stderr, "[Debug]\t\tSetting %smaterial for node \"%s\" to \"%s\"\n", (transparent?"transparent ":""), id.c_str(), texture.c_str());
#endif // DEBUG
        if (transparent)
        {
            node->setTag("transparent", "true");
        }
        if (texture != "")
        {
            setMaterial(node, texture.c_str(), NULL, 0.0);
        }
    }
    return true;
}

void AerialAssist::keyEvent(Keyboard::KeyEvent evt, int key)
{
    if (evt == Keyboard::KEY_PRESS)
    {
        switch (key)
        {
        case Keyboard::KEY_ESCAPE:
            exit();
            break;
        }
    }
}

void AerialAssist::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex)
{
    switch (evt)
    {
    case Touch::TOUCH_PRESS:
        _wireframe = !_wireframe;
        break;
    case Touch::TOUCH_RELEASE:
        break;
    case Touch::TOUCH_MOVE:
        break;
    };
}
