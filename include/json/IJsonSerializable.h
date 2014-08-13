//----------------------------------------------------------------------
//
// IJsonSerializable
//
// From http://www.danielsoltyka.com/programming/2011/04/15/simple-class-serialization-with-jsoncpp/
//----------------------------------------------------------------------

#ifndef IJSONSERIALIZABLE_H
#define IJSONSERIALIZABLE_H

class IJsonSerializable
{
    
public:
    
   virtual ~IJsonSerializable( void ) {}
    
   virtual void Serialize( Json::Value& root) const = 0;
    
   virtual void Deserialize( Json::Value& root) = 0;
    
};

#endif // IJSONSERIALIZABLE_H
