//------------------------------------------------------------------------------
// aeBinaryStream.h
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
#ifndef AEBINARYSTREAM_H
#define AEBINARYSTREAM_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeArray.h"
#include "aeString.h"

//------------------------------------------------------------------------------
// aeBinaryStream class
//------------------------------------------------------------------------------
class aeBinaryStream
{
public:
  static aeBinaryStream Writer( uint8_t* data, uint32_t length );
  static aeBinaryStream Writer( aeArray< uint8_t >* array );
  static aeBinaryStream Writer();
  static aeBinaryStream Reader( const uint8_t* data, uint32_t length );
  static aeBinaryStream Reader( const aeArray< uint8_t >& data );

  void SerializeUint8( uint8_t& v );
  void SerializeUint8( const uint8_t& v );
  void SerializeUint16( uint16_t& v );
  void SerializeUint16( const uint16_t& v );
  void SerializeUint32( uint32_t& v );
  void SerializeUint32( const uint32_t& v );
  void SerializeUint64( uint64_t& v );
  void SerializeUint64( const uint64_t& v );

  void SerializeInt8( int8_t& v );
  void SerializeInt8( const int8_t& v );
  void SerializeInt16( int16_t& v );
  void SerializeInt16( const int16_t& v );
  void SerializeInt32( int32_t& v );
  void SerializeInt32( const int32_t& v );
  void SerializeInt64( int64_t& v );
  void SerializeInt64( const int64_t& v );

  void SerializeFloat( float& v );
  void SerializeFloat( const float& v );
  void SerializeDouble( double& v );
  void SerializeDouble( const double& v );

  void SerializeBool( bool& v );
  void SerializeBool( const bool& v );
  
  template < uint32_t N >
  void SerializeString( aeStr< N >& str );
  template < uint32_t N >
  void SerializeString( const aeStr< N >& str );

  template< typename T >
  void SerializeObject( T& v );
  template< typename T >
  void SerializeObject( const T& v );
  // Use SerializeObjectConditional() when an object may not be available for serialization when writing or reading. This function correctly updates read/write offsets when skipping serialization. Sends slightly more data than SerializeObject().
  template < typename T >
  void SerializeObjectConditional( T* obj );

  template< uint32_t N >
  void SerializeArray( char (&str)[ N ] );
  template< uint32_t N >
  void SerializeArray( const char (&str)[ N ] );
  void SerializeArray( aeArray< uint8_t>& array, uint32_t maxLength = 65535 );
  void SerializeArray( const aeArray< uint8_t>& array, uint32_t maxLength = 65535 );

  template< typename T >
  void SerializeRaw( T& v );
  template< typename T >
  void SerializeRaw( const T& v );
  void SerializeRaw( void* data, uint32_t length );
  void SerializeRaw( const void* data, uint32_t length );
  void SerializeRaw( aeArray< uint8_t >& array );
  void SerializeRaw( const aeArray< uint8_t >& array );

  // Once the stream is invalid serialization calls will result in silent no-ops
  void Invalidate() { m_isValid = false; }
  bool IsValid() const { return m_isValid; }
  
  // Get mode
  bool IsWriter() const { return m_mode == Mode::WriteBuffer; }
  bool IsReader() const { return m_mode == Mode::ReadBuffer; }

  // Get data buffer
  const uint8_t* GetData() const { return ( m_data || m_GetArray().Length() == 0 ) ? m_data : &m_GetArray()[ 0 ]; }
  uint32_t GetOffset() const { return m_offset; }
  uint32_t GetLength() const { return m_length; }

  // Get data past the current read head
  const uint8_t* PeekData() const { return GetData() + m_offset; }
  uint32_t GetRemaining() const { return m_length - m_offset; }
  void Discard( uint32_t length );

// @TODO: The following should be private, while making it clear that the above static functions
//        should be used instead of directly using a constructor.
// private:
  enum class Mode
  {
    None,
    ReadBuffer,
    WriteBuffer,
  };

  aeBinaryStream() = default;
  aeBinaryStream( Mode mode, uint8_t* data, uint32_t length );
  aeBinaryStream( Mode mode, const uint8_t* data, uint32_t length );
  aeBinaryStream( Mode mode );
  aeBinaryStream( aeArray< uint8_t >* array );

  aeArray< uint8_t >& m_GetArray() { return m_extArray ? *m_extArray : m_array; }
  const aeArray< uint8_t >& m_GetArray() const { return m_extArray ? *m_extArray : m_array; }

