//------------------------------------------------------------------------------
// aeInventoryGrid.h
// 2D sparse grid which stores elements that occupy an arbitrary number of cells
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
#ifndef AEINVENTORYGRID_H
#define AEINVENTORYGRID_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeArray.h"
#include "aeList.h"
#include "aeLog.h"
#include "aeMath.h"

//------------------------------------------------------------------------------
// aeInventoryGrid class
//------------------------------------------------------------------------------
template < typename T >
class aeInventoryGrid
{
public:
  ~aeInventoryGrid();

  void Set( T& value, aeRectInt rect );
  void Set( T& value, aeInt2* cells, uint32_t cellCount );

  T* TryGet( aeInt2 pos );
  bool TryGet( aeInt2 pos, T* valueOut );
  const T* TryGet( aeInt2 pos ) const;

  void Remove( aeInt2 pos );
  void Remove( const T& value );

  uint32_t Length() const;

private:
  struct Shape
  {
    Shape() : node( this ) {}

    aeListNode< Shape > node;
    aeArray< aeInt2 > cells;
    T value;
  };

  aeList< Shape > m_shapeList;
};

template < typename T >
aeInventoryGrid< T >::~aeInventoryGrid()
{
  while ( m_shapeList.GetLast() )
  {
    aeAlloc::Release( m_shapeList.GetLast() );
  }
}

template < typename T >
void aeInventoryGrid< T >::Set( T& value, aeRectInt rect )
{
  aeArray< aeInt2 > cells( rect.w * rect.h );
  for ( uint32_t y = 0; y < rect.h; y++ )
  {
    for ( uint32_t x = 0; x < rect.w; x++ )
    {
      cells.Append( aeInt2( rect.x + x, rect.y + y ) );
    }
  }
  Set( value, &( cells[ 0 ] ), cells.Length() );
}

template < typename T >
void aeInventoryGrid< T >::Set( T& value, aeInt2* cells, uint32_t cellCount )
{
  for ( uint32_t i = 0; i < cellCount; i++ )
  {
    T* other = TryGet( cells[ i ] );
    AE_ASSERT_MSG( !other, "Cell # already occupied by #", cells[ i ], other );
  }

  Shape* shape = aeAlloc::Allocate< Shape >();
  AE_ASSERT( shape );

  shape->value = value;
  shape->cells.Append( cells, cellCount );
  m_shapeList.Append( shape->node );
}

template < typename T >
T* aeInventoryGrid< T >::TryGet( aeInt2 pos )
{
  return const_cast<T*>( const_cast<const aeInventoryGrid< T >*>( this )->TryGet( pos ) );
}

template < typename T >
bool aeInventoryGrid< T >::TryGet( aeInt2 pos, T* valueOut )
{
  const T* result = TryGet( pos );
  if ( result )
  {
    *valueOut = *result;
    return true;
  }
  return false;
}

template < typename T >
const T* aeInventoryGrid< T >::TryGet( aeInt2 pos ) const
{
  const Shape* shape = m_shapeList.GetFirst();
  while ( shape )
  {
    if ( shape->cells.Find( pos ) >= 0 )
    {
      return &shape->value;
    }
    shape = shape->node.GetNext();
  }

  return nullptr;
}

template < typename T >
void aeInventoryGrid< T >::Remove( aeInt2 pos )
{
  if ( m_shapeList.Length() == 0 )
  {
    return;
  }

  auto fn = [ pos ]( Shape* shape )
  {
    return ( shape->cells.Find( pos ) >= 0 );
  };
  Shape* shape = m_shapeList.FindFn( fn );
  if ( shape )
  {
    aeAlloc::Release( shape );
  }
}

template < typename T >
void aeInventoryGrid< T >::Remove( const T& value )
{
  AE_ASSERT( m_shapeList.Length() );

  auto fn = [ value ]( Shape* shape )
  {
    return ( shape->value == value );
  };
  Shape* shape = m_shapeList.FindFn( fn );
  AE_ASSERT( shape );
  aeAlloc::Release( shape );
}

template < typename T >
uint32_t aeInventoryGrid< T >::Length() const
{
  return m_shapeList.Length();
}

#endif
