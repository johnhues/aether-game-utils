//------------------------------------------------------------------------------
// Common.h
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
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "ae/aether.h"

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
inline void LoadPng( ae::Texture2D* texture, const char* file, ae::Texture::Filter filter, ae::Texture::Wrap wrap, bool autoGenerateMipmaps, bool isSRGB )
{
  uint32_t fileSize = ae::FileSystem::GetSize( file );
  AE_ASSERT_MSG( fileSize, "Could not load #", file );
  
  uint8_t* fileBuffer = (uint8_t*)malloc( fileSize );
  ae::FileSystem::Read( file, fileBuffer, fileSize );

  int32_t width = 0;
  int32_t height = 0;
  int32_t channels = 0;
  stbi_set_flip_vertically_on_load( 1 );
#if _AE_IOS_
  stbi_convert_iphone_png_to_rgb( 1 );
#endif
  bool is16BitImage = stbi_is_16_bit_from_memory( fileBuffer, fileSize );

  uint8_t* image;
  if (is16BitImage)
  {
     image = (uint8_t*)stbi_load_16_from_memory( fileBuffer, fileSize, &width, &height, &channels, STBI_default );
  }
  else
  {
    image = stbi_load_from_memory( fileBuffer, fileSize, &width, &height, &channels, STBI_default );
  }
  AE_ASSERT( image );

  ae::Texture::Format format;
  auto type = ae::Texture::Type::Uint8;
  switch ( channels )
  {
    case STBI_grey:
      format = ae::Texture::Format::R8;
        
      // for now only support R16Unorm
      if (is16BitImage)
      {
        format = ae::Texture::Format::R16_UNORM;
        type = ae::Texture::Type::Uint16;
      }
      break;
    case STBI_grey_alpha:
      AE_FAIL();
      break;
    case STBI_rgb:
      format = isSRGB ? ae::Texture::Format::RGB8_SRGB : ae::Texture::Format::RGB8;
      break;
    case STBI_rgb_alpha:
      format = isSRGB ? ae::Texture::Format::RGBA8_SRGB : ae::Texture::Format::RGBA8;
      break;
  }
  
  texture->Initialize( image, width, height, format, type, filter, wrap, autoGenerateMipmaps );
  
  stbi_image_free( image );
  free( fileBuffer );
}

//------------------------------------------------------------------------------
// SpriteRenderer example class
//------------------------------------------------------------------------------
class SpriteRenderer
{
public:
  void Initialize( uint32_t maxCount );
  void Terminate();
  void AddSprite( const ae::Matrix4& localToWorld, ae::Rect uvs, ae::Color color );
  void Clear();
  void Render( const ae::Matrix4& worldToProj, const ae::Texture2D* texture );
private:
  struct Vertex
  {
    ae::Vec4 pos;
    ae::Vec4 color;
    ae::Vec2 uv;
  };
  uint32_t m_count = 0;
  uint32_t m_maxCount = 0;
  ae::Shader m_shader;
  ae::VertexData m_vertexData;
};

//------------------------------------------------------------------------------
// SpriteRenderer member functions
//------------------------------------------------------------------------------
void SpriteRenderer::Initialize( uint32_t maxCount )
{
  m_maxCount = maxCount;
  m_count = 0;
  
  const char* vertShader = R"(
    AE_UNIFORM mat4 u_worldToProj;

    AE_IN_HIGHP vec4 a_position;
    AE_IN_HIGHP vec4 a_color;
    AE_IN_HIGHP vec2 a_uv;

    AE_OUT_HIGHP vec4 v_color;
    AE_OUT_HIGHP vec2 v_uv;

    void main()
    {
      v_color = a_color;
      v_uv = a_uv;
      gl_Position = u_worldToProj * a_position;
    }
  )";
  const char* fragShader = R"(
    AE_UNIFORM sampler2D u_tex;

    AE_IN_HIGHP vec4 v_color;
    AE_IN_HIGHP vec2 v_uv;

    void main()
    {
      AE_COLOR = AE_TEXTURE2D( u_tex, v_uv ) * v_color;
    }
  )";
  m_shader.Initialize( vertShader, fragShader, nullptr, 0 );
  m_shader.SetBlending( true );
  
  m_vertexData.Initialize( sizeof(Vertex), 2, 4 * m_maxCount, 6 * m_maxCount, ae::VertexData::Primitive::Triangle, ae::VertexData::Usage::Dynamic, ae::VertexData::Usage::Dynamic );
  m_vertexData.AddAttribute( "a_position", 4, ae::VertexData::Type::Float, offsetof(Vertex, pos) );
  m_vertexData.AddAttribute( "a_color", 4, ae::VertexData::Type::Float, offsetof(Vertex, color) );
  m_vertexData.AddAttribute( "a_uv", 2, ae::VertexData::Type::Float, offsetof(Vertex, uv) );
}

