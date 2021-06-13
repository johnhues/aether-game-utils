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
#include "aether.h"

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
        T* object = (T*)entry->object;
        object->~T();
        entry->next = nullptr;
        entry->allocated = false;
      }
    }
    m_open = nullptr;
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

    Entry* entry = m_GetEntry( p );
    p->~T();
    AE_ASSERT( entry->check == 0xCDCDCDCD );
    entry->next = m_open;
    entry->allocated = false;
    m_open = entry;

    m_length--;
  }

  const T* GetFirst() const
  {
    if ( !m_length )
    {
      return nullptr;
    }
    const Entry* entry = &m_pool[ 0 ];
    const T* p = (const T*)entry->object;
    return entry->allocated ? p : GetNext( p );
  }

  const T* GetNext( const T* p ) const
  {
    const Entry* entry = m_GetEntry( p );
    for ( uint32_t i = entry - m_pool + 1; i < N; i++ )
    {
      if ( m_pool[ i ].allocated )
      {
        return (const T*)m_pool[ i ].object;
      }
    }

    return nullptr;
  }

  T* GetFirst()
  {
    return const_cast< T* >( const_cast< const aeObjectPool< T, N >* >( this )->GetFirst() );
  }

  T* GetNext( T* p )
  {
    return const_cast< T* >( const_cast< const aeObjectPool< T, N >* >( this )->GetNext( p ) );
  }

  bool HasFree() const { return m_open != nullptr; }
  uint32_t Length() const { return m_length; }

private:
  struct Entry
  {
    Entry* next;
    bool allocated;
    uint32_t check = 0xCDCDCDCD;
    alignas(T) uint8_t object[ sizeof(T) ];
  };

  const Entry* m_GetEntry( const T* p ) const
  {
    Entry* entry = (Entry*)( (uint8_t*)p - offsetof( Entry, object ) );
    AE_ASSERT_MSG( m_pool <= entry && entry < m_pool + N, "Object is not in pool" );
    AE_ASSERT( entry->check == 0xCDCDCDCD );
    return entry;
  }

  Entry* m_GetEntry( T* p )
  {
    return const_cast< Entry* >( const_cast< const aeObjectPool< T, N >* >( this )->m_GetEntry( p ) );
  }

  uint32_t m_length;
  Entry* m_open;
  Entry m_pool[ N ];
};

#endif
