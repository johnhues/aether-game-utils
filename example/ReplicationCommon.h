//------------------------------------------------------------------------------
// ReplicationCommon.h
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
// Constants
//------------------------------------------------------------------------------
const uint32_t kReplicaInfoMsg = 1;

const uint32_t kReplicaType_Red = 0;
const uint32_t kReplicaType_Green = 1;
const uint32_t kReplicaType_Blue = 2;

//------------------------------------------------------------------------------
// Green class
//------------------------------------------------------------------------------
class Green
{
public:
  Green()
  {
    pos = aeFloat3( aeMath::Random( -10.0f, 10.0f ), aeMath::Random( -10.0f, 10.0f ), 0.0f );
    size = aeFloat3( aeMath::Random( 0.5f, 2.0f ), aeMath::Random( 0.5f, 2.0f ), 1.0f );
    rotation = aeMath::Random( 0.0f, aeMath::TWO_PI );
  }

  void Update( float dt, aeSpriteRenderer* spriteRenderer, const aeTexture2D* texture )
  {
    if ( !netData->IsAuthority() )
    {
      // Client - read net data
      aeBinaryStream rStream = aeBinaryStream::Reader( netData->Get(), netData->Length() );
      Serialize( &rStream );
    }

    // Update pos and rotation
    aeFloat4x4 modelToWorld = aeFloat4x4::Translation( pos );
    spriteRenderer->AddSprite( texture, modelToWorld, aeFloat2( 0.0f ), aeFloat2( 1.0f ), aeColor::Green );

    if ( netData->IsAuthority() )
    {
    // Server - write net data
      aeBinaryStream wStream = aeBinaryStream::Writer();
      Serialize( &wStream );
      netData->Set( wStream.GetData(), wStream.GetOffset() );
    }
  }

  void Serialize( aeBinaryStream* stream )
  {
    stream->SerializeFloat( pos.x );
    stream->SerializeFloat( pos.y );
    stream->SerializeFloat( pos.z );
    stream->SerializeFloat( rotation );
  }

  aeFloat3 pos;
  aeFloat3 size;
  float rotation;

  aeRef< aeNetData > netData;
};

#endif
