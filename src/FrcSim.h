#ifndef TEMPLATEGAME_H_
#define TEMPLATEGAME_H_

#include "gameplay.h"

using namespace gameplay;

#define VERT_SHADER "res/shaders/textured.vert"
#define FRAG_SHADER "res/shaders/textured.frag"
#define DEF_SHADER "SPOT_LIGHT_COUNT 1; TEXTURE_DISCARD_ALPHA"

/**
 * Main game class.
 */
class AerialAssist: public Game
{
public:

    /**
     * Constructor.
     */
    AerialAssist();

    /**
     * @see Game::keyEvent
     */
	void keyEvent(Keyboard::KeyEvent evt, int key);
	
    /**
     * @see Game::touchEvent
     */
    void touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex);

protected:

    /**
     * @see Game::initialize
     */
    void initialize();

    /**
     * @see Game::finalize
     */
    void finalize();

    /**
     * @see Game::update
     */
    void update(float elapsedTime);

    /**
     * @see Game::render
     */
    void render(float elapsedTime);
    
    /**
     *
     */
    void setMaterial(Node* node_ptr, const char* diffuse_string_ptr, const char* normal_string_ptr, float specularity);
    
    // render variables & methods
    Node* _camera_h_node;
    Node* _camera_v_node;
    Node* _spotlight_node;
    Node* _floor_node;
    Camera* _camera;
    Node* _light_node;
    Light* _light;
    Light* _spotlight;
    
    double _elapsedTime;

private:

    /**
     * Adds a node to the appropriate render queue (opaque or transparent)
     * based on the "transparent" tag set in the node.  The "transparent"
     * tag is set by loadTextureMap() based on the "transparent" boolean
     * in the JSON file.
     */
    bool buildRenderQueues(Node* node);
    
    /**
     * Sets the material for a node and binds it to the scene's light sources.
     */
    bool setSceneMaterial(Node* node);
    
    /**
     *
     */
    void createFloorModels(void);

    /**
     *
     */
    Mesh* createFloorMesh(void);
    
    /**
     *
     */
    void loadTextureMap(const string &filename);

    // Render queue indexes (in order of drawing).
    enum RenderQueue
    {
        QUEUE_OPAQUE = 0,
        QUEUE_TRANSPARENT,
        QUEUE_COUNT
    };
    
    vector<Node*> _renderQueues[QUEUE_COUNT];
    Scene* _scene;
    bool _wireframe;
    
    map<string, GPair<string, bool> > textureList;
};

#endif
