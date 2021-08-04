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

const uint32_t kReplicaInfoMsg = 1;

const uint32_t kReplicaType_Red = 0;
const uint32_t kReplicaType_Green = 1;
const uint32_t kReplicaType_Blue = 2;

struct ChangeColorRPC
{
  void Serialize( aeBinaryStream* stream )
  {
    stream->SerializeFloat( color.r );
    stream->SerializeFloat( color.g );
    stream->SerializeFloat( color.b );
    stream->SerializeFloat( color.a );
  }
  ae::Color color;
};

class Game
{
public:
  void Initialize();
  void Terminate();
  void Render( const ae::Matrix4& worldToNdc );
  
  ae::Window window;
  ae::GraphicsDevice render;
  ae::Input input;
  ae::TimeStep timeStep;
  ae::DebugLines debugLines;
};

//------------------------------------------------------------------------------
// GameObject class
//------------------------------------------------------------------------------
class GameObject
{
public:
  GameObject()
  {
    pos = ae::Vec3( 0.0f );
    size = ae::Vec3( 1.0f );
    rotation = 0.0f;
    life = ae::MaxValue< float >();

    m_color = ae::Color::Green();
  }

  void Update( Game* game )
  {
    float dt = game->timeStep.GetDt();
    if ( !netData->IsAuthority() )
    {
      // Client - read net data
      aeBinaryStream rStream = aeBinaryStream::Reader( netData->GetSyncData(), netData->SyncDataLength() );
      Serialize( &rStream );

      aeNetData::Msg msg;
      while ( netData->PumpMessages( &msg ) )
      {
        AE_LOG( "Received Message: #", (const char*)msg.data );
      }

      if ( game->input.Get( ae::Key::Space ) && !game->input.GetPrev( ae::Key::Space ) )
      {
        ae::Color colors[] =
        {
          ae::Color::White(),
          ae::Color::White(),
          ae::Color::Red(),
          ae::Color::Blue(),
          ae::Color::Orange(),
          ae::Color::Gray()
        };

        ChangeColorRPC rpc;
        rpc.color = colors[ ae::Random( 0, countof(colors) ) ];

        aeBinaryStream wStream = aeBinaryStream::Writer();
        wStream.SerializeObject( rpc );
        netData->SendMessage( wStream.GetData(), wStream.GetOffset() );
      }
    }

    // Update pos and rotation
    ae::Matrix4 modelToWorld = ae::Matrix4::Translation( pos );
    life -= dt;

    if ( netData->IsAuthority() )
    {
      // Server - write net data
      aeBinaryStream wStream = aeBinaryStream::Writer();
      Serialize( &wStream );
      netData->SetSyncData( wStream.GetData(), wStream.GetOffset() );

      const char* someMessage = "message 1";
      netData->SendMessage( someMessage, (uint32_t)strlen( someMessage ) + 1 );
      someMessage = "message number 2";
      netData->SendMessage( someMessage, (uint32_t)strlen( someMessage ) + 1 );

      aeNetData::Msg msg;
      while ( netData->PumpMessages( &msg ) )
      {
        ChangeColorRPC rpc;
        aeBinaryStream rStream = aeBinaryStream::Reader( msg.data, msg.length );
        rStream.SerializeObject( rpc );

        m_color = rpc.color;
      }
    }

    uint32_t points = ( 2.0f * ae::PI * size.x ) / 0.25f + 0.5f;
    game->debugLines.AddCircle( pos, ae::Vec3( 0, 0, 1 ), size.x, ae::Color::Red(), points );
  }

  void Serialize( aeBinaryStream* stream )
  {
    stream->SerializeFloat( pos.x );
    stream->SerializeFloat( pos.y );
    stream->SerializeFloat( pos.z );
    stream->SerializeFloat( rotation );
    stream->SerializeFloat( life );
  }

  ae::Vec3 pos;
  ae::Vec3 size;
  float rotation;
  float life;

  aeNetData* netData;

private:
  ae::Color m_color;
};

#endif