void SpriteRenderer::Terminate()
{
  m_count = 0;
  m_maxCount = 0;
  m_vertexData.Terminate();
  m_shader.Terminate();
}

void SpriteRenderer::AddSprite( const ae::Matrix4& localToWorld, ae::Rect uvs, ae::Color color )
{
  if ( m_count >= m_maxCount )
  {
    return;
  }
  
  const uint16_t indices[] = { 3, 0, 1, 3, 1, 2 };
  m_vertexData.AppendIndices( indices, countof(indices), m_vertexData.GetVertexCount() );

  ae::Vec2 min = uvs.GetMin();
  ae::Vec2 max = uvs.GetMax();
  Vertex verts[] =
  {
    { localToWorld * ae::Vec4( -0.5f, -0.5f, 0.0f, 1.0f ), color.GetLinearRGBA(), ae::Vec2( min.x, min.y ) },
    { localToWorld * ae::Vec4( 0.5f, -0.5f, 0.0f, 1.0f ), color.GetLinearRGBA(), ae::Vec2( max.x, min.y ) },
    { localToWorld * ae::Vec4( 0.5f, 0.5f, 0.0f, 1.0f ), color.GetLinearRGBA(), ae::Vec2( max.x, max.y ) },
    { localToWorld * ae::Vec4( -0.5f, 0.5f, 0.0f, 1.0f ), color.GetLinearRGBA(), ae::Vec2( min.x, max.y ) }
  };
  m_vertexData.AppendVertices( verts, countof(verts) );
  
  m_count++;
}

void SpriteRenderer::Clear()
{
  m_vertexData.ClearVertices();
  m_vertexData.ClearIndices();
  m_count = 0;
}

void SpriteRenderer::Render( const ae::Matrix4& worldToProj, const ae::Texture2D* texture )
{
  if ( !m_count )
  {
    return;
  }
  ae::UniformList uniforms;
  uniforms.Set( "u_worldToProj", worldToProj );
  uniforms.Set( "u_tex", texture );
  m_vertexData.Upload();
  m_vertexData.Render( &m_shader, uniforms, 0, m_count * 2 );
}


namespace ae
{

class ListenerSocket; // Forward decl for ae::Socket
//------------------------------------------------------------------------------
// ae::Socket class
//------------------------------------------------------------------------------

class Socket
{
public:
  enum class Protocol { None, TCP, UDP };

  Socket( ae::Tag tag );
  ~Socket();

  //! Attempts to connect to the 'address' over the given protocol. Calling ae::Socket::Connect() clears
  //! all pending sent and received data. To avoid losing received data call ae::Socket::ReceiveData() or
  //! ae::Socket::ReceiveMsg() repeatedly until they return empty before calling ae::Socket::Connect().
  //! In this scenario all pending sent data will always be lost. @TODO: Calling ae::Socket::Connect() on a
  //! socket returned with ae::ListenerSocket::Accept().
  bool Connect( ae::Socket::Protocol proto, const char* address, const char* port );
  //! Closes ths connection established with ae::Socket::Connect().
  void Disconnect();
  //! Returns true if the connection established with ae::Socket::Connect() or ae::ListenerSocket::Accept() is
  //! still active. If this ae::Socket was returned from ae::ListenerSocket::Accept() then
  //! ae::ListenerSocket::Destroy() or ae::ListenerSocket::DestroyAll() should be called to clean it up.
  //! This can return false while received data is still waiting to be read, and so ae::Socket::ReceiveData()
  //! or ae::Socket::ReceiveMsg() can still be called.
  bool IsConnected() const;

