//------------------------------------------------------------------------------
// aeList.h
// Intrusive circularly linked list
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
#ifndef AELIST_H
#define AELIST_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeLog.h"

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
template < typename T >
class aeList;

//------------------------------------------------------------------------------
// aeLinkedList class
//------------------------------------------------------------------------------
template < typename T >
class aeListNode
{
public:
  aeListNode( T* owner );
  ~aeListNode();

  void Remove();

  T* GetFirst();
  T* GetNext();
  T* GetPrev();
  T* GetLast();

  const T* GetFirst() const;
  const T* GetNext() const;
  const T* GetPrev() const;
  const T* GetLast() const;

  aeList< T >* GetList();
  const aeList< T >* GetList() const;

private:
  friend class aeList< T >;
  
  // @NOTE: These operations don't make sense when either node is in a list,
  // to avoid a potentially hard to diagnose random assert, assignment is
  // disabled altogether
  aeListNode( aeListNode& ) = delete;
  void operator = ( aeListNode& ) = delete;

  aeList< T >* m_root;
  aeListNode* m_next;
  aeListNode* m_prev;
  T* m_owner;
};

//------------------------------------------------------------------------------
// aeList class
//------------------------------------------------------------------------------
template < typename T >
class aeList
{
public:
  aeList();
  ~aeList();

  void Append( aeListNode< T >& node );

  T* GetFirst();
  T* GetLast();

  const T* GetFirst() const;
  const T* GetLast() const;

  template < typename U > T* Find( const U& value );
  template < typename Fn > T* FindFn( Fn predicateFn );

  uint32_t Length() const;

private:
  friend class aeListNode< T >;
  
  // @NOTE: Disable assignment. Assigning a list to another list technically makes sense,
  // but could result in unexpected orphaning of list nodes. Additionally disabing these
  // operations is consistent with list node.
  aeList( aeList& ) = delete;
  void operator = ( aeList& ) = delete;

  aeListNode< T >* m_first;
};

//------------------------------------------------------------------------------
// aeLinkedList member functions
//------------------------------------------------------------------------------
template < typename T >
aeListNode< T >::aeListNode( T* owner )
{
  m_root = nullptr;
  m_next = this;
  m_prev = this;
  m_owner = owner;
}

template < typename T >
aeListNode< T >::~aeListNode()
{
  Remove();
}

template < typename T >
void aeListNode< T >::Remove()
{
  if ( !m_root )
  {
    return;
  }

  AE_ASSERT( m_root->m_first );
  if ( m_root->m_first == this )
  {
    if ( m_next == this )
    {
      // Last node in list
      m_root->m_first = nullptr;
    }
    else
    {
      // Was head. Set next as head.
      m_root->m_first = m_next;
    }
  }

  m_next->m_prev = m_prev;
  m_prev->m_next = m_next;

  m_root = nullptr;
  m_next = this;
  m_prev = this;
}

template < typename T >
T* aeListNode< T >::GetFirst()
{
  return const_cast< T* >( const_cast< const aeListNode< T >* >( this )->GetFirst() );
}

template < typename T >
T* aeListNode< T >::GetNext()
{
  return const_cast< T* >( const_cast< const aeListNode< T >* >( this )->GetNext() );
}

template < typename T >
T* aeListNode< T >::GetPrev()
{
  return const_cast< T* >( const_cast< const aeListNode< T >* >( this )->GetPrev() );
}

template < typename T >
T* aeListNode< T >::GetLast()
{
  return const_cast<T*>( const_cast<const aeListNode< T >*>( this )->GetLast() );
}

template < typename T >
const T* aeListNode< T >::GetFirst() const
{
  return m_root ? m_root->GetFirst() : nullptr;
}

template < typename T >
const T* aeListNode< T >::GetNext() const
{
  if ( !m_root || m_root->m_first == m_next )
  {
    return nullptr;
  }
  return m_next->m_owner;
}

template < typename T >
const T* aeListNode< T >::GetPrev() const
{
  if ( !m_root || m_root->m_first == this )
  {
    return nullptr;
  }
  return m_prev->m_owner;
}

template < typename T >
const T* aeListNode< T >::GetLast() const
{
  return m_root ? m_root->GetLast() : nullptr;
}

template < typename T >
aeList< T >* aeListNode< T >::GetList()
{
  return m_root;
}

template < typename T >
const aeList< T >* aeListNode< T >::GetList() const
{
  return m_root;
}

//------------------------------------------------------------------------------
// aeList member functions
//------------------------------------------------------------------------------
template < typename T >
aeList< T >::aeList() : m_first( nullptr )
{}

template < typename T >
aeList< T >::~aeList()
{
  while ( m_first )
  {
    m_first->Remove();
  }
}

template < typename T >
void aeList< T >::Append( aeListNode< T >& node )
{
  if ( m_first )
  {
    node.Remove();

    node.m_root = this;

    node.m_next = m_first;
    node.m_prev = m_first->m_prev;

    node.m_next->m_prev = &node;
    node.m_prev->m_next = &node;
  }
  else
  {
    m_first = &node;
    node.m_root = this;
  }
}

template < typename T >
T* aeList< T >::GetFirst()
{
  return m_first ? m_first->m_owner : nullptr;
}

template < typename T >
T* aeList< T >::GetLast()
{
  return m_first ? m_first->m_prev->m_owner : nullptr;
}

template < typename T >
const T* aeList< T >::GetFirst() const
{
  return m_first ? m_first->m_owner : nullptr;
}

template < typename T >
const T* aeList< T >::GetLast() const
{
  return m_first ? m_first->m_prev->m_owner : nullptr;
}

template < typename T >
template < typename U >
T* aeList< T >::Find( const U& value )
{
  if ( !m_first )
  {
    return nullptr;
  }

  aeListNode< T >* current = m_first;
  do
  {
    if ( *( current->m_owner ) == value )
    {
      return current->m_owner;
    }
    current = current->m_next;
  } while ( current != m_first );

  return nullptr;
}

template < typename T >
template < typename Fn >
T* aeList< T >::FindFn( Fn predicateFn )
{
  if ( !m_first )
  {
    return nullptr;
  }

  aeListNode< T >* current = m_first;
  do
  {
    if ( predicateFn( current->m_owner ) )
    {
      return current->m_owner;
    }
    current = current->m_next;
  } while ( current != m_first );

  return nullptr;
}

template < typename T >
uint32_t aeList< T >::Length() const
{
  if ( !m_first )
  {
    return 0;
  }

  // @TODO: Should be constant time
  uint32_t count = 1;
  aeListNode< T >* current = m_first;
  while ( current->m_next != m_first )
  {
    current = current->m_next;
    count++;
  }

  return count;
}

#endif
