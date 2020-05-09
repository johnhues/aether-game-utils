//------------------------------------------------------------------------------
// 04_ReplicationServer.cpp
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
#include "04_ReplicationCommon.h"

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
  window.SetTitle( "Replication Server" );
  render.InitializeOpenGL( &window, 400, 300 );
  render.SetClearColor( aeColor::Black() );
  input.Initialize( &window, &render );
  spriteRender.Initialize( 32 );
  uint8_t texInfo[] = { 255, 255, 255 };
  texture.Initialize( texInfo, 1, 1, 3, aeTextureFilter::Nearest, aeTextureWrap::Repeat );
  timeStep.SetTimeStep( 1.0f / 10.0f );

  // Server modules
  AetherServer* server = AetherServer_New( 3500, 0, 1 );
  aeNetReplicaDB replicaDB;
  aeMap< AetherUuid, aeNetReplicaServer* > replicaServers;

  // Game data
  aeArray< Green > greens;
  auto AddGreen = [ &greens, &replicaDB ]()
  {
    Green* green = &greens.Append( Green() );
    green->pos = aeFloat3( aeMath::Random( -10.0f, 10.0f ), aeMath::Random( -10.0f, 10.0f ), 0.0f );
    green->size = aeFloat3( aeMath::Random( 0.5f, 2.0f ), aeMath::Random( 0.5f, 2.0f ), 1.0f );
    green->rotation = aeMath::Random( 0.0f, aeMath::TWO_PI );
    green->life = 5.0f + aeMath::Random( 0.7f, 1.3f );
    green->netData = replicaDB.CreateNetData( kReplicaType_Green, nullptr, 0 );
  };
  AddGreen();

  while ( !input.GetState()->exit )
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
      greens[ i ].Update( timeStep.GetTimeStep(), &spriteRender, &texture, &input );
    }

    // Destroy dead objects
    auto findDeadFn = []( const Green& object ){ return object.life <= 0.0f; };
    for ( int32_t index = greens.FindFn( findDeadFn ); index >= 0; index = greens.FindFn( findDeadFn ) )
    {
      replicaDB.DestroyNetData( greens[ index ].netData );
      greens.Remove( index );
      
      AddGreen();
    }
    
    // Send replication data
    replicaDB.UpdateSendData();
    for ( int32_t i = 0; i < server->playerCount; i++ )
    {
      AetherPlayer* player = server->allPlayers[ i ];
      aeNetReplicaServer* replicaServer = nullptr;
      if ( replicaServers.TryGet( player->uuid, &replicaServer ) )
      {
        AetherServer_QueueSendToPlayer( server, player, kReplicaInfoMsg, true, replicaServer->GetSendData(), replicaServer->GetSendLength() );
      }
    }
    AetherServer_SendAll( server );

    render.StartFrame();
    spriteRender.Render( aeFloat4x4::Scaling( aeFloat3( 1.0f / ( 10.0f * render.GetAspectRatio() ), 1.0f / 10.0f, 1.0f ) ) );
    render.EndFrame();

    timeStep.Wait();
  }

  AE_LOG( "Terminate" );

  AetherServer_Delete( server );
  server = nullptr;
  
  texture.Destroy();
  spriteRender.Destroy();
  input.Terminate();
  render.Terminate();
  window.Terminate();

  return 0;
}
