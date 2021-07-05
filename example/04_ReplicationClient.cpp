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
#include "04_ReplicationCommon.h"

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
  AE_LOG( "Initialize" );

  // System modules
  ae::Window window;
  ae::GraphicsDevice render;
  ae::Input input;
  aeSpriteRender spriteRender;
  ae::Texture2D texture;
  ae::TimeStep timeStep;
  window.Initialize( 800, 600, false, true );
  window.SetTitle( "Replication Client" );
  render.Initialize( &window );
  input.Initialize( &window );
  spriteRender.Initialize( 32 );
  uint8_t texInfo[] = { 255, 255, 255 };
  texture.Initialize( texInfo, 1, 1, ae::Texture::Format::RGB8, ae::Texture::Type::Uint8, ae::Texture::Filter::Nearest, ae::Texture::Wrap::Repeat );
  timeStep.SetTimeStep( 1.0f / 10.0f );

  // Client modules
  AetherClient* client = AetherClient_New( AetherUuid::Generate(), "127.0.0.1", 3500 );
  aeNetReplicaClient replicationClient;

  // Game data
  ae::Array< Green > greens = TAG_EXAMPLE;

  while ( !input.quit )
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
    while ( aeNetData* netData = replicationClient.PumpCreate() )
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
      Green* green = &greens[ i ];
      green->Update( timeStep.GetTimeStep(), &spriteRender, &texture, &input );
      if ( green->netData->IsPendingDestroy() )
      {
        replicationClient.Destroy( green->netData );
        green->netData = nullptr;
      }
    }
    // Remove objects that no longer have replication data
    greens.RemoveAllFn( []( Green& green ) { return !green.netData; } );
    

    // Send messages generated during game update
    AetherClient_SendAll( client );

    render.Activate();
    render.Clear( ae::Color::PicoDarkPurple() );
    spriteRender.Render( ae::Matrix4::Scaling( ae::Vec3( 1.0f / ( 10.0f * render.GetAspectRatio() ), 1.0f / 10.0f, 1.0f ) ) );
    render.Present();

    timeStep.Wait();
  }

  AE_LOG( "Terminate" );

  AetherClient_Delete( client );
  client = nullptr;
  
  texture.Destroy();
  spriteRender.Destroy();
  //input.Terminate();
  render.Terminate();
  window.Terminate();

  return 0;
}
