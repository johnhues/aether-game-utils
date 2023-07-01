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
namespace ae {

//------------------------------------------------------------------------------
// ae::SpriteFont
//------------------------------------------------------------------------------
class SpriteFont
{
public:
	void SetGlyph( char c, ae::Rect quad, ae::Rect uvs, float advance );
	bool GetGlyph( char c, ae::Rect* quad, ae::Rect* uv, float* advance, float uiSize ) const;
	float GetTextWidth( const char* text, float uiSize ) const;

private:
	struct GlyphData
	{
		GlyphData();
		ae::Rect quad;
		ae::Rect uvs;
		float advance;
	};
	GlyphData m_glyphs[ 96 ];
};

//------------------------------------------------------------------------------
// ae::SpriteRenderer utility
//------------------------------------------------------------------------------
class SpriteRenderer
{
public:
	SpriteRenderer( const ae::Tag& tag );
	void Initialize( uint32_t maxGroups, uint32_t maxCount );
	void Terminate();

	void AddSprite( uint32_t group, ae::Vec2 pos, ae::Vec2 size, ae::Rect uvs, ae::Color color );
	void AddSprite( uint32_t group, ae::Rect quad, ae::Rect uvs, ae::Color color );
	void AddSprite( uint32_t group, const ae::Matrix4& transform, ae::Rect uvs, ae::Color color );
	void AddText( uint32_t group, const char* text, const SpriteFont* font, ae::Rect region, float fontSize, float lineHeight, ae::Color color );

	//! Vertex attributes: a_position (4 floats), a_color (4 floats), a_uv
	//! (2 floats) are all provided to the vertex shader. See example below.
	// Example vertex shader:
	/*
		AE_IN_HIGHP vec4 a_position;
		AE_IN_HIGHP vec4 a_color;
		AE_IN_HIGHP vec2 a_uv;

		AE_OUT_HIGHP vec4 v_color;
		AE_OUT_HIGHP vec2 v_uv;

		void main()
		{
			v_color = a_color;
			v_uv = a_uv;
			gl_Position = a_position;
		}
	*/
	// Example fragment shader:
	/*
		AE_UNIFORM sampler2D u_tex;

		AE_IN_HIGHP vec4 v_color;
		AE_IN_HIGHP vec2 v_uv;

		void main()
		{
			AE_COLOR = v_color * AE_TEXTURE2D( u_tex, v_uv );
		}
	*/
	void SetParams( uint32_t group, const ae::Shader* shader, const ae::UniformList& uniforms );
	void Render();
	void Clear();

private:
	struct GroupParams
	{
		const ae::Shader* shader = nullptr;
		ae::UniformList uniforms;
	};
	ae::Array< GroupParams > m_params;
	ae::Array< uint32_t > m_spriteGroups;
	ae::VertexArray m_vertexArray;
};

} // ae namespace
#endif
