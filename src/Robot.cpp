//
//  Robot.cpp
//  FrcSim
//
//  Created by Tom Dunn on 8/11/14.
//
//

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

#include <gameplay.h>

using namespace gameplay;

#include "json/IJsonSerializable.h"
#include "Robot.h"
#include "FrcSim.h"

#ifdef ANDROID
#include <android/log.h>
#define fprintf(a, ...) ((void)__android_log_print(ANDROID_LOG_INFO, "FrcSim", __VA_ARGS__))
#endif // ANDROID

//----------------------------------------------------------------------
//
// Robot()
//
//----------------------------------------------------------------------
Robot::Robot() :
    _robot_node(NULL),
    _velocity(0.0),
    _velocity_setpoint(0.0),
    _max_acceleration(0.0),
    _max_velocity(0.0),
    _mass(0.0)
{
}

//----------------------------------------------------------------------
//
// Robot()
//
//----------------------------------------------------------------------
Robot::Robot(const Robot &robot) :
    _robot_node(NULL),
    _origin_offset(robot._origin_offset),
    _position(robot._position),
    _rotation(robot._rotation),
    _velocity(robot._velocity),
    _velocity_setpoint(robot._velocity_setpoint),
    _max_acceleration(robot._max_acceleration),
    _max_velocity(robot._max_velocity),
    _mass(robot._mass)
{
    if (robot._robot_node)
    {
        _robot_node = robot._robot_node->clone();
    }
}

//----------------------------------------------------------------------
//
// Robot()
//
//----------------------------------------------------------------------
Robot::Robot(const GFileName &configFile) :
    _robot_node(NULL),
    _velocity(0.0),
    _velocity_setpoint(0.0),
    _max_acceleration(0.0),
    _max_velocity(0.0),
    _mass(0.0)
{
    LoadConfig(configFile);
}

//----------------------------------------------------------------------
//
// LoadConfig()
//
//----------------------------------------------------------------------
void Robot::LoadConfig(const GFileName &filename)
{
    std::string jsonInput;
    std::ifstream inFile;
    inFile.open((const char*)(GFileName(FileSystem::getResourcePath()) + filename), std::ios_base::in);
    if (inFile)
    {
        inFile.seekg(0, std::ios::end);
        unsigned long length = inFile.tellg();
        jsonInput.resize(length);
        inFile.seekg(0, std::ios::beg);
        inFile.read(&jsonInput[0], jsonInput.size());
        inFile.close();
        
        Json::Value root;
        Json::Reader reader;
        if ( !reader.parse(jsonInput, root) )
        {
#if DEBUG
            fprintf(stderr, "[ERROR] File \"%s\" not parsed\n", (const char*)filename);
#endif // DEBUG
            return;
        }
        Deserialize(root);
    }
}

//----------------------------------------------------------------------
//
// update()
//
//----------------------------------------------------------------------
void Robot::update(float elapsedTime) throw(GNullPointerException)
{
    if (!_robot_node)
    {
        return;
    }
    if (_velocity_setpoint < _velocity)
    {
        _velocity += (_max_acceleration * elapsedTime);
        if (_velocity > _velocity_setpoint)
        {
            _velocity = _velocity_setpoint;
        }
    }
    else
    {
        _velocity -= (_max_acceleration * elapsedTime);
        if (_velocity < _velocity_setpoint)
        {
            _velocity = _velocity_setpoint;
        }
    }
    _position = _robot_node->getTranslationWorld();
    PhysicsCharacter* character = dynamic_cast<PhysicsCharacter*>(_robot_node->getCollisionObject());
    if (character)
    {
        if (_velocity == 0.0)
        {
            character->setVelocity(Vector3::zero());
            character->setForwardVelocity(0.0);
            character->setRightVelocity(0.0);
        }
        else
        {
            character->setForwardVelocity(_velocity);
        }
#ifdef DEBUG
        Vector3 velocity = character->getCurrentVelocity();
        fprintf(stderr, "[Debug] %5.2f: Robot facing is %5.2f, distance: %4.2f, velocity: (%f, %f, %f), location: (%5.2f, %5.2f, %5.2f)\n", elapsedTime, getYaw(), _velocity, velocity.x, velocity.y, velocity.z, _position.x, _position.y, _position.z);
#endif // DEBUG
    }
}

//----------------------------------------------------------------------
//
// setRoll()
//
//----------------------------------------------------------------------
void Robot::setVelocity(float velocityPercent)
{
    _velocity_setpoint = velocityPercent * _max_velocity;
}

//----------------------------------------------------------------------
//
// setRoll()
//
//----------------------------------------------------------------------
void Robot::setRoll(float roll)
{
    _rotation.y = roll;
}

//----------------------------------------------------------------------
//
// setPitch()
//
//----------------------------------------------------------------------
void Robot::setPitch(float pitch)
{
    _rotation.x = pitch;
}

//----------------------------------------------------------------------
//
// setYaw()
//
//----------------------------------------------------------------------
void Robot::setYaw(float yaw)
{
    _rotation.z = yaw;
}

