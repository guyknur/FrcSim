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
#include <ghoul/GRegEx.H>

using namespace std;

#include <gameplay.h>

using namespace gameplay;

#include "json/IJsonSerializable.h"
#include "Robot.h"
#include "FrcSim.h"

#ifdef ANDROID
#include <android/log.h>
#define fprintf(a, ...) ((void)__android_log_print(ANDROID_LOG_INFO, "FrcSim", __VA_ARGS__))
#endif // ANDROID

// Declare our game instance
AerialAssist game;

const GFileName AerialAssist::_kFieldBundle = "res/models/AerialAssistField.gpb";
const GFileName AerialAssist::_kFieldTextureMap = "/res/data/AerialAssistFieldTextureMap.json";
const int AerialAssist::kHudWidth = 320;
const int AerialAssist::kHudHeight = 200;

//----------------------------------------------------------------------
//
// AerialAssist()
//
//----------------------------------------------------------------------
AerialAssist::AerialAssist() :
    _spotlight_node(NULL),
    _scene(NULL),
    _spotlight(NULL),
    _offscreen_framebuffer(NULL),
    _robot(NULL),
    _font(NULL),
    _blank(NULL),
    _elapsedTime(0.0),
    _active_camera(DriverStation),
    _hud_camera(Overhead),
    _wireframe(false),
    _view_frustrum_culling(true)
{
    for (int i = 0; i < CameraCount; i++)
    {
        _camera[i] = NULL;
    }
}

//----------------------------------------------------------------------
//
// initialize()
//
//----------------------------------------------------------------------
void AerialAssist::initialize()
{
    // create offscreen framebuffer for use by HUD camera view
    _offscreen_framebuffer = FrameBuffer::create("hud", getWidth(), getHeight());
#ifdef DEBUG
    fprintf(stderr, "[Debug] Created offscreen framebuffer with RenderTarget of %s\n", (_offscreen_framebuffer->getRenderTargetCount()?_offscreen_framebuffer->getRenderTarget()->getId():"NULL"));
#endif // DEBUG
    
    _blank = SpriteBatch::create("res/textures/black.png");
    
	// Create the font and scene
    _font = Font::create("res/ui/arial.gpb");
    
    GFileName resPath = FileSystem::getResourcePath();
    GFileName textureMapFile = _kFieldTextureMap;
    GFileName AerialAssistField = _kFieldBundle;
    GString fullPath = resPath + textureMapFile;
    
    // Copy files from "res" directory to Android SD card
    FileSystem::createFileFromAsset(textureMapFile);
    
    textureList.clear();
    loadTextureMap((const char*)fullPath);
    
#ifdef DEBUG
    fprintf(stderr, "[Debug] Loading field model from GPB \"%s\"\n", (const char*)AerialAssistField);
#endif // DEBUG
    
    // load the scene from the gameplay binary file
    Bundle* field_bundle_ptr = Bundle::create(AerialAssistField);

    // create scene
    _scene = field_bundle_ptr->loadScene();
    
    // set ambient light color
    _scene->setAmbientColor(0.25f, 0.25f, 0.25f);
    
    _robot = new Robot("/res/data/AerialAssist2028.json");
    Node *robot_node = _robot->getNode();
    if (robot_node)
    {
        GFileName textureMap = resPath + _robot->getTextureMapFile();
        loadTextureMap((const char*)textureMap);
        _scene->addNode(robot_node);
        
        // $$$ FIX ME - Set robot to midcourt for now
        robot_node->setTranslation(Vector3(0, 0, 0));
        robot_node->setRotation(Vector3(1.0, 1.0, 1.0), 0.0);
        
        Node* chase_node = createCamera("Chase", &_camera[Chase]);
        robot_node->addChild(chase_node);
        chase_node->setTranslation(0.0, 108.0, 36.0);
        
        Node* side_node = createCamera("Side", &_camera[Side]);
        robot_node->addChild(side_node);
        side_node->setTranslation(-84.0, 0.0, 24.0);
        Node *hor = side_node->findNode("camera_h");
        if (hor)
        {
            hor->setRotation(Vector3(0.0f, 0.0f, 1.0f), MATH_DEG_TO_RAD(-90));
        }
    }
    
#ifdef DEBUG
    fprintf(stderr, "[Debug] Creating cameras\n");
#endif // DEBUG
    Node* camera = createCamera("Driver", &_camera[DriverStation]);
    camera->setTranslation(0.0, 384.0, 58.0);
    _scene->addNode(camera);
    
    Node* overhead = createCamera("Overhead", &_camera[Overhead]);
    overhead->setTranslation(0.0, 0.0, 180.0);
    _scene->addNode(overhead);
    Node *vert = overhead->findNode("camera_v");
    if (vert)
    {
        vert->setRotation(Vector3(1.0f, 0.0f, 0.0f), MATH_DEG_TO_RAD(180));
    }
    
#ifdef DEBUG
    fprintf(stderr, "[Debug] Creating light\n");
#endif // DEBUG
    
	// Create a spotlight and create a reference icon for the light
	_spotlight = Light::createSpot(Vector3::one(), 2500.0f, MATH_DEG_TO_RAD(0.001), MATH_DEG_TO_RAD(40.0));
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
    
    // Add a floor to the scene
    createFloorModel();
}

