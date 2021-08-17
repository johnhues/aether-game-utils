//------------------------------------------------------------------------------
// aeMesh.cpp
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
// Headers
//------------------------------------------------------------------------------
#include "aeMesh.h"

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
void ae::Mesh::Vertex::Serialize( const SerializationParams& params, ae::BinaryStream* stream )
{
  bool reader = stream->IsReader();
  if ( params.position )
  {
    stream->SerializeFloat( position.x );
    stream->SerializeFloat( position.y );
    stream->SerializeFloat( position.z );
    if ( reader )
    {
      position.w = 1.0f;
    }
  }
  if ( params.normal )
  {
    stream->SerializeFloat( normal.x );
    stream->SerializeFloat( normal.y );
    stream->SerializeFloat( normal.z );
    if ( reader )
    {
      normal.w = 0.0f;
    }
  }
  for ( uint32_t i = 0; i < params.uvSets; i++ )
  {
    stream->SerializeFloat( tex[ i ].x );
    stream->SerializeFloat( tex[ i ].y );
  }
  for ( uint32_t i = 0; i < params.colorSets; i++ )
  {
    stream->SerializeFloat( color[ i ].r );
    stream->SerializeFloat( color[ i ].g );
    stream->SerializeFloat( color[ i ].b );
    stream->SerializeFloat( color[ i ].a );
  }
  for ( uint32_t i = 0; i < params.userDataCount; i++ )
  {
    stream->SerializeUint8( userData[ i ] );
  }
}

// @TODO: Move to aeMath
bool IntersectRayTriangle( aeFloat3 p, aeFloat3 dir, aeFloat3 a, aeFloat3 b, aeFloat3 c, bool limitRay, bool ccw, bool cw, aeFloat3* pOut, aeFloat3* nOut, float* tOut )
{
  aeFloat3 ab = b - a;
  aeFloat3 ac = c - a;
  aeFloat3 n = ab.Cross( ac );
  aeFloat3 qp = -dir;
  
  // Compute denominator d
  float d = qp.Dot( n );
  if ( !ccw && d > 0.0f )
  {
    return false;
  }
  if ( !cw && d < 0.0f )
  {
    return false;
  }
  // Parallel
  if ( d * d < 0.001f )
  {
    return false;
  }
  float ood = 1.0f / d;
  
  // Compute intersection t value of pq with plane of triangle
  aeFloat3 ap = p - a;
  float t = ap.Dot( n ) * ood;
  // Ray intersects if 0 <= t
  if ( t < 0.0f )
  {
    return false;
  }
  // Segment intersects if 0 <= t <= 1
  if ( limitRay && t > 1.0f )
  {
    return false;
  }
  
  // Compute barycentric coordinate components and test if within bounds
  aeFloat3 e = qp.Cross( ap );
  float v = ac.Dot( e ) * ood;
  if ( v < 0.0f || v > 1.0f )
  {
    return false;
  }
  float w = -ab.Dot( e ) * ood;
  if ( w < 0.0f || v + w > 1.0f )
  {
    return false;
  }
  
  // Result
  if ( pOut )
  {
    *pOut = p + dir * t;
  }
  if ( nOut )
  {
    *nOut = n.SafeNormalizeCopy();
  }
  if ( tOut )
  {
    *tOut = t;
  }
  return true;
}

//------------------------------------------------------------------------------
// aeMesh member functions
//------------------------------------------------------------------------------
#if _AE_EMSCRIPTEN_

bool aeMesh::LoadFileData( const uint8_t* data, uint32_t length, const char* extension, bool skipMeshOptimization ) { return false; }
void aeMesh::Transform( aeFloat4x4 transform ) {}
const aeMeshVertex* aeMesh::GetVertices() const { return nullptr; }
const aeMeshIndex* aeMesh::GetIndices() const { return nullptr; }
uint32_t aeMesh::GetVertexCount() const { return 0; }
uint32_t aeMesh::GetIndexCount() const { return 0; }

#else // !_AE_EMSCRIPTEN_

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

