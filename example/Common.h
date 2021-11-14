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
  enum class Protocol { TCP, UDP };

  Socket();
  ~Socket();

  //! Attempts to connect to the given address over the given protocol. When
  //! re-connecting this clears any pending sent or received data. To avoid
  //! losing received data call ae::Socket::ReceiveData() or ae::Socket::ReceiveMsg()
  //! repeatedly until they return empty before calling ae::Socket::Connect().
  //! In this scenario pending sent data will be lost. @TODO: Calling ae::Socket::Connect
  //! on a socket returned with ae::ListenerSocket::Accept().
  bool Connect( ae::Socket::Protocol proto, const char* address, const char* port );
  //! Closes ths connection established with ae::Socket::Connect().
  void Disconnect();
  //! Returns true if the connection established with ae::Socket::Connect() or
  //! ae::ListenerSocket::Accept() is still active. If this ae::Socket was returned
  //! from ae::ListenerSocket::Accept() then ae::ListenerSocket::Destroy() or
  //! ae::ListenerSocket::DestroyAll() should be called to clean it up. This can
  //! return false while received data is still waiting to be read, and so
  //! ae::Socket::ReceiveData() or ae::Socket::ReceiveMsg() can still be called.
  bool IsConnected() const;
    
  //! Queues 'length' data to be sent with ae::Socket::SendAll(). Call
  //! ae::Socket::QueueData() multiple times to batch data sent with
  //! ae::Socket::SendAll(). Data sent with ae::Socket::QueueData() can be read with
  //! ae::Socket::PeekData() and ae::Socket::ReceiveData(). It's advised that
  //! you do not mix ae::Socket::QueueMsg() and ae::Socket::QueueData().
  void QueueData( const void* data, uint32_t length );
  //! 
  bool PeekData( void* dataOut, uint16_t length );
  bool ReceiveData( void* dataOut, uint16_t length );
  bool DiscardData( uint16_t length );

  //! Queues data for sending. Ideally you should call ae::Socket::QueueMsg() for each
  //! logical chunk of data you need to send over a 'network tick' and then
  //! finally call ae::Socket::SendAll() once. A two byte (network order) message header is
  //! prepended to the given message. It's unadvised to mix ae::Socket::QueueMsg() calls
  //! with ae::Socket::QueueData().
  void QueueMsg( const void* data, uint16_t length );
  //! Can return a value greater than maxLength, in which case 'dataOut' is not
  //! written to. Call ae::Socket::ReceiveMessage() again with a big enough buffer or call
  //! ae::Socket::DiscardMessage(). Uses a two byte (network order) message header.
  //! It's unadvised to mix ae::Socket::ReceiveMsg() calls with ae::Socket::ReceiveData().
  uint16_t ReceiveMsg( void* dataOut, uint16_t maxLength );
  //! Discards one received sent with ae::Socket::QueueMsg(). Uses the two byte (network order)
  //! message header to determine discard data size.
  bool DiscardMsg();

  //! Sends all queued data from ae::Socket::QueueData() and ae::Socket::QueueMsg().
  void SendAll();

private:
  int m_sock = 0;
  ae::Array< uint8_t > m_sendData = AE_ALLOC_TAG_NET;
  ae::Array< uint8_t > m_recvData = AE_ALLOC_TAG_NET;
public: // Internal
  explicit Socket( int s );
};

//------------------------------------------------------------------------------
// ae::ListenerSocket class
//------------------------------------------------------------------------------
class ListenerSocket
{
public:
  ListenerSocket();
  ~ListenerSocket();

  bool Listen( ae::Socket::Protocol proto, const char* port, uint32_t maxConnections );
  ae::Socket* Accept(); //!< Returns a socket if a connection has been established.
  void Reset(); //!< Disconnects all connections and stops listening. Must still call Destroy() or DestroyAll() on existing connections.
  void Destroy( ae::Socket* sock ); //!< Disconnects and releases an existing socket from Accept().
  void DestroyAll(); //!< Disconnects and releases all existing sockets from Accept(). It is not safe to access released sockets after calling this.