  void m_SerializeArrayLength( uint32_t& length, uint32_t maxLength );
  Mode m_mode = Mode::None;
  bool m_isValid = false;
  uint8_t* m_data = nullptr;
  uint32_t m_length = 0;
  uint32_t m_offset = 0;
  aeArray< uint8_t >* m_extArray = nullptr;
  aeArray< uint8_t > m_array;

public:
  // Prevent the above functions from being called accidentally through automatic conversions
  template < typename T > void SerializeUint8( T ) = delete;
  template < typename T > void SerializeUint16( T ) = delete;
  template < typename T > void SerializeUint32( T ) = delete;
  template < typename T > void SerializeUint64( T ) = delete;
  template < typename T > void SerializeInt8( T ) = delete;
  template < typename T > void SerializeInt16( T ) = delete;
  template < typename T > void SerializeInt32( T ) = delete;
  template < typename T > void SerializeInt64( T ) = delete;
  template < typename T > void SerializeFloat( T ) = delete;
  template < typename T > void SerializeDouble( T ) = delete;
  template < typename T > void SerializeBool( T ) = delete;
  template < typename T > void SerializeString( T ) = delete;
};

//------------------------------------------------------------------------------
// aeBinaryStream member functions
//------------------------------------------------------------------------------
template < uint32_t N >
void aeBinaryStream::SerializeString( aeStr< N >& str )
{
  if ( IsWriter() )
  {
    const uint16_t len = str.Length();
    SerializeUint16( len );
    SerializeRaw( str.c_str(), len );
  }
  else if ( IsReader() )
  {
    uint16_t len = 0;
    SerializeUint16( len );
    if ( !IsValid() )
    {
      return;
    }

    if ( len > aeStr< N >::MaxLength() || GetRemaining() < len )
    {
      Invalidate();
    }
    else
    {
      str = aeStr< N >( len, (const char*)PeekData() );
      Discard( len );
    }
  }
}

template < uint32_t N >
void aeBinaryStream::SerializeString( const aeStr< N >& str )
{
  AE_ASSERT( m_mode == Mode::WriteBuffer );
  const uint16_t len = str.Length();
  SerializeUint16( len );
  SerializeRaw( str.c_str(), len );
}

template< typename T >
void aeBinaryStream_SerializeObjectInternal( aeBinaryStream* stream, T& v, decltype( &T::Serialize ) )
{
  v.Serialize( stream );
}

template< typename T >
void aeBinaryStream_SerializeObjectInternalConst( aeBinaryStream* stream, const T& v, decltype( &T::Serialize ) )
{
  v.Serialize( stream );
}

template< typename T >
void aeBinaryStream_SerializeObjectInternal( aeBinaryStream* stream, T& v, ... )
{
  Serialize( stream, &v );
}

template< typename T >
void aeBinaryStream_SerializeObjectInternalConst( aeBinaryStream* stream, const T& v, ... )
{
  Serialize( stream, &v );
}

template< typename T >
void aeBinaryStream::SerializeObject( T& v )
{
  aeBinaryStream_SerializeObjectInternal< T >( this, v, 0 );
}

template< typename T >
void aeBinaryStream::SerializeObject( const T& v )
{
  AE_ASSERT_MSG( m_mode == Mode::WriteBuffer, "Only write mode can be used when serializing a const type." );
  aeBinaryStream_SerializeObjectInternalConst< T >( this, v, 0 );
}

template< typename T >
void aeBinaryStream::SerializeRaw( T& v )
{
  if ( !m_isValid )
  {
    return;
  }
  else if ( m_mode == Mode::ReadBuffer )
  {
    AE_ASSERT( m_offset + sizeof(T) <= m_length );
    memcpy( &v, m_data + m_offset, sizeof(T) );
    m_offset += sizeof(T);
  }
  else if ( m_mode == Mode::WriteBuffer )
  {
    if ( m_data )
    {
      AE_ASSERT( sizeof(T) <= m_length - m_offset );
      memcpy( m_data + m_offset, &v, sizeof(T) );
      m_offset += sizeof(T);
    }
    else
    {
      aeArray< uint8_t >& array = m_GetArray();
      array.Append( (uint8_t*)&v, sizeof(T) );
      m_offset = array.Length();
      m_length = array.Size();
    }
  }
  else
  {
    AE_FAIL_MSG( "Binary stream must be initialized with aeBinaryStream::Writer or aeBinaryStream::Reader static functions." );
  }
}

