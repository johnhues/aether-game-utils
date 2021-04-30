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
#include "aeBinaryStream.h"

// @TODO: Move to aeMath
bool IntersectRayTriangle( aeFloat3 p, aeFloat3 dir, aeFloat3 a, aeFloat3 b, aeFloat3 c, bool limitRay, bool ccw, bool cw, aeFloat3* pOut, aeFloat3* nOut, float* tOut )
{
  aeFloat3 ab = b - a;
  aeFloat3 ac = c - a;
  aeFloat3 n = ab % ac;
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
  aeFloat3 e = qp % ap;
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

//------------------------------------------------------------------------------
// aeMesh member functions
//------------------------------------------------------------------------------
bool aeMesh::LoadFileData( const uint8_t* data, uint32_t length, const char* extension, bool skipMeshOptimization )
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
    uint32_t uvCount = aeMath::Min( mesh->GetNumUVChannels(), countof(aeMeshVertex::tex) );
    uint32_t colorCount = aeMath::Min( mesh->GetNumColorChannels(), countof(aeMeshVertex::color) );
    AE_DEBUG( "Submesh: vertCount # uvCount # colorCount #", vertCount, uvCount, colorCount );
    
    if ( vertCount )
    {
      aiVector3D p = mesh->mVertices[ 0 ];
      m_aabb = aeAABB( aeFloat3( p.x, p.y, p.z ), aeFloat3( p.x, p.y, p.z ) );
    }

    for ( uint32_t j = 0; j < vertCount; j++ )
    {
      aeMeshVertex vertex;
      
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

void aeMesh::Load( aeMeshParams params )
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
      aeMeshVertex vert;
      memset( &vert, 0, sizeof(vert) );
      
      vert.position = aeFloat4( *params.positions, 1.0f );
      params.positions = (aeFloat3*)( (uint8_t*)params.positions + params.positionStride );
      
      if ( params.normals )
      {
        vert.normal = aeFloat4( *params.normals, 0.0f );
        params.normals = (aeFloat3*)( (uint8_t*)params.normals + params.normalStride );
      }
      
      m_aabb.Expand( vert.position.GetXYZ() );
      m_vertices.Append( vert );
    }
  }
  
  m_indices.Append( params.indices16, params.indexCount );
}

const uint32_t kAeMeshVersion = 1;
void aeMesh::Serialize( aeBinaryStream* stream )
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
  
  const uint32_t vertexSize = sizeof(*m_vertices.Begin());
  const uint32_t indexSize = sizeof(*m_indices.Begin());
  
  if ( stream->IsWriter() )
  {
    uint32_t vertexDataLen = m_vertices.Length() * vertexSize;
    stream->SerializeUint32( vertexDataLen );
    stream->SerializeRaw( m_vertices.Begin(), vertexDataLen );
    
    uint32_t indexDataLen = m_indices.Length() * indexSize;
    stream->SerializeUint32( indexDataLen );
    stream->SerializeRaw( m_indices.Begin(), indexDataLen );
  }
  else
  {
    AE_ASSERT( stream->IsReader() );
    
    uint32_t vertexDataLength = 0;
    stream->SerializeUint32( vertexDataLength );
    if ( vertexDataLength % vertexSize != 0 || stream->GetRemaining() < vertexDataLength )
    {
      stream->Invalidate();
      return;
    }
    m_vertices.Append( (const aeMeshVertex*)stream->PeekData(), vertexDataLength / vertexSize );
    stream->Discard( vertexDataLength );
    
    uint32_t indexDataLength = 0;
    stream->SerializeUint32( indexDataLength );
    if ( indexDataLength % indexSize != 0 || stream->GetRemaining() < indexDataLength )
    {
      stream->Invalidate();
      return;
    }
    m_indices.Append( (const aeMeshIndex*)stream->PeekData(), indexDataLength / indexSize );
    stream->Discard( indexDataLength );
  }
}

void aeMesh::Transform( aeFloat4x4 transform )
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

void aeMesh::Clear()
{
  m_vertices.Clear();
  m_indices.Clear();
  m_aabb = aeAABB();
}

const aeMeshVertex* aeMesh::GetVertices() const
{
  return m_vertices.Length() ? &m_vertices[ 0 ] : nullptr;
}

const aeMeshIndex* aeMesh::GetIndices() const
{
  return m_indices.Length() ? &m_indices[ 0 ] : nullptr;
}

