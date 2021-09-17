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
//#include "aeMesh.h"

//------------------------------------------------------------------------------
// aeMesh member functions
//------------------------------------------------------------------------------
#if _AE_EMSCRIPTEN_

bool aeMesh::LoadFileData( const uint8_t* data, uint32_t length, const char* extension, bool skipMeshOptimization ) { return false; }
void aeMesh::Transform( ae::Matrix4 transform ) {}
const aeMeshVertex* aeMesh::GetVertices() const { return nullptr; }
const aeMeshIndex* aeMesh::GetIndices() const { return nullptr; }
uint32_t aeMesh::GetVertexCount() const { return 0; }
uint32_t aeMesh::GetIndexCount() const { return 0; }

#else // !_AE_EMSCRIPTEN_

//#include <assimp/cimport.h>
//#include <assimp/scene.h>
//#include <assimp/postprocess.h>
//
//bool ae::CollisionMesh::LoadFileData( const uint8_t* data, uint32_t length, const char* extension, bool skipMeshOptimization )
//{
//  m_vertices.Clear();
//  m_indices.Clear();
//
//#if _AE_DEBUG_
//  aiLogStream stream = aiGetPredefinedLogStream( aiDefaultLogStream_STDOUT, nullptr );
//  aiAttachLogStream( &stream );
//#endif
//
//  uint32_t importFlags = 0;
//
//  // Assimp doesn't process normals, tangents, bitangents correctly, so if skipMeshOptimization is true
//  // then don't run any Assimp processing for tangent or normal generation.  Just use the original mesh.
//  // Otherwise thin features have normals flipped and creases handling isn't correct either.
//  if ( !skipMeshOptimization )
//  {
//    importFlags |= aiProcessPreset_TargetRealtime_MaxQuality;
//    importFlags |= aiProcess_JoinIdenticalVertices;
//  }
//  importFlags |= aiProcess_PreTransformVertices;
//
//  const struct aiScene* scene = aiImportFileFromMemory( (const char*)data, length, importFlags, extension );
//  if ( !scene )
//  {
//    return false;
//  }
//
//  uint32_t meshCount = scene->mNumMeshes;
//  for ( uint32_t i = 0; i < meshCount; i++ )
//  {
//    const struct aiMesh* mesh = scene->mMeshes[ i ];
//
//    uint32_t vertCount = mesh->mNumVertices;
//    uint32_t uvCount = aeMath::Min( mesh->GetNumUVChannels(), countof(Vertex::tex) );
//    uint32_t colorCount = aeMath::Min( mesh->GetNumColorChannels(), countof(Vertex::color) );
//    AE_DEBUG( "Submesh: vertCount # uvCount # colorCount #", vertCount, uvCount, colorCount );
//
//    if ( vertCount )
//    {
//      aiVector3D p = mesh->mVertices[ 0 ];
//      m_aabb = aeAABB( ae::Vec3( p.x, p.y, p.z ), ae::Vec3( p.x, p.y, p.z ) );
//    }
//
//    for ( uint32_t j = 0; j < vertCount; j++ )
//    {
//      Vertex vertex;
//
//      // Position
//      aiVector3D p = mesh->mVertices[ j ];
//      vertex.position = ae::Vec4( p.x, p.y, p.z, 1.0f );
//      m_aabb.Expand( vertex.position.GetXYZ() );
//
//      // Normal
//      aiVector3D n = mesh->mNormals[ j ];
//      vertex.normal = ae::Vec4( n.x, n.y, n.z, 0.0f );
//
//      // UVs
//      memset( vertex.tex, 0, sizeof(vertex.tex) );
//      for ( uint32_t k = 0; k < uvCount; k++ )
//      {
//        AE_ASSERT( mesh->mTextureCoords[ k ] );
//        // @NOTE: An aiMesh may contain 0 to AI_MAX_NUMBER_OF_TEXTURECOORDS per vertex.
//        //        NULL if not present. The array is mNumVertices in size.
//        aiVector3D t = mesh->mTextureCoords[ k ][ j ];
//        vertex.tex[ k ] = ae::Vec2( t.x, t.y );
//      }
//
//      // Color
//      memset( vertex.color, 0, sizeof(vertex.color) );
//      for ( uint32_t k = 0; k < colorCount; k++ )
//      {
//        AE_ASSERT( mesh->mColors[ k ] );
//        aiColor4D c = mesh->mColors[ k ][ j ];
//        vertex.color[ k ] = ae::Color( c.r, c.g, c.b, c.a );
//      }
//
//      m_vertices.Append( vertex );
//    }
//  }
//
//  uint32_t currentVertexCount = 0;
//  for ( uint32_t i = 0; i < meshCount; i++ )
//  {
//    const struct aiMesh* mesh = scene->mMeshes[ i ];
//    uint32_t faceCount = mesh->mNumFaces;
//
//    for ( uint32_t j = 0; j < faceCount; j++ )
//    {
//      uint32_t faceIndexCount = mesh->mFaces[ j ].mNumIndices;
//      if ( faceIndexCount == 3 )
//      {
//        for ( uint32_t k = 0; k < 3; k++ )
//        {
//          uint32_t index = currentVertexCount + mesh->mFaces[ j ].mIndices[ k ];
//          AE_ASSERT( index < std::numeric_limits< uint16_t >::max() );
//          m_indices.Append( (uint16_t)index );
//        }
//      }
//    }
//
//    currentVertexCount += mesh->mNumVertices;
//  }
//
//  aiReleaseImport( scene );
//#if _AE_DEBUG_
//  aiDetachAllLogStreams();
//#endif
//
//  return true;
//}

