//------------------------------------------------------------------------------
// 04_NetObjectServer.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2021 John Hughes
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
#include "04_NetObjectCommon.h"

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
  AE_LOG( "Initialize" );

  // Init
  Game game;
  game.Initialize( "NetObject Server" );
  AetherServer* server = AetherServer_New( 3500, 0, 16 );
  ae::NetObjectServer netObjectServer;
  ae::Map< AetherUuid, ae::NetObjectConnection* > netObjectConnections = TAG_EXAMPLE;
  ae::Array< GameObject > gameObjects = TAG_EXAMPLE;
  double nextSend = 0.0;

  // Update
  while ( !game.input.quit )
  {
    // Poll input and net modules
    game.input.Pump();
    AetherServer_Update( server );
    
    ServerReceiveInfo receiveInfo;
    while ( AetherServer_Receive( server, &receiveInfo ) )
    {
      AetherUuid playerId = receiveInfo.player->uuid;
      int32_t objIdx = gameObjects.FindFn( [ playerId ]( const GameObject& o ){ return o.playerId == playerId; } );
      GameObject* obj = ( objIdx >= 0 ) ? &gameObjects[ objIdx ] : nullptr;
      
      switch ( receiveInfo.msgId )
      {
        case kSysMsgPlayerConnect:
        {
          AE_LOG( "Player # connected", receiveInfo.player->uuid );
          netObjectConnections.Set( receiveInfo.player->uuid, netObjectServer.CreateConnection() );

          obj = &gameObjects.Append( GameObject( ae::Color::HSV( ae::Random01(), 1.0f, 1.0f ) ) );
          obj->playerId = receiveInfo.player->uuid;
          obj->netObject = netObjectServer.CreateNetObject();
          obj->netObject->SetInitData( nullptr, 0 );
          break;
        }
        case kSysMsgPlayerDisconnect:
        {
          AE_LOG( "Player # disconnected", playerId );

          // Kill player gameobject
          if ( obj )
          {
            obj->alive = false;
          }

          // Remove player from replica db
          ae::NetObjectConnection* conn = nullptr;
          if ( netObjectConnections.TryGet( playerId, &conn ) )
          {
            netObjectConnections.Remove( playerId );
            netObjectServer.DestroyConnection( conn );
          }

          break;
        }
        case kInputInfoMsg:
        {
          if ( obj )
          {
            ae::BinaryStream stream = ae::BinaryStream::Reader( receiveInfo.data );
            stream.SerializeObject( obj->input );
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
      netObjectServer.DestroyNetObject( gameObjects[ index ].netObject );
      gameObjects.Remove( index );
    }
    
    // Send replication data
    double time = ae::GetTime();
    if ( nextSend < time )
    {
      netObjectServer.UpdateSendData();
      for ( int32_t i = 0; i < server->playerCount; i++ )
      {
        AetherPlayer* player = server->allPlayers[ i ];
        ae::NetObjectConnection* conn = nullptr;
        if ( netObjectConnections.TryGet( player->uuid, &conn ) )
        {
          AetherServer_QueueSendToPlayer( server, player, kObjectInfoMsg, true, conn->GetSendData(), conn->GetSendLength() );
        }
      }
      AetherServer_SendAll( server );
      nextSend = time + kNetTickSeconds;
    }

    // Render
    game.Render( ae::Matrix4::Scaling( ae::Vec3( 1.0f / ( 10.0f * game.render.GetAspectRatio() ), 1.0f / 10.0f, 1.0f ) ) );
  }

  AE_LOG( "Terminate" );
  AetherServer_Delete( server );
  game.Terminate();

  return 0;
}
