//------------------------------------------------------------------------------
// aeNetClient.cpp
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
#include "aeNet.h"
#include <vector>
#include <algorithm>
#include <enet/enet.h>
// #include "HVN_WebConn.h"

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
#if defined(DAEMON) && defined(__linux__) && !defined(__chip__)
  #define USE_WEBWOCKETS 1
#endif

#ifdef USE_WEBWOCKETS
#include <libwebsockets.h>

static int ws_service_callback( lws *wsi, lws_callback_reasons reason, void *userdata, void *msgData, size_t msgLength );

struct SessionData
{
    uint32_t id;
};
#endif

//------------------------------------------------------------------------------
// aeServer member functions
//------------------------------------------------------------------------------
namespace
{
  struct AetherServerInternal
  {
    AetherServer pub;
    struct
    {
      std::vector<AetherPlayer*> players;
      ENetHost* host;
#ifdef USE_WEBWOCKETS
      WebConnectionManager webConnManager;
      lws_context* webContext;
      lws_protocols webProtocols[ 2 ];
      lws_context_creation_info webInfo;
#endif
    } priv;
  };
}

AetherPlayer* AetherServer_GetPlayer( AetherServerInternal* as, AetherUuid uuid )
{
  int32_t playerCount = as->pub.playerCount;
  for ( int32_t i = 0; i < playerCount; i++ )
  {
    AetherPlayer* p = as->pub.allPlayers[ i ];
    if ( memcmp( &p->uuid, &uuid, sizeof(uuid) ) == 0 ) { return p; }
  }
  
  AE_FAIL();
  return nullptr;
}

AetherPlayer* AetherServer_AddPlayer( AetherServerInternal* as, AetherUuid uuid )
{
  AetherPlayer* player = new AetherPlayer();
  player->uuid = uuid;
  player->netId = 0;
  player->userData = nullptr;
  player->alive = true;
  player->pendingLevel = "";
  player->pendingLink = "";
  player->hasPendingLevelChange = false;
  
  as->priv.players.push_back( player );
  as->pub.playerCount = (int32_t)as->priv.players.size();
  as->pub.allPlayers = as->priv.players.data();
  
  return player;
}

AetherServer* AetherServer_New( uint16_t port, uint16_t webPort, uint32_t maxPlayers )
{
  enet_initialize();
  AetherServerInternal* as = new AetherServerInternal();
  
  ENetAddress hostAddress;
  hostAddress.host = ENET_HOST_ANY;
  hostAddress.port = port;
  as->priv.host = enet_host_create( &hostAddress, maxPlayers, 2, 0, 0 );
  AE_ASSERT( as->priv.host );
  
  as->pub.playerCount = 0;
  as->pub.allPlayers = as->priv.players.data();

#ifdef USE_WEBWOCKETS
  as->priv.webConnManager.Initialize();

  memset( as->priv.webProtocols, 0, sizeof(as->priv.webProtocols) );
  lws_protocols* protocols = as->priv.webProtocols;
  protocols[ 0 ].name = "binary";
  protocols[ 0 ].callback = ws_service_callback;
  protocols[ 0 ].per_session_data_size = sizeof(SessionData);
  protocols[ 0 ].rx_buffer_size = 0;
  protocols[ 0 ].user = &as->priv.webConnManager;
  // Last protocol in list must have null callback
  protocols[ 1 ].callback = nullptr;

  memset( &as->priv.webInfo, 0, sizeof(as->priv.webInfo) );
  lws_context_creation_info* info = &as->priv.webInfo;
  info->port = webPort;
  info->protocols = as->priv.webProtocols;
  info->gid = -1;
  info->uid = -1;

  as->priv.webContext = lws_create_context( info );
  AE_ASSERT( as->priv.webContext );
#endif
  
  return (AetherServer*)as;
}

void AetherServer_Delete( AetherServer* _as )
{
  AetherServerInternal* as = (AetherServerInternal*)_as;
#ifdef USE_WEBWOCKETS
  lws_context_destroy( as->priv.webContext );
#endif

  ENetPeer* peers = as->priv.host->peers;
  int32_t peerCount = (int32_t)as->priv.host->peerCount;
  for ( uint32_t i = 0; i < peerCount; i++ )
  {
    enet_peer_disconnect( &peers[ i ], 0 );
  }
  enet_host_flush( as->priv.host );

  enet_host_destroy( as->priv.host );
  delete as;
  enet_deinitialize();
}

