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
#include "aeBinaryStream.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
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
  aeColor color;
};

//------------------------------------------------------------------------------
// Green class
//------------------------------------------------------------------------------
class Green
{
public:
  Green()
  {
    pos = aeFloat3( 0.0f );
    size = aeFloat3( 1.0f );
    rotation = 0.0f;
    life = aeMath::MaxValue< float >();

    m_color = aeColor::Green;
  }

  void Update( float dt, aeSpriteRender* spriteRender, const aeTexture2D* texture, aeInput* input )
  {
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

      if ( input->GetState()->space && !input->GetPrevState()->space )
      {
        aeColor colors[] =
        {
          aeColor::White,
          aeColor::White,
          aeColor::Red,
          aeColor::Blue,
          aeColor::Orange,
          aeColor::Gray
        };

        ChangeColorRPC rpc;
        rpc.color = colors[ aeMath::Random( 0, countof(colors) ) ];

        aeBinaryStream wStream = aeBinaryStream::Writer();
        wStream.SerializeObject( rpc );
        netData->SendMessage( wStream.GetData(), wStream.GetOffset() );
      }
    }

    // Update pos and rotation
    aeFloat4x4 modelToWorld = aeFloat4x4::Translation( pos );
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

    spriteRender->AddSprite( texture, modelToWorld, aeFloat2( 0.0f ), aeFloat2( 1.0f ), m_color );
  }

  void Serialize( aeBinaryStream* stream )
  {
    stream->SerializeFloat( pos.x );
    stream->SerializeFloat( pos.y );
    stream->SerializeFloat( pos.z );
    stream->SerializeFloat( rotation );
    stream->SerializeFloat( life );
  }

  aeFloat3 pos;
  aeFloat3 size;
  float rotation;
  float life;

  aeRef< aeNetData > netData;

private:
  aeColor m_color;
};

#endif