template< typename T >
void aeBinaryStream::SerializeRaw( const T& v )
{
  AE_ASSERT_MSG( m_mode == Mode::WriteBuffer, "Only write mode can be used when serializing a const type." );
  SerializeRaw( *const_cast< T* >( &v ) );
}

template< uint32_t N >
void aeBinaryStream::SerializeArray( char (&str)[ N ] )
{
  // @TODO: Cleanup and use m_SerializeArrayLength()
  uint16_t len = 0;
  if ( !m_isValid )
  {
    return;
  }
  else if ( m_mode == Mode::ReadBuffer )
  {
    AE_ASSERT( m_offset + sizeof(len) <= m_length ); // @TODO: Remove this and invalidate stream instead
    memcpy( &len, m_data + m_offset, sizeof(len) );
    m_offset += sizeof(len);

    AE_ASSERT( m_offset + len + 1 <= m_length ); // @TODO: Remove this and invalidate stream instead
    memcpy( str, m_data + m_offset, len );
    str[ len ] = 0;
    m_offset += len;
  }
  else if ( m_mode == Mode::WriteBuffer )
  {
    len = strlen( str );

    if ( m_data )
    {
      AE_ASSERT( sizeof(len) <= m_length - m_offset ); // @TODO: Remove this and invalidate stream instead
      memcpy( m_data + m_offset, &len, sizeof(len) );
      m_offset += sizeof(len);

      AE_ASSERT( len <= m_length - m_offset ); // @TODO: Remove this and invalidate stream instead
      memcpy( m_data + m_offset, str, len );
      m_offset += len;
    }
    else
    {
      aeArray< uint8_t >& array = m_GetArray();
      array.Append( (uint8_t*)&len, sizeof(len) );
      array.Append( (uint8_t*)&str, len );
      m_offset = array.Length();
      m_length = array.Size();
    }
  }
  else
  {
    AE_FAIL_MSG( "Binary stream must be initialized with aeBinaryStream::Writer or aeBinaryStream::Reader static functions." );
  }
}

template< uint32_t N >
void aeBinaryStream::SerializeArray( const char (&str)[ N ] )
{
  AE_ASSERT_MSG( m_mode == Mode::WriteBuffer, "Only write mode can be used when serializing a const array." );
  SerializeArray( const_cast< char[ N ] >( str ) );
}

template < typename T >
void aeBinaryStream::SerializeObjectConditional( T* obj )
{
  if ( !m_isValid )
  {
    return;
  }
  else if ( m_mode == Mode::ReadBuffer )
  {
    uint16_t length = 0;
    SerializeRaw( &length, sizeof( length ) );

    if ( length )
    {
      if ( obj )
      {
        // Read object
        uint32_t prevOffset = m_offset;
        SerializeObject( *obj );

        // Object should always read everything it wrote
        if ( prevOffset + length != m_offset )
        {
          Invalidate();
        }
      }
      else
      {
        Discard( length );
      }
    }
  }
  else if ( m_mode == Mode::WriteBuffer )
  {
    if ( obj )
    {
      // Reserve length
      uint32_t lengthOffset = m_offset;
      uint16_t lengthFake = 0xCDCD;
      SerializeRaw( &lengthFake, sizeof( lengthFake ) ); // Raw to avoid compression

      // Write object
      uint32_t prevOffset = m_offset;
      SerializeObject( *obj );

      // Rewrite previously serialized value
      uint32_t writeLength = m_offset - prevOffset;
      if ( writeLength > aeMath::MaxValue< uint16_t >() )
      {
        Invalidate(); // Object is too large to serialize
      }
      else if ( IsValid() ) // Can become invalid while writing by running out of memory
      {
        // @NOTE: Use length offset from above (and not a pointer into the data buffer) because the data buffer may not yet bet allocated or may be reallocated while serializing
        AE_ASSERT( GetData() );
        uint16_t* length = (uint16_t*)( GetData() + lengthOffset );
        AE_ASSERT( *length == 0xCDCD );
        *length = writeLength;
      }
    }
    else
    {
      uint16_t length = 0;
      SerializeRaw( &length, sizeof( length ) ); // Raw to avoid compression
    }
  }
  else
  {
    AE_FAIL_MSG( "Binary stream must be initialized with aeBinaryStream::Writer or aeBinaryStream::Reader static functions." );
  }
}

#endif
