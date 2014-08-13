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
    _origin_offset_x(0.0),
    _origin_offset_y(0.0),
    _origin_offset_z(0.0)
{
}

//----------------------------------------------------------------------
//
// Robot()
//
//----------------------------------------------------------------------
Robot::Robot(const Robot &robot) :
    _robot_node(NULL),
    _origin_offset_x(robot._origin_offset_x),
    _origin_offset_y(robot._origin_offset_y),
    _origin_offset_z(robot._origin_offset_z)
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
    _origin_offset_x(0.0),
    _origin_offset_y(0.0),
    _origin_offset_z(0.0)
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
    _origin_offset_x = root.get("originOffsetX", 0.0).asDouble();
    _origin_offset_y = root.get("originOffsetY", 0.0).asDouble();
    _origin_offset_z = root.get("originOffsetZ", 0.0).asDouble();
    // load robot
#ifdef DEBUG
    fprintf(stderr, "[Debug] Loading robot model from GPB \"%s\"\n", (const char*)_bundle_file);
#endif // DEBUG
    Bundle *robot_bundle_ptr = Bundle::create(GFileName(FileSystem::getResourcePath()) + _bundle_file);
    Node* robot = robot_bundle_ptr->loadNode(_top_node_id)->clone();
    if (robot)
    {
    //        _catapult_node = robot->findNode("Catapult");
        _robot_node = Node::create("Robot");
        _robot_node->addChild(robot);
        robot->setTranslation(_origin_offset_x, _origin_offset_y, _origin_offset_z);
#ifdef DEBUG
        fprintf(stderr, "[Debug] Loaded model (%f, %f, %f)\n", _robot_node->getTranslationX(), _robot_node->getTranslationY(), _robot_node->getTranslationZ());
#endif // DEBUG
    //        _scene->addNode(_robot_node);
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
        _origin_offset_x = robot._origin_offset_x;
        _origin_offset_y = robot._origin_offset_y;
        _origin_offset_z = robot._origin_offset_z;
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