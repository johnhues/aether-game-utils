//------------------------------------------------------------------------------
// 04_NetObjectClient.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2025 John Hughes
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
  // Init
  AE_LOG( "Initialize" );
  Game game;
  game.Initialize();
  AetherClient* client = AetherClient_New( AetherUuid::Generate(), "127.0.0.1", 3500 );
  ae::NetObjectClient netObjectClient;
  ae::Array< GameObject > gameObjects = TAG_EXAMPLE;
  double nextSend = 0.0;

  // Update
  while( !game.input.quit )
  {
    double time = ae::GetTime();
    game.input.Pump();
    game.window.SetTitle( client->IsConnected() ? "Spaceships Client (Connected)" : "Spaceships Client (Connecting...)" );
    
    //--------------------------------------------------------------------------
    // Net update
    //--------------------------------------------------------------------------
    // Update connection to server
    if( !client->IsConnected() && !client->IsConnecting() )
    {
      AE_LOG( "Connecting to server" );
      AetherClient_Connect( client );
    }
    // Handle messages from server
    ReceiveInfo receiveInfo;
    while( AetherClient_Receive( client, &receiveInfo ) )
    {
      switch( receiveInfo.msgId )
      {
        case kSysMsgServerConnect:
          AE_LOG( "Connected to server" );
          break;
        case kSysMsgServerDisconnect:
          AE_LOG( "Disconnected from server" );
          break;
        case kObjectInfoMsg:
          netObjectClient.ReceiveData( receiveInfo.data.Data(), receiveInfo.data.Length() );
          break;
        default:
          break;
      }
    }
    // Create new replicated objects
    while( ae::NetObject* netObject = netObjectClient.PumpCreate() )
    {
      ae::BinaryReader readStream( netObject->GetSyncData(), netObject->SyncDataLength() );
      GameObject* obj = &gameObjects.Append( GameObject( ae::Color::White() ) );
      obj->netObject = netObject;
      obj->Serialize( &readStream );
    }
    
    //------------------------------------------------------------------------------
    // Game Update
    //------------------------------------------------------------------------------
    for( GameObject& obj : gameObjects )
    {
      obj.Update( &game );
    }

    //------------------------------------------------------------------------------
    // Delete networked objects
    //------------------------------------------------------------------------------
    for( GameObject& obj : gameObjects )
    {
      if( obj.netObject->IsPendingDestroy() )
      {
        netObjectClient.Destroy( obj.netObject );
        obj.netObject = nullptr;
      }
    }
    // Remove objects that no longer have replication data
    gameObjects.RemoveAllFn( []( const GameObject& o ) { return !o.netObject; } );
    
    //------------------------------------------------------------------------------
    // Send input to server
    //------------------------------------------------------------------------------
    if( client->IsConnected() && nextSend < time )
    {
      AetherUuid playerId = client->localPlayer->uuid;
      int32_t objIdx = gameObjects.FindFn( [ playerId ]( const GameObject& o ){ return o.playerId == playerId; } );
      GameObject* obj = ( objIdx >= 0 ) ? &gameObjects[ objIdx ] : nullptr;
      if( obj )
      {
        uint8_t buffer[ 64 ];
        ae::BinaryWriter stream( buffer, countof(buffer) );
        obj->input = InputInfo();
        obj->input.accel += game.input.Get( ae::Key::Up ) ? 1.0f : 0.0f;
        obj->input.accel -= game.input.Get( ae::Key::Down ) ? 1.0f : 0.0f;
        obj->input.turn -= game.input.Get( ae::Key::Right ) ? 1.0f : 0.0f;
        obj->input.turn += game.input.Get( ae::Key::Left ) ? 1.0f : 0.0f;
        stream.SerializeObject( obj->input );
        AetherClient_QueueSend( client, kInputInfoMsg, false, stream.GetData(), stream.GetOffset() );
      }
      AetherClient_SendAll( client ); // Send messages generated during game update
      nextSend = time + kNetTickSeconds;
    }

    //------------------------------------------------------------------------------
    // Render
    //------------------------------------------------------------------------------
    game.Render( ae::Matrix4::Scaling( ae::Vec3( 1.0f / ( 10.0f * game.render.GetAspectRatio() ), 1.0f / 10.0f, 1.0f ) ) );
  }

  AE_LOG( "Terminate" );
  AetherClient_Delete( client );
  client = nullptr;
  game.Terminate();

  return 0;
}
