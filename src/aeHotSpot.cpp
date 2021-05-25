//------------------------------------------------------------------------------
// aeHotSpot.cpp
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
#include "aeHotSpot.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const float kGroundDistanceEpsilon = 0.01f;

//------------------------------------------------------------------------------
// HotSpotWorld member functions
//------------------------------------------------------------------------------
void HotSpotWorld::Initialize( float timeStep )
{
  AE_ASSERT( timeStep > 0.0f );
  m_timeStep = timeStep;
  m_timeAccumulator = 0.0f;
}

void HotSpotWorld::Update( float dt )
{
  m_timeAccumulator += dt;

  while ( m_timeAccumulator >= m_timeStep )
  {
    for ( uint32_t i = 0; i < m_objects.Length(); i++ )
    {
      HotSpotObject* object = m_objects[ i ];
      object->Update( this, m_timeStep );
    }

    m_timeAccumulator -= m_timeStep;
  }

  for ( uint32_t i = 0; i < m_objects.Length(); i++ )
  {
    // @NOTE: Reset user forces for next frame after all simulation steps
    HotSpotObject* object = m_objects[ i ];
    object->m_forces = aeFloat2( 0.0f );
    object->m_gravity = aeFloat2( 0.0f );
  }
}

void HotSpotWorld::SetTileProperties( uint32_t type, uint32_t mask )
{
  m_tileProperties.Set( type, mask );
}

uint32_t HotSpotWorld::GetTileProperties( uint32_t type ) const
{
  return m_tileProperties.Get( type, 0 );
}

void HotSpotWorld::SetTileFluidDensity( uint32_t type, float density )
{
  m_tileDensity.Set( type, density );
}

float HotSpotWorld::GetTileFluidDensity( uint32_t type ) const
{
  return m_tileDensity.Get( type, 0.0f );
}

void HotSpotWorld::SetCollisionMask( uint32_t mask )
{
  m_collisionMask = mask;
}

uint32_t HotSpotWorld::GetCollisionMask() const
{
  return m_collisionMask;
}

void HotSpotWorld::LoadTiles( const uint32_t* tiles, uint32_t width, uint32_t height, bool flipVertical )
{
  for ( uint32_t y = 0; y < height; y++ )
  {
    for ( uint32_t x = 0; x < width; x++ )
    {
      uint32_t y1 = flipVertical ? height - 1 - y : y;
      m_tiles.Set( aeInt3( x, y1, 0 ), tiles[ y * width + x ] );
    }
  }
}

void HotSpotWorld::SetTile( aeInt2 pos, uint32_t type )
{
  m_tiles.Set( aeInt3( pos, 0 ), type );
}

uint32_t HotSpotWorld::GetTile( aeInt2 pos ) const
{
  const uint32_t* type = m_tiles.TryGet( aeInt3( pos, 0 ) );
  return type ? *type : 0;
}

HotSpotObject* HotSpotWorld::CreateObject()
{
  HotSpotObject* obj = ae::Allocate< HotSpotObject >();
  m_objects.Append( obj );
  return obj;
}

HotSpotObject* HotSpotWorld::GetObject( uint32_t index )
{
  return m_objects[ index ];
}

uint32_t HotSpotWorld::GetObjectCount() const
{
  return m_objects.Length();
}

aeInt2 HotSpotWorld::_GetTilePos( aeFloat2 pos )
{
  return aeInt2( aeMath::Round( pos.x ), aeMath::Round( pos.y ) );
}

//------------------------------------------------------------------------------
// HotSpotObject member functions
//------------------------------------------------------------------------------
void HotSpotObject::SetMass( float kilograms )
{
  m_mass = kilograms;
}

void HotSpotObject::SetRestitution( float groundPercent, float wallPercent )
{
  m_groundRestitution = groundPercent;
  m_wallRestitution = wallPercent;
}

void HotSpotObject::SetVolume( float meters )
{
  m_volume = meters;
}

float HotSpotObject::GetMass() const
{
  return m_mass;
}

void HotSpotObject::Warp( aeFloat2 meters )
{
  m_position = meters;
}

