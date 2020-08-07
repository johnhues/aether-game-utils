//------------------------------------------------------------------------------
// aeCompactingAllocator.h
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
#ifndef AECOMPACTINGALLOCATOR_H
#define AECOMPACTINGALLOCATOR_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeAlloc.h"

//------------------------------------------------------------------------------
// aeCompactingAllocator class
//------------------------------------------------------------------------------
class aeCompactingAllocator
{
public:
  aeCompactingAllocator( uint32_t size );

  // @NOTE: p must remain valid and writable until Free() is called
  void Allocate( void** p, uint32_t size );
  void Reallocate( void** p, uint32_t size );
  void Free( void* p );
  
  void Compact();
  void Print();

private:
  struct Header
  {
    uint32_t check;
    uint16_t size;
    Header* next;
    Header* prev;
    void** external;
  };

  uint8_t* m_data;
  Header* m_tail;
  uint32_t m_size;
  uint32_t m_allocated = 0;
};

//------------------------------------------------------------------------------
// aeCompactingAllocator member functions
//------------------------------------------------------------------------------
aeCompactingAllocator::aeCompactingAllocator( uint32_t size )
{
  m_size = size;
  m_tail = nullptr;
  m_data = aeAlloc::AllocateRaw( 1, 16, size );
}

void aeCompactingAllocator::Allocate( void** p, uint32_t size )
{
  if ( m_tail )
  {
    Header* next = (Header*)( (uint8_t*)m_tail + sizeof( Header ) + m_tail->size );
    AE_ASSERT( (uint8_t*)next - m_data + size + sizeof( Header ) < m_size );
    next->size = size;
    next->next = nullptr;
    next->external = p;
    next->check = 0xABABABAB;
    *p = (uint8_t*)next + sizeof( Header );
    next->prev = m_tail;
    m_tail->next = next;
    m_tail = next;
  }
  else
  {
    m_tail = (Header*)m_data;
    m_tail->size = size;
    m_tail->next = nullptr;
    m_tail->prev = nullptr;
    m_tail->external = p;
    m_tail->check = 0xABABABAB;
    *p = (uint8_t*)m_tail + sizeof( Header );
  }

  m_allocated = (int32_t)( (uint8_t*)*p + size - (uint8_t*)m_data );
}

void aeCompactingAllocator::Reallocate( void** p, uint32_t size )
{
  Header* oldHeader = m_tail;
  while ( oldHeader )
  {
    AE_ASSERT( oldHeader->check == 0xABABABAB );
    if ( oldHeader->external == p )
    {
      break;
    }
    oldHeader = oldHeader->prev;
  }
  AE_ASSERT( oldHeader );

  if ( oldHeader->size > size )
  {
    oldHeader->size = size;
  }
  else if ( oldHeader->size < size )
  {
    Header* newHeader = (Header*)( (uint8_t*)m_tail + sizeof( Header ) + m_tail->size );
    AE_ASSERT( (uint8_t*)newHeader - m_data + size + sizeof( Header ) < m_size );
    memmove( newHeader, oldHeader, sizeof( Header ) + oldHeader->size );
    *( newHeader->external ) = (uint8_t*)newHeader + sizeof( Header );
    newHeader->size = size;
    newHeader->next = nullptr;
    newHeader->prev = m_tail;
    m_tail->next = newHeader;
    m_tail = newHeader;
  }
}

void aeCompactingAllocator::Free( void* p )
{
  Header* header = (Header*)( (uint8_t*)p - sizeof( Header ) );
  AE_ASSERT( *header->external == p );
  AE_ASSERT( header->check == 0xABABABAB );
  header->external = nullptr;
}

void aeCompactingAllocator::Compact()
{
  if ( m_tail == nullptr )
  {
    m_allocated = 0;
    return;
  }

  Header* current = m_tail;
  AE_ASSERT( current->check == 0xABABABAB );
  while ( current->prev )
  {
    AE_ASSERT( current->prev->check == 0xABABABAB );
    current = current->prev;
  }

  uint8_t* open = m_data;
  while ( current )
  {
    if ( current->external )
    {
      if ( open != (uint8_t*)current )
      {
        current->prev = m_tail;
        memmove( open, current, sizeof( Header ) + current->size );
        Header* currentHeader = (Header*)open;
        *( currentHeader->external ) = open + sizeof( Header );
        current = currentHeader;
        m_tail->next = current;
      }
      open += sizeof( Header ) + current->size;
      m_tail = current;
    }
    current = current->next;
  }
  if ( open == m_data )
  {
    m_tail = nullptr;
  }
  if ( m_tail )
  {
    m_tail->next = nullptr;
  }

  if ( m_tail )
  {
    uint8_t* end = (uint8_t*)m_tail + sizeof( Header ) + m_tail->size;
    m_allocated = (int32_t)( end - m_data );
  }
  else
  {
    m_allocated = 0;
  }

  current = m_tail;
  if ( !current )
  {
    return;
  }
  AE_ASSERT( current->check == 0xABABABAB );
  while ( current->prev )
  {
    AE_ASSERT( current->prev->check == 0xABABABAB );
    current = current->prev;
  }
  while ( current )
  {
    AE_ASSERT( current->check == 0xABABABAB );
    current = current->next;
  }
}

void aeCompactingAllocator::Print()
{
  if ( m_tail == nullptr )
  {
    return;
  }

  Header* current = m_tail;
  while ( current->prev )
  {
    current = current->prev;
  }

  while ( current )
  {
    AE_LOG( "s:# a:#", current->size, current->external != nullptr );
    current = current->next;
  }
}

#endif
