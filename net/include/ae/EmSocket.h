//
//  EmSocket.h
//
//  Created by John Hughes on 07/25/16.
//  Copyright (c) 2016 John Hughes. All rights reserved.
//

#ifndef HVN_EMSOCKET
#define HVN_EMSOCKET

#include <cstdint>

class EmSocket
{
public:
  EmSocket();
  ~EmSocket();
  void Initialize( uint8_t* buffer,  uint32_t length );
  
  void Connect( const char* address, uint16_t port );
  bool Service( float dt );

  bool IsOpen();
  void Close();

  void Send( const void* data, uint32_t length );
  uint32_t Recv( void* data, uint32_t maxLength );

private:
  void operator=( EmSocket& ) {}
  EmSocket( EmSocket& ) {}

  int m_socket;
  bool m_connected;
  bool m_pending;
  float m_connectRetry;

  uint8_t* m_recvBuffer;
  uint32_t m_recvLength;
  uint32_t m_recvCurrent;

  uint8_t* m_sendBuffer;
  uint32_t m_sendLength;
  uint32_t m_sendCurrent;
};

#endif