bool ae::Mesh::LoadFileData( const uint8_t* data, uint32_t length, const char* extension, bool skipMeshOptimization )
{
  m_vertices.Clear();
  m_indices.Clear();

#if _AE_DEBUG_
  aiLogStream stream = aiGetPredefinedLogStream( aiDefaultLogStream_STDOUT, nullptr );
  aiAttachLogStream( &stream );
#endif

  uint32_t importFlags = 0;
	
  // Assimp doesn't process normals, tangents, bitangents correctly, so if skipMeshOptimization is true
  // then don't run any Assimp processing for tangent or normal generation.  Just use the original mesh.
  // Otherwise thin features have normals flipped and creases handling isn't correct either.
  if ( !skipMeshOptimization )
  {
    importFlags |= aiProcessPreset_TargetRealtime_MaxQuality;
    importFlags |= aiProcess_JoinIdenticalVertices;
  }
  importFlags |= aiProcess_PreTransformVertices;
  
  const struct aiScene* scene = aiImportFileFromMemory( (const char*)data, length, importFlags, extension );
  if ( !scene )
  {
    return false;
  }
  
  uint32_t meshCount = scene->mNumMeshes;
  for ( uint32_t i = 0; i < meshCount; i++ )
  {
    const struct aiMesh* mesh = scene->mMeshes[ i ];

    uint32_t vertCount = mesh->mNumVertices;
    uint32_t uvCount = aeMath::Min( mesh->GetNumUVChannels(), countof(Vertex::tex) );
    uint32_t colorCount = aeMath::Min( mesh->GetNumColorChannels(), countof(Vertex::color) );
    AE_DEBUG( "Submesh: vertCount # uvCount # colorCount #", vertCount, uvCount, colorCount );
    
    if ( vertCount )
    {
      aiVector3D p = mesh->mVertices[ 0 ];
      m_aabb = aeAABB( aeFloat3( p.x, p.y, p.z ), aeFloat3( p.x, p.y, p.z ) );
    }

    for ( uint32_t j = 0; j < vertCount; j++ )
    {
      Vertex vertex;
      
      // Position
      aiVector3D p = mesh->mVertices[ j ];
      vertex.position = aeFloat4( p.x, p.y, p.z, 1.0f );
      m_aabb.Expand( vertex.position.GetXYZ() );

      // Normal
      aiVector3D n = mesh->mNormals[ j ];
      vertex.normal = aeFloat4( n.x, n.y, n.z, 0.0f );

      // UVs
      memset( vertex.tex, 0, sizeof(vertex.tex) );
      for ( uint32_t k = 0; k < uvCount; k++ )
      {
        AE_ASSERT( mesh->mTextureCoords[ k ] );
        // @NOTE: An aiMesh may contain 0 to AI_MAX_NUMBER_OF_TEXTURECOORDS per vertex.
        //        NULL if not present. The array is mNumVertices in size.
        aiVector3D t = mesh->mTextureCoords[ k ][ j ];
        vertex.tex[ k ] = aeFloat2( t.x, t.y );
      }

      // Color
      memset( vertex.color, 0, sizeof(vertex.color) );
      for ( uint32_t k = 0; k < colorCount; k++ )
      {
        AE_ASSERT( mesh->mColors[ k ] );
        aiColor4D c = mesh->mColors[ k ][ j ];
        vertex.color[ k ] = aeColor( c.r, c.g, c.b, c.a );
      }

      m_vertices.Append( vertex );
    }
  }
  
  uint32_t currentVertexCount = 0;
  for ( uint32_t i = 0; i < meshCount; i++ )
  {
    const struct aiMesh* mesh = scene->mMeshes[ i ];
    uint32_t faceCount = mesh->mNumFaces;
    
    for ( uint32_t j = 0; j < faceCount; j++ )
    {
      uint32_t faceIndexCount = mesh->mFaces[ j ].mNumIndices;
      if ( faceIndexCount == 3 )
      {
        for ( uint32_t k = 0; k < 3; k++ )
        {
          uint32_t index = currentVertexCount + mesh->mFaces[ j ].mIndices[ k ];
          AE_ASSERT( index < std::numeric_limits< uint16_t >::max() );
          m_indices.Append( (uint16_t)index );
        }
      }
    }
    
    currentVertexCount += mesh->mNumVertices;
  }
  
  aiReleaseImport( scene );
#if _AE_DEBUG_
  aiDetachAllLogStreams();
#endif

  return true;
}

