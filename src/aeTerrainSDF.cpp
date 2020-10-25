//------------------------------------------------------------------------------
// aeTerrainSDF.cpp
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

//------------------------------------------------------------------------------
// Sdf helpers
//------------------------------------------------------------------------------
float aeUnion( float d1, float d2 )
{
  return aeMath::Min( d1, d2 );
}

float aeSubtraction( float d1, float d2 )
{
  return aeMath::Max( -d1, d2 );
}

float aeIntersection( float d1, float d2 )
{
  return aeMath::Max( d1, d2 );
}

float aeSmoothUnion( float d1, float d2, float k )
{
  float h = aeMath::Clip01( 0.5f + 0.5f * ( d2 - d1 ) / k );
  return aeMath::Lerp( d2, d1, h ) - k * h * ( 1.0f - h );
}

float aeSmoothSubtraction( float d1, float d2, float k )
{
  float h = aeMath::Clip01( 0.5f - 0.5f * ( d2 + d1 ) / k );
  return aeMath::Lerp( d2, -d1, h ) + k * h * ( 1.0f - h );
}

//------------------------------------------------------------------------------
// Sdf member functions
//------------------------------------------------------------------------------
ae::Sdf::Shape::Shape() :
  m_aabb( aeAABB( aeFloat3( 0.0f ), aeFloat3( 0.0f ) ) ),
  m_localToWorld( aeFloat4x4::Identity() ),
  m_worldToLocal( aeFloat4x4::Identity() ),
  m_aabbPrev( aeAABB( aeFloat3( 0.0f ), aeFloat3( 0.0f ) ) )
{}

