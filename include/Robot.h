//
//  Robot.h
//  FrcSim
//
//  Created by Tom Dunn on 8/11/14.
//
//

#ifndef _ROBOT
#define _ROBOT

class Robot  : public IJsonSerializable
{
    
public:
    
    /**
     * Default constructor.
     */
    Robot();
    
    /**
     * Copy constructor.
     *
     * @param robot an existing instance to copy from
     */
    Robot(const Robot &robot);
    
    /**
     * Overloaded constructor.
     *
     * @param configFile Filename of JSON configuration file
     */
    Robot(const GFileName &configFile);
    
    /**
     * Returns the pointer to the robot's top-level node.
     *
     * @return pointer to the robot's top-level node
     */
    Node *getNode() const { return _robot_node; }
    
    /**
     * Returns the filename to the robot model's texture map JSON file.
     *
     * @return GFileName contains the robot model's texture map JSON file
     */
    GFileName getTextureMapFile() const { return _texture_map_file; }
    
    /**
     * Load robot configuration from JSON file.
     *
     * @param filename relative file name to load configuration from
     */
    void LoadConfig(const GFileName &filename);
    
    /**
     * Update the robot's node position and rotation in the scene.
     *
     * Moves the robot based on its current facing, velocity, acceleration
     * and position of the wheels.
     *
     * @param elapsedTime elapsed time since the last frame in seconds
     */
    virtual void update(float elapsedTime) throw(GNullPointerException);
    
    /**
     * Change the robot's velocity setpoint.
     *
     * @param velocityPercent new velocity setpoint value (in percent of maximum)
     */
    void setVelocity(float velocityPercent);
    
    /**
     * Change the robot's roll.
     *
     * @param roll new roll value
     */
    void setRoll(float roll);
    
    /**
     * Change the robot's pitch.
     *
     * @param pitch new pitch value
     */
    void setPitch(float pitch);
    
    /**
     * Change the robot's yaw.
     *
     * @param yaw new yaw value
     */
    void setYaw(float yaw);
    
    /**
     * Get a copy of the robot's roll.
     *
     * @return current roll value
     */
    float getRoll() const { return _rotation.y; }

    /**
     * Get a copy of the robot's pitch.
     *
     * @return current pitch value
     */
    float getPitch() const { return _rotation.x; }
    
    /**
     * Get a copy of the robot's yaw.
     *
     * @return current yaw value
     */
    float getYaw() const { return _rotation.z; }
    
    /**
     * Gets a copy of the robot's translation coordinates.
     *
     * @return copy of the robot's position (x, y and z).
     */
    Vector3 getPosition() const { return _position; }
    
    /**
     * Method to write Robot configuration to JSON file.
     *
     * @param root JsonCPP root node to write to
     */
    virtual void Serialize(Json::Value &root) const;
    
    /**
     * Method to read Robot configuration from JSON file.
     *
     * @param root JsonCPP root node to read from
     */
    virtual void Deserialize(Json::Value &root);
    
    /**
     * Assignment operator.
     *
     * @param robot an existing instance to copy from
     * @return reference to this instance (after assignment completes)
     */
    Robot &operator=(const Robot &robot);
    
    /*
     * Destructor.
     */
    virtual ~Robot();
    
protected:
    
    Node* _robot_node;             /**< Pointer to GamePlay's Node instance for the
                                         top level object, used for robot translation
                                         and rotation                                 */
    
    GString _top_node_id;          /**< Name of top-level node ID in bundle           */
    
    Vector3 _origin_offset;        /**< Offset (in inches) of robot's center on floor */
    
    Vector3 _position;             /**< Position of robot on field (in inches)        */
    
    Vector3 _rotation;             /**< Rotation angle (pitch=x, roll=y, yaw=z) in
                                        degrees                                       */
    
    GFileName _bundle_file;        /**< Name of GamePlay's bundle file                */
    
    GFileName _texture_map_file;   /**< Name of JSON file specifying robot's textures */
    
    double _velocity;              /**< Current velocity of robot in inches/sec       */
    
    double _velocity_setpoint;     /**< Desired velocity of the robot in inches/sec   */
    
    double _max_acceleration;      /**< Maximum acceleration in delta inches/sec      */
    
    double _max_velocity;          /**< Maximum velocity in inches/sec                */
    
};

#endif // _ROBOT
