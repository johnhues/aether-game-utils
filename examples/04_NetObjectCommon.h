//------------------------------------------------------------------------------
// 04_NetObjectCommon.h
//------------------------------------------------------------------------------
// Copyright (c) 2021 John Hughes
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
#ifndef NETOBJECTCOMMON_H
#define NETOBJECTCOMMON_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"
#include "ae/aeNet.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const ae::Tag TAG_EXAMPLE = "example";
const AetherMsgId kObjectInfoMsg = 1;
const AetherMsgId kInputInfoMsg = 2;
const double kNetTickSeconds = 0.1;

//------------------------------------------------------------------------------
// Game class
//------------------------------------------------------------------------------
class Game
{
public:
  void Initialize()
  {
    window.Initialize( 800, 600, false, true );
    render.Initialize( &window );
    debugLines.Initialize( 32 );
    input.Initialize( &window );
    timeStep.SetTimeStep( 1.0f / 60.0f );
    ae::RandomSeed();
  }

  void Terminate()
  {
    debugLines.Terminate();
    input.Terminate();
    render.Terminate();
    window.Terminate();
  }

  void Render( const ae::Matrix4& worldToNdc )
  {
    render.Activate();
    render.Clear( ae::Color::PicoBlack() );
    
    debugLines.Render( worldToNdc );
    
    render.Present();
    timeStep.Tick();
  }
  
  ae::Window window;
  ae::GraphicsDevice render;
  ae::Input input;
  ae::TimeStep timeStep;
  ae::DebugLines debugLines = TAG_EXAMPLE;
};

//------------------------------------------------------------------------------
// InputInfo struct
//------------------------------------------------------------------------------
struct InputInfo
{
  float accel = 0.0f;
  float turn = 0.0f;

  void Serialize( ae::BinaryStream* stream )
  {
    stream->SerializeFloat( accel );
    stream->SerializeFloat( turn );
  }
};

//------------------------------------------------------------------------------
// GameObject class
//------------------------------------------------------------------------------
class GameObject
{
public:
  GameObject( ae::Color color )
  {
    m_color = color;
  }

  void Update( Game* game )
  {
    const float dt = game->timeStep.GetDt();
    
    // Client - read net data
    if ( !netObject->IsAuthority() )
    {
      ae::BinaryStream rStream = ae::BinaryStream::Reader( netObject->GetSyncData(), netObject->SyncDataLength() );
      Serialize( &rStream );
      if ( ( m_pos - m_netPos ).Length() > 2.5f )
      {
        m_pos = m_netPos;
      }
      else
      {
        m_pos = ae::DtLerp( m_pos, 0.1f, dt, m_netPos );
      }
      m_rotation = ae::DtLerp( m_rotation, 0.1f, dt, m_netRotation );
    }
    
    // Both server and client update
    m_vel += ae::Vec3( 0.0f, 7.0f, 0.0f ).AddRotationXYCopy( m_rotation ) * input.accel * game->timeStep.GetDt();
    m_vel = m_vel.DtSlerp( ae::Vec3( 0.0f ), 0.25f, game->timeStep.GetDt() );
    m_pos += m_vel * game->timeStep.GetDt();
    m_rotationVel += 2.5f * input.turn * game->timeStep.GetDt();
    m_rotationVel = ae::DtLerp( m_rotationVel, 0.5f, dt, 0.0f );
    m_rotation += m_rotationVel * game->timeStep.GetDt();

    // Server - write net data
    if ( netObject->IsAuthority() )
    {
      m_netPos = m_pos;
      m_netRotation = m_rotation;
      ae::Array< uint8_t > buffer = TAG_EXAMPLE;
      ae::BinaryStream wStream = ae::BinaryStream::Writer( &buffer );
      Serialize( &wStream );
      netObject->SetSyncData( wStream.GetData(), wStream.GetOffset() );
    }

    // Draw
    ae::Vec3 p0 = ae::Vec3( 0.0f, 1.0f, 0.0f ).AddRotationXYCopy( m_rotation ) + m_pos;
    ae::Vec3 p1 = ae::Vec3( 0.7f, -1.0f, 0.0f ).AddRotationXYCopy( m_rotation ) + m_pos;
    ae::Vec3 p2 = ae::Vec3( -0.7f, -1.0f, 0.0f ).AddRotationXYCopy( m_rotation ) + m_pos;
    game->debugLines.AddLine( p0, p1, m_color );
    game->debugLines.AddLine( p0, p2, m_color );
    game->debugLines.AddLine( p1, p2, m_color );
  }

  void Serialize( ae::BinaryStream* stream )
  {
    stream->SerializeRaw( &playerId, sizeof( playerId ) );
    stream->SerializeFloat( m_netPos.x );
    stream->SerializeFloat( m_netPos.y );
    stream->SerializeFloat( m_netRotation );
    stream->SerializeFloat( m_vel.x );
    stream->SerializeFloat( m_vel.y );
    stream->SerializeFloat( m_rotationVel );
    stream->SerializeFloat( m_color.r );
    stream->SerializeFloat( m_color.g );
    stream->SerializeFloat( m_color.b );
  }

  ae::NetObject* netObject = nullptr;
  bool alive = true;
  AetherUuid playerId = AetherUuid::Zero();
  InputInfo input;

private:
  ae::Vec3 m_pos = ae::Vec3( ae::Random( -10.0f, 10.0f ), ae::Random( -10.0f, 10.0f ), 0.0f );
  float m_rotation = 0.0f;
  ae::Vec3 m_netPos = ae::Vec3( 0.0f );
  float m_netRotation = 0.0f;
  ae::Vec3 m_vel = ae::Vec3( 0.0f );
  float m_rotationVel = 0.0f;
  ae::Color m_color = ae::Color::White();
};

#endif
