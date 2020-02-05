//------------------------------------------------------------------------------
// ReplicationServer.cpp
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
// ClientInfo class
//------------------------------------------------------------------------------
class ClientInfo
{
public:
  AetherUuid uuid;
  aeNetReplicaServer* replicaServer = nullptr;
};

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
  window.SetTitle( "Replication Server" );
  renderer.Initialize( &window, 400, 300 );
  renderer.SetClearColor( aeColor::Black );
  input.Initialize( &window, &renderer );
  spriteRenderer.Initialize( 32 );
  uint8_t texInfo[] = { 255, 255, 255 };
  texture.Initialize( texInfo, 1, 1, 3, aeTextureFilter::Nearest, aeTextureWrap::Repeat );
  timeStep.SetTimeStep( 1.0f / 60.0f );

  // Server modules
  AetherServer* server = AetherServer_New( 3500, 0 );
  aeNetReplicaDB replicaDB;
  aeMap< AetherUuid, aeNetReplicaServer* > replicaServers;

  // Game data
  aeArray< Green > greens;
  greens.Append( Green() ).netData = replicaDB.CreateNetData( kReplicaType_Green, nullptr, 0 );
  greens.Append( Green() ).netData = replicaDB.CreateNetData( kReplicaType_Green, nullptr, 0 );
  greens.Append( Green() ).netData = replicaDB.CreateNetData( kReplicaType_Green, nullptr, 0 );

  while ( !input.GetState()->esc )
  {
    input.Pump();
    AetherServer_Update( server );
    
    ServerReceiveInfo receiveInfo;
    while ( AetherServer_Receive( server, &receiveInfo ) )
    {
      switch ( receiveInfo.msgId )
      {
        case kSysMsgPlayerConnect:
        {
          AE_LOG( "Player # connected", receiveInfo.player->uuid );
          replicaServers.Set( receiveInfo.player->uuid, replicaDB.CreateServer() );
          break;
        }
        case kSysMsgPlayerDisconnect:
        {
          AE_LOG( "Player # disconnected", receiveInfo.player->uuid );

          aeNetReplicaServer* replicaServer = nullptr;
          if ( replicaServers.TryGet( receiveInfo.player->uuid, &replicaServer ) )
          {
            replicaServers.Remove( receiveInfo.player->uuid );
            replicaDB.DestroyServer( replicaServer );
          }

          break;
        }
        default:
        {
          break;
        }
      }
    }

    // Game Update
    for ( uint32_t i = 0; i < greens.Length(); i++ )
    {
      greens[ i ].Update( timeStep.GetTimeStep(), &spriteRenderer, &texture );
    }
    
    // Send replication data
    for ( uint32_t i = 0; i < server->playerCount; i++ )
    {
      AetherPlayer* player = server->allPlayers[ i ];
      aeNetReplicaServer* replicaServer = nullptr;
      if ( replicaServers.TryGet( player->uuid, &replicaServer ) )
      {
        replicaServer->UpdateSendData();
        AetherServer_QueueSendToPlayer( server, player, kReplicaInfoMsg, true, replicaServer->GetSendData(), replicaServer->GetSendLength() );
      }
    }
    AetherServer_SendAll( server );

    renderer.StartFrame();
    spriteRenderer.Render( aeFloat4x4::Scaling( aeFloat3( 1.0f / ( 10.0f * renderer.GetAspectRatio() ), 1.0f / 10.0f, 1.0f ) ) );
    renderer.EndFrame();

    timeStep.Wait();
  }

  AE_LOG( "Terminate" );

  AetherServer_Delete( server );
  server = nullptr;
  
  texture.Destroy();
  spriteRenderer.Destroy();
  input.Terminate();
  renderer.Terminate();
  window.Terminate();

  return 0;
}
