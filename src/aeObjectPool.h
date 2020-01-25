//------------------------------------------------------------------------------
// aeObjectPool.h
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
#ifndef AEOBJECTPOOL_H
#define AEOBJECTPOOL_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include <cstdint>
#include "aeLog.h"

//------------------------------------------------------------------------------
// aeObjectPool class
//------------------------------------------------------------------------------
template< typename T, uint32_t N >
class aeObjectPool
{  
public:
  aeObjectPool()
  {
    m_length = 0;

    for ( uint32_t i = 0; i < N - 1; i++ )
    {
      m_pool[ i ].next = &m_pool[ i + 1 ];
      m_pool[ i ].allocated = false;
    }
    m_pool[ N - 1 ].next = nullptr;
    m_pool[ N - 1 ].allocated = false;
    
    m_open = &m_pool[ 0 ];
  }

  ~aeObjectPool()
  {
    for ( uint32_t i = 0; i < N; i++ )
    {
      Entry* entry = &m_pool[ i ];
      if ( entry->allocated )
      {
        ( (T*)entry->object )->~T();
        entry->next = nullptr;
        entry->allocated = false;
      }
    }
    m_open = nullptr;
  }

  uint32_t GetAllocated( T* (&out)[ N ] )
  {
    uint32_t count = 0;
    for ( uint32_t i = 0; i < N; i++ )
    {
      if ( m_pool[ i ].allocated )
      {
        out[ count ] = (T*)&m_pool[ i ].object;
        count++;
      }
    }
    AE_ASSERT( count == m_length );
    return count;
  }

  uint32_t GetAllocated( const T* (&out)[ N ] ) const
  {
    uint32_t count = 0;
    for ( uint32_t i = 0; i < N; i++ )
    {
      if ( m_pool[ i ].allocated )
      {
        out[ count ] = (T*)&m_pool[ i ].object;
        count++;
      }
    }
    AE_ASSERT( count == m_length );
    return count;
  }

  T* Allocate()
  {
    if ( !m_open )
    {
      return nullptr;
    }

    Entry* entry = m_open;
    m_open = m_open->next;
    entry->next = nullptr;
    entry->allocated = true;

    m_length++;

    return new( (T*)entry->object ) T();
  }

  void Free( T* p )
  {
    if ( !p )
    {
      return;
    }

    p->~T();
    Entry* entry = (Entry*)( (uint8_t*)p - offsetof( Entry, object ) );
    AE_ASSERT( m_pool <= entry && entry < m_pool + N );
    entry->next = m_open;
    entry->allocated = false;
    m_open = entry;

    m_length--;
  }

  bool HasFree() const { return m_open != nullptr; }
  uint32_t Length() const { return m_length; }

private:
  struct Entry
  {
    Entry* next;
    bool allocated;
    alignas(T) uint8_t object[ sizeof(T) ];
  };

  uint32_t m_length;
  Entry* m_open;
  Entry m_pool[ N ];
};

#endif
