//------------------------------------------------------------------------------
// EmSocket.h
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