ae::CollisionMesh::CollisionMesh( ae::Tag tag ) :
  m_vertices( tag ),
  m_indices( tag )
{
  Clear();
}

void ae::CollisionMesh::Load( Params params )
{
  AE_ASSERT_MSG( !m_vertices.Length(), "TODO! Should allow multiple Load()s without clearing" );
  
  AE_ASSERT( params.positions );
  AE_ASSERT_MSG( params.positionStride > sizeof(float) * 3, "Must specify the number of bytes between each position" );
  if ( params.indexCount )
  {
    AE_ASSERT( params.indexCount % 3 == 0 );
  }
  else
  {
    AE_ASSERT( params.vertexCount % 3 == 0 );
  }

  bool identityTransform = ( params.transform == ae::Matrix4::Identity() );

  ae::Vec3 p0 = params.positions[ 0 ];
  if ( !identityTransform )
  {
    p0 = ( params.transform * ae::Vec4( p0, 1.0f ) ).GetXYZ();
  }
  m_aabb = ae::AABB( p0, p0 );

  m_vertices.Reserve( params.vertexCount );
  for ( uint32_t i = 0; i < params.vertexCount; i++ )
  {
    Vertex vert;
    memset( &vert, 0, sizeof(vert) );
    
    vert.position = ae::Vec4( *params.positions, 1.0f );
    if ( !identityTransform )
    {
      vert.position = params.transform * vert.position;
    }
    params.positions = (ae::Vec3*)( (uint8_t*)params.positions + params.positionStride );
    
    m_aabb.Expand( vert.position.GetXYZ() );
    m_vertices.Append( vert );
  }
  
  m_indices.Append( params.indices16, params.indexCount );
}

void ae::CollisionMesh::Clear()
{
  m_vertices.Clear();
  m_indices.Clear();
  m_aabb = ae::AABB();
}

