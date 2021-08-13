//------------------------------------------------------------------------------
// 04_ReplicationCommon.h
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
#ifndef REPLICATIONCOMMON_H
#define REPLICATIONCOMMON_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "ae/aether.h"
#include "ae/aetherEXT.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const ae::Tag TAG_EXAMPLE = "example";
const AetherMsgId kReplicaInfoMsg = 1;

//------------------------------------------------------------------------------
// Game class
//------------------------------------------------------------------------------
class Game
{
public:
  void Game::Initialize( const char* windowTitle )
  {
    window.Initialize( 800, 600, false, true );
    window.SetTitle( windowTitle );
    render.Initialize( &window );
    input.Initialize( &window );
    timeStep.SetTimeStep( 1.0f / 10.0f );
    debugLines.Initialize( 32 );
    //text.Initialize( "font.png", aeTextureFilter::Nearest, 8 );
  }

  void Game::Terminate()
  {
    //text.Terminate();
    debugLines.Terminate();
    //input.Terminate();
    render.Terminate();
    window.Terminate();
  }

  void Game::Render( const ae::Matrix4& worldToNdc )
  {
    render.Activate();
    render.Clear( aeColor::PicoBlack() );
    
    //text.Render( ae::Matrix4::Scaling( ae::Vec3( 0.1f ) ) );
    debugLines.Render( worldToNdc );
    
    render.Present();
    timeStep.Wait();
  }
  
  ae::Window window;
  ae::GraphicsDevice render;
  ae::Input input;
  ae::TimeStep timeStep;
  ae::DebugLines debugLines;
  //aeTextRender text;
};

//------------------------------------------------------------------------------
// GameObject class
//------------------------------------------------------------------------------
class GameObject
{
public:
  GameObject( ae::Color color )
  {
    netData = nullptr;
    alive = true;
    playerId = AetherUuid::Zero();
    m_pos = aeFloat3( aeMath::Random( -10.0f, 10.0f ), aeMath::Random( -10.0f, 10.0f ), 0.0f );
    m_radius = aeMath::Random( 0.5f, 2.0f );
    m_color = color;
  }

  void Update( Game* game )
  {
    // Client - read net data
    if ( !netData->IsAuthority() )
    {
      aeBinaryStream rStream = aeBinaryStream::Reader( netData->GetSyncData(), netData->SyncDataLength() );
      Serialize( &rStream );
    }

    // Server - write net data
    if ( netData->IsAuthority() )
    {
      aeBinaryStream wStream = aeBinaryStream::Writer();
      Serialize( &wStream );
      netData->SetSyncData( wStream.GetData(), wStream.GetOffset() );
    }

    // Draw
    uint32_t points = ( 2.0f * ae::PI * m_radius ) / 0.25f + 0.5f;
    game->debugLines.AddCircle( m_pos, ae::Vec3( 0, 0, 1 ), m_radius, m_color, points );
  }

  void Serialize( aeBinaryStream* stream )
  {
    stream->SerializeFloat( m_pos.x );
    stream->SerializeFloat( m_pos.y );
    stream->SerializeFloat( m_pos.z );
    stream->SerializeFloat( m_radius );
    stream->SerializeFloat( m_color.r );
    stream->SerializeFloat( m_color.g );
    stream->SerializeFloat( m_color.b );
  }

  ae::NetReplica* netData;
  bool alive;
  AetherUuid playerId;

private:
  ae::Vec3 m_pos;
  float m_radius;
  ae::Color m_color;
};

#endif