//----------------------------------------------------------------------
//
// createCamera()
//
//----------------------------------------------------------------------
Node* AerialAssist::createCamera(const GString &cameraName, Camera** camera)
{
    Node *camera_node = Node::create((const char*)cameraName);
    Node *camera_h_node = Node::create("camera_h");
    camera_node->addChild(camera_h_node);
    camera_node->setRotation(Vector3(0.0f, 1.0f, 0.0f), MATH_DEG_TO_RAD(180));
    Node *camera_v_node = Node::create("camera_v");
    camera_h_node->addChild(camera_v_node);
    *camera = Camera::createPerspective(50.0f, getAspectRatio(), 1.0f, 2000.0f);
    camera_v_node->setCamera(*camera);
    camera_v_node->setRotation(Vector3(1.0f, 0.0f, 0.0f), MATH_DEG_TO_RAD(-90));
    return camera_node;
}

//----------------------------------------------------------------------
//
// setMaterial()
//
//----------------------------------------------------------------------
void AerialAssist::setMaterial(Node* node_ptr, const char* diffuse_string_ptr, const char* normal_string_ptr, float specularity)
{
    if (node_ptr->getModel() == NULL)
    {
        return;
    }
#ifdef DEBUG
//    fprintf(stderr, "[Debug]\t\tAssigning material for node \"%s\" to \"%s\"\n", node_ptr->getId(), diffuse_string_ptr);
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

//----------------------------------------------------------------------
//
// finalize()
//
//----------------------------------------------------------------------
void AerialAssist::finalize()
{
    SAFE_RELEASE(_offscreen_framebuffer);
    SAFE_RELEASE(_spotlight);
    SAFE_RELEASE(_spotlight_node);
    SAFE_RELEASE(_scene);
}

//----------------------------------------------------------------------
//
// createFloorModels()
//
//----------------------------------------------------------------------
Node* AerialAssist::createFloorModel(void)
{
    float size = 1.0f;
    Mesh* tile_mesh_ptr = createFloorMesh();
    
    // create node
    Node* floor = Node::create("floor");
    _scene->addNode(floor);
    
    Model* tile_model_ptr = Model::create(tile_mesh_ptr);
    floor->setModel(tile_model_ptr);
    
    // create material
    setMaterial(floor, "res/textures/brown.png", NULL, 0.0f);
    
    // set position (converting ornament 2D space into model 3D space)
    float x = 0;
    float y = 0;
    float z = 0;
    floor->setTranslation(Vector3(x, y, z));
//    floor->release();
    SAFE_RELEASE(tile_model_ptr);
    SAFE_RELEASE(tile_mesh_ptr);
    return floor;
}

//----------------------------------------------------------------------
//
// createFloorMesh()
//
//----------------------------------------------------------------------
Mesh* AerialAssist::createFloorMesh(void)
{
    float vertices[] =
    {
        // bottom (-z)
        -500, -500, 0.0,   0, 0, 1,   0, 0,
         500, -500, 0.0,   0, 0, 1,   1, 0,
        -500,  500, 0.0,   0, 0, 1,   0, 1,
        -500,  500, 0.0,   0, 0, 1,   0, 1,
         500, -500, 0.0,   0, 0, 1,   1, 0,
         500,  500, 0.0,   0, 0, 1,   1, 1
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
    BoundingSphere boundingSphere(Vector3(0.0f, 0.0f, 0.0f), 1414.21356f);
    mesh->setBoundingSphere(boundingSphere);
    return mesh;
}

//----------------------------------------------------------------------
//
// loadTextureMap()
//
//----------------------------------------------------------------------
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
                fprintf(stderr, "[Debug]\t\tReading texture \"%s\" for node \"%s\" (alpha %s)\n", texture.c_str(), id.c_str(), transparent?"true":"false");
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

//----------------------------------------------------------------------
//
// update()
//
//----------------------------------------------------------------------
void AerialAssist::update(float elapsedTime)
{
    _elapsedTime += elapsedTime;
#ifdef DEBUG
//    fprintf(stderr, "[Trace] elapsedTime=%f\n", _elapsedTime / 1000.0);
#endif // DEBUG
    Node* robot_node = NULL;
    if (_robot)
    {
        robot_node = _robot->getNode();
    }
    
    if (robot_node)
    {
//        robot_node->setTranslationY(sin(_elapsedTime / 1500.0) * 150);
        Node* catapult_node = robot_node->findNode("Catapult");
        if (catapult_node)
        {
            catapult_node->setRotation(Vector3(1.0f, 0.0f, 0.0f), abs(sin(_elapsedTime / 500) * 1.5));
        }
    }
    
    Node* cam_node = _scene->findNode("Overhead");
    if (cam_node && robot_node)
    {
        float robot_pos_x = robot_node->getTranslationX();
        float robot_pos_y = robot_node->getTranslationY();
        cam_node->setTranslationX(robot_pos_x);
        cam_node->setTranslationY(robot_pos_y);
    }
}

//----------------------------------------------------------------------
//
// render()
//
//----------------------------------------------------------------------
void AerialAssist::render(float elapsedTime)
{
    
    // Clear the color and depth buffers
    clear(CLEAR_COLOR_DEPTH, Vector4(0.0, 0.0, 0.0, 1.0), 1.0f, 0);
    
    Rectangle default_viewport = getViewport();
    drawScreen(_active_camera);
    
//    _blank->start();
//    _blank->draw(Rectangle(getWidth() - kHudWidth - 12, 8, kHudWidth + 4, kHudHeight + 4), Rectangle(0, 0, 1, 1), Vector4::one());
//    _blank->finish();
    Rectangle hud(getWidth() - kHudWidth - 10, getHeight() - 10 - kHudHeight, kHudWidth, kHudHeight);
    setViewport(hud);
    drawScreen(_hud_camera);
    setViewport(default_viewport);
    
    // draw the frame rate
    drawFrameRate(_font, Vector4::one(), 5, 1, getFrameRate());
}

//----------------------------------------------------------------------
//
// drawScreen()
//
//----------------------------------------------------------------------
void AerialAssist::drawScreen(CameraPosition camera)
{
    _scene->setActiveCamera(_camera[camera]);
    
    // Visit all the nodes in the scene to build our render queues, we have to
    // do this for every camera so buildRenderQueues() can do frustrum culling
    for (unsigned int i = 0; i < QUEUE_COUNT; ++i)
    {
        _renderQueues[i].clear();
    }
    _scene->visit(this, &AerialAssist::buildRenderQueues);
    
    // Iterate through each render queue and draw its nodes
    for (unsigned int i = 0; i < QUEUE_COUNT; ++i)
    {
        std::vector<Node*>& queue = _renderQueues[i];
#ifdef DEBUG
//        fprintf(stderr, "[Debug] Rendering %s queue with %lu nodes for camera %s\n", i==0?"opaque":"transparent", queue.size(), (camera==Overhead?"overhead":(camera==Chase?"chase":"driver")));
#endif // DEBUG
        for (size_t j = 0, ncount = queue.size(); j < ncount; ++j)
        {
            queue[j]->getModel()->draw(_wireframe);
        }
    }
}

//----------------------------------------------------------------------
//
// drawFrameRate()
//
//----------------------------------------------------------------------
void AerialAssist::drawFrameRate(Font* font, const Vector4& color, unsigned int x, unsigned int y, unsigned int fps)
{
    char buffer[10];
    sprintf(buffer, "%u", fps);
    font->start();
    font->drawText(buffer, x, y, color, font->getSize());
    font->finish();
}

//----------------------------------------------------------------------
//
// buildRenderQueues()
//
//----------------------------------------------------------------------
bool AerialAssist::buildRenderQueues(Node* node)
{
    Model* model = node->getModel();
    if (model)
    {
        // Perform view-frustum culling for this node
        if (_view_frustrum_culling && node->getBoundingSphere().intersects(_scene->getActiveCamera()->getFrustum()))
        {
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
    }
    return true;
}

//----------------------------------------------------------------------
//
// setSceneMaterial()
//
//----------------------------------------------------------------------
bool AerialAssist::setSceneMaterial(Node* node)
{
    // If the node visited contains a model, draw it
    Model* model = node->getModel();
    if (model)
    {
        string id = node->getId();
        string texture = "res/textures/gray.png";
        bool transparent = false;
        map<string, GPair<string, bool> >::const_iterator it;
        for (it = textureList.begin(); it != textureList.end(); it++)
        {
            GString regex = it->first.c_str();
            if (RegExp(id.c_str(), regex))
            {
                texture = it->second.first;
                transparent = it->second.second;
#ifdef DEBUG
                fprintf(stderr, "[Debug]\t\tRegEx match for \"%s\" on \"%s\"\n", (const char*)regex, id.c_str());
#endif // DEBUG
                break;
            }
        }
#ifdef DEBUG
        fprintf(stderr, "[Debug]\tSetting %smaterial for node \"%s\" to \"%s\"\n", (transparent?"transparent ":""), id.c_str(), texture.c_str());
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

//----------------------------------------------------------------------
//
// keyEvent()
//
//----------------------------------------------------------------------
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

//----------------------------------------------------------------------
//
// getNextCamera()
//
//----------------------------------------------------------------------
AerialAssist::CameraPosition AerialAssist::getNextCamera(AerialAssist::CameraPosition current) const
{
    int cam = (int)current;
    do
    {
        if (++cam == (int)CameraCount)
        {
            cam = (int)DriverStation;
        }
    }
    while (_camera[current] == NULL);
    return (CameraPosition)cam;
}

//----------------------------------------------------------------------
//
// touchEvent()
//
//----------------------------------------------------------------------
void AerialAssist::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex)
{
    switch (evt)
    {
    case Touch::TOUCH_PRESS:
        _hud_camera = getNextCamera(_hud_camera);
        _active_camera = getNextCamera(_active_camera);
        break;
    case Touch::TOUCH_RELEASE:
        break;
    case Touch::TOUCH_MOVE:
        break;
    };
}