void HotSpotObject::SetVelocity( aeFloat2 metersPerSecond )
{
  m_velocity = metersPerSecond;
}

void HotSpotObject::AddForce( aeFloat2 newtons )
{
  m_forces += newtons;
}

void HotSpotObject::AddImpulse( aeFloat2 newtons )
{
  m_velocity += newtons / m_mass; // @TODO: Do these units match up?
}

void HotSpotObject::AddGravity( aeFloat2 acceleration )
{
  m_gravity += acceleration;
  m_forces += acceleration * m_mass;
}

aeFloat2 HotSpotObject::GetPosition() const
{
  return m_position;
}

aeFloat2 HotSpotObject::GetVelocity() const
{
  return m_velocity;
}

bool HotSpotObject::IsOnGround() const
{
  return m_airTimer < 0.1f;
}

void HotSpotObject::Update( HotSpotWorld* world, float dt )
{
  // @NOTE: Apply derived forces (drag etc) to a temporary value to
  //        allow multiple simulation steps per frame.
  aeFloat2 forces = m_forces;

  // @HACK: Shouldn't assume side-on platformer with gravity pointing -y
  {
    aeInt2 tilePos;
    uint32_t tileType;
    uint32_t tileProperties;

    tilePos = HotSpotWorld::_GetTilePos( aeFloat2( m_position.x, m_position.y - 0.5f - kGroundDistanceEpsilon ) );
    tileType = world->GetTile( tilePos );
    tileProperties = world->GetTileProperties( tileType );
    if ( tileProperties & world->GetCollisionMask() )
    {
      m_airTimer = 0.0f;
    }
    else
    {
      m_airTimer += dt;
    }

    const float kFrictionCoefficient = 0.2f;
    if ( IsOnGround() && forces.y < 0.0f && kFrictionCoefficient > 0.0f )
    {
      float friction = -forces.y * kFrictionCoefficient;
      AE_ASSERT( friction >= 0.0f );
      friction *= aeMath::Delerp01( 0.0f, 0.1f, aeMath::Abs( m_velocity.x ) );
      if ( m_velocity.x > 0.0f )
      {
        forces.x -= friction;
      }
      else if ( m_velocity.x < 0.0f )
      {
        forces.x += friction;
      }
    }
  }

  // Drag: Fd = Surface Coeffiecient * Area * Density * V^2 * 0.5
  aeInt2 tilePos = HotSpotWorld::_GetTilePos( m_position );
  uint32_t tileType = world->GetTile( tilePos );
  float density = world->GetTileFluidDensity( tileType );
  if ( density > 0.0f )
  {
    // Assume surface coefficient and area of 1.0
    float speed2 = m_velocity.LengthSquared();
    aeFloat2 velDir = m_velocity.SafeNormalizeCopy();
    forces -= velDir * ( speed2 * density * 0.5f );
  }

  {
    ae::Map< aeInt2, int32_t, 5 > intersections;
    intersections.Set( HotSpotWorld::_GetTilePos( m_position ), 1 );
    intersections.Set( HotSpotWorld::_GetTilePos( m_position + aeFloat2( -0.5f ) ), 1 );
    intersections.Set( HotSpotWorld::_GetTilePos( m_position + aeFloat2( 0.5f ) ), 1 );
    intersections.Set( HotSpotWorld::_GetTilePos( m_position + aeFloat2( -0.5f, 0.5f ) ), 1 );
    intersections.Set( HotSpotWorld::_GetTilePos( m_position + aeFloat2( 0.5f, -0.5f ) ), 1 );
    aeRect objRect( m_position.x - 0.5f, m_position.y - 0.5f, 1.0f, 1.0f );
    for ( uint32_t i = 0; i < intersections.Length(); i++ )
    {
      aeInt2 tilePos = intersections.GetKey( i );
      uint32_t tileType = world->GetTile( tilePos );
      float density = world->GetTileFluidDensity( tileType );
      if ( density > 0.0f )
      {
        aeRect intersection;
        aeRect tileRect( tilePos.x - 0.5f, tilePos.y - 0.5f, 1.0f, 1.0f );
        if ( tileRect.GetIntersection( objRect, &intersection ) )
        {
          // Buoyant force = (density of liquid(kg/m3))*(gravitational acceleration(m/s2))*(volume of liquid(m3))
          float displaced = intersection.w * intersection.h * m_volume;
          forces -= m_gravity * ( density * displaced );
        }
      }
    }
  }

  // F = ma
  aeFloat2 acceleration = forces / m_mass;
  m_velocity += acceleration * dt;
  m_position += m_velocity * dt;

  // @HACK: Shouldn't assume side-on platformer with gravity pointing -y
  if ( m_airTimer == 0.0f && m_velocity.y > 0.0f )
  {
    m_airTimer += dt;
  }

  if ( !m_CheckCollision( world, aeInt2( 0, -1 ) ) )
  {
    m_CheckCollision( world, aeInt2( 0, 1 ) );
  }

  if ( !m_CheckCollision( world, aeInt2( -1, 0 ) ) )
  {
    m_CheckCollision( world, aeInt2( 1, 0 ) );
  }
}

