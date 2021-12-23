//------------------------------------------------------------------------------
// loaders.h
//------------------------------------------------------------------------------
// Copyright (c) 2021 John Hughes
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
#ifndef AE_LOADERS
#define AE_LOADERS
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "ae/aether.h"

struct ae_VertexLoaderInfo
{
	uint32_t size = 0;

	int32_t posOffset = -1;
	int32_t normalOffset = -1;
	int32_t colorOffset = -1;
	int32_t uvOffset = -1;
	
	const char* posAttrib = "a_position";
	const char* normalAttrib = "a_normal";
	const char* colorAttrib = "a_color";
	const char* uvAttrib = "a_uv";
	
	uint32_t posComponents = 4;
	uint32_t normalComponents = 4;
	uint32_t colorComponents = 4;
	uint32_t uvComponents = 2;
	
	void SetPosition( void* vertices, uint32_t index, ae::Vec4 position ) const { if ( posOffset >= 0 && size ) { *(ae::Vec4*)( (uint8_t*)vertices + index * size + posOffset ) = position; } }
	void SetNormal( void* vertices, uint32_t index, ae::Vec4 normal ) const { if ( normalOffset >= 0 && size ) { *(ae::Vec4*)( (uint8_t*)vertices + index * size + normalOffset ) = normal; } }
	void SetColor( void* vertices, uint32_t index, ae::Vec4 color ) const { if ( colorOffset >= 0 && size ) { *(ae::Vec4*)( (uint8_t*)vertices + index * size + colorOffset ) = color; } }
	void SetUV( void* vertices, uint32_t index, ae::Vec2 uv ) const { if ( uvOffset >= 0 && size ) { *(ae::Vec2*)( (uint8_t*)vertices + index * size + uvOffset ) = uv; } }
};

//------------------------------------------------------------------------------
// stb
//------------------------------------------------------------------------------
void ae_stb_LoadPng( ae::Texture2D* texture, const char* file, ae::Texture::Filter filter, ae::Texture::Wrap wrap, bool autoGenerateMipmaps, bool isSRGB );

//------------------------------------------------------------------------------
// ofbx
//------------------------------------------------------------------------------
bool ae_ofbx_LoadSkinnedMesh( const ae::Tag& tag, ae::FileSystem* fileSystem, const char* fileName, const ae_VertexLoaderInfo& vertexInfo, ae::VertexData* vertexData, ae::Skin* skinOut, ae::Animation* animOut );

#endif
