//------------------------------------------------------------------------------
// aeTerrainSdf.cpp
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
#include "aeTerrain.h"

namespace AE_NAMESPACE {

//------------------------------------------------------------------------------
// Sdf helpers
//------------------------------------------------------------------------------
inline float SdfUnion( float d1, float d2 )
{
  return aeMath::Min( d1, d2 );
}

inline float SdfSubtraction( float d1, float d2 )
{
  return aeMath::Max( -d1, d2 );
}

inline float SdfIntersection( float d1, float d2 )
{
  return aeMath::Max( d1, d2 );
}

inline float SdfSmoothUnion( float d1, float d2, float k )
{
  float h = aeMath::Clip01( 0.5f + 0.5f * ( d2 - d1 ) / k );
  return aeMath::Lerp( d2, d1, h ) - k * h * ( 1.0f - h );
}

inline float SdfSmoothSubtraction( float d1, float d2, float k )
{
  float h = aeMath::Clip01( 0.5f - 0.5f * ( d2 + d1 ) / k );
  return aeMath::Lerp( d2, -d1, h ) + k * h * ( 1.0f - h );
}

//------------------------------------------------------------------------------
// Sdf member functions
//------------------------------------------------------------------------------
Sdf::Sdf() :
  m_aabb( aeAABB( aeFloat3( 0.0f ), aeFloat3( 0.0f ) ) ),
  m_halfSize( 0.5f ),
  m_localToWorld( aeFloat4x4::Identity() ),
  m_removeTR( aeFloat4x4::Identity() ),
  m_aabbPrev( aeAABB( aeFloat3( 0.0f ), aeFloat3( 0.0f ) ) )
{}

float Sdf::GetValue( aeFloat3 p ) const
{
  p = ( GetRemoveTRMatrix() * aeFloat4( p, 1.0f ) ).GetXYZ();
  float f = GetValue( p, 0 );
  if ( topNoiseStrength || noiseStrength )
  {
    float n = topNoiseScale.x * topNoiseScale.y * topNoiseScale.z;
    n *= noiseScale.x * noiseScale.y * noiseScale.z;
    // Prevent scaling to 0 which causes invalid normals
    if ( n < -0.0001f || 0.0001f < n )
    {
      // Get base shape surface normal
      aeFloat3 normal0;
      for ( int32_t i = 0; i < 3; i++ )
      {
        aeFloat3 nt = p;
        nt[ i ] += 0.025f;
        normal0[ i ] = GetValue( nt, 0 );
      }
      normal0 -= aeFloat3( f );
      aeFloat3 normal1;
      for ( int32_t i = 0; i < 3; i++ )
      {
        aeFloat3 nt = p;
        nt[ i ] -= 0.025f;
        normal1[ i ] = GetValue( nt, 0 );
      }
      normal1 = aeFloat3( f ) - normal1;
      ae::Vec3 normal = ( normal1 + normal0 ).SafeNormalizeCopy();
      float vertical = ae::Max( 0.0f, normal.z );
      
      if ( vertical && topNoiseStrength )
      {
        float iqH = 1.0f;
        float iqG = exp2(-iqH);
        float iqf = 1.0f;
        float iqa = 1.0f;
        float iqt = 0.0f;
        for ( uint32_t i = 0; i < 3; i++ )
        {
          ae::Vec3 iqx = topNoiseOffset + p * TerrainNoiseScale / ( GetHalfSize() * topNoiseScale );
          iqt += iqa * noise->Get< aeMath::CosineInterpolate >( iqx * iqf );
          iqf *= 2.0f;
          iqa *= iqG;
        }
        f += iqt * topNoiseStrength * vertical;
      }
      
      if ( noiseStrength )
      {
        float iqH = 1.0f;
        float iqG = exp2(-iqH);
        float iqf = 1.0f;
        float iqa = 1.0f;
        float iqt = 0.0f;
        for ( uint32_t i = 0; i < 3; i++ )
        {
          ae::Vec3 iqx = noiseOffset + p * TerrainNoiseScale / ( GetHalfSize() * noiseScale );
          iqt += iqa * noise->Get< aeMath::CosineInterpolate >( iqx * iqf );
          iqf *= 2.0f;
          iqa *= iqG;
        }
        f += iqt * noiseStrength * ( 1.0f - vertical );
      }
    }
  }
  return f;
}

void Sdf::SetTransform( const aeFloat4x4& transform )
{
  if ( m_localToWorld == transform )
  {
    return;
  }

  m_localToWorld = transform;

  // World to local with scaling removed. Translation and rotation
  // are handled by the base Shape. Sdf functions only need to take
  // object scaling into consideration.
  aeFloat4x4 scaledToWorld = m_localToWorld;
  scaledToWorld.RemoveScaling();
  m_removeTR = scaledToWorld.Inverse();

  // Set scale of inherited Shape object
  m_halfSize = m_localToWorld.GetScale() * 0.5f;

  // Update shape world space AABB
  aeFloat4 corners[] =
  {
    m_localToWorld * aeFloat4( -0.5f, -0.5f, -0.5f, 1.0f ),
    m_localToWorld * aeFloat4( 0.5f, -0.5f, -0.5f, 1.0f ),
    m_localToWorld * aeFloat4( 0.5f, 0.5f, -0.5f, 1.0f ),
    m_localToWorld * aeFloat4( -0.5f, 0.5f, -0.5f, 1.0f ),
    m_localToWorld * aeFloat4( -0.5f, -0.5f, 0.5f, 1.0f ),
    m_localToWorld * aeFloat4( 0.5f, -0.5f, 0.5f, 1.0f ),
    m_localToWorld * aeFloat4( 0.5f, 0.5f, 0.5f, 1.0f ),
    m_localToWorld * aeFloat4( -0.5f, 0.5f, 0.5f, 1.0f ),
  };
  m_aabb = aeAABB( corners[ 0 ].GetXYZ(), corners[ 1 ].GetXYZ() );
  for ( uint32_t i = 2; i < countof( corners ); i++ )
  {
    m_aabb.Expand( corners[ i ].GetXYZ() );
  }
}

ae::Hash Sdf::GetBaseHash( ae::Hash hash ) const
{
  hash = hash.HashBasicType( type );
  hash = hash.HashBasicType( materialId );
  hash = hash.HashBasicType( smoothing );
  hash = hash.HashBasicType( order );
  
  hash = hash.HashBasicType( topNoiseStrength );
  hash = hash.HashBasicType( topNoiseOffset );
  hash = hash.HashBasicType( topNoiseScale );
  
  hash = hash.HashBasicType( noiseStrength );
  hash = hash.HashBasicType( noiseOffset );
  hash = hash.HashBasicType( noiseScale );
  
  hash = hash.HashBasicType( m_localToWorld );
  return hash;
}

//------------------------------------------------------------------------------
// SdfBox member functions
//------------------------------------------------------------------------------
Sdf* SdfBox::Clone() const
{
  SdfBox* box = ae::New< SdfBox >( AE_ALLOC_TAG_TERRAIN );
  *box = *this;
  return box;
}

ae::Hash SdfBox::Hash( ae::Hash hash ) const
{
  hash = GetBaseHash( hash );
  hash = hash.HashBasicType( cornerRadius );
  return hash;
}

float SdfBox::GetValue( aeFloat3 p, int ) const
{
  aeFloat3 q = aeMath::Abs( p ) - ( GetHalfSize() - aeFloat3( cornerRadius ) );
  return ( aeMath::Max( q, aeFloat3( 0.0f ) ) ).Length() + aeMath::Min( aeMath::Max( q.x, aeMath::Max( q.y, q.z ) ), 0.0f ) - cornerRadius;
}

//------------------------------------------------------------------------------
// SdfCylinder member functions
//------------------------------------------------------------------------------
Sdf* SdfCylinder::Clone() const
{
  SdfCylinder* cylinder = ae::New< SdfCylinder >( AE_ALLOC_TAG_TERRAIN );
  *cylinder = *this;
  return cylinder;
}

ae::Hash SdfCylinder::Hash( ae::Hash hash ) const
{
  hash = GetBaseHash( hash );
  hash = hash.HashBasicType( top );
  hash = hash.HashBasicType( bottom );
  return hash;
}

float SdfCylinder::GetValue( aeFloat3 p, int ) const
{
  aeFloat3 halfSize = GetHalfSize();
	
  float scale;
  if ( halfSize.x > halfSize.y )
  {
    scale = halfSize.x;
    p.y *= halfSize.x / halfSize.y;
  }
  else
  {
    scale = halfSize.y;
    p.x *= halfSize.y / halfSize.x;
  }

  float r1 = aeMath::Clip01( bottom ) * scale;
  float r2 = aeMath::Clip01( top ) * scale;
  float h = halfSize.z;

  aeFloat2 q( p.GetXY().Length(), p.z );
  aeFloat2 k1(r2,h);
  aeFloat2 k2(r2-r1,2.0*h);
  aeFloat2 ca(q.x-aeMath::Min(q.x,(q.y<0.0)?r1:r2), aeMath::Abs(q.y)-h);
  aeFloat2 cb = q - k1 + k2*aeMath::Clip01( (k1-q).Dot(k2)/k2.Dot(k2) );
  float s = (cb.x<0.0 && ca.y<0.0) ? -1.0 : 1.0;
  return s*sqrt( aeMath::Min(ca.Dot(ca),cb.Dot(cb)) );
}

//------------------------------------------------------------------------------
// SdfHeightmap member functions
//------------------------------------------------------------------------------
Sdf* SdfHeightmap::Clone() const
{
  SdfHeightmap* heightmap = ae::New< ae::SdfHeightmap >( AE_ALLOC_TAG_TERRAIN );
  *heightmap = *this;
  return heightmap;
}

ae::Hash SdfHeightmap::Hash( ae::Hash hash ) const
{
  return GetBaseHash( hash );
}

float SdfHeightmap::GetValue( aeFloat3 p, int ) const
{
  AE_ASSERT_MSG( m_heightMap, "Heightmap image not set" );

  aeFloat3 halfSize = GetHalfSize();

  aeFloat2 p2 = ( p.GetXY() + halfSize.GetXY() ) / ( halfSize.GetXY() * 2.0f );
  p2 *= aeFloat2( m_heightMap->GetWidth(), m_heightMap->GetHeight() );
  float v0 = m_heightMap->Get( p2, ae::Image::Interpolation::Cosine ).r;
  v0 = p.z + halfSize.z - v0 * halfSize.z * 2.0f;

  aeFloat3 q = aeMath::Abs( p ) - halfSize;
  float v1 = ( aeMath::Max( q, aeFloat3( 0.0f ) ) ).Length() + aeMath::Min( aeMath::Max( q.x, aeMath::Max( q.y, q.z ) ), 0.0f );

  return aeMath::Max( v0, v1 );
}

//------------------------------------------------------------------------------
// TerrainJob member functions
//------------------------------------------------------------------------------
float TerrainJob::GetValue( aeFloat3 pos ) const
{
  if ( !m_shapes.Length() )
  {
    return 0.0f;
  }
  AE_ASSERT( m_shapes[ 0 ]->IsSolid() );
  
  float f = m_shapes[ 0 ]->GetValue( pos ) - m_p.smoothingAmount;
  for ( uint32_t i = 1; i < m_shapes.Length(); i++ )
  {
    Sdf* shape = m_shapes[ i ];
    if ( shape->type == Sdf::Type::Material )
    {
      continue;
    }

    float value = shape->GetValue( pos ) - m_p.smoothingAmount;
#if _AE_DEBUG_
    AE_ASSERT_MSG( value == value, "SDF function returned NAN" );
#endif

    if ( shape->type == Sdf::Type::Union )
    {
      f = SdfUnion( value, f );
    }
    else if ( shape->type == Sdf::Type::Subtraction )
    {
      f = SdfSubtraction( value, f );
    }
    else if ( shape->type == Sdf::Type::SmoothUnion )
    {
      f = SdfSmoothUnion( value, f, shape->smoothing );
    }
    else if ( shape->type == Sdf::Type::SmoothSubtraction )
    {
      f = SdfSmoothSubtraction( value, f, shape->smoothing );
    }
  }

#if _AE_DEBUG_
  AE_ASSERT_MSG( f == f, "Terrain SDF function returned NAN" );
#endif
  return f - m_p.smoothingAmount; // @NOTE: This value isn't used directly, it populates TerrainSdfCache
}

//------------------------------------------------------------------------------
// TerrainSdf member functions
//------------------------------------------------------------------------------
TerrainSdf::TerrainSdf( Terrain* terrain ) :
  m_terrain( terrain )
{
  aeRandom r( -1.0f, 1.0f );
  ae::Scratch< aeStaticImage3D< float, TerrainNoiseSize, TerrainNoiseSize, TerrainNoiseSize > > tempScratch( AE_ALLOC_TAG_TERRAIN, 1 );
  auto& temp = *tempScratch.Data();
  for ( uint32_t z = 0; z < temp.GetDepth(); z++ )
  for ( uint32_t y = 0; y < temp.GetHeight(); y++ )
  for ( uint32_t x = 0; x < temp.GetWidth(); x++ )
  {
    temp.Set( aeInt3( x, y, z ), r.Get() );
  }
  
  for ( uint32_t z = 0; z < noise.GetDepth(); z++ )
  for ( uint32_t y = 0; y < noise.GetHeight(); y++ )
  for ( uint32_t x = 0; x < noise.GetWidth(); x++ )
  {
    noise.Set( aeInt3( x, y, z ), temp.Get< aeMath::CosineInterpolate >( aeFloat3( x, y, z ) / (float)TerrainNoiseScale ) );
  }
}

aeFloat3 TerrainJob::GetDerivative( aeFloat3 p ) const
{
  // https://iquilezles.org/www/articles/normalsSDF/normalsSDF.htm
  const float h = 0.0001f;
  const aeFloat3 xyy( 1.0f, -1.0f, -1.0f );
  const aeFloat3 yyx( -1.0f, -1.0f, 1.0f );
  const aeFloat3 yxy( -1.0f, 1.0f, -1.0f );
  const aeFloat3 xxx( 1.0f, 1.0f, 1.0f );
  const aeFloat3 n =
    xyy * GetValue( p + xyy * h ) +
    yyx * GetValue( p + yyx * h ) +
    yxy * GetValue( p + yxy * h ) +
    xxx * GetValue( p + xxx * h );
  return n.SafeNormalizeCopy();
}

TerrainMaterialId TerrainJob::GetMaterial( aeFloat3 pos, aeFloat3 normal ) const
{
  // Use the normal to nudge the material sample position to avoid aliasing
  TerrainMaterialId materialId = 0;
  for ( uint32_t i = 0; i < m_shapes.Length(); i++ )
  {
    Sdf* shape = m_shapes[ i ];
    // Nudge the sample position out of the surface for paint shapes
    if ( !shape->IsSolid() && shape->GetValue( pos + normal * 0.1f  ) - m_p.smoothingAmount <= 0.0f )
    {
      materialId = shape->materialId;
    }
    // Nudge the sample position into the surface for solid shapes
    else if ( shape->GetValue( pos - normal * 0.1f ) - m_p.smoothingAmount <= 0.0f )
    {
      materialId = shape->materialId;
    }
  }
  return materialId;
}

void TerrainSdf::DestroySdf( Sdf* sdf )
{
  if ( !sdf )
  {
    return;
  }

  m_pendingDestroy.Append( sdf );
}

void TerrainSdf::UpdatePending()
{
  // @NOTE: UpdatePending() is called when no terrain jobs are running,
  // so it's safe to modify the terrain shapes array

  // Old
  for ( uint32_t i = 0; i < m_pendingDestroy.Length(); i++ )
  {
    Sdf* shape = m_pendingDestroy[ i ];
    
    int32_t index = m_shapes.Find( shape );
    AE_ASSERT( index >= 0 );
    m_shapes.Remove( index );

    m_terrain->m_Dirty( shape->GetAABB() );
    ae::Delete( shape );
  }
  m_pendingDestroy.Clear();

  // New
  for ( uint32_t i = 0; i < m_pendingCreated.Length(); i++ )
  {
    Sdf* shape = m_pendingCreated[ i ];
    m_shapes.Append( shape );
    m_terrain->m_Dirty( shape->GetAABB() );

    shape->m_dirty = false;
    shape->m_aabbPrev = shape->GetAABB();
  }
  m_pendingCreated.Clear();
}

bool TerrainSdf::HasPending() const
{
  return m_pendingCreated.Length() || m_pendingDestroy.Length();
}

void TerrainSdf::RenderDebug( aeDebugRender* debug )
{
  for ( uint32_t i = 0; i < m_shapes.Length(); i++ )
  {
    aeAABB aabb = m_shapes[ i ]->GetAABB();
    aabb.Expand( kSdfBoundary );
    aeFloat3 center = ( aabb.GetMin() + aabb.GetMax() ) * 0.5f;
    aeFloat3 halfSize = aabb.GetMax() - center;
    debug->AddAABB( center, halfSize, aeColor::Red() );
  }

  for ( uint32_t i = 0; i < m_pendingCreated.Length(); i++ )
  {
    aeAABB aabb = m_pendingCreated[ i ]->GetAABB();
    aabb.Expand( kSdfBoundary );
    aeFloat3 center = ( aabb.GetMin() + aabb.GetMax() ) * 0.5f;
    aeFloat3 halfSize = aabb.GetMax() - center;
    debug->AddAABB( center, halfSize, aeColor::Blue() );
  }
}

} // AE_NAMESPACE
