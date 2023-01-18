//------------------------------------------------------------------------------
// aeRingBuffer.h
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
#ifndef AERINGBUFFER_H
#define AERINGBUFFER_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"

//------------------------------------------------------------------------------
// aeRingBuffer class
//------------------------------------------------------------------------------
template < typename T, uint32_t N >
class aeRingBuffer
{
public:
  aeRingBuffer();
  void Append( const T& val );
  void Clear();

  const T& Get( uint32_t index ) const;
  uint32_t Length() const { return m_buffer.Length(); }
  uint32_t Size() const { return N; }

private:
  uint32_t m_first;
  ae::Array< T, N > m_buffer;
};

//------------------------------------------------------------------------------
// aeRingBuffer member functions
//------------------------------------------------------------------------------
template < typename T, uint32_t N >
aeRingBuffer< T, N >::aeRingBuffer() :
  m_first( 0 )
{}

template < typename T, uint32_t N >
void aeRingBuffer< T, N >::Append( const T& val )
{
  if ( m_buffer.Length() < N )
  {
    m_buffer.Append( val );
  }
  else
  {
	AE_DEBUG_ASSERT( m_buffer.Length() == N );
    m_buffer[ m_first % N ] = val;
    m_first++;
  }
}

template < typename T, uint32_t N >
void aeRingBuffer< T, N >::Clear()
{
  m_first = 0;
  m_buffer.Clear();
}

template < typename T, uint32_t N >
const T& aeRingBuffer< T, N >::Get( uint32_t index ) const
{
  AE_ASSERT( index < m_buffer.Length() );
  return m_buffer[ ( m_first + index ) % N ];
}

#endif
