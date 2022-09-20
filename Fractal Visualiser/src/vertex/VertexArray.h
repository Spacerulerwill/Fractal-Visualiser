#pragma once

#include "vertex/VertexBuffer.h"
#include "vertex/VertexBufferLayout.h"

class VertexArray {

public:
	VertexArray();
	~VertexArray();

	unsigned int getId() { return m_ID; }

	void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);
	void Bind() const;
	void Unbind() const;
private:
	unsigned int m_ID;

};