//------------------------------------------------------------------------------
// aeMesh.h
//------------------------------------------------------------------------------
// Copyright (c) 2020 John Hughes
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
#ifndef AEMESH_H
#define AEMESH_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeArray.h"
#include "aeRender.h" // aeColor

//------------------------------------------------------------------------------
// aeMesh types
//------------------------------------------------------------------------------
struct aeMeshVertex
{
  aeFloat4 position;
  aeFloat4 normal;
  aeFloat2 tex[ 4 ];
  aeColor color[ 4 ];
};

typedef uint16_t aeMeshIndex;

//------------------------------------------------------------------------------
// aeMesh class
//------------------------------------------------------------------------------
class aeMesh
{
public:
  // Initialization
  bool LoadFileData( const uint8_t* data, uint32_t length, const char* extension, bool skipMeshOptimization = false );
  void Transform( aeFloat4x4 transform ); // Permanently pre-transform loaded verts

  // Mesh data
  const aeMeshVertex* GetVertices() const;
  const aeMeshIndex* GetIndices() const;
  uint32_t GetVertexCount() const;
  uint32_t GetIndexCount() const;
  
  // Sphere collision
  struct PushOutParams
  {
    //aeFloat4x4 transform; // @TODO: Support transforming
    aeSphere sphere;
    aeFloat3 velocity = aeFloat3( 0.0f );
    
    class aeDebugRender* debug = nullptr; // Draw collision results
    aeColor debugColor = aeColor::Red();
  };
  struct PushOutResult
  {
    aeFloat3 position = aeFloat3( 0.0f );
    aeFloat3 velocity = aeFloat3( 0.0f );
    
    uint32_t hitCount = 0;
    aeFloat3 hitPos[ 8 ];
    aeFloat3 hitNorm[ 8 ];
  };
  bool PushOut( const PushOutParams& params, PushOutResult* outResult ) const;

private:
  aeArray< aeMeshVertex > m_vertices;
  aeArray< aeMeshIndex > m_indices;
};

#endif
