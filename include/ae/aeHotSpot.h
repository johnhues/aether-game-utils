//------------------------------------------------------------------------------
// aeHotSpot.h
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
#ifndef AE_HOTSPOT
#define AE_HOTSPOT

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeMath.h"
#include "aeSignal.h"
#include "aeSparseGrid.h"

//------------------------------------------------------------------------------
// HotSpotWorld class
//------------------------------------------------------------------------------
class HotSpotWorld
{
public:
  void Initialize( float timeStep );
  void Update( float dt );

  // 1) Tiles - Setup tile properties and enable collision
  void SetTileProperties( uint32_t type, uint32_t mask );
  uint32_t GetTileProperties( uint32_t type ) const;
  void SetTileFluidDensity( uint32_t type, float density ); // water is 1000 kg/m^3
  float GetTileFluidDensity( uint32_t type ) const;
  void SetCollisionMask( uint32_t mask );
  uint32_t GetCollisionMask() const;

  // 2) Map - Populate map tiles
  void LoadTiles( const uint32_t* tiles, uint32_t width, uint32_t height, bool flipVertical );
  void SetTile( aeInt2 pos, uint32_t type );
  uint32_t GetTile( aeInt2 pos ) const;

  // 3) Objects - Create objects for simulation
  class HotSpotObject* CreateObject();
  class HotSpotObject* GetObject( uint32_t index );
  uint32_t GetObjectCount() const;

private:
  float m_timeStep = 0.0f;
  float m_timeAccumulator = 0.0f;

  aeSparseGrid< aeSparseGridZone< uint32_t, 16, 16, 1 > > m_tiles;

  aeMap< uint32_t, uint32_t > m_tileProperties;
  aeMap< uint32_t, float > m_tileDensity;
  uint32_t m_collisionMask = 0;

  aeArray< class HotSpotObject* > m_objects;

public:
  static aeInt2 _GetTilePos( aeFloat2 pos );
};

//------------------------------------------------------------------------------
// HotSpotObject class
//------------------------------------------------------------------------------
class HotSpotObject
{
public:
  void SetMass( float kilograms ); // Average human: 155lb ~= 70kg
  void SetRestitution( float groundPercent, float wallPercent ); // Percent of velocity kept on collision
  void SetVolume( float meters ); // meters^3: A humans volume is roughly their weight in kg/1050 (slightly denser than water). Volume is required for correct buoyancy behavior

  float GetMass() const;

  void Warp( aeFloat2 meters ); // Position in meters
  void SetVelocity( aeFloat2 metersPerSecond ); // Distance traveled per second
  // 1 Newton: Force required to accelerate an object with a mass of 1 kg 1 m/s^2
  void AddForce( aeFloat2 newtons ); // Apply a regular force to the object (scaled by frame time)
  void AddImpulse( aeFloat2 newtons ); // Apply a force and change velocity directly (without respect to frame time)
  void AddGravity( aeFloat2 acceleration ); // Gravitational acceleration, call each frame

  aeFloat2 GetPosition() const;
  aeFloat2 GetVelocity() const;
  bool IsOnGround() const;

  struct CollisionInfo
  {
    aeInt2 position;
    aeInt2 normal;
    uint32_t tile;
    uint32_t properties;
  };
  aeSignalList< const CollisionInfo* > onCollision;

private:
  friend HotSpotWorld;
  void Update( HotSpotWorld* world, float dt );
  bool m_CheckCollision( const HotSpotWorld* world, aeInt2 dir );
  bool m_TestSide( const HotSpotWorld* world, aeFloat2 p0, aeFloat2 p1, aeFloat2* pOut );

  aeFloat2 m_position = aeFloat2( 0.0f );
  aeFloat2 m_velocity = aeFloat2( 0.0f );
  aeFloat2 m_forces = aeFloat2( 0.0f );
  aeFloat2 m_gravity = aeFloat2( 0.0f );
  float m_mass = 1.0f;
  float m_groundRestitution = 0.0f;
  float m_wallRestitution = 0.3f;
  float m_volume = 1.0f;
  float m_airTimer = 0.0f;
};

#endif
