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
#include "ae/aetherEXT.h"
#include "04_ReplicationCommon.h"

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
  AE_LOG( "Initialize" );

  // Init
  Game game;
  game.Initialize();
  AetherServer* server = AetherServer_New( 3500, 0, 16 );
  aeNetReplicaDB replicaDB;  
  ae::Map< AetherUuid, aeNetReplicaServer* > replicaServers = TAG_EXAMPLE;
  ae::Array< GameObject > gameObjects = TAG_EXAMPLE;

  // Load level objects
  while ( gameObjects.Length() < 3 )
  {
    GameObject* obj = &gameObjects.Append( GameObject( ae::Color::Gray() ) );
    obj->netData = replicaDB.CreateNetData();
    obj->netData->SetInitData( nullptr, 0 );
  }
  
  // Update
  while ( !game.input.quit )
  {
    // Poll input and net modules
    game.input.Pump();
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

          GameObject* obj = &gameObjects.Append( GameObject( ae::Color::Green() ) );
          obj->playerId = receiveInfo.player->uuid;
          obj->netData = replicaDB.CreateNetData();
          obj->netData->SetInitData( nullptr, 0 );
          break;
        }
        case kSysMsgPlayerDisconnect:
        {
          AetherUuid playerId = receiveInfo.player->uuid;
          AE_LOG( "Player # disconnected", playerId );

          // Kill player gameobject
          int32_t playerIndex = gameObjects.FindFn( [=]( const GameObject& o ){ return o.playerId == playerId; } );
          if ( playerIndex >= 0 )
          {
            gameObjects[ playerIndex ].alive = false;
          }

          // Remove player from replica db
          aeNetReplicaServer* replicaServer = nullptr;
          if ( replicaServers.TryGet( playerId, &replicaServer ) )
          {
            replicaServers.Remove( playerId );
            replicaDB.DestroyServer( replicaServer );
          }

          break;
        }
        default:
          break;
      }
    }

    
    // Game Update
    for ( uint32_t i = 0; i < gameObjects.Length(); i++ )
    {
      gameObjects[ i ].Update( &game );
    }
    // Destroy dead objects
    auto findDeadFn = []( const GameObject& object ){ return !object.alive; };
    for ( int32_t index = gameObjects.FindFn( findDeadFn ); index >= 0; index = gameObjects.FindFn( findDeadFn ) )
    {
      replicaDB.DestroyNetData( gameObjects[ index ].netData );
      gameObjects.Remove( index );
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

    // Render
    game.Render( ae::Matrix4::Scaling( aeFloat3( 1.0f / ( 10.0f * game.render.GetAspectRatio() ), 1.0f / 10.0f, 1.0f ) ) );
  }

  AE_LOG( "Terminate" );
  AetherServer_Delete( server );
  game.Terminate();

  return 0;
}
