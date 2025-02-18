//------------------------------------------------------------------------------
// loaders.h
//------------------------------------------------------------------------------
// Copyright (c) 2025 John Hughes
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
#include "aether.h"

namespace ae {

//------------------------------------------------------------------------------
// ae::VertexDescriptor
//------------------------------------------------------------------------------
struct VertexDescriptor
{
	uint32_t vertexSize = 0;
	uint32_t indexSize = 0;

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
	
	void SetPosition( void* vertices, uint32_t index, ae::Vec4 position ) const { if ( posOffset >= 0 && vertexSize ) { *(ae::Vec4*)( (uint8_t*)vertices + index * vertexSize + posOffset ) = position; } }
	void SetNormal( void* vertices, uint32_t index, ae::Vec4 normal ) const { if ( normalOffset >= 0 && vertexSize ) { *(ae::Vec4*)( (uint8_t*)vertices + index * vertexSize + normalOffset ) = normal; } }
	void SetColor( void* vertices, uint32_t index, ae::Vec4 color ) const { if ( colorOffset >= 0 && vertexSize ) { *(ae::Vec4*)( (uint8_t*)vertices + index * vertexSize + colorOffset ) = color; } }
	void SetUV( void* vertices, uint32_t index, ae::Vec2 uv ) const { if ( uvOffset >= 0 && vertexSize ) { *(ae::Vec2*)( (uint8_t*)vertices + index * vertexSize + uvOffset ) = uv; } }
	ae::Vec4& GetPosition( void* vertices, uint32_t index ) const { AE_ASSERT( posOffset >= 0 && vertexSize ); return *(ae::Vec4*)( (uint8_t*)vertices + index * vertexSize + posOffset ); }
	ae::Vec4& GetNormal( void* vertices, uint32_t index ) const { AE_ASSERT( normalOffset >= 0 && vertexSize ); return *(ae::Vec4*)( (uint8_t*)vertices + index * vertexSize + normalOffset ); }
	ae::Vec4& GetColor( void* vertices, uint32_t index ) const { AE_ASSERT( colorOffset >= 0 && vertexSize ); return *(ae::Vec4*)( (uint8_t*)vertices + index * vertexSize + colorOffset ); }
	ae::Vec2& GetUV( void* vertices, uint32_t index ) const { AE_ASSERT( uvOffset >= 0 && vertexSize ); return *(ae::Vec2*)( (uint8_t*)vertices + index * vertexSize + uvOffset ); }
};
typedef VertexDescriptor VertexLoaderHelper;

//------------------------------------------------------------------------------
// stb
//------------------------------------------------------------------------------
void stbLoadPng( ae::Texture2D* texture, const uint8_t* data, uint32_t dataLen, ae::Texture::Filter filter, ae::Texture::Wrap wrap, bool autoGenerateMipmaps, bool isSRGB );

//------------------------------------------------------------------------------
// ae::FbxLoaderParams struct
//------------------------------------------------------------------------------
struct FbxLoaderParams
{
	ae::VertexLoaderHelper descriptor;
	
	ae::VertexBuffer* vertexData = nullptr;
	ae::CollisionMesh<>* collisionMesh = nullptr;
	ae::Skin* skin = nullptr;
	ae::Skeleton* skeleton = nullptr;
	ae::Animation* anim = nullptr;
	
	void* vertexOut = nullptr;
	void* indexOut = nullptr;
	uint32_t maxVerts = 0;
	uint32_t maxIndex = 0;
};

//------------------------------------------------------------------------------
// ae::FbxLoader class (requires 'loaders/ofbx.cpp' and ofbx library)
// @TODO: Enable FbxLoader impl when ofbx.h is included before AE_MAIN and aether.h
//------------------------------------------------------------------------------
class FbxLoader
{
public:
	FbxLoader( const ae::Tag& tag );
	~FbxLoader();
	bool Initialize( const void* fileData, uint32_t fileDataLen );
	void Terminate();
	
	uint32_t GetMeshCount() const;
	const char* GetMeshName( uint32_t idx ) const;
	uint32_t GetMeshVertexCount( uint32_t idx ) const;
	uint32_t GetMeshIndexCount( uint32_t idx ) const;
	uint32_t GetMeshBoneCount( uint32_t idx ) const;
	uint32_t GetMeshVertexCount( const char* name ) const;
	uint32_t GetMeshIndexCount( const char* name ) const;
	uint32_t GetMeshBoneCount( const char* name ) const;
	
	bool Load( const char* meshName, const ae::FbxLoaderParams& params ) const;
	
private:
	const ae::Tag m_tag;
	struct FbxLoaderImpl* m_state = nullptr;
};

} // End ae namespace
#endif
