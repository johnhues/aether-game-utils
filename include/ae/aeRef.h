//------------------------------------------------------------------------------
// aeRef.h
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
#ifndef AEREF_H
#define AEREF_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeMeta.h"

//------------------------------------------------------------------------------
// aeRef interface helper macro
//------------------------------------------------------------------------------
#define AE_REFABLE( _C )\
aeId< _C > GetId() const { return m_refable.GetId(); }\
static _C* GetById( aeId< _C > id ) { return aeRefable< _C >::GetById( id ); }\
aeRefable< _C > m_refable = this;

//------------------------------------------------------------------------------
// aeId class
//------------------------------------------------------------------------------
template < typename T >
class aeId
{
public:
  static aeId CreateNew() { static uint32_t s_current = 0; s_current++; return aeId( s_current ); }

  aeId() { m_id = 0; }

  explicit operator bool() const { return m_id != 0; }
  bool operator == ( aeId other ) const { return m_id == other.m_id; }
  bool operator != ( aeId other ) const { return !operator == ( other ); }

#if _AE_WINDOWS_
  // @TODO: Remove this
  template< typename U >
  aeId( const aeId< U >& other )
  {
      m_id = other.m_id;
  }
#else
  // @TODO: The following has stopped working with MSVC
  template< typename U, typename std::enable_if< std::is_base_of< T, U >{}, int>::type = 0 > // Allow automatic downcast to U
  aeId( const aeId< U >& other ) { m_id = other.m_id; }

  template< typename U, typename std::enable_if< std::is_base_of< U, T >{}, int>::type = 0 > // Allow explicit upcast to U
  explicit aeId( const aeId< U >& other ) { m_id = other.m_id; }

private:
  template < typename T2 >
  friend class aeId;

  template < typename T2 >
  friend std::ostream& operator<<( std::ostream& os, const aeId< T2 >& id );
#endif

  explicit aeId( uint32_t id ) { m_id = id; }
  
  uint32_t m_id;
public:
  void SetInternalId( uint32_t id ) { m_id = id; }
  uint32_t GetInternalId() const { return m_id; }
};

template < typename T >
inline std::ostream& operator<<( std::ostream& os, const aeId< T >& id )
{
  return os << id.m_id;
}

//------------------------------------------------------------------------------
// aeRefable class
//------------------------------------------------------------------------------
template < typename T >
class aeRefable
{
public:
  aeRefable( T* owner ) : m_id( aeId< T >::CreateNew() ) { s_GetMap()->Set( m_id, owner ); }
  ~aeRefable() { s_GetMap()->Remove( m_id ); }

  aeId< T > GetId() const { return m_id; }
  static T* GetById( aeId< T > id ) { return s_GetMap()->Get( id, nullptr ); }

private:
  //aeRefable( const aeRefable& ) = delete; // @HACK: Disabled this to support automatic 'this' assignment with AE_REFABLE
  aeRefable& operator= ( const aeRefable& ) = delete;

  // uint32_t s_sequence = 1; @TODO: Increment a 'dirty' sequence number whenever a T is destroyed so aeRef can keep a pointer to the referenced object and avoid a lookup
  typedef ae::Map< aeId< T >, T* > RefMap;
  static RefMap* s_GetMap() { static RefMap s_map = AE_ALLOC_TAG_FIXME; return &s_map; }

  aeId< T > m_id;
};

//------------------------------------------------------------------------------
// aeRef class
//------------------------------------------------------------------------------
template < typename T >
class aeRef
{
public:
  aeRef() = default;
  aeRef( T* obj );
  aeRef( aeId< T > id );
  aeRef( const aeRef& ) = default;

  aeId< T > GetId() const { return m_id; }

  explicit operator bool() const;
  operator const T*() const;
  operator T*();

  const T* operator ->() const;
  const T& operator *() const;
  T* operator ->();
  T& operator *();

  bool Lost() const;

private:
  // Provide m_Get() defaults to allow templated types that don't support T::GetById()
  template < typename U, bool result = std::is_same< decltype( ((U*)nullptr)->GetById( aeId< U >() ) ), U* >::value >
  const U* m_Get( int ) const;
  
  template < typename U, bool result = std::is_same< decltype( ((U*)nullptr)->GetById( aeId< U >() ) ), U* >::value >
  U* m_Get( int );
  
