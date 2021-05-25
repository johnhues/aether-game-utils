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
#include <inttypes.h>
#include <vector>
#include "aeUuid.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include "EmSocket.h"
#else
#include <enet/enet.h>
#endif

//------------------------------------------------------------------------------
// AetherUuid member functions
//------------------------------------------------------------------------------
AetherUuid::AetherUuid( const char* str )
{
#define _aescn8 "%2" SCNx8
  sscanf( str, _aescn8 _aescn8 _aescn8 _aescn8 "-"
    _aescn8 _aescn8 "-" _aescn8 _aescn8 "-" _aescn8 _aescn8 "-"
    _aescn8 _aescn8 _aescn8 _aescn8 _aescn8 _aescn8,
    &uuid[0], &uuid[1], &uuid[2], &uuid[3],
    &uuid[4], &uuid[5], &uuid[6], &uuid[7], &uuid[8], &uuid[9],
    &uuid[10], &uuid[11], &uuid[12], &uuid[13], &uuid[14], &uuid[15] );
#undef _aescn8
}

AetherUuid AetherUuid::Generate()
{
  aeUuidGenerator gen;
  aeUuid localUuid = gen.newGuid(); // @TODO: Should match naming convention
  AE_ASSERT( localUuid._bytes.size() == 16 );

  AetherUuid result;
  memcpy( result.uuid, localUuid._bytes.data(), 16 );
  return result;
}

AetherUuid AetherUuid::Zero()
{
  AetherUuid result;
  memset( &result, 0, sizeof(result) );
  return result;
}

void AetherUuid::ToString( char* str, uint32_t max ) const
{
  AE_ASSERT( max >= 37 );
  sprintf( str, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x", 
    uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
    uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15] );
}

std::ostream& operator<<( std::ostream& os, const AetherUuid& uuid )
{
  char str[ 64 ];
  uuid.ToString( str, countof(str) );
  return os << str;
}

