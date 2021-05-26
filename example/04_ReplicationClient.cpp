//------------------------------------------------------------------------------
// 04_ReplicationClient.cpp
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
#include "ae/aetherEXT.h"
#include "04_ReplicationCommon.h"

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
  AE_LOG( "Initialize" );

  // System modules
  aeWindow window;
  aeRender render;
  aeInput input;
  aeSpriteRender spriteRender;
  aeTexture2D texture;
  aeFixedTimeStep timeStep;
  window.Initialize( 800, 600, false, true );
  window.SetTitle( "Replication Client" );
  render.InitializeOpenGL( &window );
  input.Initialize( &window );
  spriteRender.Initialize( 32 );
  uint8_t texInfo[] = { 255, 255, 255 };
  texture.Initialize( texInfo, 1, 1, aeTextureFormat::RGB8, aeTextureType::Uint8, aeTextureFilter::Nearest, aeTextureWrap::Repeat );
  timeStep.SetTimeStep( 1.0f / 10.0f );

  // Client modules
  AetherClient* client = AetherClient_New( AetherUuid::Generate(), "127.0.0.1", 3500 );
  aeNetReplicaClient replicationClient;

  // Game data
  ae::Array< Green > greens = TAG_EXAMPLE;

  while ( !input.GetState()->exit )
  {
    input.Pump();
    
    // Update connection to server
    if ( !client->IsConnected() && !client->IsConnecting() )
    {
      AE_LOG( "Connecting to server" );
      AetherClient_Connect( client );
    }

    // Handle messages from server
    ReceiveInfo receiveInfo;
    while ( AetherClient_Receive( client, &receiveInfo ) )
    {
      switch ( receiveInfo.msgId )
      {
        case kSysMsgServerConnect:
          AE_LOG( "Connected to server" );
          break;
        case kSysMsgServerDisconnect:
          AE_LOG( "Disconnected from server" );
          break;
        case kReplicaInfoMsg:
          if ( receiveInfo.data.Length() )
          {
            replicationClient.ReceiveData( &receiveInfo.data[ 0 ], receiveInfo.data.Length() );
          }
          break;
        default:
          break;
      }
    }
    
    // Create new replicated objects
    while ( aeRef< aeNetData > netData = replicationClient.PumpCreated() )
    {
      uint32_t type = 0;
      aeBinaryStream readStream = aeBinaryStream::Reader( netData->GetInitData(), netData->InitDataLength() );
      readStream.SerializeUint32( type );
      
      if ( type == kReplicaType_Green )
      {
        AE_LOG( "Create green" );
        greens.Append( Green() ).netData = netData;
      }
      else
      {
        AE_FAIL();
      }
    }
    
    // Game Update
    for ( uint32_t i = 0; i < greens.Length(); i++ )
    {
      greens[ i ].Update( timeStep.GetTimeStep(), &spriteRender, &texture, &input );
    }
    
    // Remove objects that no longer have replication data
    replicationClient.DestroyPending();
    greens.RemoveAllFn( []( Green& green ) { return !green.netData; } );

    // Send messages generated during game update
    AetherClient_SendAll( client );

    render.Activate();
    render.Clear( aeColor::PicoDarkPurple() );
    spriteRender.Render( aeFloat4x4::Scaling( aeFloat3( 1.0f / ( 10.0f * render.GetAspectRatio() ), 1.0f / 10.0f, 1.0f ) ) );
    render.Present();

    timeStep.Wait();
  }

  AE_LOG( "Terminate" );

  AetherClient_Delete( client );
  client = nullptr;
  
  texture.Destroy();
  spriteRender.Destroy();
  input.Terminate();
  render.Terminate();
  window.Terminate();

  return 0;
}