#ifdef USE_WEBWOCKETS
static int ws_service_callback( lws *wsi, lws_callback_reasons reason, void *userdata, void *msgData, size_t msgLength )
{
  const lws_protocols* proto = lws_get_protocol( wsi );
  WebConnectionManager* manager = proto ? (WebConnectionManager*)proto->user : nullptr;
  SessionData* session = (SessionData*)userdata;
  
  switch (reason)
  {
    case LWS_CALLBACK_ESTABLISHED:
    {
      lws_callback_on_writable( wsi );
      WebConnection* conn = manager->AllocateConnection( LWS_SEND_BUFFER_PRE_PADDING );
      
      WebConnEvent e;
      e.type = kWebConn_Connect;
      e.conn = conn;
      e.userdata = conn->userdata;
      manager->PushEvent( e );
      
      session->id = conn->GetId();

      printf( "[Main Service] Connection established. id: %u\n", session->id );
      break;
    }
    case LWS_CALLBACK_RECEIVE:
    {
      WebConnection* conn = manager->GetConnection( session->id );

      WebConnEvent e;
      e.type = kWebConn_Receive;
      e.conn = conn;
      e.userdata = conn->userdata;
      manager->PushEvent( e );

      conn->RecvBufferPush( (const uint8_t*)msgData, msgLength );

      // lws_callback_on_writable( wsi );

      break;
    }
    case LWS_CALLBACK_SERVER_WRITEABLE:
    {
      WebConnection* conn = manager->GetConnection( session->id );
      if ( !conn )
      {
        break;
      }
      uint8_t* data;
      uint32_t length;
      if ( conn->SendBufferPeek( &data, &length ) )
      {
        int result = lws_write( wsi, data, length, LWS_WRITE_BINARY );
        if ( result > 0 )
        {
          conn->SendBufferPop( result );
        }
      }
      // lws_callback_on_writable( wsi );
      break;
    }
    case LWS_CALLBACK_CLOSED:
    {
      WebConnection* conn = manager->GetConnection( session->id );
      if ( conn )
      {
        WebConnEvent e;
        e.type = kWebConn_Disconnect;
        e.conn = conn;
        e.userdata = conn->userdata;
        manager->PushEvent( e );
      }

      printf( "[Main Service] Client close id: %u\n", session->id );
      break;
    }
    default:
      break;
  }

  return 0;
}

#endif

void AetherServer_Update( AetherServer* _as )
{
  AetherServerInternal* as = (AetherServerInternal*)_as;

#ifdef USE_WEBWOCKETS
  lws_service( as->priv.webContext, 0 );
#endif

  auto removePlayers = [&]( AetherPlayer* player ) -> bool
  {
    if ( !player->alive )
    {
      char uuidStr[ 64 ];
      player->uuid.ToString( uuidStr, sizeof(uuidStr) );
      // AE_LOG( "AETHER remove disconnected player %s", uuidStr );

      delete player;

      return true;
    }
    return false;
  };
  auto iter = std::remove_if( as->priv.players.begin(), as->priv.players.end(), removePlayers );
  as->priv.players.erase( iter, as->priv.players.end() );

  as->pub.playerCount = as->priv.players.size();
  as->pub.allPlayers = as->priv.players.data();
}

bool AetherServer_SystemReceive( AetherServerInternal* as, void** userdata, AetherClientHeader header, const uint8_t* data, int32_t length, ServerReceiveInfo* infoOut )
{
  switch ( header.msgId )
  {
    case kSysMsgPlayerConnect:
    {
      char uuidStr[ 64 ];
      header.uuid.ToString( uuidStr, sizeof(uuidStr) );
      // AE_LOG( "AETHER player connect %s", uuidStr );
      
      // TODO: Make sure player isn't already connected
      AetherPlayer* player = AetherServer_AddPlayer( as, header.uuid );
      
      infoOut->msgId = kSysMsgPlayerConnect;
      infoOut->player = player;

      
      *userdata = (void*)player;
      
      return true;
    }
    default:
    {
      AE_FAIL();
      return false;
    }
  }
}

