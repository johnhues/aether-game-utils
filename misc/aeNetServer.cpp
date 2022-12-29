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
#include "ae/aeNet.h"
#include <vector>
#include <algorithm>
#include <enet/enet.h>

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
#ifdef USE_WEBSOCKETS
#include <libwebsockets.h>

const uint32_t kMaxConnections = 32;
const uint32_t kConnectionBufferSize = 8192;
const uint32_t kMaxWebEvents = 2048;

enum WebConnEventType
{
  kWebConn_Connect,
  kWebConn_Receive,
  kWebConn_Disconnect
};

struct WebConnEvent
{
  WebConnEventType type;
  struct WebConnection* conn;
  void* userdata;
};

class WebConnection
{
public:
  void Initialize( uint32_t id, uint32_t sendBufferPadBytes );

  uint32_t GetId() { return m_id; }
  bool IsValid() { return m_isValid; }

  void Send( const uint8_t* data, uint32_t length );
  uint32_t Recv( uint8_t* data, uint32_t maxLength );

  bool SendBufferPeek( uint8_t** dataOut, uint32_t* lengthOut );
  void SendBufferPop( uint32_t length );
  void RecvBufferPush( const uint8_t* data, uint32_t length );

  void* userdata;

private:
  uint32_t m_id;
  bool m_isValid;

  uint8_t m_recvBuffer[ kConnectionBufferSize ];
  uint32_t m_recvCurrent;

  uint32_t m_sendPadBytes;
  uint8_t m_sendBuffer[ kConnectionBufferSize ];
  uint32_t m_sendCurrent;
};

class WebConnectionManager
{
public:
  void Initialize();
  WebConnection* AllocateConnection( uint32_t padBytes );
  bool Service( WebConnEvent* event );
  uint32_t GetConnections( WebConnection* (&conn)[ kMaxConnections ] );

private:
  void m_FreeConnection( uint32_t id );
  
  WebConnEvent m_events[ kMaxWebEvents ];
  uint32_t m_firstEvent;
  uint32_t m_eventCount;

  bool m_isActive[ kMaxConnections ];
  WebConnection m_connections[ kMaxConnections ];

public:
  WebConnection* GetConnection( uint32_t id );
  void PushEvent( const WebConnEvent& event );
};

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
#ifdef USE_WEBSOCKETS
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

#ifdef USE_WEBSOCKETS
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
#ifdef USE_WEBSOCKETS
  lws_context_destroy( as->priv.webContext );
#endif

  ENetPeer* peers = as->priv.host->peers;
  int32_t peerCount = (int32_t)as->priv.host->peerCount;
  for ( int32_t i = 0; i < peerCount; i++ )
  {
    enet_peer_disconnect( &peers[ i ], 0 );
  }
  enet_host_flush( as->priv.host );

  enet_host_destroy( as->priv.host );
  delete as;
  enet_deinitialize();
}

#ifdef USE_WEBSOCKETS
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

void AetherServer_Disconnect( AetherServer* _as, AetherPlayer* player )
{
	if ( !_as || !player )
	{
		return;
	}
	
	AetherServerInternal* as = (AetherServerInternal*)_as;
	ENetPeer* peers = as->priv.host->peers;
	int32_t peerCount = (int32_t)as->priv.host->peerCount;
	
	ENetPeer* peer = nullptr;
	for ( uint32_t i = 0; i < peerCount; i++ )
	{
		if ( player == (AetherPlayer*)peers[ i ].data )
		{
			peer = &peers[ i ];
			break;
		}
	}

	if ( peer )
	{
		enet_peer_disconnect_later( peer, 0 );
	}
}

