#pragma once

#include <map>
#include "BufferBase.h"
#include "VertexBuffer.h"

class VertexArray : public BufferBase
{
public:
	VertexArray();
	~VertexArray() override;
	
	template<typename T>
	int resolveBufferIndex(const VertexBuffer<T>& buffer)
	{
		if (!_mappedBuffers.contains(&buffer))
		{
			int newIndex = _mappedBuffers.size();
			_mappedBuffers[&buffer] = newIndex;
			glVertexArrayVertexBuffer(_handle, newIndex, buffer.getHandle(), 0, sizeof(T) * buffer.getElementCountPerVertex());
		}
		
		return _mappedBuffers[&buffer];
	}
	
	template<typename T>
	void registerAttrib(const VertexBuffer<T>& buffer, int location, int elementCount, GLenum elementType, int offset)
	{
		int bindingIndex = resolveBufferIndex(buffer);
		
		glEnableVertexArrayAttrib(_handle, location);
		glVertexArrayAttribFormat(_handle, location, elementCount, elementType, false, offset);
		glVertexArrayAttribBinding(_handle, location, bindingIndex);
	}
	
	void registerIndexBuffer(const BufferBase& buffer);
	
	void bind();

private:
	std::map<const BufferBase*, int> _mappedBuffers;
};