void ae::Mesh::Load( LoadParams params )
{
  Clear();
  
  if ( params.indexCount )
  {
    AE_ASSERT( params.indexCount % 3 == 0 );
  }
  else
  {
    AE_ASSERT( params.vertexCount % 3 == 0 );
  }
  
  if ( params.vertexCount )
  {
    AE_ASSERT( params.positions );
    m_aabb = aeAABB( params.positions[ 0 ], params.positions[ 0 ] );
    
    m_vertices.Reserve( params.vertexCount );
    for ( uint32_t i = 0; i < params.vertexCount; i++ )
    {
      Vertex vert;
      memset( &vert, 0, sizeof(vert) );
      
      vert.position = aeFloat4( *params.positions, 1.0f );
      params.positions = (aeFloat3*)( (uint8_t*)params.positions + params.positionStride );
      
      if ( params.normals )
      {
        vert.normal = aeFloat4( *params.normals, 0.0f );
        params.normals = (aeFloat3*)( (uint8_t*)params.normals + params.normalStride );
      }
      
      if ( params.userData )
      {
        memcpy( vert.userData, params.userData, sizeof(Vertex::userData) );
        params.userData = params.userData + params.userDataStride;
      }
      
      m_aabb.Expand( vert.position.GetXYZ() );
      m_vertices.Append( vert );
    }
  }
  
  m_indices.Append( params.indices16, params.indexCount );
}

const uint32_t kAeMeshVersion = 2;
void ae::Mesh::Serialize( const SerializationParams& params, ae::BinaryStream* stream )
{
  if ( stream->IsReader() )
  {
    Clear();
  }
  
  uint32_t version = kAeMeshVersion;
  stream->SerializeUint32( version );
  if ( version != kAeMeshVersion )
  {
    stream->Invalidate();
    return;
  }
  
  stream->SerializeRaw( &m_aabb, sizeof(m_aabb) );
  
  const uint32_t indexSize = sizeof(*m_indices.Begin());
  
  if ( stream->IsWriter() )
  {
    uint32_t vertexCount = m_vertices.Length();
    stream->SerializeUint32( vertexCount );
    for ( uint32_t i = 0; i < vertexCount; i++ )
    {
      m_vertices[ i ].Serialize( params, stream );
    }
    
    uint32_t indexDataLen = m_indices.Length() * indexSize;
    stream->SerializeUint32( indexDataLen );
    stream->SerializeRaw( m_indices.Begin(), indexDataLen );
  }
  else
  {
    AE_ASSERT( stream->IsReader() );
    
    uint32_t vertexCount = 0;
    stream->SerializeUint32( vertexCount );
    m_vertices.Reserve( vertexCount );
    for ( uint32_t i = 0; i < vertexCount; i++ )
    {
      Vertex v;
      v.Serialize( params, stream );
      m_vertices.Append( v );
    }
    
    uint32_t indexDataLength = 0;
    stream->SerializeUint32( indexDataLength );
    if ( indexDataLength % indexSize != 0 || stream->GetRemaining() < indexDataLength )
    {
      stream->Invalidate();
      return;
    }
    m_indices.Append( (const Index*)stream->PeekData(), indexDataLength / indexSize );
    stream->Discard( indexDataLength );
  }
}

void ae::Mesh::Transform( aeFloat4x4 transform )
{
  if ( !m_vertices.Length() )
  {
    return;
  }
  
  aeFloat3 p( transform * m_vertices[ 0 ].position );
  m_aabb = aeAABB( p, p );
  
  // @TODO: Transform normals
  for ( uint32_t i = 0; i < m_vertices.Length(); i++ )
  {
    m_vertices[ i ].position = transform * m_vertices[ i ].position;
    m_aabb.Expand( m_vertices[ i ].position.GetXYZ() );
  }
}

void ae::Mesh::Clear()
{
  m_vertices.Clear();
  m_indices.Clear();
  m_aabb = aeAABB();
}

const ae::Mesh::Vertex* ae::Mesh::GetVertices() const
{
  return m_vertices.Length() ? &m_vertices[ 0 ] : nullptr;
}

const ae::Mesh::Index* ae::Mesh::GetIndices() const
{
  return m_indices.Length() ? &m_indices[ 0 ] : nullptr;
}

uint32_t ae::Mesh::GetVertexCount() const
{
  return m_vertices.Length();
}

uint32_t ae::Mesh::GetIndexCount() const
{
  return m_indices.Length();
}

