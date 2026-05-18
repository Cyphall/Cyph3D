#include "ObjectSerialization.h"

nlohmann::ordered_json c3d::ObjectSerialization::toJson()
{
	nlohmann::ordered_json result;
	result["data"] = data;
	result["version"] = version;
	result["identifier"] = identifier;
	return result;
}

c3d::ObjectSerialization c3d::ObjectSerialization::fromJson(const nlohmann::ordered_json& json)
{
	ObjectSerialization result;
	result.version = json["version"].get<int>();
	result.data = json["data"];
	result.identifier = json["identifier"].get<std::string>();
	return result;
}