bool ae::CollisionMesh::Raycast( const RaycastParams& params, RaycastResult* outResult ) const
{
  // Early out for parameters that will give no results
  if ( params.maxLength < 0.0f || params.maxHits == 0 )
  {
    return false;
  }
  
  bool limitRay = params.maxLength != 0.0f;
  ae::Vec3 normParamsDir = params.direction.SafeNormalizeCopy();

  // Obb in world space
  {
    float obbDistance = ae::MaxValue< float >();
    ae::OBB obb( params.transform * m_aabb.GetTransform() );
    if ( !obb.IntersectRay( params.source, normParamsDir, nullptr, &obbDistance )
      || ( limitRay && obbDistance > params.maxLength ) )
    {
      if ( ae::DebugLines* debug = params.debug )
      {
        ae::Vec3 rayEnd = params.source + normParamsDir * ( limitRay ? params.maxLength : 1000.0f );
        debug->AddLine( params.source, rayEnd, params.debugColor );
      }
      return false; // Early out if ray doesn't touch obb
    }
    
    if ( ae::DebugLines* debug = params.debug )
    {
      // Ray intersects obb
      debug->AddOBB( obb.GetTransform(), params.debugColor );
    }
  }
  
  const ae::Matrix4 invTransform = params.transform.GetInverse();
  const ae::Vec3 source( invTransform * ae::Vec4( params.source, 1.0f ) );
  const ae::Vec3 ray( invTransform * ae::Vec4( normParamsDir * ( limitRay ? params.maxLength : 1.0f ), 0.0f ) );
  const ae::Vec3 normDir = ray.SafeNormalizeCopy();
  const bool ccw = params.hitCounterclockwise;
  const bool cw = params.hitClockwise;
  
  const uint32_t triCount = m_indices.Length() / 3;
  const Index* indices = m_indices.Begin();
  const Vertex* vertices = &m_vertices[ 0 ];

  uint32_t hitCount  = 0;
  RaycastResult::Hit hits[ countof(RaycastResult::hits) + 1 ];
  const uint32_t maxHits = aeMath::Min( params.maxHits, countof(RaycastResult::hits) );
  for ( uint32_t i = 0; i < triCount; i++ )
  {
    ae::Vec3 p, n;
    ae::Vec3 a = vertices[ indices[ i * 3 ] ].position.GetXYZ();
    ae::Vec3 b = vertices[ indices[ i * 3 + 1 ] ].position.GetXYZ();
    ae::Vec3 c = vertices[ indices[ i * 3 + 2 ] ].position.GetXYZ();
    if ( IntersectRayTriangle( source, ray, a, b, c, limitRay, ccw, cw, &p, &n, nullptr ) )
    {
      RaycastResult::Hit& outHit = hits[ hitCount ];
      hitCount++;
      AE_ASSERT( hitCount <= maxHits + 1 ); // Allow one extra hit, then sort and remove last hit below
      
      outHit.position = ae::Vec3( params.transform * ae::Vec4( p, 1.0f ) );
      outHit.normal = ae::Vec3( params.transform * ae::Vec4( n, 0.0f ) );
      outHit.distance = ( outHit.position - params.source ).Length(); // Calculate here because transform might not have uniform scale
      outHit.userData = params.userData;
      
      if ( hitCount > maxHits )
      {
        std::sort( hits, hits + hitCount, []( const RaycastResult::Hit& a, const RaycastResult::Hit& b ) { return a.distance < b.distance; } );
        hitCount = maxHits;
      }
    }
  }
  
  if ( ae::DebugLines* debug = params.debug )
  {
    ae::Vec3 rayEnd;
    if ( !limitRay )
    {
      if ( hitCount )
      {
        rayEnd = hits[ hitCount - 1 ].position;
      }
      else
      {
        rayEnd = params.source + normParamsDir * 1000.0f;
      }
    }
    else
    {
      rayEnd = params.source + normParamsDir * params.maxLength;
    }
    debug->AddLine( params.source, rayEnd, params.debugColor );
    
    for ( uint32_t i = 0; i < hitCount; i++ )
    {
      const RaycastResult::Hit* hit = &hits[ i ];
      const ae::Vec3 p = hit->position;
      const ae::Vec3 n = hit->normal;
      float s = ( hitCount > 1 ) ? ( i / ( hitCount - 1.0f ) ) : 1.0f;
      debug->AddCircle( p, n, aeMath::Lerp( 0.25f, 0.3f, s ), params.debugColor, 8 );
      debug->AddLine( p, p + n, params.debugColor );
    }
  }
  
  if ( outResult )
  {
    std::sort( hits, hits + hitCount, []( const RaycastResult::Hit& a, const RaycastResult::Hit& b ) { return a.distance < b.distance; } );
    outResult->hitCount = hitCount;
    for ( uint32_t i = 0; i < hitCount; i++ )
    {
      hits[ i ].normal.SafeNormalize();
      outResult->hits[ i ] = hits[ i ];
    }
  }
  return hitCount;
}

ae::CollisionMesh::RaycastResult ae::CollisionMesh::Raycast( const RaycastParams& params, const RaycastResult& prevResult ) const
{
  // @TODO: For params requesting a single hit set max length of ray to prevResult hit distance for obb early out
  RaycastResult nextResult;
  if ( Raycast( params, &nextResult ) )
  {
    ae::CollisionMesh::RaycastResult::Accumulate( params, prevResult, &nextResult );
    return nextResult;
  }
  return prevResult;
}