bool ae::Mesh::Raycast( const RaycastParams& params, RaycastResult* outResult ) const
{
  // Early out for parameters that will give no results
  if ( params.maxLength < 0.0f || params.maxHits == 0 )
  {
    return false;
  }
  
  // Obb in world space
  {
    aeOBB obb( params.transform * m_aabb.GetTransform() );
    if ( !obb.IntersectRay( params.source, params.direction ) )
    {
      return false; // Early out if ray doesn't touch obb
    }
    
    if ( aeDebugRender* debug = params.debug )
    {
      // Ray intersects obb
      debug->AddCube( obb.GetTransform(), params.debugColor );
    }
  }
  
  bool limitRay = params.maxLength != 0.0f;
  const aeFloat4x4 invTransform = params.transform.GetInverse();
  const aeFloat3 source( invTransform * aeFloat4( params.source, 1.0f ) );
  const aeFloat3 ray( invTransform * aeFloat4( limitRay ? params.direction.SafeNormalizeCopy() * params.maxLength : params.direction, 0.0f ) );
  const aeFloat3 normDir = ray.SafeNormalizeCopy();
  const bool ccw = params.hitCounterclockwise;
  const bool cw = params.hitClockwise;
  
  const uint32_t triCount = GetIndexCount() / 3;
  const Index* indices = GetIndices();
  const Vertex* vertices = GetVertices();

  uint32_t hitCount  = 0;
  RaycastResult::Hit hits[ countof(RaycastResult::hits) + 1 ];
  const uint32_t maxHits = aeMath::Min( params.maxHits, countof(RaycastResult::hits) );
  for ( uint32_t i = 0; i < triCount; i++ )
  {
    aeFloat3 a = vertices[ indices[ i * 3 ] ].position.GetXYZ();
    aeFloat3 b = vertices[ indices[ i * 3 + 1 ] ].position.GetXYZ();
    aeFloat3 c = vertices[ indices[ i * 3 + 2 ] ].position.GetXYZ();
    
    aeFloat3 p;
    aeFloat3 n;
    if ( IntersectRayTriangle( source, ray, a, b, c, limitRay, ccw, cw, &p, &n, nullptr ) )
    {
      RaycastResult::Hit* outHit = &hits[ hitCount ];
      if ( hitCount <= maxHits ) // Allow one extra hit, then sort array and remove last
      {
        hitCount++;
      }
      
      p = aeFloat3( params.transform * aeFloat4( p, 1.0f ) );
      n = aeFloat3( params.transform * aeFloat4( n, 0.0f ) );
      
      outHit->position = p;
      outHit->normal = n;
      outHit->t = normDir.Dot( p - params.source ); // Calculate here because transform might not have uniform scale
      
      if ( hitCount > maxHits )
      {
        std::sort( hits, hits + hitCount, []( const RaycastResult::Hit& a, const RaycastResult::Hit& b ) { return a.t < b.t; } );
        hitCount = maxHits;
      }
    }
  }
  
  if ( aeDebugRender* debug = params.debug )
  {
    if ( hitCount && !limitRay )
    {
      debug->AddLine( params.source, hits[ hitCount - 1 ].position, params.debugColor );
    }
    else
    {
      debug->AddLine( params.source, params.source + ray, params.debugColor );
    }
    
    for ( uint32_t i = 0; i < hitCount; i++ )
    {
      const RaycastResult::Hit* hit = &hits[ i ];
      const aeFloat3 p = hit->position;
      const aeFloat3 n = hit->normal;
      float s = ( hitCount > 1 ) ? ( i / ( hitCount - 1.0f ) ) : 1.0f;
      debug->AddCircle( p, n, aeMath::Lerp( 0.25f, 0.3f, s ), params.debugColor, 8 );
      debug->AddLine( p, p + n, params.debugColor );
    }
  }
  
  if ( outResult )
  {
    outResult->hitCount = hitCount;
    for ( uint32_t i = 0; i < hitCount; i++ )
    {
      outResult->hits[ i ] = hits[ i ];
    }
  }
  return hitCount;
}

