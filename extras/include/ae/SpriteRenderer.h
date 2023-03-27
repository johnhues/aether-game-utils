//------------------------------------------------------------------------------
// SpriteRender.h
//------------------------------------------------------------------------------
// Copyright (c) 2022 John Hughes
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//------------------------------------------------------------------------------
#ifndef AESPRITERENDER_H
#define AESPRITERENDER_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"

//------------------------------------------------------------------------------
// SpriteRenderer example class
//------------------------------------------------------------------------------
class SpriteRenderer
{
public:
	void Initialize( uint32_t maxCount );
	void Terminate();
	void AddSprite( const ae::Matrix4& localToWorld, ae::Rect uvs, ae::Color color );
	void Clear();
	void Render( const ae::Matrix4& worldToProj, const ae::Texture2D* texture );
private:
	struct Vertex
	{
		ae::Vec4 pos;
		ae::Vec4 color;
		ae::Vec2 uv;
	};
	uint32_t m_count = 0;
	uint32_t m_maxCount = 0;
	ae::Shader m_shader;
	ae::VertexArray m_vertexData;
};

#endif
