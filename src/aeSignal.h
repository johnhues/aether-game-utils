//------------------------------------------------------------------------------
// aeSignal.h
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
#ifndef AESIGNAL_H
#define AESIGNAL_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeArray.h"
#include "aeMeta.h"
#include "aeRef.h"

//------------------------------------------------------------------------------
// aeSignalBase class
//------------------------------------------------------------------------------
class aeSignalBase
{
public:
  aeSignalBase() {}
  virtual ~aeSignalBase() {}

  virtual bool Send() = 0;

public:
  // Internal use
  virtual void* GetObj() = 0;
};

//------------------------------------------------------------------------------
// aeSignalT class
//------------------------------------------------------------------------------
template < typename S >
class aeSignalT : public aeSignalBase
{
public:
  aeSignalT();
  aeSignalT( const S& val );
  virtual ~aeSignalT() {}

  virtual bool Send( const S& value ) = 0;

protected:
  bool m_valSet;
  S m_val;

public:
  // Internal use
  bool HasValue();
};

//------------------------------------------------------------------------------
// aeSignal class
//------------------------------------------------------------------------------
template < typename T, typename S = int32_t >
class aeSignal : public aeSignalT< S >
{
public:
  aeSignal();
  ~aeSignal();
  
  aeSignal( T* obj, void ( T::* fn ) () );
  aeSignal( T* obj, void ( T::* fn ) ( S ) );
  aeSignal( T* obj, void ( T::* fn ) ( S ), const S& value );

  aeSignal( aeRef< T >& ref, void ( T::* fn ) () );
  aeSignal( aeRef< T >& ref, void ( T::* fn ) ( S ) );
  aeSignal( aeRef< T >& ref, void ( T::* fn ) ( S ), const S& value );

  bool Send() override;
  bool Send( const S& value ) override;

private:
  T* m_obj;
  aeRef< T > m_ref;
  
  void ( T::* m_fn )();
  void ( T::* m_fnVal )( S );

  bool m_isCalled;

public:
  // Internal use
  void* GetObj() override;
};

//------------------------------------------------------------------------------
// aeSignalList class
//------------------------------------------------------------------------------
template < typename S = int >
class aeSignalList
{
public:
  ~aeSignalList();

  template < typename T, typename Fn > void Add( T* obj, Fn fn );
  template < typename T, typename Fn > void Add( aeRef< T > ref, Fn fn );

  void Remove( void* obj );
  
  void Send();
  template < typename T > void Send( const T& value );

  uint32_t Length() const;

private:
  aeArray< aeSignalT< S >* > m_signals;
};

//------------------------------------------------------------------------------
// aeSignalT member functions
//------------------------------------------------------------------------------
template < typename S >
aeSignalT< S >::aeSignalT() :
  m_valSet( false ),
  m_val( S() )
{}

template < typename S >
aeSignalT< S >::aeSignalT( const S& val ) :
  m_valSet( true ),
  m_val( val )
{}

// @TODO: Const
template < typename S >
bool aeSignalT< S >::HasValue()
{
  return m_valSet;
}

//------------------------------------------------------------------------------
// aeSignal member functions
//------------------------------------------------------------------------------
template < typename T, typename S >
aeSignal< T, S >::aeSignal() :
  m_obj( nullptr ),
  m_fn( nullptr ),
  m_fnVal( nullptr ),
  m_isCalled( false )
{}

template < typename T, typename S >
aeSignal< T, S >::aeSignal( T* obj, void ( T::* fn ) () ) :
  m_obj( obj ),
  m_fn( fn ),
  m_fnVal( nullptr ),
  m_isCalled( false )
{}

template < typename T, typename S >
aeSignal< T, S >::aeSignal( T* obj, void ( T::* fn ) ( S ) ) :
  m_obj( obj ),
  m_fn( nullptr ),
  m_fnVal( fn ),
  m_isCalled( false )
{}

template < typename T, typename S >
aeSignal< T, S >::aeSignal( T* obj, void ( T::* fn ) ( S ), const S& value ) :
  aeSignalT< S >( value ),
  m_obj( obj ),
  m_fn( nullptr ),
  m_fnVal( fn ),
  m_isCalled( false )
{}

template < typename T, typename S >
aeSignal< T, S >::aeSignal( aeRef< T >& ref, void ( T::* fn ) () ) :
  m_obj( nullptr ),
  m_ref( ref ),
  m_fn( fn ),
  m_fnVal( nullptr ),
  m_isCalled( false )
{}

template < typename T, typename S >
aeSignal< T, S >::aeSignal( aeRef< T >& ref, void ( T::* fn ) ( S ) ) :
  m_obj( nullptr ),
  m_ref( ref ),
  m_fn( nullptr ),
  m_fnVal( fn ),
  m_isCalled( false )
{}

template < typename T, typename S >
aeSignal< T, S >::aeSignal( aeRef< T >& ref, void ( T::* fn ) ( S ), const S& value ) :
  aeSignalT< S >( value ),
  m_obj( nullptr ),
  m_ref( ref ),
  m_fn( nullptr ),
  m_fnVal( fn ),
  m_isCalled( false )
{}

