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
#include "aeRender.h"
#include "aeMath.h"

namespace ae {

//------------------------------------------------------------------------------
// ae::CollisionMesh class
//------------------------------------------------------------------------------
class CollisionMesh
{
public:
  // Initialization params
  struct Params
  {
    uint32_t vertexCount = 0;
    const ae::Vec3* positions = nullptr;
    const ae::Vec3* normals = nullptr;
    uint32_t positionStride = sizeof(ae::Vec3);
    uint32_t normalStride = sizeof(ae::Vec3);

    uint32_t indexCount = 0;
    const uint16_t* indices16 = nullptr;
  };
  // RaycastParams
  struct RaycastParams
  {
    ae::Matrix4 transform = ae::Matrix4::Identity();
    ae::Vec3 source = ae::Vec3( 0.0f );
    ae::Vec3 direction = ae::Vec3( 0.0f, 0.0f, -1.0f );
    float maxLength = 0.0f;
    uint32_t maxHits = 1;
    bool hitCounterclockwise = true;
    bool hitClockwise = false;
    ae::DebugLines* debug = nullptr; // Draw collision results
    ae::Color debugColor = ae::Color::Red();
  };
  // RaycastResult
  struct RaycastResult
  {
    uint32_t hitCount = 0;
    struct Hit
    {
      ae::Vec3 position = ae::Vec3( 0.0f );
      ae::Vec3 normal = ae::Vec3( 0.0f );
      float t = 0.0f;
    } hits[ 8 ];
    
    void Accumulate( const RaycastParams& params, const RaycastResult& result );
  };
  // Sphere collision PushOutParams
  struct PushOutParams
  {
    ae::Matrix4 transform = ae::Matrix4::Identity();
    ae::DebugLines* debug = nullptr; // Draw collision results
    ae::Color debugColor = ae::Color::Red();
  };
  // Sphere collision PushOutInfo
  struct PushOutInfo
  {
    ae::Sphere sphere;
    ae::Vec3 velocity = ae::Vec3( 0.0f );
    struct Hit
    {
      ae::Vec3 position;
      ae::Vec3 normal;
    };
    ae::Array< Hit, 8 > hits;

    static void Accumulate( const PushOutParams& params, const PushOutInfo& prev, PushOutInfo* next );
  };
  
  CollisionMesh( ae::Tag tag );
  void Load( Params params ); // @TODO: Should allow multiple Load()s without clearing
  void Transform( ae::Matrix4 transform ); // Pre-transform loaded verts
  void Clear();
  bool Raycast( const RaycastParams& params, RaycastResult* outResult ) const;
  PushOutInfo PushOut( const PushOutParams& params, const PushOutInfo& info ) const;
  ae::AABB GetAABB() const { return m_aabb; }

private:
  typedef uint16_t Index;
  struct Vertex { ae::Vec4 position; ae::Vec4 normal; };
  ae::Array< Vertex > m_vertices;
  ae::Array< Index > m_indices;
  ae::AABB m_aabb;
};

bool LoadFileData( const uint8_t* data, uint32_t length, const char* extension, bool skipMeshOptimization = false );

}

#ifdef AE_MAIN
#include "../../src/aeMesh.cpp"
#endif

#endif