bool HotSpotObject::m_CheckCollision( const HotSpotWorld* world, aeInt2 dir )
{
  AE_ASSERT( dir.x + dir.y == -1 || dir.x + dir.y == 1 );

  // @HACK: Shouldn't assume side-on platformer with gravity pointing -y
  float hotSpotOffset = dir.y ? 0.4f : 0.3f;

  aeFloat2 collisionPos;
  aeFloat2 b0 = m_position + ( dir * 0.5f ) + ( aeInt2( -dir.y, dir.x ) * hotSpotOffset );
  aeFloat2 b1 = m_position + ( dir * 0.5f ) + ( aeInt2( dir.y, -dir.x ) * hotSpotOffset );

  if ( dir.x )
  {
    // @HACK: Shouldn't assume side-on platformer with gravity pointing -y
    // Slide side hot spots up slightly so it's easier to get on ledges
    b0.y += 0.05f;
    b1.y += 0.05f;
  }

  if ( m_TestSide( world, b0, b1, &collisionPos ) )
  {
    aeInt2 tilePos = HotSpotWorld::_GetTilePos( collisionPos );

    if ( dir.y > 0 )
    {
      m_position.y = tilePos.y - dir.y;
      // @HACK: Shouldn't assume side-on platformer with gravity pointing -y
      m_velocity.y *= -m_groundRestitution;
    }
    else if ( dir.y < 0 )
    {
      m_position.y = tilePos.y - dir.y;
      m_velocity.y *= -m_wallRestitution;
    }
    else
    {
      m_position.x = tilePos.x - dir.x;
      m_velocity.x *= -m_wallRestitution;
    }


    CollisionInfo info;
    info.position = tilePos;
    info.normal = -dir;
    info.tile = world->GetTile( tilePos );
    info.properties = world->GetTileProperties( info.tile );
    AE_ASSERT( info.properties & world->GetCollisionMask() );
    onCollision.Send( &info );

    return true;
  }

  return false;
}

bool HotSpotObject::m_TestSide( const HotSpotWorld* world, aeFloat2 p0, aeFloat2 p1, aeFloat2* pOut )
{
  aeInt2 tilePos = HotSpotWorld::_GetTilePos( p0 );
  uint32_t tileType = world->GetTile( tilePos );
  uint32_t tileProperties = world->GetTileProperties( tileType );
  bool c0 = ( tileProperties & world->GetCollisionMask() );

  tilePos = HotSpotWorld::_GetTilePos( p1 );
  tileType = world->GetTile( tilePos );
  tileProperties = world->GetTileProperties( tileType );
  bool c1 = ( tileProperties & world->GetCollisionMask() );

  if ( c0 && c1 )
  {
    *pOut = ( p0 + p1 ) * 0.5f;
    return true;
  }
  else if ( c0 )
  {
    *pOut = p0;
    return true;
  }
  else if ( c1 )
  {
    *pOut = p1;
    return true;
  }
  else
  {
    return false;
  }
}