template < typename T, typename S >
aeSignal< T, S >::~aeSignal()
{
  AE_ASSERT_MSG( !m_isCalled, "Signal call destroyed itself" );
}

template < typename T, typename S >
bool aeSignal< T, S >::Send()
{
  AE_ASSERT_MSG( !m_isCalled, "aeSignal::Send() called recursively" );

  T* obj = m_obj;
  obj = obj ? obj : (T*)m_ref;

  if ( !obj )
  {
    return false;
  }

  if ( m_fn )
  {
    m_isCalled = true;

    ( obj->*m_fn )();

    m_isCalled = false;
    return true;
  }
  else if ( m_fnVal && aeSignalT< S >::m_valSet )
  {
    m_isCalled = true;

    ( obj->*m_fnVal )( aeSignalT< S >::m_val );

    m_isCalled = false;
    return true;
  }
  else
  {
    return false;
  }
}

template < typename T, typename S >
bool aeSignal< T, S >::Send( const S& value )
{
  AE_ASSERT_MSG( !m_isCalled, "aeSignal::Send() called recursively" );

  T* obj = m_obj;
  obj = obj ? obj : (T*)m_ref;
  
  if ( !obj || !m_fnVal )
  {
    return false;
  }

  m_isCalled = true;

  ( obj->*m_fnVal )( value );

  m_isCalled = false;
  return true;
}

// @TODO: Const
template < typename T, typename S >
void* aeSignal< T, S >::GetObj()
{
  T* obj = m_obj;
  obj = obj ? obj : (T*)m_ref;
  return obj;
}

//------------------------------------------------------------------------------
// aeSignalList member functions
//------------------------------------------------------------------------------
template < typename S >
aeSignalList< S >::~aeSignalList()
{
  for ( uint32_t i = 0; i < m_signals.Length(); i++ )
  {
    aeAlloc::Release( m_signals[ i ] );
  }
}

template < typename S >
template < typename T, typename Fn >
void aeSignalList< S >::Add( T* obj, Fn fn )
{
  if ( !obj )
  {
    return;
  }

  auto findFn = [ obj ]( aeSignalBase* signal, int )
  {
    // @HACK: Should also check fn, so an object can register multiple functions at a time
    return signal->GetObj() == obj;
  };
  int32_t signalIndex = m_signals.Find( findFn, 0 );
  if ( signalIndex >= 0 )
  {
    *m_signals[ signalIndex ] = aeSignal< T, S >( obj, fn );
    return;
  }

  m_signals.Append( aeAlloc::Allocate< aeSignal< T, S > >( obj, fn ) );
}

template < typename S >
template < typename T, typename Fn >
void aeSignalList< S >::Add( aeRef< T > ref, Fn fn )
{
  T* obj = (T*)ref;
  if ( !obj )
  {
    return;
  }

  auto findFn = [ obj ]( aeSignalBase* signal, int )
  {
    // @HACK: Should also check fn, so an object can register multiple functions at a time
    return signal->GetObj() == obj;
  };
  int32_t signalIndex = m_signals.Find( findFn, 0 );
  if ( signalIndex >= 0 )
  {
    *m_signals[ signalIndex ] = aeSignal< T, S >( ref, fn );
    return;
  }

  m_signals.Append( aeAlloc::Allocate< aeSignal< T, S > >( ref, fn ) );
}

template < typename S >
void aeSignalList< S >::Remove( void* obj )
{
  auto fn = []( aeSignalBase* s, void* obj )
  {
    void* o = s->GetObj();
    return !o || ( o == obj );
  };

  int32_t index = 0;
  while ( ( index = m_signals.Find( fn, obj ) ) >= 0 )
  {
    aeAlloc::Release( m_signals[ index ] );
    m_signals.Remove( index ); // Remove signals for the given obj and any signals with null references
  }
}

template < typename S >
void aeSignalList< S >::Send()
{
  auto fn = []( aeSignalBase* s, int )
  {
    return !s->GetObj();
  };

  int32_t index = 0;
  while ( ( index = m_signals.Find( fn, 0 ) ) >= 0 )
  {
    aeAlloc::Release( m_signals[ index ] );
    m_signals.Remove( index ); // Remove null references before send
  }

  for ( uint32_t i = 0; i < m_signals.Length(); i++ )
  {
    aeSignalT< S >* signal = m_signals[ i ];
    ((aeSignalBase*)signal)->Send();
  }
}

template < typename S >
template < typename T >
void aeSignalList< S >::Send( const T& value )
{
  auto fn = []( aeSignalBase* s, int )
  {
    return !s->GetObj();
  };

  int32_t index = -1;
  while ( ( index = m_signals.Find( fn, 0 ) ) >= 0 )
  {
    aeAlloc::Release( m_signals[ index ] );
    m_signals.Remove( index ); // Remove null references before send
  }

  for ( uint32_t i = 0; i < m_signals.Length(); i++ )
  {
    aeSignalT< S >* signal = m_signals[ i ];
    if ( !signal->Send( value ) )
    {
      ((aeSignalBase*)signal)->Send();
    }
  }
}

template < typename S >
uint32_t aeSignalList< S >::Length() const
{
  return m_signals.Length();
}

#endif