  //! Queues 'length' data to be sent with ae::Socket::SendAll(). Call ae::Socket::QueueData() multiple times
  //! to batch data sent with ae::Socket::SendAll(). Data sent with ae::Socket::QueueData() can be read by
  //! the receiver with ae::Socket::PeekData() and ae::Socket::ReceiveData(). It's advised that you do not
  //! mix ae::Socket::QueueMsg() and ae::Socket::QueueData().
  void QueueData( const void* data, uint32_t length );
  //! Returns true if 'length' bytes have been received. If 'dataOut' is non-null and 'length' bytes have been
  //! received the data will be written to 'dataOut'. The read head will not move, so subsequent calls to
  //! ae::Socket::PeekData() will return the same result. It's useful to call ae::Socket::DiscardData() and pass
  //! it 'length' after receiving data through ae::Socket::PeekData().
  bool PeekData( void* dataOut, uint16_t length, uint32_t offset );
  //! Returns true if 'length' + 'offset' bytes have been received. If 'dataOut' is also non-null, pending received
  //! data at 'offset' will be written to 'dataOut'. In this case the read head will move forward 'length' bytes.
  //! Calling ae::Socket::ReceiveData() with a null 'dataOut' and calling ae::Socket::DiscardData() has the
  //! exact same effect.
  bool ReceiveData( void* dataOut, uint16_t length );
  //! Returns true if 'length' bytes have been received. In this case the read head will move forward 'length' bytes.
  //! Calling ae::Socket::DiscardData() and calling ae::Socket::ReceiveData() with a null 'dataOut' has
  //! the exact same effect.
  bool DiscardData( uint16_t length );

  //! Queues data for sending. A two byte (network order) message header is prepended to the given
  //! message. Ideally you should call ae::Socket::QueueMsg() for each logical chunk of data you need to
  //! send over a 'network tick' and then finally call ae::Socket::SendAll() once. It's unadvised to mix
  //! ae::Socket::QueueMsg() calls with ae::Socket::QueueData().
  void QueueMsg( const void* data, uint16_t length );
  //! Can return a value greater than maxLength, in which case 'dataOut' is not modified.
  //! Call ae::Socket::ReceiveMessage() again with a big enough buffer or skip the message by calling
  //! ae::Socket::DiscardMessage(). Uses a two byte (network order) message header.
  //! It's unadvised to mix ae::Socket::ReceiveMsg() calls with ae::Socket::ReceiveData().
  uint16_t ReceiveMsg( void* dataOut, uint16_t maxLength );
  //! Discards one received sent with ae::Socket::QueueMsg(). Uses the two byte (network order) message
  //! header to determine discard data size.
  bool DiscardMsg();

  //! Returns the number of bytes sent. Sends all queued data from ae::Socket::QueueData() and
  //! ae::Socket::QueueMsg(). If the connection is lost all pending sent data will be discarded.
  //! See ae::Socket::Connect() for more information.
  uint32_t SendAll();

private:
  bool m_SocketSetup();
  int m_sock = 0;
  Protocol m_protocol = Protocol::None;
  uint32_t m_readHead = 0;
  ae::Array< uint8_t > m_sendData;
  ae::Array< uint8_t > m_recvData;
public: // Internal
  Socket( ae::Tag tag, int s, Protocol proto );
};

//------------------------------------------------------------------------------
// ae::ListenerSocket class
//------------------------------------------------------------------------------
class ListenerSocket
{
public:
  ListenerSocket( ae::Tag tag );
  ~ListenerSocket();

  bool Listen( ae::Socket::Protocol proto, const char* port, uint32_t maxConnections );
  bool IsListening() const;
  ae::Socket* Accept(); //!< Returns a socket if a connection has been established.
  void Reset(); //!< Disconnects all connections and stops listening. Must still call Destroy() or DestroyAll() on existing connections.
  void Destroy( ae::Socket* sock ); //!< Disconnects and releases an existing socket from Accept().
  void DestroyAll(); //!< Disconnects and releases all existing sockets from Accept(). It is not safe to access released sockets after calling this.

  ae::Socket* GetConnection( uint32_t idx );
  uint32_t GetConnectionCount() const;

private:
  ae::Tag m_tag;
  int m_sock = 0;
  ae::Socket::Protocol m_protocol = ae::Socket::Protocol::None;
  ae::Array< ae::Socket* > m_connections;
};

}

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#if _AE_WINDOWS_
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>
#endif

