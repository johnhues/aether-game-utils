//------------------------------------------------------------------------------
// ReplicationClient.cpp
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
#include "aeClock.h"
#include "aeInput.h"
#include "aeLog.h"
#include "aeNet.h"
#include "aeRender.h"
#include "aeWindow.h"
#include "ReplicationCommon.h"

//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
int main()
{
  AE_LOG( "Initialize" );

  // System modules
  aeWindow window;
  aeRenderer renderer;
  aeInput input;
  aeSpriteRenderer spriteRenderer;
  aeTexture2D texture;
  aeFixedTimeStep timeStep;
  window.Initialize( 800, 600, false, true );
  window.SetTitle( "Replication Client" );
  renderer.Initialize( &window, 400, 300 );
  renderer.SetClearColor( aeColor::Black );
  input.Initialize( &window, &renderer );
  spriteRenderer.Initialize( 32 );
  uint8_t texInfo[] = { 255, 255, 255 };
  texture.Initialize( texInfo, 1, 1, 3, aeTextureFilter::Nearest, aeTextureWrap::Repeat );
  timeStep.SetTimeStep( 1.0f / 60.0f );

  // Client modules
  AetherClient* client = AetherClient_New( AetherUuid::Generate(), "127.0.0.1", 3500 );
  aeNetReplicaClient replicationClient;

  // Game data
  aeArray< Green > greens;

  while ( !input.GetState()->esc )
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
          replicationClient.ReceiveData( receiveInfo.data, receiveInfo.length );
          break;
        default:
          break;
      }
    }
    // Create new replicated objects
    while ( aeRef< aeNetData > netData = replicationClient.PumpCreated() )
    {
      if ( netData->GetType() == kReplicaType_Green )
      {
        AE_LOG( "Create green" );
        greens.Append( Green() ).netData = netData;
      }
      else
      {
        AE_FAIL();
      }
    }
    // Remove objects that no longer have replication data
    greens.RemoveAllFn( []( Green& green ){ return !green.netData; } );

    // Game Update
    for ( uint32_t i = 0; i < greens.Length(); i++ )
    {
      greens[ i ].Update( timeStep.GetTimeStep(), &spriteRenderer, &texture );
    }

    // Send messages generated during game update
    AetherClient_SendAll( client );

    renderer.StartFrame();
    spriteRenderer.Render( aeFloat4x4::Scaling( aeFloat3( 1.0f / ( 10.0f * renderer.GetAspectRatio() ), 1.0f / 10.0f, 1.0f ) ) );
    renderer.EndFrame();

    timeStep.Wait();
  }

  AE_LOG( "Terminate" );

  AetherClient_Delete( client );
  client = nullptr;
  
  texture.Destroy();
  spriteRenderer.Destroy();
  input.Terminate();
  renderer.Terminate();
  window.Terminate();

  return 0;
}