uint32_t aeMesh::GetVertexCount() const
{
  return m_vertices.Length();
}

uint32_t aeMesh::GetIndexCount() const
{
  return m_indices.Length();
}

bool aeMesh::Raycast( const RaycastParams& params, RaycastResult* outResult ) const
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
  const aeFloat4x4 invTransform = params.transform.Inverse();
  const aeFloat3 source( invTransform * aeFloat4( params.source, 1.0f ) );
  const aeFloat3 ray( invTransform * aeFloat4( limitRay ? params.direction.SafeNormalizeCopy() * params.maxLength : params.direction, 0.0f ) );
  const aeFloat3 normDir = ray.SafeNormalizeCopy();
  const bool ccw = params.hitCounterclockwise;
  const bool cw = params.hitClockwise;
  
  const uint32_t triCount = GetIndexCount() / 3;
  const aeMeshIndex* indices = GetIndices();
  const aeMeshVertex* vertices = GetVertices();

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

bool aeMesh::PushOut( const PushOutParams& params, PushOutResult* outResult ) const
{
  aeOBB obb( params.transform * m_aabb.GetTransform() );
  if ( obb.GetMinDistance( params.sphere.center ) > params.sphere.radius )
  {
    return false; // Early out if sphere is to far from mesh
  }
  
  if ( aeDebugRender* debug = params.debug )
  {
    // Sphere intersects obb
    debug->AddCube( obb.GetTransform(), params.debugColor );
  }
  
  PushOutResult result;
  aeSphere sphere = params.sphere;
  result.velocity = params.velocity;
  uint32_t collisionCount = 0;
  bool identityTransform = ( params.transform == aeFloat4x4::Identity() );
  
  const uint32_t triCount = GetIndexCount() / 3;
  const aeMeshIndex* indices = GetIndices();
  const aeMeshVertex* vertices = GetVertices();
  
  for ( uint32_t i = 0; i < triCount; i++ )
  {
    aeFloat3 a, b, c;
    if ( identityTransform )
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
    
    aeFloat3 triNormal = ( ( b - a ) % ( c - a ) ).SafeNormalizeCopy();
    aeFloat3 triCenter( ( a + b + c ) / 3.0f );
    
    aeFloat3 triToSphereDir = ( sphere.center - triCenter ).SafeNormalizeCopy();
    if ( triNormal.Dot( triToSphereDir ) < 0.0f )
    {
      continue;
    }
    
    aeFloat3 triHitPos;
    if ( sphere.IntersectTriangle( a, b, c, &triHitPos ) )
    {
      triToSphereDir = ( sphere.center - triHitPos ).SafeNormalizeCopy();
      if ( triNormal.Dot( triToSphereDir ) < 0.3f )
      {
        continue;
      }
      
      aeFloat3 closestSpherePoint = ( triHitPos - sphere.center ).SafeNormalizeCopy();
      closestSpherePoint *= sphere.radius;
      closestSpherePoint += sphere.center;
      
      sphere.center += triHitPos - closestSpherePoint;
      result.velocity.ZeroDirection( -triNormal );
      
      if ( result.hitCount < countof( result.hitPos ) )
      {
        result.hitPos[ result.hitCount ] = triHitPos;
        result.hitNorm[ result.hitCount ] = triNormal;
        result.hitCount++;
      }
      
      collisionCount++;
      
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
  
  if ( collisionCount )
  {
    if ( outResult )
    {
      result.position = sphere.center;
      *outResult = result;
    }
    return true;
  }
  else
  {
    return false;
  }
}

//------------------------------------------------------------------------------
// RaycastResult
//------------------------------------------------------------------------------
void aeMesh::RaycastResult::Accumulate( const RaycastParams& params, const RaycastResult& result )
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
// PushOutResult
//------------------------------------------------------------------------------
void aeMesh::PushOutResult::Accumulate( const PushOutParams& params, const PushOutResult& result )
{
  position = result.position;
  velocity = result.velocity;
  
  // @TODO: Should hits be sorted in any way? Currently excess hits are discarded in a random order
  for ( uint32_t i = 0; i < result.hitCount && hitCount < countof(hitPos); i++ )
  {
    hitPos[ hitCount ] = result.hitPos[ i ];
    hitNorm[ hitCount ] = result.hitNorm[ i ];
    hitCount++;
  }
}

#endif