  template < typename U >
  const U* m_Get( ... ) const;
  
  template < typename U >
  U* m_Get( ... );

  aeId< T > m_id;
  // uint32_t m_sequence = 0; @TODO: Keep a pointer to the referenced object and a local copy aeRef< T >::s_sequence so sequential lookups are fast
  // T* m_cached = nullptr;
};

//------------------------------------------------------------------------------
// aeRefPair class
//------------------------------------------------------------------------------
template < typename T, typename S >
class aeRefPair
{
public:
  aeRefPair( T* self );
  ~aeRefPair();
  
  void Pair( aeRefPair< S, T >& other );
  void Clear();
  
  S* Get();
  const S* Get() const;
  
private:
  friend class aeRefPair< S, T >;
  T* m_self;
  aeRefPair< S, T >* m_other;
};

//------------------------------------------------------------------------------
// aeRef member functions
//------------------------------------------------------------------------------
template < typename T >
aeRef< T >::aeRef( T* obj )
{
  // Make sure T::GetById() is implemented for T at compile time only if a ref is ever actually constructed
  auto GetByID_NotImplemented = &T::GetById;

  if ( obj )
  {
    m_id = aeId< T >( obj->GetId() );
    AE_ASSERT( m_id );
  }
  else
  {
    m_id = aeId< T >();
  }
}

template < typename T >
aeRef< T >::aeRef( aeId< T > id )
  : m_id( id )
{}

template < typename T >
aeRef< T >::operator bool() const
{
  return m_Get< T >( 0 ) != nullptr;
}

template < typename T >
aeRef< T >::operator const T*() const
{
  return m_Get< T >( 0 );
}

template < typename T >
aeRef< T >::operator T*()
{
  return m_Get< T >( 0 );
}

template < typename T >
const T* aeRef< T >::operator ->() const
{
  return &(operator *());
}

template < typename T >
T* aeRef< T >::operator ->()
{
  return &(operator *());
}

template < typename T >
const T& aeRef< T >::operator *() const
{
  AE_ASSERT( m_id );
  auto obj = m_Get< T >( 0 );
  AE_ASSERT( obj );
  return *aeCast< T >( obj );
}

template < typename T >
T& aeRef< T >::operator *()
{
  AE_ASSERT( m_id );
  auto obj = m_Get< T >( 0 );
  AE_ASSERT( obj );
  return *aeCast< T >( obj );
}

template < typename T >
bool aeRef< T >::Lost() const
{
  return m_Get< T >( 0 ) == nullptr && m_id != aeId< T >();
}

template < typename T >
template < typename U, bool >
const U* aeRef< T >::m_Get( int ) const
{
  return m_id ? aeCast< T >( T::GetById( m_id ) ) : nullptr;
}

template < typename T >
template < typename U, bool >
U* aeRef< T >::m_Get( int )
{
  return m_id ? aeCast< T >( T::GetById( m_id ) ) : nullptr;
}

template < typename T >
template < typename U >
const U* aeRef< T >::m_Get( ... ) const
{
  return nullptr;
}

template < typename T >
template < typename U >
U* aeRef< T >::m_Get( ... )
{
  return nullptr;
}

//------------------------------------------------------------------------------
// aeRefPair member functions
//------------------------------------------------------------------------------
template < typename T, typename S >
aeRefPair< T, S >::aeRefPair( T* self ) :
  m_self( self ),
  m_other( nullptr )
{
  AE_ASSERT( self );
}

template < typename T, typename S >
aeRefPair< T, S >::~aeRefPair()
{
  Clear();
}

template < typename T, typename S >
void aeRefPair< T, S >::Pair( aeRefPair< S, T >& other )
{
  if ( m_other == &other )
  {
    return;
  }
  
  if ( m_other )
  {
    m_other->Clear();
  }
  other.Clear();
  
  m_other = &other;
  other.m_other = this;
}

template < typename T, typename S >
void aeRefPair< T, S >::Clear()
{
  if ( m_other )
  {
    m_other->m_other = nullptr;
    m_other = nullptr;
  }
}

template < typename T, typename S >
S* aeRefPair< T, S >::Get()
{
  return m_other ? m_other->m_self : nullptr;
}

template < typename T, typename S >
const S* aeRefPair< T, S >::Get() const
{
  return m_other ? m_other->m_self : nullptr;
}

#endif