namespace ae
{

//------------------------------------------------------------------------------
// ae::Socket member functions
//------------------------------------------------------------------------------
Socket::Socket( ae::Tag tag ) :
  m_sendData( tag ),
  m_recvData( tag )
{}

Socket::Socket( ae::Tag tag, int s, Protocol proto ) :
  m_sock( s ),
  m_protocol( proto ),
  m_sendData( tag ),
  m_recvData( tag )
{
  if ( !m_SocketSetup() )
  {
    Disconnect();
  }
}

Socket::~Socket()
{
  Disconnect();
}

bool Socket::Connect( ae::Socket::Protocol proto, const char* address, const char* port )
{
  m_readHead = 0;
  m_sendData.Clear();
  m_recvData.Clear();
  
  if ( proto == Protocol::None )
  {
    return false;
  }
  m_protocol = proto;
  
#if _AE_WINDOWS_
  WSADATA wsaData;
  if ( WSAStartup( MAKEWORD( 1, 1 ), &wsaData ) != 0 )
  {
    return false;
  }
#endif

  addrinfo hints;
  memset( &hints, 0, sizeof hints );
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = ( proto == ae::Socket::Protocol::TCP ) ? SOCK_STREAM : SOCK_DGRAM;

  addrinfo* servinfo = nullptr;
  if ( int result = getaddrinfo( address, port, &hints, &servinfo ) )
  {
    EAI_AGAIN;
    AE_INFO( "result #", result );
    return false;
  }

  addrinfo* p;
  for ( p = servinfo; p; p = p->ai_next )
  {
    if ( ( m_sock = socket( p->ai_family, p->ai_socktype, p->ai_protocol ) ) == -1 )
    {
      continue;
    }

    if ( connect( m_sock, p->ai_addr, p->ai_addrlen ) != -1 )
    {
      break; // Success!
    }

    Disconnect();
  }

  if ( !p )
  {
    return false;
  }

  freeaddrinfo( servinfo );

  if ( !m_SocketSetup() )
  {
    Disconnect();
    return false;
  }

  return true;
}

bool Socket::m_SocketSetup()
{
// Disable blocking
#if _AE_WINDOWS_
  u_long mode = 1;
  if ( ioctlsocket( m_sock, FIONBIO, &mode ) )
#else
  if ( fcntl( m_sock, F_SETFL, O_NONBLOCK ) )
#endif
  {
    return false;
  }

  // Disable Naggles algorithm
#if _AE_WINDOWS_
  const char* yes = "1";
  socklen_t optlen = 1;
#else
  int yesValue = 1;
  int* yes = &yesValue;
  socklen_t optlen = sizeof(int);
#endif
  if ( setsockopt( m_sock, SOL_SOCKET, TCP_NODELAY, yes, optlen ) == -1 )
  {
    return false;
  }

  return true;
}

void Socket::Disconnect()
{
  if ( !IsConnected() )
  {
    AE_ASSERT( m_protocol == Protocol::None );
    return;
  }
#if _AE_WINDOWS_
  closesocket( m_sock );
#else
  close( m_sock );
#endif
  // @NOTE: Do not modify buffers here, Connect() will perform actual cleanup
  m_sock = 0;
  m_protocol = Protocol::None;
}

bool Socket::IsConnected() const
{
  return ( m_sock != 0 );
}

void Socket::QueueData( const void* data, uint32_t length )
{
  if ( !IsConnected() )
  {
    return;
  }
  m_sendData.Append( (const uint8_t*)data, length );
}

bool Socket::PeekData( void* dataOut, uint16_t length, uint32_t offset )
{
  if ( !length )
  {
    return false;
  }
  
  while ( IsConnected() && m_recvData.Length() < m_readHead + offset + length )
  {
    uint32_t readSize = 0;
    if ( m_protocol == Protocol::TCP )
    {
      int availableBytes = 0;
      if ( ioctl( m_sock, FIONREAD, &availableBytes ) == 0 )
      {
        readSize = availableBytes;
      }
    }
    else if ( m_protocol == Protocol::UDP )
    {
      AE_FAIL_MSG( "Not implemented" );
      // Get full datagram size
      //int buflen = recv(sockfd, NULL, 0, MSG_PEEK | MSG_TRUNC);
      //if (buflen < 0)
      //{
      //  // handle error
      //  return;
      //}
      //uint8_t buf[ buflen ];
      //rxlen = recv(sockfd, buf, buflen, 0);
      //if (rxlen < 0)
      //{
      //  // again, handle error
      //  return;
      //}
    }
    else
    {
      AE_FAIL_MSG( "Invalid protocol" );
    }

    if ( !readSize )
    {
      // Check for closed connection
      if ( m_protocol == Protocol::TCP )
      {
        uint8_t buffer;
        int result = recv( m_sock, &buffer, 1, MSG_PEEK );
        if ( !result || ( result == -1 && errno != EWOULDBLOCK && errno != EAGAIN ) )
        {
          Disconnect(); // Orderly shutdown
        }
      }
      return false;
    }

    AE_ASSERT( readSize );
    uint32_t totalSize = m_recvData.Length() + readSize;
    m_recvData.Reserve( totalSize );
    uint8_t* buffer = m_recvData.End();
    while ( m_recvData.Length() < totalSize ) { m_recvData.Append( {} ); } // @TODO: Should be single function call
    AE_ASSERT( buffer == m_recvData.End() - readSize );
    
    int result = recv( m_sock, buffer, readSize, 0 );
    if ( result < 0 && ( errno == EWOULDBLOCK || errno == EAGAIN ) )
    {
      return false;
    }
    else if ( result == 0 && m_protocol == Protocol::TCP )
    {
      Disconnect(); // Orderly shutdown
      return false;
    }
    else if ( result != readSize )
    {
      Disconnect();
      return false;
    }
    else if ( result )
    {
      break; // Received new data!
    }
  }
  
  if ( m_recvData.Length() >= m_readHead + offset + length )
  {
    if ( dataOut )
    {
      memcpy( dataOut, m_recvData.Begin() + m_readHead + offset, length );
    }
    return true;
  }
  return false;
}

bool Socket::ReceiveData( void* dataOut, uint16_t length )
{
  if ( PeekData( dataOut, length, 0 ) )
  {
    DiscardData( length );
  }
  return false;
}

bool Socket::DiscardData( uint16_t length )
{
  if ( m_readHead + length >= m_recvData.Length() )
  {
    m_readHead += length;
    if ( m_readHead == m_recvData.Length() )
    {
      m_recvData.Clear();
      m_readHead = 0;
    }
    return true;
  }
  return false;
}

void Socket::QueueMsg( const void* data, uint16_t length )
{
  if ( !IsConnected() || !length )
  {
    return;
  }
  AE_ASSERT( length <= ae::MaxValue< uint16_t >() );
  uint16_t length16 = htons( length );
  m_sendData.Append( (const uint8_t*)&length16, sizeof(length16) );
  m_sendData.Append( (const uint8_t*)data, length );
}

uint16_t Socket::ReceiveMsg( void* dataOut, uint16_t maxLength )
{
  uint16_t length = 0;
  if ( PeekData( &length, sizeof(length), 0 ) )
  {
    length = ntohs( length );
    if ( length > maxLength )
    {
      return length;
    }
    else if ( PeekData( dataOut, length, 2 ) )
    {
      DiscardData( length + 2 );
      return length;
    }
  }
  return 0;
}

bool Socket::DiscardMsg()
{
  uint16_t length = 0;
  if ( PeekData( &length, sizeof(length), 0 ) )
  {
    length = ntohs( length );
    if ( PeekData( nullptr, length, 2 ) )
    {
      DiscardData( length + 2 );
      return true;
    }
  }
  return false;
}

uint32_t Socket::SendAll()
{
  if ( !IsConnected() || !m_sendData.Length() )
  {
    return 0;
  }
  
  int result = send( m_sock, m_sendData.Begin(), m_sendData.Length(), 0 );
  if ( result == -1 && errno != EAGAIN && errno != EWOULDBLOCK )
  {
    Disconnect();
    return 0;
  }
  else
  {
    AE_ASSERT( m_sendData.Length() == result );
    m_sendData.Clear();
    return result;
  }
  return 0;
}

//------------------------------------------------------------------------------
// ae::ListenerSocket member functions
//------------------------------------------------------------------------------
ListenerSocket::ListenerSocket( ae::Tag tag ) :
  m_tag( tag ),
  m_connections( tag )
{}

ListenerSocket::~ListenerSocket()
{
  AE_ASSERT_MSG( !m_connections.Length(), "Allocated connections must be destroyed before ae::ListenerSocket destruction" );
  Reset();
}

bool ListenerSocket::Listen( ae::Socket::Protocol proto, const char* port, uint32_t maxConnections )
{
  if ( proto == ae::Socket::Protocol::None )
  {
    return false;
  }
  
#if _AE_WINDOWS_
  WSADATA wsaData;
  if ( WSAStartup( MAKEWORD( 1, 1 ), &wsaData ) != 0 )
  {
    return false;
  }
#endif

  addrinfo hints;
  memset( &hints, 0, sizeof(hints) );
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = ( proto == ae::Socket::Protocol::TCP ) ? SOCK_STREAM : SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

  addrinfo* servinfo;
  if ( getaddrinfo( "localhost", port, &hints, &servinfo ) )
  {
    AE_ASSERT( m_sock == 0 && m_protocol == ae::Socket::Protocol::None );
    return false;
  }

  addrinfo* p = nullptr;
  for ( p = servinfo; p; p = p->ai_next )
  {
    m_sock = socket( p->ai_family, p->ai_socktype, p->ai_protocol );
    if ( m_sock == -1 )
    {
      continue;
    }

#if _AE_WINDOWS_
    const char* yes = "1";
    socklen_t optlen = 1;
#else
    int yesValue = 1;
    int* yes = &yesValue;
    socklen_t optlen = sizeof(int);
#endif
    if ( setsockopt( m_sock, SOL_SOCKET, SO_REUSEADDR, yes, optlen ) == -1 )
    {
      m_sock = 0;
      AE_ASSERT( m_protocol == ae::Socket::Protocol::None );
      return false;
    }

    if ( bind( m_sock, p->ai_addr, p->ai_addrlen ) != -1 )
    {
      break; // Success!
    }

#if _AE_WINDOWS_
    closesocket( m_sock );
#else
    close( m_sock );
#endif
    m_sock = 0;
  }

  freeaddrinfo( servinfo );

  if ( !p )
  {
    AE_ASSERT( m_sock == 0 && m_protocol == ae::Socket::Protocol::None );
    return false;
  }

  int backlog = 10;
  if ( listen( m_sock, backlog ) == -1 )
  {
    m_sock = 0;
    AE_ASSERT( m_protocol == ae::Socket::Protocol::None );
    return false;
  }

  // Disable blocking
#if _AE_WINDOWS_
  u_long mode = 1;
  ioctlsocket( m_sock, FIONBIO, &mode );
#else
  fcntl( m_sock, F_SETFL, O_NONBLOCK );
#endif

  m_protocol = proto;
  return true;
}

bool ListenerSocket::IsListening() const
{
  if ( m_sock )
  {
    return true;
  }
  AE_ASSERT( m_protocol == ae::Socket::Protocol::None );
  return false;
}

ae::Socket* ListenerSocket::Accept()
{
  if ( !m_sock )
  {
    AE_ASSERT( m_protocol == ae::Socket::Protocol::None );
    return nullptr;
  }
  
  sockaddr_storage their_addr;
  socklen_t sin_size = sizeof(their_addr);
  int new_fd = accept( m_sock, (sockaddr*)&their_addr, &sin_size );
  if ( new_fd == -1 )
  {
    return nullptr;
  }

  // Disable blocking
#if _AE_WINDOWS_
  u_long mode = 1;
  ioctlsocket( new_fd, FIONBIO, &mode );
#else
  fcntl( m_sock, F_SETFL, O_NONBLOCK );
#endif

  AE_ASSERT( m_protocol != ae::Socket::Protocol::None );
  ae::Socket* newSock = ae::New< ae::Socket >( m_tag, m_tag, new_fd, m_protocol );
  m_connections.Append( newSock );
  return newSock;
}

void ListenerSocket::Reset()
{
  for ( ae::Socket* sock : m_connections )
  {
    sock->Disconnect();
  }
#if _AE_WINDOWS_
  closesocket( m_sock );
#else
  close( m_sock );
#endif
  m_sock = 0;
  m_protocol = ae::Socket::Protocol::None;
}

void ListenerSocket::Destroy( ae::Socket* sock )
{
  ae::Delete( sock );
  m_connections.RemoveAll( sock );
}

void ListenerSocket::DestroyAll()
{
  for ( ae::Socket* sock : m_connections )
  {
    ae::Delete( sock );
  }
  m_connections.Clear();
}

ae::Socket* ListenerSocket::GetConnection( uint32_t idx )
{
  return m_connections[ idx ];
}

uint32_t ListenerSocket::GetConnectionCount() const
{
  return m_connections.Length();
}

}