bool AetherServer_Receive( AetherServer* _as, ServerReceiveInfo* infoOut )
{
  AetherServerInternal* as = (AetherServerInternal*)_as;
  
  ENetEvent e;
  memset( &e, 0, sizeof(e) );
  while ( enet_host_service( as->priv.host, &e, 0 ) > 0 )
  {
    switch( e.type )
    {
      case ENET_EVENT_TYPE_CONNECT:
      {
        // AE_LOG( "ENET Connect" );
        // uint32_t timeoutLimit = 32;
        // uint32_t timeoutMinimum = 2000;
        // uint32_t timeoutMaximum = 4000;
        // enet_peer_timeout ( e.peer, timeoutLimit, timeoutMinimum, timeoutMaximum );
        break;
      }
      case ENET_EVENT_TYPE_RECEIVE:
      {
        AetherClientHeader header = *(AetherClientHeader*)e.packet->data;
        uint8_t* data = e.packet->data + sizeof(header);
        int32_t length = e.packet->dataLength - sizeof(header);
        
        bool success = false;
        if ( header.msgId & kSysMsgMask )
        {
          if ( AetherServer_SystemReceive( as, &e.peer->data, header, data, length, infoOut ) )
          {
            success = true;
          }
        }
        else
        {
          infoOut->msgId = header.msgId;
          infoOut->player = AetherServer_GetPlayer( as, header.uuid );
          infoOut->data.Clear();
          infoOut->data.Append( data, length );
          success = true;
        }

        enet_packet_destroy( e.packet );
        if ( success )
        {
          return true;
        }

        break;
      }
      case ENET_EVENT_TYPE_DISCONNECT:
      {

        AetherPlayer* player = (AetherPlayer*)e.peer->data;
        if ( player )
        {
          char uuidStr[ 64 ];
          player->uuid.ToString( uuidStr, sizeof(uuidStr) );
          // AE_LOG( "ENET Disconnect %s", uuidStr );

          player->alive = false;
          
          infoOut->msgId = kSysMsgPlayerDisconnect;
          infoOut->player = player;
          
          return true;
        }

        // AE_LOG( "ENET Player disconnected before finishing connection process" );
        break;
      }
      default:
      {
        AE_FAIL();
        break;
      }
    }
  }

#ifdef USE_WEBWOCKETS
  WebConnEvent event;
  while ( as->priv.webConnManager.Service( &event ) )
  {
    switch( event.type )
    {
      case kWebConn_Connect:
      {
        // AE_LOG( "WEBCONN Connect" );
        break;
      }
      case kWebConn_Receive:
      {
        uint8_t buf[ 2048 ];
        uint32_t bufLen = event.conn->Recv( buf, sizeof(buf) );
        if ( bufLen )
        {
          AE_ASSERT( bufLen >= sizeof(AetherClientHeader) );
          AetherClientHeader header = *(AetherClientHeader*)buf;
          uint8_t* data = buf + sizeof(header);
          int32_t length = bufLen - sizeof(header);
          
          if ( header.msgId & kSysMsgMask )
          {
            if ( AetherServer_SystemReceive( as, &event.conn->userdata, header, data, length, infoOut ) )
            {
              return true;
            }
          }
          else
          {
            infoOut->msgId = header.msgId;
            infoOut->player = AetherServer_GetPlayer( as, header.uuid );
            infoOut->length = length;
            memcpy( infoOut->data, data, length );
            return true;
          }
        }
        break;
      }
      case kWebConn_Disconnect:
      {
        // AE_LOG( "WEBCONN Disconnect" );
        
        AetherPlayer* player = (AetherPlayer*)event.userdata;
        if ( player )
        {
          player->alive = false;
          
          infoOut->msgId = kSysMsgPlayerDisconnect;
          infoOut->player = player;
          infoOut->length = 0;
          return true;
        }
        break;
      }
    }
  }
#endif
  
  return false;
}

