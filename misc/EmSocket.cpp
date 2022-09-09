//
//  EmSocket.cpp
//
//  Created by John Hughes on 07/25/16.
//  Copyright (c) 2016 John Hughes. All rights reserved.
//

#include "ae/EmSocket.h"

#ifdef __EMSCRIPTEN__
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <emscripten.h>
#endif

#ifndef __EMSCRIPTEN__

EmSocket::EmSocket() {}
EmSocket::~EmSocket() {}
void EmSocket::Initialize( uint8_t* buffer,  uint32_t length ) {}
void EmSocket::Connect( const char* address, uint16_t port ) {}
bool EmSocket::Service( float dt ) { return false; }
bool EmSocket::IsOpen() { return false; }
void EmSocket::Close() {}
void EmSocket::Send( const void* data, uint32_t length ) {}
uint32_t EmSocket::Recv( void* data, uint32_t maxLength ) { return 0; }

#else

EmSocket::EmSocket()
{
  memset( this, 0, sizeof(*this) );
  m_socket = -1;
}

EmSocket::~EmSocket()
{
  Close();
}

void EmSocket::Initialize( uint8_t* buffer,  uint32_t length )
{
  //HVN_ASSERT( !m_buffer );

  m_recvBuffer = buffer;
  m_recvLength = length / 2;
  m_sendBuffer = buffer + m_recvLength;
  m_sendLength = length - m_recvLength;
}

void EmSocket::Connect( const char* address, uint16_t port )
{
  Close();
  printf( "Connecting\n" );
  struct sockaddr_in addr;
  memset( &addr, 0, sizeof(addr) );
  addr.sin_family = AF_INET;
  addr.sin_port = htons( port );
  int result = inet_pton( AF_INET, address, &addr.sin_addr );
  printf( "%d\n", result );
  if ( result == -1 )
  {
    printf( "Error converting address %s:%hu %d\n", address, port, errno );
    return;
  }
  else if ( result == 0 )
  {
    printf( "Invalid address %s:%hu\n", address, port );
    return;
  }

  m_socket = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
  if ( m_socket == -1 )
  {
    printf( "Connect 1\n" );
    Close();
    return;
  }
  fcntl( m_socket, F_SETFL, O_NONBLOCK );

  int res = connect( m_socket, (struct sockaddr*)&addr, sizeof(addr) );
  if ( res == -1 && errno != EINPROGRESS )
  {
    printf( "Connect 2\n" );
    Close();
    return;
  }

  m_connectRetry = 5.0f;

  printf( "Connect 3\n" );
}

bool EmSocket::Service( float dt )
{
  fd_set fdr;
  fd_set fdw;
  FD_ZERO( &fdr );
  FD_ZERO( &fdw );
  FD_SET( m_socket, &fdr );
  FD_SET( m_socket, &fdw );
  
  if ( select( 64, &fdr, &fdw, NULL, NULL ) == -1 )
  {
    Close();
    return false;
  }

  bool ready = FD_ISSET( m_socket, &fdw );
  if ( ready && !m_connected )
  {
    printf( "ready\n" );
    m_connected = true;
  }
  else if ( !ready && m_connected )
  {
    printf( "disconnected\n" );
    Close();
    return false;
  }

  if ( !m_connected )
  {
    m_connectRetry -= dt;
    if ( m_connectRetry <= 0.0f )
    {
      Close();
      return false;
    }
  }

  return m_connected;
}

bool EmSocket::IsOpen()
{
  return m_socket >= 0;
}

void EmSocket::Close()
{
  printf( "close\n" );

  if ( m_socket >= 0 )
  {
    close( m_socket );
  }
  
  m_socket = -1;
  m_connected = false;
  m_connectRetry = 0.0f;
  m_recvCurrent = 0;
  m_sendCurrent = 0;
}

void EmSocket::Send( const void* data, uint32_t length )
{
  uint16_t msgLen = htons( length );
  if ( m_sendCurrent + sizeof(msgLen) + length > m_sendLength )
  {
    Close();
    return;
  }

  memcpy( m_sendBuffer + m_sendCurrent, &msgLen, sizeof(msgLen) );
  m_sendCurrent += sizeof(msgLen);
  memcpy( m_sendBuffer + m_sendCurrent, (uint8_t*)data, length );
  m_sendCurrent += length;

  int res = send( m_socket, m_sendBuffer, m_sendCurrent, 0 );
  if ( res == -1 )
  {
    if ( errno != EAGAIN )
    {
      Close();
    }
    return;
  }

  m_sendCurrent -= res;
  memmove( m_sendBuffer, m_sendBuffer + res, m_sendCurrent );
}

uint32_t EmSocket::Recv( void* data, uint32_t maxLength )
{
  int res = recvfrom( m_socket, m_recvBuffer + m_recvCurrent, m_recvLength - m_recvCurrent, 0, nullptr, nullptr );
  if ( res > 0 )
  {
    m_recvCurrent += res;
  }
  else if ( res == -1 && errno != EAGAIN )
  {
    Close();
    return 0;
  }
  else if ( res == 0 )
  {
    Close();
    return 0;
  }

  uint16_t msgLen;
  if ( m_recvCurrent <= sizeof(msgLen) )
  {
    return 0;
  }

  msgLen = *(uint16_t*)m_recvBuffer;
  msgLen = ntohs( msgLen );
  if ( msgLen > maxLength )
  {
    Close();
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

#endif