bool AetherAddress::IsLocalhost() const
{
    if ( strcmp(host, "localhost") == 0 || strcmp(host, "127.0.0.1") == 0 || strcmp(host, "::1") == 0 )
    {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// AetherClient member functions
//------------------------------------------------------------------------------
namespace
{
  struct AetherClientInternal
  {
    AetherClient pub;
    struct
    {
#ifdef __EMSCRIPTEN__
      EmSocket sock;
      uint8_t connBuffer[ 2048 ];
      double prevTime;
#else
      ENetHost* host;
#endif
    } priv;
  };
}

void AetherClient_QueueSend( AetherClient* _ac, const SendInfo* info );
	
AetherPlayer* AetherClient_GetPlayer( AetherClientInternal* ac, AetherUuid uuid )
{
  int32_t playerCount = ac->pub.allPlayers.Length();
  for ( int32_t i = 0; i < playerCount; i++ )
  {
    AetherPlayer* p = ac->pub.allPlayers[ i ];
    if ( memcmp( &p->uuid, &uuid, sizeof(uuid) ) == 0 ) { return p; }
  }
  
  AE_FAIL();
  return nullptr;
}

AetherPlayer* AetherClient_AddPlayer( AetherClientInternal* ac, AetherUuid uuid )
{
  AetherPlayer* player = ae::New< AetherPlayer >( AE_ALLOC_TAG_NET );
  player->uuid = uuid;
  player->netId = 0;
  player->userData = nullptr;
  player->alive = true;
  player->pendingLevel = "";
  player->pendingLink = "";
  player->hasPendingLevelChange = false;
  
  ac->pub.allPlayers.Append( player );
  
  return player;
}

void AetherClient_Connect( AetherClient* _ac )
{
  AetherClientInternal* ac = (AetherClientInternal*)_ac;

  if ( ac->pub.IsConnected() || ac->pub.IsConnecting() )
  {
    return;
  }

#ifdef __EMSCRIPTEN__
  ac->priv.sock.Connect( ac->pub.serverAddress.host, ac->pub.serverAddress.port );
#else
  ENetAddress address;
  enet_address_set_host( &address, ac->pub.serverAddress.host );
  address.port = ac->pub.serverAddress.port;
  enet_host_connect( ac->priv.host, &address, 2, 0 );
#endif

  ac->pub.isConnected = false;
  ac->pub.m_isConnecting = true;
}

AetherClient* AetherClient_New( AetherUuid uuid, const char* ip, uint16_t port )
{
  AetherClientInternal* ac = new AetherClientInternal();

#ifdef __EMSCRIPTEN__
  ac->priv.sock.Initialize( ac->priv.connBuffer, sizeof(ac->priv.connBuffer) );
  ac->priv.prevTime = 0.0;
#else
  enet_initialize();
  ac->priv.host = enet_host_create( nullptr, 1, 2, 0, 0 );
  AE_ASSERT( ac->priv.host );
#endif
  
  ac->pub.localPlayer = AetherClient_AddPlayer( ac, uuid );
  
  ac->pub.isConnected = false;
  ac->pub.m_isConnecting = false;
  
  strcpy( ac->pub.serverAddress.host, ip );
  ac->pub.serverAddress.port = port;
  
  return (AetherClient*)ac;
}

void AetherClient_Delete( AetherClient* _ac )
{
  AetherClientInternal* ac = (AetherClientInternal*)_ac;
  if ( !ac )
  {
    return;
  }

  for ( uint32_t i = 0; i < ac->pub.allPlayers.Length(); i++ )
  {
    ae::Delete( ac->pub.allPlayers[ i ] );
  }
  ac->pub.allPlayers.Clear();

#ifndef __EMSCRIPTEN__
  ENetPeer* peer = ac->priv.host->peerCount ? ac->priv.host->peers : nullptr;
  if ( peer && peer->state == ENET_PEER_STATE_CONNECTED )
  {
    // @TODO: Make sure all queued packets are sent
    enet_peer_disconnect( ac->priv.host->peers, 0 );
    enet_host_flush( ac->priv.host );
  }
  
  enet_host_destroy( ac->priv.host );
  enet_deinitialize();
#endif

  delete ac;
}

bool AetherClient_SystemReceive( AetherClientInternal* ac, AetherServerHeader header, const uint8_t* data, int32_t length, ReceiveInfo* infoOut )
{
  switch ( header.msgId )
  {
    case kSysMsgPlayerConnect:
    {
      AetherMsgConnect msg;
      AE_ASSERT( sizeof(AetherMsgConnect) == length );
      memcpy( &msg, data, length );
      
      AetherClient_AddPlayer( ac, msg.uuid );
      infoOut->msgId = kSysMsgPlayerConnect;
      return true;
    }
    default:
    {
      AE_FAIL();
      return false;
    }
  }
}

bool AetherClient_Receive( AetherClient* _ac, ReceiveInfo* infoOut )
{
  AetherClientInternal* ac = (AetherClientInternal*)_ac;
  
#ifdef __EMSCRIPTEN__

  double t = emscripten_get_now();
  float dt = ( t - ac->priv.prevTime ) / 1000.0;
  ac->priv.prevTime = t;

  EmSocket* s = &ac->priv.sock;
  if ( !s->IsOpen() )
  {
    ac->pub.isConnected = false;
    ac->pub.m_isConnecting = false;
    return false;
  }
  else if ( s->Service( dt ) )
  {
    if ( ac->pub.m_isConnecting )
    {
      ac->pub.isConnected = true;
      ac->pub.m_isConnecting = false;

      // AE_LOG( "EMSCRIPTEN Connect" );
      
      AetherMsgConnect msg;
      msg.uuid = ac->pub.localPlayer->uuid;
      
      SendInfo info;
      info.msgId = kSysMsgPlayerConnect;
      info.length = sizeof(AetherMsgConnect);
      memcpy( info.data, &msg, info.length );
      info.reliable = true;
      AetherClient_QueueSend( _ac, &info );
      
      infoOut->msgId = kSysMsgServerConnect;
      // infoOut->player = ac->pub.localPlayer;
      infoOut->length = 0;
      
      return true;
    }

    uint8_t msg[ kMaxMessageSize ];
    while ( 1 )
    {
      uint32_t msgLength = s->Recv( msg, sizeof(msg) );
      if ( msgLength == 0 )
      {
        break;
      }

      AetherServerHeader header = *(AetherServerHeader*)msg;
      uint8_t* data = msg + sizeof(header);
      uint32_t length = msgLength - sizeof(header);

      // AE_LOG( "server seq %u", header.msgSeq );
      
      if ( header.msgId & kSysMsgMask )
      {
        if ( AetherClient_SystemReceive( ac, header, data, length, infoOut ) )
        {
          return true;
        }
      }
      else
      {
        infoOut->msgId = header.msgId;
        // infoOut->player = nullptr;
        infoOut->length = length;
        memcpy( infoOut->data, data, length );
        return true;
      }
    }
  }

#else

  ENetEvent e;
  memset( &e, 0, sizeof(e) );
  while ( enet_host_service( ac->priv.host, &e, 0 ) > 0 )
  {
    switch( e.type )
    {
      case ENET_EVENT_TYPE_CONNECT:
      {
        // AE_LOG( "ENET Connect" );
        
        AetherMsgConnect msg;
        msg.uuid = ac->pub.localPlayer->uuid;
        
        SendInfo info;
        info.msgId = kSysMsgPlayerConnect;
        info.length = sizeof( msg );
        info.data = &msg;
        info.reliable = true;
        AetherClient_QueueSend( _ac, &info );
        
        ac->pub.isConnected = true;
        ac->pub.m_isConnecting = false;
        
        infoOut->msgId = kSysMsgServerConnect;
        
        return true;
      }
      case ENET_EVENT_TYPE_RECEIVE:
      {
        AetherServerHeader header = *(AetherServerHeader*)e.packet->data;
        uint8_t* data = e.packet->data + sizeof(header);
        uint32_t length = (uint32_t)e.packet->dataLength - sizeof(header);
        
        bool success = false;
        if ( header.msgId & kSysMsgMask )
        {
          if ( AetherClient_SystemReceive( ac, header, data, length, infoOut ) )
          {
            success = true;
          }
        }
        else
        {
          infoOut->msgId = header.msgId;
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
        if ( ac->pub.isConnected )
        {
          // AE_LOG( "ENET Disconnect" );

          AE_ASSERT( ac->pub.allPlayers.Length() );
          AE_ASSERT( ac->pub.allPlayers[ 0 ] == ac->pub.localPlayer );

          for ( uint32_t i = ac->pub.allPlayers.Length() - 1; i > 1; i-- )
          {
            ae::Delete( ac->pub.allPlayers[ i ] );
            ac->pub.allPlayers.Remove( i );
          }
          
          ac->pub.isConnected = false;
          ac->pub.m_isConnecting = false;
          
          infoOut->msgId = kSysMsgServerDisconnect;

          AE_ASSERT( ac->pub.allPlayers.Length() == 1 );
          AE_ASSERT( ac->pub.allPlayers[ 0 ] == ac->pub.localPlayer );
          
          return true;
        }
        else
        {
          // AE_LOG( "ENET Could not connect to Aether" );
          ac->pub.m_isConnecting = false;

          break;
        }
      }
      default:
      {
        AE_FAIL();
        break;
      }
    }
  }

#endif
  
  return false;
}

void AetherClient_QueueSend( AetherClient* _ac, const SendInfo* info )
{
  AE_ASSERT( info->msgId != kInvalidAetherMsgId );

  AetherClientInternal* ac = (AetherClientInternal*)_ac;
#ifdef __EMSCRIPTEN__
  if ( !ac->pub.isConnected )
  {
    return;
  }
#else
  ENetPeer* peer = ac->priv.host->peers;
  if ( peer->state != ENET_PEER_STATE_CONNECTED )
  {
    return;
  }
#endif

	uint32_t dataLength = sizeof(AetherClientHeader) + info->length;
	
	AetherClientHeader header;
	header.msgId = info->msgId;
	header.uuid = ac->pub.localPlayer->uuid;
	
#ifdef __EMSCRIPTEN__
//	AE_ASSERT( dataLength <= kMaxMessageSize);
//	uint8_t data[ kMaxMessageSize ];
	memcpy( data, &header, sizeof(header) );
	memcpy( data + sizeof(AetherClientHeader), info->data, info->length );

  ac->priv.sock.Send( data, dataLength );
#else
  bool reliable = info->reliable;
  uint32_t flags = reliable ? ENET_PACKET_FLAG_RELIABLE : 0;
  int32_t channel = reliable ? kNetChannelReliable : kNetChannelUnreliable;
  ENetPacket* p = enet_packet_create( nullptr, dataLength, flags );

  memcpy( p->data, &header, sizeof(header) );
  memcpy( p->data + sizeof(AetherClientHeader), info->data, info->length );

  enet_peer_send( peer, channel, p );
#endif
}

void AetherClient_SendAll( AetherClient* _ac )
{
#ifndef __EMSCRIPTEN__
  AetherClientInternal* ac = (AetherClientInternal*)_ac;
  enet_host_flush( ac->priv.host );
#endif
}

void AetherClient_QueueSend( AetherClient* ac, AetherMsgId msgId, bool reliable, const void* data, uint32_t length )
{
  SendInfo info;
  info.msgId = msgId;
  info.reliable = reliable;
  info.data = data;
  info.length = length;
  AetherClient_QueueSend( ac, &info );
}
