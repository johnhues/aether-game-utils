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
float Box( aeFloat3 p, aeFloat3 b )
{
  aeFloat3 d;
  d.x = aeMath::Abs( p.x ) - b.x;
  d.y = aeMath::Abs( p.y ) - b.y;
  d.z = aeMath::Abs( p.z ) - b.z;
  aeFloat3 d0;
  d0.x = aeMath::Max( d.x, 0.0f );
  d0.y = aeMath::Max( d.y, 0.0f );
  d0.z = aeMath::Max( d.z, 0.0f );
  float f = aeFloat3( d0 ).Length();
  return f + fmin( fmax( d.x, fmax( d.y, d.z ) ), 0.0f );
}

float Cylinder( aeFloat3 p, aeFloat2 h )
{
  aeFloat2 d;
  d.x = aeFloat2( p.x, p.y ).Length();
  d.y = p.z;
  d.x = fabs( d.x );
  d.y = fabs( d.y );
  d -= h;
  aeFloat2 d0( fmax( d.x, 0.0f ), fmax( d.y, 0.0f ) );
  return fmin( fmax( d.x, d.y ), 0.0f ) + d0.Length();
}

float Sphere( aeFloat3 center, float radius, aeFloat3 p )
{
  return ( p - center ).Length() - radius;
}

float Ground( float height, aeFloat3 p )
{
  return p.z - height;
}

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
  m_worldToScaled( aeFloat4x4::Identity() )
{}