void AetherServer_Update( AetherServer* _as )
{
  AetherServerInternal* as = (AetherServerInternal*)_as;

#ifdef USE_WEBSOCKETS
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

  as->pub.playerCount = (int32_t)as->priv.players.size();
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
        uint32_t length = (uint32_t)( e.packet->dataLength - sizeof(header) );
        
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

#ifdef USE_WEBSOCKETS
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
            infoOut->data.Clear();
            infoOut->data.Append( data, length );
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
          infoOut->data.Clear();
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
  int32_t peerCount = (int32_t)as->priv.host->peerCount;

#ifdef USE_WEBSOCKETS
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

#ifdef USE_WEBSOCKETS
  uint32_t sendSize =  sizeof(header) + info->length;
  ae::Scratch< uint8_t > data( sendSize );
  memcpy( data.Data(), &header, sizeof( header ) );
  memcpy( data.Data() + sizeof( header ), info->data, info->length );

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

    connections[ i ]->Send( data.Data(), data.Length() );
    
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

#ifdef USE_WEBSOCKETS
  lws_callback_on_writable_all_protocol( as->priv.webContext, as->priv.webProtocols );
#endif
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

#ifdef USE_WEBSOCKETS

void WebConnection::Initialize( uint32_t id, uint32_t padBytes )
{
  m_id = id;
  m_isValid = true;
  m_recvCurrent = 0;
  m_sendCurrent = 0;
  m_sendPadBytes = padBytes;

  userdata = nullptr;
}

void WebConnection::Send( const uint8_t* data, uint32_t length )
{
  if ( !m_isValid ) { return; }

  uint16_t msgLen = htons( (uint16_t)length );
  if ( m_sendPadBytes + m_sendCurrent + sizeof(msgLen) + length > kConnectionBufferSize )
  {
    m_isValid = false;
    return;
  }

  memcpy( m_sendBuffer + m_sendPadBytes + m_sendCurrent, &msgLen, sizeof(msgLen) );
  m_sendCurrent += sizeof(msgLen);
  memcpy( m_sendBuffer + m_sendPadBytes + m_sendCurrent, (uint8_t*)data, length );
  m_sendCurrent += length;
}

uint32_t WebConnection::Recv( uint8_t* data, uint32_t maxLength )
{
  if ( !m_isValid ) { return 0; }

  uint16_t msgLen;
  if ( m_recvCurrent <= sizeof(msgLen) )
  {
    return 0;
  }

  msgLen = *(uint16_t*)m_recvBuffer;
  msgLen = ntohs( msgLen );
  if ( msgLen > maxLength )
  {
    m_isValid = false;
    return 0;
  }

  if ( m_recvCurrent >= sizeof(msgLen) + msgLen )
  {
    memcpy( data, m_recvBuffer + sizeof(msgLen), msgLen );
    m_recvCurrent -= sizeof(msgLen) + msgLen;
    memmove( m_recvBuffer, m_recvBuffer + sizeof(msgLen) + msgLen, m_recvCurrent );
    return msgLen;
  }

  return 0;
}

bool WebConnection::SendBufferPeek( uint8_t** dataOut, uint32_t* lengthOut )
{
  if ( !m_isValid || m_sendCurrent == 0 )
  {
    return false;
  }

  *dataOut = m_sendBuffer + m_sendPadBytes;
  *lengthOut = m_sendCurrent;

  return true;
}

void WebConnection::SendBufferPop( uint32_t length )
{
  if ( !m_isValid ) { return; }

  if ( length > m_sendCurrent )
  {
    m_isValid = false;
    return;
  }

  m_sendCurrent -= length;
  memmove( m_sendBuffer + m_sendPadBytes, m_sendBuffer + m_sendPadBytes + length, m_sendCurrent );
}

void WebConnection::RecvBufferPush( const uint8_t* data, uint32_t length )
{
  if ( !m_isValid ) { return; }

  if ( m_recvCurrent + length > kConnectionBufferSize )
  {
    m_isValid = false;
    return;
  }

  memcpy( m_recvBuffer + m_recvCurrent, (uint8_t*)data, length );
  m_recvCurrent += length;
}

void WebConnectionManager::Initialize()
{
  m_firstEvent = 0;
  m_eventCount = 0;
  memset( m_isActive, 0, sizeof(m_isActive) );
}

WebConnection* WebConnectionManager::AllocateConnection( uint32_t padBytes )
{
  for ( uint32_t i = 0; i < kMaxConnections; i++ )
  {
    if ( !m_isActive[ i ] )
    {
      m_isActive[ i ] = true;
      m_connections[ i ].Initialize( i, padBytes );
      return &m_connections[ i ];
    }
  }
  return nullptr;
}

void WebConnectionManager::m_FreeConnection( uint32_t id )
{
  AE_ASSERT( id >= 0 && id < kMaxConnections );
  m_isActive[ id ] = false;
}

bool WebConnectionManager::Service( WebConnEvent* event )
{
  for ( uint32_t i = 0; i < kMaxConnections; i++ )
  {
    if ( m_isActive[ i ] && !m_connections[ i ].IsValid() )
    {
      WebConnEvent e;
      e.type = kWebConn_Disconnect;
      e.conn = &m_connections[ i ];
      e.userdata = m_connections[ i ].userdata;
      PushEvent( e );
    }
  }

  if ( m_eventCount )
  {
    *event = m_events[ m_firstEvent ];
    m_firstEvent++;
    m_eventCount--;

    if ( event->type == kWebConn_Disconnect )
    {
      m_FreeConnection( event->conn->GetId() );
      event->conn = nullptr;
    }

    return true;
  }
  return false;
}

WebConnection* WebConnectionManager::GetConnection( uint32_t id )
{
  if ( m_isActive[ id ] )
  {
    return &m_connections[ id ];
  }
  return nullptr;
}

uint32_t WebConnectionManager::GetConnections( WebConnection* (&connections)[ kMaxConnections ] )
{
  uint32_t result = 0;
  for ( uint32_t i = 0; i < kMaxConnections; i++ )
  {
    if ( m_isActive[ i ] )
    {
      connections[ result ] = &m_connections[ i ];
      result++;
    }
  }
  return result;
}

void WebConnectionManager::PushEvent( const WebConnEvent& event )
{
  if ( m_eventCount == 0 )
  {
    m_firstEvent = 0;
  }
  AE_ASSERT( m_firstEvent + m_eventCount < kMaxWebEvents );
  m_events[ m_firstEvent + m_eventCount ] = event;
  m_eventCount++;
}

#endif