void ae::Sdf::Shape::SetTransform( const aeFloat4x4& transform )
{
  if ( m_localToWorld == transform )
  {
    return;
  }

  m_localToWorld = transform;
  m_worldToLocal = transform.Inverse();

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

//------------------------------------------------------------------------------
// Box member functions
//------------------------------------------------------------------------------
float ae::Sdf::Box::GetValue( aeFloat3 p ) const
{
  p = ( GetInverseTransform() * aeFloat4( p, 1.0f ) ).GetXYZ();

  aeFloat3 m_halfSize( 0.5f );
  aeFloat3 q = aeMath::Abs( p ) - ( m_halfSize - aeFloat3( cornerRadius ) );
  return ( aeMath::Max( q, aeFloat3( 0.0f ) ) ).Length() + aeMath::Min( aeMath::Max( q.x, aeMath::Max( q.y, q.z ) ), 0.0f ) - cornerRadius;
}

//------------------------------------------------------------------------------
// Cylinder member functions
//------------------------------------------------------------------------------
float ae::Sdf::Cylinder::GetValue( aeFloat3 p ) const
{
  p = ( GetInverseTransform() * aeFloat4( p, 1.0f ) ).GetXYZ();
	
  float scale;
  if ( 0.5f > 0.5f )
  {
    scale = 0.5f;
    p.y *= 0.5f / 0.5f;
  }
  else
  {
    scale = 0.5f;
    p.x *= 0.5f / 0.5f;
  }

  float r1 = aeMath::Clip01( bottom ) * scale;
  float r2 = aeMath::Clip01( top ) * scale;
  float h = 0.5f;

  aeFloat2 q( p.GetXY().Length(), p.z );
  aeFloat2 k1(r2,h);
  aeFloat2 k2(r2-r1,2.0*h);
  aeFloat2 ca(q.x-aeMath::Min(q.x,(q.y<0.0)?r1:r2), aeMath::Abs(q.y)-h);
  aeFloat2 cb = q - k1 + k2*aeMath::Clip01( (k1-q).Dot(k2)/k2.Dot(k2) );
  float s = (cb.x<0.0 && ca.y<0.0) ? -1.0 : 1.0;
  return s*sqrt( aeMath::Min(ca.Dot(ca),cb.Dot(cb)) );
}

//------------------------------------------------------------------------------
// Heightmap member functions
//------------------------------------------------------------------------------
float ae::Sdf::Heightmap::GetValue( aeFloat3 p ) const
{
  AE_ASSERT_MSG( m_heightMap, "Heightmap image not set" );

  p = ( GetInverseTransform() * aeFloat4( p, 1.0f ) ).GetXYZ();

  aeFloat2 p2 = ( p.GetXY() + aeFloat2( 0.5f, 0.5f ) ) / ( aeFloat2( 0.5f, 0.5f ) * 2.0f );
  p2 *= aeFloat2( m_heightMap->GetWidth(), m_heightMap->GetHeight() );
  float v0 = m_heightMap->Get( p2, ae::Image::Interpolation::Cosine ).r;
  v0 = p.z + 0.5f - v0 * 0.5f * 2.0f;

  aeFloat3 q = aeMath::Abs( p ) - aeFloat3( 0.5f );
  float v1 = ( aeMath::Max( q, aeFloat3( 0.0f ) ) ).Length() + aeMath::Min( aeMath::Max( q.x, aeMath::Max( q.y, q.z ) ), 0.0f );

  return aeMath::Max( v0, v1 );

}

//------------------------------------------------------------------------------
// aeTerrainSDF member functions
//------------------------------------------------------------------------------
float aeTerrainSDF::GetValue( aeFloat3 pos ) const
{
  float f = 0.0f;
  if ( !m_shapes.Length() )
  {
    return f;
  }

  int32_t firstShapeIndex = m_shapes.FindFn( []( const ae::Sdf::Shape* sdf ){ return sdf->type != ae::Sdf::Shape::Type::Material; } );
  if ( firstShapeIndex >= 0 )
  {
    f = m_shapes[ firstShapeIndex ]->GetValue( pos );
    for ( uint32_t i = firstShapeIndex + 1; i < m_shapes.Length(); i++ )
    {
      ae::Sdf::Shape* shape = m_shapes[ i ];
      if ( shape->type == ae::Sdf::Shape::Type::Material )
      {
        continue;
      }

      float value = shape->GetValue( pos );
#if _AE_DEBUG_
      AE_ASSERT_MSG( value == value, "SDF function returned NAN" );
#endif

      if ( shape->type == ae::Sdf::Shape::Type::Union )
      {
        f = aeUnion( value, f );
      }
      else if ( shape->type == ae::Sdf::Shape::Type::Subtraction )
      {
        f = aeSubtraction( value, f );
      }
      else if ( shape->type == ae::Sdf::Shape::Type::SmoothUnion )
      {
        f = aeSmoothUnion( value, f, shape->smoothing );
      }
      else if ( shape->type == ae::Sdf::Shape::Type::SmoothSubtraction )
      {
        f = aeSmoothSubtraction( value, f, shape->smoothing );
      }
    }
  }

#if _AE_DEBUG_
  AE_ASSERT_MSG( f == f, "Terrain SDF function returned NAN" );
#endif
  return f;
}

//------------------------------------------------------------------------------
// aeTerrainSDF member functions
//------------------------------------------------------------------------------
aeTerrainSDF::aeTerrainSDF( aeTerrain* terrain ) :
  m_terrain( terrain )
{}

aeFloat3 aeTerrainSDF::GetDerivative( aeFloat3 p ) const
{
  aeFloat3 normal0;
  for ( int32_t i = 0; i < 3; i++ )
  {
    aeFloat3 nt = p;
    nt[ i ] += 0.2f;
    normal0[ i ] = GetValue( nt );
  }
  // This should be really close to 0 because it's really
  // close to the surface but not close enough to ignore.
  normal0 -= aeFloat3( GetValue( p ) );
  normal0.SafeNormalize();
  AE_ASSERT( normal0 != aeFloat3( 0.0f ) );
  AE_ASSERT( normal0 == normal0 );

  aeFloat3 normal1;
  for ( int32_t i = 0; i < 3; i++ )
  {
    aeFloat3 nt = p;
    nt[ i ] -= 0.2f;
    normal1[ i ] = GetValue( nt );
  }
  // This should be really close to 0 because it's really
  // close to the surface but not close enough to ignore.
  normal1 = aeFloat3( GetValue( p ) ) - normal1;
  normal1.SafeNormalize();
  AE_ASSERT( normal1 != aeFloat3( 0.0f ) );
  AE_ASSERT( normal1 == normal1 );

  return ( normal1 + normal0 ).SafeNormalizeCopy();
}

aeTerrainMaterialId aeTerrainSDF::GetMaterial( aeFloat3 pos ) const
{
  uint32_t i = 0;
  aeTerrainMaterialId materialId = 0;
  
//  for ( ; i < m_shapes.Length(); i++ )
//  {
//    ae::Sdf::Shape* sdf = m_shapes[ i ];
//    if ( sdf->type != ae::Sdf::Shape::Type::Material && sdf->GetValue( pos ) <= 0.0f )
//    {
//      materialId = sdf->materialId;
//    }
//  }
  
  for ( ; i < m_shapes.Length(); i++ )
  {
    ae::Sdf::Shape* sdf = m_shapes[ i ];
    if ( sdf->type == ae::Sdf::Shape::Type::Material && sdf->GetValue( pos ) <= 0.0f )
    {
      materialId = sdf->materialId;
    }
  }
  
  return materialId;
}

void aeTerrainSDF::DestroySdf( ae::Sdf::Shape* sdf )
{
  if ( !sdf )
  {
    return;
  }

  m_pendingDestroy.Append( sdf );
}

void aeTerrainSDF::UpdatePending()
{
  // @NOTE: UpdatePending() is called when no terrain jobs are running,
  // so it's safe to modify the terrain shapes array

  // Old
  for ( uint32_t i = 0; i < m_pendingDestroy.Length(); i++ )
  {
    ae::Sdf::Shape* shape = m_pendingDestroy[ i ];
    
    int32_t index = m_shapes.Find( shape );
    AE_ASSERT( index >= 0 );
    m_shapes.Remove( index );

    m_terrain->m_Dirty( shape->GetAABB() );
    aeAlloc::Release( shape );
  }
  m_pendingDestroy.Clear();

  // New
  for ( uint32_t i = 0; i < m_pendingCreated.Length(); i++ )
  {
    ae::Sdf::Shape* shape = m_pendingCreated[ i ];
    m_shapes.Append( shape );
    m_terrain->m_Dirty( shape->GetAABB() );

    shape->m_dirty = false;
    shape->m_aabbPrev = shape->GetAABB();
  }
  m_pendingCreated.Clear();
}

bool aeTerrainSDF::HasPending() const
{
  return m_pendingCreated.Length() || m_pendingDestroy.Length();
}

void aeTerrainSDF::RenderDebug( aeDebugRender* debug )
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