void ae::Sdf::Shape::SetTransform( const aeFloat4x4& transform )
{
  m_localToWorld = transform;

  aeFloat4x4 scaledToWorld = transform;
  scaledToWorld.RemoveScaling();
  m_worldToScaled = scaledToWorld.Inverse();

  aeAABB scaledAABB = OnSetTransform( transform.GetScale() );
  aeFloat4x4 localToAABB = scaledAABB.GetTransform();
  aeFloat4x4 aabbToWorld = scaledToWorld * localToAABB;

  aeFloat4 corners[] =
  {
    aabbToWorld * aeFloat4( -0.5f, -0.5f, -0.5f, 1.0f ),
    aabbToWorld * aeFloat4( 0.5f, -0.5f, -0.5f, 1.0f ),
    aabbToWorld * aeFloat4( 0.5f, 0.5f, -0.5f, 1.0f ),
    aabbToWorld * aeFloat4( -0.5f, 0.5f, -0.5f, 1.0f ),
    aabbToWorld * aeFloat4( -0.5f, -0.5f, 0.5f, 1.0f ),
    aabbToWorld * aeFloat4( 0.5f, -0.5f, 0.5f, 1.0f ),
    aabbToWorld * aeFloat4( 0.5f, 0.5f, 0.5f, 1.0f ),
    aabbToWorld * aeFloat4( -0.5f, 0.5f, 0.5f, 1.0f ),
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
  p = ( GetWorldToScaled() * aeFloat4( p, 1.0f ) ).GetXYZ();

  aeFloat3 q = aeMath::Abs( p ) - ( m_halfSize - aeFloat3( m_r ) );
  return ( aeMath::Max( q, aeFloat3( 0.0f ) ) ).Length() + aeMath::Min( aeMath::Max( q.x, aeMath::Max( q.y, q.z ) ), 0.0f ) - m_r;
}

aeAABB ae::Sdf::Box::OnSetTransform( aeFloat3 scale )
{
  m_halfSize = scale * 0.5f;
  return aeAABB( -m_halfSize, m_halfSize );
}

//------------------------------------------------------------------------------
// Heightmap member functions
//------------------------------------------------------------------------------
float ae::Sdf::Heightmap::GetValue( aeFloat3 p ) const
{
  AE_ASSERT_MSG( m_heightMap, "Heightmap image not set" );

  p = ( GetWorldToScaled() * aeFloat4( p, 1.0f ) ).GetXYZ();

  aeFloat2 p2 = ( p.GetXY() + m_halfSize.GetXY() ) / ( m_halfSize.GetXY() * 2.0f );
  p2 *= aeFloat2( m_heightMap->GetWidth(), m_heightMap->GetHeight() );
  float v0 = m_heightMap->Get( p2, ae::Image::Interpolation::Cosine ).r;
  v0 = p.z + m_halfSize.z - v0 * m_halfSize.z * 2.0f;

  aeFloat3 q = aeMath::Abs( p ) - m_halfSize;
  float v1 = ( aeMath::Max( q, aeFloat3( 0.0f ) ) ).Length() + aeMath::Min( aeMath::Max( q.x, aeMath::Max( q.y, q.z ) ), 0.0f );

  return aeMath::Max( v0, v1 );

}

aeAABB ae::Sdf::Heightmap::OnSetTransform( aeFloat3 scale )
{
  m_halfSize = scale * 0.5f;
  return aeAABB( -m_halfSize, m_halfSize );
}

//------------------------------------------------------------------------------
// aeTerrainSDF member functions
//------------------------------------------------------------------------------
float aeTerrainSDF::GetValue( aeFloat3 pos ) const
{
  float f = 0.0f;
  if ( m_fn2 )
  {
    f = m_fn2( m_userdata, pos );
  }
  else if ( m_fn1 )
  {
    f = m_fn1( pos );
  }
  else
  {
    if ( !m_shapes.Length() )
    {
      return 0.0f;
    }

    int32_t firstShapeIndex = m_shapes.FindFn( []( const ae::Sdf::Shape* sdf ){ return sdf->type != ae::Sdf::Shape::Type::Material; } );
    if ( firstShapeIndex >= 0 )
    {
      f = m_shapes[ firstShapeIndex ]->GetValue( pos );
      for ( uint32_t i = firstShapeIndex + 1; i < m_shapes.Length(); i++ )
      {
        if ( m_shapes[ i ]->type == ae::Sdf::Shape::Type::Union )
        {
          float value = m_shapes[ i ]->GetValue( pos );
#if _AE_DEBUG_
          AE_ASSERT_MSG( value == value, "SDF function returned NAN" );
#endif
          f = aeUnion( value, f );
        }
        else if ( m_shapes[ i ]->type == ae::Sdf::Shape::Type::Subtraction )
        {
          float value = m_shapes[ i ]->GetValue( pos );
#if _AE_DEBUG_
          AE_ASSERT_MSG( value == value, "SDF function returned NAN" );
#endif
          f = aeSubtraction( value, f );
        }
      }
    }
  }

#if _AE_DEBUG_
  AE_ASSERT_MSG( f == f, "Terrain SDF function returned NAN" );
#endif
  return f;
}

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
  aeTerrainMaterialId materialId = 0;
  for ( uint32_t i = 0; i < m_shapes.Length(); i++ )
  {
    ae::Sdf::Shape* sdf = m_shapes[ i ];
    if ( sdf->type == ae::Sdf::Shape::Type::Material && sdf->GetValue( pos ) <= 0.0f )
    {
      materialId = sdf->materialId;
    }
  }
  return materialId;
}

bool aeTerrainSDF::TestAABB( aeAABB aabb ) const
{
  if ( m_fn1 || m_fn2 )
  {
    return true;
  }
  
  for ( uint32_t i = 0; i < m_shapes.Length(); i++ )
  {
    aeAABB shapeAABB = m_shapes[ i ]->GetAABB();
    if ( shapeAABB.Intersect( aabb ) )
    {
      return true;
    }
  }

  return false;
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
  for ( uint32_t i = 0; i < m_pendingCreated.Length(); i++ )
  {
    m_shapes.Append( m_pendingCreated[ i ] );
  }
  m_pendingCreated.Clear();

  for ( uint32_t i = 0; i < m_pendingDestroy.Length(); i++ )
  {
    aeAlloc::Release( m_pendingDestroy[ i ] );
  }
  m_pendingDestroy.Clear();
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
