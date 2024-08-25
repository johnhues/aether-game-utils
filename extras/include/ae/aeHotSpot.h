//------------------------------------------------------------------------------
// aeHotSpot.h
//------------------------------------------------------------------------------
// Copyright (c) 2024 John Hughes
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
#include "aether.h"
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
  uint32_t GetWidth() const { return m_width; }
  uint32_t GetHeight() const { return m_height; }

  // 1) Tiles - Setup tile properties and enable collision
  void SetTileProperties( uint32_t type, uint32_t mask );
  uint32_t GetTileProperties( uint32_t type ) const;
  void SetTileFluidDensity( uint32_t type, float density ); // water is 1000 kg/m^3
  float GetTileFluidDensity( uint32_t type ) const;
  void SetCollisionMask( uint32_t mask );
  uint32_t GetCollisionMask() const;

  // 2) Map - Populate map tiles
  void LoadTiles( const uint32_t* tiles, uint32_t width, uint32_t height, bool flipVertical );
  void SetTile( ae::Int2 pos, uint32_t type );
  uint32_t GetTile( ae::Int2 pos ) const;

  // 3) Objects - Create objects for simulation
  class HotSpotObject* CreateObject();
  class HotSpotObject* GetObject( uint32_t index );
  uint32_t GetObjectCount() const;

private:
  float m_timeStep = 0.0f;
  float m_timeAccumulator = 0.0f;
  uint32_t m_width = 0;
  uint32_t m_height = 0;

  aeSparseGrid< aeSparseGridZone< uint32_t, 16, 16, 1 > > m_tiles;

  ae::Map< uint32_t, uint32_t > m_tileProperties = AE_ALLOC_TAG_HOTSPOT;
  ae::Map< uint32_t, float > m_tileDensity = AE_ALLOC_TAG_HOTSPOT;
  uint32_t m_collisionMask = 0;

  ae::Array< class HotSpotObject* > m_objects = AE_ALLOC_TAG_HOTSPOT;

public:
  static ae::Int2 _GetTilePos( ae::Vec2 pos );
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

  void Warp( ae::Vec2 meters ); // Position in meters
  void SetVelocity( ae::Vec2 metersPerSecond ); // Distance traveled per second
  // 1 Newton: Force required to accelerate an object with a mass of 1 kg 1 m/s^2
  void AddForce( ae::Vec2 newtons ); // Apply a regular force to the object (scaled by frame time)
  void AddImpulse( ae::Vec2 newtons ); // Apply a force and change velocity directly (without respect to frame time)
  void AddGravity( ae::Vec2 acceleration ); // Gravitational acceleration, call each frame

  ae::Vec2 GetPosition() const;
  ae::Vec2 GetVelocity() const;
  bool IsOnGround() const;

  struct CollisionInfo
  {
    ae::Int2 position;
    ae::Int2 normal;
    uint32_t tile;
    uint32_t properties;
  };
  aeSignalList< const CollisionInfo* > onCollision;

private:
  friend HotSpotWorld;
  void Update( HotSpotWorld* world, float dt );
  bool m_CheckCollision( const HotSpotWorld* world, ae::Int2 dir );
  bool m_TestSide( const HotSpotWorld* world, ae::Vec2 p0, ae::Vec2 p1, ae::Vec2* pOut );

  ae::Vec2 m_position = ae::Vec2( 0.0f );
  ae::Vec2 m_velocity = ae::Vec2( 0.0f );
  ae::Vec2 m_forces = ae::Vec2( 0.0f );
  ae::Vec2 m_gravity = ae::Vec2( 0.0f );
  float m_mass = 1.0f;
  float m_groundRestitution = 0.0f;
  float m_wallRestitution = 0.3f;
  float m_volume = 1.0f;
  float m_airTimer = 0.0f;
};

#endif
