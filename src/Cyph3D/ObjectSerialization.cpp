#include "Cyph3D/ObjectSerialization.h"

nlohmann::ordered_json ObjectSerialization::toJson()
{
	nlohmann::ordered_json result;
	result["data"] = data;
	result["version"] = version;
	result["identifier"] = identifier;
	return result;
}

ObjectSerialization ObjectSerialization::fromJson(const nlohmann::ordered_json& json)
{
	ObjectSerialization result;
	result.version = json["version"].get<int>();
	result.data = json["data"];
	result.identifier = json["identifier"].get<std::string>();
	return result;
}