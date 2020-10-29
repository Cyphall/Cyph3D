#pragma once

#include <vector>
#include <string>

struct ShaderProgramCreateInfo
{
	std::map<GLenum, std::vector<std::string>> shadersFiles;
	
	bool operator ==(const ShaderProgramCreateInfo& other) const
	{
		if (shadersFiles.size() != other.shadersFiles.size()) return false;
		
		for (const auto& [shaderType, shaderFiles] : shadersFiles)
		{
			if (!other.shadersFiles.contains(shaderType)) return false;
			
			auto& otherShaderFiles = other.shadersFiles.at(shaderType);
			
			if (shaderFiles.size() != otherShaderFiles.size()) return false;
			
			for (int i = 0; i < shaderFiles.size(); ++i)
			{
				if (shaderFiles[i] != otherShaderFiles[i]) return false;
			}
		}
		
		return true;
	}
};