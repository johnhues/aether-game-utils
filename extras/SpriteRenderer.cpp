//------------------------------------------------------------------------------
// SpriteRender.cpp
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
// Headers
//------------------------------------------------------------------------------
#include "ae/SpriteRenderer.h"
namespace ae {

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
struct _SpriteVertex
{
	ae::Vec4 pos;
	ae::Vec4 color;
	ae::Vec2 uv;
};
using _SpriteIndex = uint16_t;

//------------------------------------------------------------------------------
// ae::SpriteFont
//------------------------------------------------------------------------------
SpriteFont::GlyphData::GlyphData()
{
	quad = ae::Rect::FromPoints( ae::Vec2( 0.0f ), ae::Vec2( 0.0f ) );
	uvs = ae::Rect::FromPoints( ae::Vec2( 0.0f ), ae::Vec2( 0.0f ) );
	advance = 0.0f;
}

void SpriteFont::SetGlyph( char c, ae::Rect quad, ae::Rect uvs, float advance )
{
	if ( c < 32 || c >= 32 + countof(m_glyphs) )
	{
		return;
	}
	m_glyphs[ c - 32 ].quad = quad;
	m_glyphs[ c - 32 ].uvs = uvs;
	m_glyphs[ c - 32 ].advance = advance;
}

bool SpriteFont::GetGlyph( char c, ae::Rect* quad, ae::Rect* uv, float* advance, float uiSize ) const
{
	if ( c < 32 || c >= 32 + countof(m_glyphs) )
	{
		return false;
	}
	if ( quad ) { *quad = m_glyphs[ c - 32 ].quad * uiSize; }
	if ( uv ) { *uv = m_glyphs[ c - 32 ].uvs; }
	if ( advance ) { *advance = m_glyphs[ c - 32 ].advance * uiSize; }
	return true;
}

float SpriteFont::GetTextWidth( const char* text, float uiSize ) const
{
	float width = 0.0f;
	float advance = 0.0f;
	while ( *text )
	{
		if ( *text == '\r' || *text == '\n' )
		{
			width = std::max( width, advance );
			advance = 0.0f;
		}
		else
		{
			GetGlyph( *text, nullptr, nullptr, &advance, uiSize );
		}
		text++;
	}
	return std::max( width, advance );
}

//------------------------------------------------------------------------------
// ae::SpriteRenderer member functions
//------------------------------------------------------------------------------
SpriteRenderer::SpriteRenderer( const ae::Tag& tag ) :
	m_params( tag ),
	m_spriteGroups( tag ),
	m_vertexArray( tag )
{}

void SpriteRenderer::Initialize( uint32_t maxGroups, uint32_t maxCount )
{
	m_params.Append( {}, maxGroups );
	m_spriteGroups.Reserve( maxCount );

	m_vertexBuffer.Initialize(
		sizeof(_SpriteVertex), sizeof(_SpriteIndex),
		4 * maxCount, 6 * maxCount,
		ae::Vertex::Primitive::Triangle,
		ae::Vertex::Usage::Dynamic, ae::Vertex::Usage::Static
	);
	m_vertexBuffer.AddAttribute( "a_position", 4, ae::Vertex::Type::Float, offsetof(_SpriteVertex, pos) );
	m_vertexBuffer.AddAttribute( "a_color", 4, ae::Vertex::Type::Float, offsetof(_SpriteVertex, color) );
	m_vertexBuffer.AddAttribute( "a_uv", 2, ae::Vertex::Type::Float, offsetof(_SpriteVertex, uv) );
	m_vertexArray.Initialize( &m_vertexBuffer );

	const uint16_t indices[] = { 3, 0, 1, 3, 1, 2 };
	ae::Scratch< _SpriteIndex > indexBuffer( 6 * maxCount );
	for ( uint32_t i = 0; i < maxCount; i++ )
	{
		const uint32_t offset = 4 * i;
		indexBuffer[ i * 6 ] = offset + indices[ 0 ];
		indexBuffer[ i * 6 + 1 ] = offset + indices[ 1 ];
		indexBuffer[ i * 6 + 2 ] = offset + indices[ 2 ];
		indexBuffer[ i * 6 + 3 ] = offset + indices[ 3 ];
		indexBuffer[ i * 6 + 4 ] = offset + indices[ 4 ];
		indexBuffer[ i * 6 + 5 ] = offset + indices[ 5 ];
	}
	m_vertexArray.SetIndices( indexBuffer.Data(), indexBuffer.Length() );
}

void SpriteRenderer::Terminate()
{
	m_vertexArray.Terminate();
	m_vertexBuffer.Terminate();
	m_spriteGroups.Clear();
	m_params.Clear();
}

void SpriteRenderer::AddSprite( uint32_t group, ae::Vec2 pos, ae::Vec2 size, ae::Rect uvs, ae::Color color )
{
	ae::Matrix4 localToScreen = ae::Matrix4::Translation( pos.x, pos.y, 0.0f ) * ae::Matrix4::Scaling( size.x, size.y, 1.0f );
	AddSprite( group, localToScreen, uvs, color );
}

void SpriteRenderer::AddSprite( uint32_t group, ae::Rect quad, ae::Rect uvs, ae::Color color )
{
	ae::Vec2 pos = quad.GetMin() + quad.GetSize() * 0.5f;
	ae::Vec2 size = quad.GetSize();
	ae::Matrix4 localToScreen = ae::Matrix4::Translation( pos.x, pos.y, 0.0f ) * ae::Matrix4::Scaling( size.x, size.y, 1.0f );
	AddSprite( group, localToScreen, uvs, color );
}

void SpriteRenderer::AddSprite( uint32_t group, const ae::Matrix4& transform, ae::Rect uvs, ae::Color color )
{
	if ( m_vertexArray.GetVertexCount() >= m_vertexArray.GetMaxVertexCount() )
	{
		return;
	}
	
	ae::Vec2 min = uvs.GetMin();
	ae::Vec2 max = uvs.GetMax();
	_SpriteVertex verts[] =
	{
		{ transform * ae::Vec4( -0.5f, -0.5f, 0.0f, 1.0f ), color.GetLinearRGBA(), ae::Vec2( min.x, min.y ) },
		{ transform * ae::Vec4( 0.5f, -0.5f, 0.0f, 1.0f ), color.GetLinearRGBA(), ae::Vec2( max.x, min.y ) },
		{ transform * ae::Vec4( 0.5f, 0.5f, 0.0f, 1.0f ), color.GetLinearRGBA(), ae::Vec2( max.x, max.y ) },
		{ transform * ae::Vec4( -0.5f, 0.5f, 0.0f, 1.0f ), color.GetLinearRGBA(), ae::Vec2( min.x, max.y ) }
	};
	m_vertexArray.AppendVertices( verts, countof(verts) );
	m_spriteGroups.Append( group );
}

void SpriteRenderer::AddText( uint32_t group, const char* text, const SpriteFont* font, ae::Rect region, float fontSize, float lineHeight, ae::Color color )
{
	ae::Vec2 offset( region.GetMin().x, region.GetMax().y - lineHeight );
	while ( *text )
	{
		const char c = *text;
		const bool isSpace = isspace( c );
		ae::Rect quad, uv;
		float advance;
		font->GetGlyph( *text, &quad, &uv, &advance, fontSize );
		if ( !isSpace )
		{
			AddSprite( group, quad + offset, uv, color );
		}
		offset.x += advance;
		text++;

		bool newline = false;
		if ( isSpace && !isspace( *text ) )
		{
			const char* word = text;
			float wordSize = 0.0f;
			while ( *word && !isspace( *word ) )
			{
				float advance2 = 0.0f;
				font->GetGlyph( *word, nullptr, nullptr, &advance2, fontSize );
				wordSize += advance2;
				word++;
			}
			if ( offset.x + wordSize > region.GetMax().x )
			{
				newline = true;
			}
		}
		else if ( c == '\r' || c == '\n' )
		{
			newline = true;
		}
		if ( newline )
		{
			offset.x = region.GetMin().x;
			offset.y -= lineHeight;
		}
	}
}

void SpriteRenderer::SetParams( uint32_t group, const ae::Shader* shader, const ae::UniformList& uniforms )
{
	m_params[ group ] = { shader, uniforms };
}

void SpriteRenderer::Render()
{
	m_vertexArray.Upload();

	const uint32_t spriteCount = m_spriteGroups.Length();
	for ( uint32_t i = 0; i < spriteCount; i++ )
	{
		const uint32_t group = m_spriteGroups[ i ];
		const GroupParams& params = m_params[ group ];
		// @TODO: Combine draw calls when consecutive sprites are the same group
		if ( params.shader )
		{
			m_vertexBuffer.Bind( params.shader, params.uniforms );
			m_vertexBuffer.Draw( i * 2, 2 );
		}
	}

	Clear();
}

void SpriteRenderer::Clear()
{
	m_vertexArray.ClearVertices();
	m_spriteGroups.Clear();
	for ( auto& p : m_params )
	{
		p = {};
	}
}

} // ae namespace