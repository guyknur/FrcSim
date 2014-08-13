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
    
    Node* _robot_node;      /**< Pointer to GamePlay's Node instance for the top
                                 level object, used for robot translation and 
                                 rotation */
    
    
};

#endif // _ROBOT