ae::Mesh::PushOutInfo ae::Mesh::PushOut( const PushOutParams& params, const PushOutInfo& info ) const
{
  aeOBB obb( params.transform * m_aabb.GetTransform() );
  if ( obb.GetMinDistance( info.sphere.center ) > info.sphere.radius )
  {
    return info; // Early out if sphere is to far from mesh
  }
  
  if ( aeDebugRender* debug = params.debug )
  {
    // Sphere intersects obb
    debug->AddCube( obb.GetTransform(), params.debugColor );
  }
  
  PushOutInfo result;
  result.sphere = info.sphere;
  result.velocity = info.velocity;
  bool hasIdentityTransform = ( params.transform == aeFloat4x4::Identity() );
  
  const uint32_t triCount = GetIndexCount() / 3;
  const Index* indices = GetIndices();
  const Vertex* vertices = GetVertices();
  
  for ( uint32_t i = 0; i < triCount; i++ )
  {
    aeFloat3 a, b, c;
    if ( hasIdentityTransform )
    {
      a = vertices[ indices[ i * 3 ] ].position.GetXYZ();
      b = vertices[ indices[ i * 3 + 1 ] ].position.GetXYZ();
      c = vertices[ indices[ i * 3 + 2 ] ].position.GetXYZ();
    }
    else
    {
      a = aeFloat3( params.transform * vertices[ indices[ i * 3 ] ].position );
      b = aeFloat3( params.transform * vertices[ indices[ i * 3 + 1 ] ].position );
      c = aeFloat3( params.transform * vertices[ indices[ i * 3 + 2 ] ].position );
    }
    
    aeFloat3 triNormal = ( ( b - a ).Cross( c - a ) ).SafeNormalizeCopy();
    aeFloat3 triCenter( ( a + b + c ) / 3.0f );
    
    aeFloat3 triToSphereDir = ( result.sphere.center - triCenter );
    if ( triNormal.Dot( triToSphereDir ) < 0.0f )
    {
      continue;
    }
    
    aeFloat3 triHitPos;
    if ( result.sphere.IntersectTriangle( a, b, c, &triHitPos ) )
    {
      triToSphereDir = ( result.sphere.center - triHitPos );
      if ( triNormal.Dot( triToSphereDir ) < 0.0f )
      {
        continue;
      }
      
      aeFloat3 closestSpherePoint = ( triHitPos - result.sphere.center ).SafeNormalizeCopy();
      closestSpherePoint *= result.sphere.radius;
      closestSpherePoint += result.sphere.center;
      
      result.sphere.center += triHitPos - closestSpherePoint;
      result.velocity.ZeroDirection( -triNormal );
      
      // @TODO: Sort. Shouldn't randomly discard hits.
      if ( result.hits.Length() < result.hits.Size() )
      {
        result.hits.Append( { triHitPos, triNormal } );
      }
      
      if ( aeDebugRender* debug = params.debug )
      {
        debug->AddLine( a, b, params.debugColor );
        debug->AddLine( b, c, params.debugColor );
        debug->AddLine( c, a, params.debugColor );
        
        debug->AddLine( triHitPos, triHitPos + triNormal * 2.0f, params.debugColor );
        debug->AddSphere( triHitPos, 0.05f, params.debugColor, 4 );
      }
    }
  }
  
  if ( result.hits.Length() )
  {
    PushOutInfo::Accumulate( params, info, &result );
    return result;
  }
  else
  {
    return info;
  }
}

//------------------------------------------------------------------------------
// RaycastResult
//------------------------------------------------------------------------------
void ae::Mesh::RaycastResult::Accumulate( const RaycastParams& params, const RaycastResult& result )
{
  uint32_t accumHitCount = 0;
  Hit accumHits[ countof(hits) * 2 ];
  
  for ( uint32_t i = 0; i < hitCount; i++ )
  {
    accumHits[ accumHitCount ] = hits[ i ];
    accumHitCount++;
  }
  for ( uint32_t i = 0; i < result.hitCount; i++ )
  {
    accumHits[ accumHitCount ] = result.hits[ i ];
    accumHitCount++;
  }
  std::sort( accumHits, accumHits + accumHitCount, []( const Hit& h0, const Hit& h1 ){ return h0.t < h1.t; } );
  
  hitCount = aeMath::Min( accumHitCount, countof(hits) );
  for ( uint32_t i = 0; i < hitCount; i++ )
  {
    hits[ i ] = accumHits[ i ];
  }
}

//------------------------------------------------------------------------------
// PushOutInfo
//------------------------------------------------------------------------------
void ae::Mesh::PushOutInfo::Accumulate( const PushOutParams& params, const PushOutInfo& prev, PushOutInfo* next )
{
  // @NOTE: Leave next::position/velocity unchanged since it's the latest
  // @TODO: Params are currently not used, but they could be used for sorting later
  auto&& nHits = next->hits;
  for ( auto&& hit : prev.hits )
  {
    if ( nHits.Length() < nHits.Size() )
    {
      nHits.Append( hit );
    }
    else
    {
      break;
    }
  }
}

#endif