void AetherServer_QueueSend( AetherServer* _as, const ServerSendInfo* info )
{
  AE_ASSERT( info->msgId != kInvalidAetherMsgId );

  AetherServerInternal* as = (AetherServerInternal*)_as;
  AE_ASSERT( as );
  AE_ASSERT( as->priv.host );
  
  bool reliable = info->reliable;
  uint32_t flags = reliable ? ENET_PACKET_FLAG_RELIABLE : 0;
  int32_t channel = reliable ? kNetChannelReliable : kNetChannelUnreliable;
  
  uint32_t dataLength = sizeof(AetherServerHeader) + info->length;
  
  AetherServerHeader header;
  header.msgId = info->msgId;
  
  ENetPeer* peers = as->priv.host->peers;
  int32_t peerCount = as->priv.host->peerCount;

#ifdef USE_WEBWOCKETS
  WebConnection* connections[ kMaxConnections ];
  uint32_t webConnCount = as->priv.webConnManager.GetConnections( connections );
#endif

  ENetPacket* enetPacket = nullptr;
  for ( int32_t i = 0; i < peerCount; i++ )
  {
    AetherPlayer* player = (AetherPlayer*)peers[ i ].data;
    if ( !player )
    {
      continue;
    }

    // Only send to players in connected state
    if ( peers[ i ].state != ENET_PEER_STATE_CONNECTED )
    {
      continue;
    }
    // Only send to player
    if ( info->player && info->player != player )
    {
      continue;
    }
    // Only send to group
    if ( info->group && ( info->group != player->userData || !player->userData ) )
    {
      continue;
    }
    // Don't send to player
    if ( info->playerFilter && info->playerFilter == player )
    {
      continue;
    }
    // Don't send to group
    if ( info->groupFilter && info->groupFilter == player->userData )
    {
      continue;
    }

    if ( !enetPacket )
    {
      enetPacket = enet_packet_create( nullptr, dataLength, flags );
      AE_ASSERT( enetPacket );
      memcpy( enetPacket->data, &header, sizeof( header ) );
      memcpy( enetPacket->data + sizeof( header ), info->data, info->length );
    }
    enet_peer_send( peers + i , channel, enetPacket );

    if ( info->player )
    {
      // Send only to specified player
      return;
    }
  }

#ifdef USE_WEBWOCKETS
  AE_ASSERT( dataLength <= kMaxMessageSize );
  uint8_t data[ kMaxMessageSize ];

  memcpy( data, &header, sizeof( header ) );
  memcpy( data + sizeof( header ), info->data, info->length );

  for ( uint32_t i = 0; i < webConnCount; i++ )
  {
    AetherPlayer* player = (AetherPlayer*)connections[ i ]->userdata;
    if ( !player )
    {
      continue;
    }

    // Only send to player
    if ( info->player && info->player != player )
    {
      continue;
    }
    // Only send to group
    if ( info->group && ( info->group != player->userData || !player->userData ) )
    {
      continue;
    }
    // Don't send to player
    if ( info->playerFilter && info->playerFilter == player )
    {
      continue;
    }
    // Don't send to group
    if ( info->groupFilter && info->groupFilter == player->userData )
    {
      continue;
    }

    connections[ i ]->Send( data, dataLength );
    
    if ( info->player )
    {
      // Send only to specified player
      return;
    }
  }
#endif
}

void AetherServer_SendAll( AetherServer* _as )
{
  AetherServerInternal* as = (AetherServerInternal*)_as;
  enet_host_flush( as->priv.host );

#ifdef USE_WEBWOCKETS
  lws_callback_on_writable_all_protocol( as->priv.webContext, as->priv.webProtocols );
#endif
}

AetherPlayer* AetherServer_GetPlayerByNetInstId( AetherServer* _as, NetInstId id )
{
  AetherServerInternal* as = (AetherServerInternal*)_as;
  for ( uint32_t i = 0; i < as->pub.playerCount; i++ )
  {
    if ( as->pub.allPlayers[ i ]->netId == id )
    {
      return as->pub.allPlayers[ i ];
    }
  }
  return nullptr;
}

uint32_t AetherServer_GetPlayerByUserData( AetherServer* as, const void* userData, AetherPlayer* (&playersOut)[ 32 ] )
{
  uint32_t count = 0;
  
  uint32_t playerCount = as->playerCount;
  for ( uint32_t i = 0; i < playerCount && count < 32; i++ )
  {
    AetherPlayer* player = as->allPlayers[ i ];
    if (  player->userData == userData && player->alive )
    {
      playersOut[ count ] = player;
      count++;
    }
  }

  return count;
}

void AetherServer_QueueBroadcast( AetherServer* as, AetherMsgId msgId, bool reliable, const void* data, uint32_t length )
{
  ServerSendInfo info;
  info.msgId = msgId;
  info.reliable = reliable;
  info.length = length;
  info.data = data;
  AetherServer_QueueSend( as, &info );
}

void AetherServer_QueueSendToPlayer( AetherServer* as, AetherPlayer* player, AetherMsgId msgId, bool reliable, const void* data, uint32_t length )
{
  AE_ASSERT( player );
  ServerSendInfo info;
  info.msgId = msgId;
  info.reliable = reliable;
  info.length = length;
  info.data = data;
  info.player = player;
  AetherServer_QueueSend( as, &info );
}

void AetherServer_QueueSendToGroup( AetherServer* as, void* group, AetherMsgId msgId, bool reliable, const void* data, uint32_t length )
{
  AE_ASSERT( group );
  ServerSendInfo info;
  info.msgId = msgId;
  info.reliable = reliable;
  info.length = length;
  info.data = data;
  info.group = group;
  AetherServer_QueueSend( as, &info );
}