  ae::Socket* GetConnected( uint32_t idx );
  uint32_t GetConnectedCount() const;

private:
  int m_sock = 0;
  ae::Array< ae::Socket* > m_connections = AE_ALLOC_TAG_NET;
};

}

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <WinSock2.h>
#include <WS2tcpip.h>

namespace ae
{

//------------------------------------------------------------------------------
// ae::Socket member functions
//------------------------------------------------------------------------------
Socket::Socket()
{
}

Socket::Socket( int s )
  : m_sock( s )
{}

Socket::~Socket()
{
}

bool Socket::Connect( ae::Socket::Protocol proto, const char* address, const char* port )
{
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
  hints.ai_socktype = SOCK_STREAM;

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

  // Disable blocking
#if _AE_WINDOWS_
  u_long mode = 1;
  ioctlsocket( m_sock, FIONBIO, &mode );
#else
  fcntl( m_sock, F_SETFL, O_NONBLOCK );
#endif

  return true;
}

void Socket::Disconnect()
{
#if _AE_WINDOWS_
  closesocket( m_sock );
#else
  close( m_sock );
#endif
  m_sock = 0;
}

bool Socket::IsConnected() const
{
  return ( m_sock != 0 );
}

void Socket::QueueData( const void* data, uint32_t length )
{
}

bool Socket::PeekData( void* dataOut, uint16_t maxLength )
{
  return false;
}

bool Socket::ReceiveData( void* dataOut, uint16_t maxLength )
{
  return false;
}

bool Socket::DiscardData( uint16_t length )
{
  return false;
}

void Socket::QueueMsg( const void* data, uint16_t length )
{

}

uint16_t Socket::ReceiveMsg( void* dataOut, uint16_t maxLength )
{
  return 0;
}

bool Socket::DiscardMsg()
{
  return false;
}

void Socket::SendAll()
{}

//------------------------------------------------------------------------------
// ae::ListenerSocket member functions
//------------------------------------------------------------------------------
ListenerSocket::ListenerSocket()
{
}

ListenerSocket::~ListenerSocket()
{
}

bool ListenerSocket::Listen( ae::Socket::Protocol proto, const char* port, uint32_t maxConnections )
{
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
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  addrinfo* servinfo;
  if ( getaddrinfo( nullptr, port, &hints, &servinfo ) )
  {
    AE_ERR( "error");
    return 1;
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
#else
    int yesValue = 1;
    int* yes = &yesValue;
#endif
    if ( setsockopt( m_sock, SOL_SOCKET, SO_REUSEADDR, yes, sizeof( int ) ) == -1 )
    {
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
  }

  freeaddrinfo( servinfo );

  if ( !p )
  {
    return false;
  }

  int backlog = 10;
  if ( listen( m_sock, backlog ) == -1 )
  {
    return false;
  }

  // Disable blocking
#if _AE_WINDOWS_
  u_long mode = 1;
  ioctlsocket( m_sock, FIONBIO, &mode );
#else
  fcntl( m_sock, F_SETFL, O_NONBLOCK );
#endif

  return true;
}

ae::Socket* ListenerSocket::Accept()
{
  sockaddr_storage their_addr;
  socklen_t sin_size = sizeof(their_addr);
  int new_fd = accept( m_sock, (sockaddr*)&their_addr, &sin_size );
  if ( new_fd == -1 )
  {
    return false;
  }

  // Disable blocking
#if _AE_WINDOWS_
  u_long mode = 1;
  ioctlsocket( new_fd, FIONBIO, &mode );
#else
  fcntl( m_sock, F_SETFL, O_NONBLOCK );
#endif

  ae::Socket* newSock = ae::New< ae::Socket >( AE_ALLOC_TAG_NET, new_fd );
  m_connections.Append( newSock );
  return newSock;
}

void ListenerSocket::Reset()
{
}

void ListenerSocket::Destroy( ae::Socket* sock )
{
}

void ListenerSocket::DestroyAll()
{
}

ae::Socket* ListenerSocket::GetConnected( uint32_t idx )
{
  return nullptr;
}

uint32_t ListenerSocket::GetConnectedCount() const
{
  return 0;
}

}
