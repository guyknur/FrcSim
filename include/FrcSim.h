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

    const static GFileName _kFieldBundle;
    const static GFileName _kFieldTextureMap;
    const static GFileName _kSceneFile;
    const static float _joystickDeadband;

    enum CameraPosition
    {
        High = 0,
        DriverStation,
        Overhead,
        Chase,
        RightSide,
        LeftSide,
        CameraCount
    };
    
    /**
     * Constructor.
     */
    AerialAssist();

    /**
     * @see Game::keyEvent
     */
	void keyEvent(Keyboard::KeyEvent evt, int key);
    
    /**
     * Return a pointer to the current scene.
     *
     * @return pointer to the game's scene
     */
    Scene* getScene() const { return _scene; }
	
    /**
     * @see Game::touchEvent
     */
    void touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex);

    /**
     * @see Game::gamepadEvent
     */
    void gamepadEvent(Gamepad::GamepadEvent evt, Gamepad* gamepad);
    
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
    virtual void update(float elapsedTime);

    /**
     * @see Game::render
     */
    virtual void render(float elapsedTime);
    
    /**
     * Draws the screen by visitin each node in the model.
     */
    void drawScreen(CameraPosition camera);
    
    /**
     * Creates a camera and a hierarchy of nodes to allow easy rotation
     * relative to the X, Y and Z axis.
     */
    Node* createCamera(const GString &cameraName, Camera** camera);
    
    /**
     *
     */
    void setMaterial(Node* node_ptr, const char* diffuse_string_ptr, const char* normal_string_ptr, float specularity);
    
    /**
     *
     */
    static void drawFrameRate(Font* font, const Vector4& color, unsigned int x, unsigned int y, unsigned int fps);
    
    /**
     *
     */
    bool isInDeadband(float value) const;
    
    // render variables & methods
    Node* _spotlight_node;
    
    Camera* _camera[CameraCount];
    
    Light* _spotlight;
    
    Robot *_robot;
    
    double _elapsedTime;
    
    CameraPosition _active_camera;

    CameraPosition _hud_camera;

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
    Node* createFloorModel(void);

    /**
     *
     */
    Mesh* createFloorMesh(void);
    
    /**
     * Determines the next camera position.
     *
     * @param current existing camera position
     * @return next available camera position
     */
    CameraPosition getNextCamera(CameraPosition current) const;
    
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
    
    Font* _font;
    
    Gamepad* _gamepad;
    
    bool _wireframe;
    
    bool _physicsDebug;
    
    bool _ball_in_play;
    
    bool _view_frustrum_culling;
    
    map<string, GPair<string, bool> > textureList;
    
    static const int kHudWidth;
    
    static const int kHudHeight;
    
};

#endif