//----------------------------------------------------------------------
//
// Serialize()
//
//----------------------------------------------------------------------
void Robot::Serialize(Json::Value &root) const
{
}

//----------------------------------------------------------------------
//
// Deserialize()
//
//----------------------------------------------------------------------
void Robot::Deserialize(Json::Value &root)
{
    _bundle_file = root.get("bundle", "").asCString();
    _texture_map_file = root.get("textureMap", "").asCString();
    _top_node_id = root.get("topNodeId", "").asCString();
    _origin_offset.x = root.get("originOffsetX", 0.0).asDouble();
    _origin_offset.y = root.get("originOffsetY", 0.0).asDouble();
    _origin_offset.z = root.get("originOffsetZ", 0.0).asDouble();
    _position.x = root.get("positionX", 0.0).asDouble();
    _position.y = root.get("positionY", 0.0).asDouble();
    _position.z = root.get("positionZ", 0.0).asDouble();
    _rotation.x = root.get("rotationX", 0.0).asDouble();
    _rotation.y = root.get("rotationY", 0.0).asDouble();
    _rotation.z = root.get("rotationZ", 0.0).asDouble();
    _velocity = root.get("velocity", 0.0).asDouble();
    _velocity_setpoint = root.get("velocitySetpoint", 0.0).asDouble();
    _max_acceleration = root.get("maxAcceleration", 0.0).asDouble();
    _max_velocity = root.get("maxVelocity", 0.0).asDouble();
    _mass = root.get("mass", 0.0).asDouble();
    // load robot
#ifdef DEBUG
    fprintf(stderr, "[Debug] Loading robot model from GPB \"%s\"\n", (const char*)_bundle_file);
#endif // DEBUG
    Scene* scene = NULL;
    AerialAssist* game = dynamic_cast<AerialAssist*>(Game::getInstance());
    if (game)
    {
        scene = game->getScene();
        if (scene)
        {
            _robot_node = scene->findNode(_top_node_id);
        }
    }
    Bundle *robot_bundle_ptr = Bundle::create(GFileName(FileSystem::getResourcePath()) + _bundle_file);
    Node* robot = robot_bundle_ptr->loadNode(_top_node_id)->clone();
    if (robot)
    {
        _robot_node = Node::create("Robot");
        Node* robot_h = Node::create("robot_h");
        _robot_node->addChild(robot_h);
        Node* robot_v = Node::create("robot_v");
        robot_h->addChild(robot_v);
        robot_v->addChild(robot);
        robot->setTranslation(_origin_offset);
#ifdef DEBUG
        if (_robot_node)
        {
            fprintf(stderr, "[Debug] Loaded model (%f, %f, %f)\n", _robot_node->getTranslationX(), _robot_node->getTranslationY(), _robot_node->getTranslationZ());
        }
#endif // DEBUG
       // Enable physics for robot
        _robot_node->setCollisionObject("res/frcsim.physics#robot");
        PhysicsVehicle* vehicle = dynamic_cast<PhysicsVehicle*>(_robot_node->getCollisionObject());
//        Node* wheel_front_right = _robot_node->findNode("Wheel - Front Right");
//        if (wheel_front_right)
//        {
//            // Enable physics for wheel
//            PhysicsRigidBody::Parameters rbParams;
//            rbParams.mass = 0.5f;
//            rbParams.friction = 0.5f;
//            rbParams.restitution = 0.75f;
//            rbParams.linearDamping = 0.025f;
//            rbParams.angularDamping = 0.16f;
//            wheel_front_right->setCollisionObject(PhysicsCollisionObject::VEHICLE_WHEEL, PhysicsCollisionShape::sphere(2.0), &rbParams);
//            PhysicsVehicleWheel* wheel = dynamic_cast<PhysicsVehicleWheel*>(wheel_front_right->getCollisionObject());
//            if (wheel)
//            {
//                vehicle->addWheel(wheel);
//                wheel->setSteerable(false);
//                wheel->setWheelAxle(Vector3(1.0, 0.0, 0.0));
//                wheel->setWheelRadius(2.0);
//            }
//        }
    }
    SAFE_RELEASE(robot_bundle_ptr);
}

//----------------------------------------------------------------------
//
// operator=()
//
//----------------------------------------------------------------------
Robot &Robot::operator=(const Robot &robot)
{
    if (this != &robot)
    {
        // Perform deep copy
        _top_node_id = robot._top_node_id;
        _bundle_file = robot._bundle_file;
        _texture_map_file = robot._texture_map_file;
        _origin_offset = robot._origin_offset;
        _position = robot._position;
        _rotation = robot._rotation;
        _velocity = robot._velocity;
        _velocity_setpoint = robot._velocity_setpoint;
        _max_acceleration = robot._max_acceleration;
        _max_velocity = robot._max_velocity;
        if (robot._robot_node)
        {
            _robot_node = robot._robot_node->clone();
        }
    }
    return *this;
}

//----------------------------------------------------------------------
//
// ~Robot()
//
//----------------------------------------------------------------------
Robot::~Robot()
{
    SAFE_RELEASE(_robot_node);
}