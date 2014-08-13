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
                                         and rotation */
    GString _top_node_id;
    
    double _origin_offset_x;
    
    double _origin_offset_y;
    
    double _origin_offset_z;
    
    GFileName _bundle_file;        /**< Name of GamePlay's bundle file                */
    
    GFileName _texture_map_file;   /**< Name of JSON file specifying robot's textures */
    
    
};

#endif // _ROBOT