ae::CollisionMesh::PushOutInfo ae::CollisionMesh::PushOut( const PushOutParams& params, const PushOutInfo& prevInfo ) const
{
  if ( ae::DebugLines* debug = params.debug )
  {
    debug->AddSphere( prevInfo.sphere.center, prevInfo.sphere.radius, params.debugColor, 8 );
  }

  ae::OBB obb( params.transform * m_aabb.GetTransform() );
  if ( obb.GetSignedDistanceFromSurface( prevInfo.sphere.center ) > prevInfo.sphere.radius )
  {
    return prevInfo; // Early out if sphere is to far from mesh
  }
  
  if ( ae::DebugLines* debug = params.debug )
  {
    // Sphere intersects obb
    debug->AddOBB( obb.GetTransform(), params.debugColor );
  }
  
  PushOutInfo result;
  result.sphere = prevInfo.sphere;
  result.velocity = prevInfo.velocity;
  bool hasIdentityTransform = ( params.transform == ae::Matrix4::Identity() );
  
  const uint32_t triCount = m_indices.Length() / 3;
  const Index* indices = m_indices.Begin();
  const Vertex* vertices = &m_vertices[ 0 ];
  
  for ( uint32_t i = 0; i < triCount; i++ )
  {
    ae::Vec3 a, b, c;
    if ( hasIdentityTransform )
    {
      a = vertices[ indices[ i * 3 ] ].position.GetXYZ();
      b = vertices[ indices[ i * 3 + 1 ] ].position.GetXYZ();
      c = vertices[ indices[ i * 3 + 2 ] ].position.GetXYZ();
    }
    else
    {
      a = ae::Vec3( params.transform * vertices[ indices[ i * 3 ] ].position );
      b = ae::Vec3( params.transform * vertices[ indices[ i * 3 + 1 ] ].position );
      c = ae::Vec3( params.transform * vertices[ indices[ i * 3 + 2 ] ].position );
    }
    
    ae::Vec3 triNormal = ( ( b - a ).Cross( c - a ) ).SafeNormalizeCopy();
    ae::Vec3 triCenter( ( a + b + c ) / 3.0f );
    
    ae::Vec3 triToSphereDir = ( result.sphere.center - triCenter );
    if ( triNormal.Dot( triToSphereDir ) < 0.0f )
    {
      continue;
    }
    
    ae::Vec3 triHitPos;
    if ( result.sphere.IntersectTriangle( a, b, c, &triHitPos ) )
    {
      triToSphereDir = ( result.sphere.center - triHitPos );
      if ( triNormal.Dot( triToSphereDir ) < 0.0f )
      {
        continue;
      }
      
      ae::Vec3 closestSpherePoint = ( triHitPos - result.sphere.center ).SafeNormalizeCopy();
      closestSpherePoint *= result.sphere.radius;
      closestSpherePoint += result.sphere.center;
      
      result.sphere.center += triHitPos - closestSpherePoint;
      result.velocity.ZeroDirection( -triNormal );
      
      // @TODO: Sort. Shouldn't randomly discard hits.
      if ( result.hits.Length() < result.hits.Size() )
      {
        result.hits.Append( { triHitPos, triNormal } );
      }
      
      if ( ae::DebugLines* debug = params.debug )
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
    PushOutInfo::Accumulate( params, prevInfo, &result );
    return result;
  }
  else
  {
    return prevInfo;
  }
}

//------------------------------------------------------------------------------
// RaycastResult
//------------------------------------------------------------------------------
void ae::CollisionMesh::RaycastResult::Accumulate( const RaycastParams& params, const RaycastResult& prev, RaycastResult* next )
{
  uint32_t accumHitCount = 0;
  Hit accumHits[ countof(next->hits) * 2 ];
  
  for ( uint32_t i = 0; i < next->hitCount; i++ )
  {
    accumHits[ accumHitCount ] = next->hits[ i ];
    accumHitCount++;
  }
  for ( uint32_t i = 0; i < prev.hitCount; i++ )
  {
    accumHits[ accumHitCount ] = prev.hits[ i ];
    accumHitCount++;
  }
  std::sort( accumHits, accumHits + accumHitCount, []( const Hit& h0, const Hit& h1 ){ return h0.distance < h1.distance; } );
  
  next->hitCount = aeMath::Min( accumHitCount, params.maxHits, countof(next->hits) );
  for ( uint32_t i = 0; i < next->hitCount; i++ )
  {
    next->hits[ i ] = accumHits[ i ];
  }
}

//------------------------------------------------------------------------------
// PushOutInfo
//------------------------------------------------------------------------------
void ae::CollisionMesh::PushOutInfo::Accumulate( const PushOutParams& params, const PushOutInfo& prev, PushOutInfo* next )
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
