//------------------------------------------------------------------------------
// aeSparseGrid.h
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
#ifndef AESPARSEGRID_H
#define AESPARSEGRID_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeInitializer.h"

//------------------------------------------------------------------------------
// aeSparseGridZone class
//------------------------------------------------------------------------------
template < typename T, int32_t DimX, int32_t DimY, int32_t DimZ >
class aeSparseGridZone
{
public:
  // aeSparseGrid interface start
  void SetZoneInfo( ae::Int3 offset ) {}

  void Set( ae::Int3 pos, const T& value ) { m_values[ pos.z ][ pos.y ][ pos.x ] = value; }
  T& Get( ae::Int3 pos ) { return m_values[ pos.z ][ pos.y ][ pos.x ]; }
  const T& Get( ae::Int3 pos ) const { return m_values[ pos.z ][ pos.y ][ pos.x ]; }

  static ae::Int3 GetSize() { return ae::Int3( DimX, DimY, DimZ ); }
  typedef T GridType;
  // aeSparseGrid interface end

private:
  T m_values[ DimZ ][ DimY ][ DimX ] = {}; // Default initialization
};

//------------------------------------------------------------------------------
// aeSparseGrid class
//------------------------------------------------------------------------------
template < typename Z >
class aeSparseGrid
{
public:
  aeSparseGrid() {}
  aeSparseGrid( aeInitInfo< Z > initInfo ) : m_initInfo( initInfo ) {}
  ~aeSparseGrid() { Clear(); }

  void Reset( const aeInitInfo< Z >& initInfo ) { Clear(); m_initInfo = initInfo; }
  void Clear();

  void Set( ae::Int3 pos, const typename Z::GridType& value );
  typename Z::GridType* TryGet( ae::Int3 pos );
  const typename Z::GridType* TryGet( ae::Int3 pos ) const;

  uint32_t Length() const;
  Z* GetZone( uint32_t i );
  const Z* GetZone( uint32_t i ) const;
  Z* GetZone( ae::Int3 pos );
  const Z* GetZone( ae::Int3 pos ) const;

private:
  aeSparseGrid( const aeSparseGrid& ) = delete;
  aeSparseGrid& operator=( const aeSparseGrid& ) = delete;

  ae::Int3 m_GetSlot( ae::Int3 pos ) const;
  ae::Int3 m_GetLocal( ae::Int3 pos ) const;

  aeInitInfo< Z > m_initInfo;
  ae::Map< ae::Int3, aeInitializer< Z >* > m_zones = AE_ALLOC_TAG_FIXME;
};

//------------------------------------------------------------------------------
// aeSparseGrid member functions
//------------------------------------------------------------------------------
template < typename Z >
ae::Int3 aeSparseGrid< Z >::m_GetSlot( ae::Int3 pos ) const
{
  const ae::Int3 size = Z::GetSize();
  return ae::Int3(
    ae::Floor( (float)pos.x / size.x ),
    ae::Floor( (float)pos.y / size.y ),
    ae::Floor( (float)pos.z / size.z )
  );
}

template < typename Z >
ae::Int3 aeSparseGrid< Z >::m_GetLocal( ae::Int3 pos ) const
{
  const ae::Int3 size = Z::GetSize();
  return ae::Int3(
    ae::Mod( pos.x, size.x ),
    ae::Mod( pos.y, size.y ),
    ae::Mod( pos.z, size.z )
  );
}

template < typename Z >
void aeSparseGrid< Z >::Clear()
{
  uint32_t zoneCount = m_zones.Length();
  for ( uint32_t i = 0; i < zoneCount; i++ )
  {
    ae::Delete( m_zones.GetValue( i ) );
  }
  m_zones.Clear();
}

template < typename Z >
void aeSparseGrid< Z >::Set( ae::Int3 pos, const typename Z::GridType& value )
{
  ae::Int3 slot = m_GetSlot( pos );
  aeInitializer< Z >* zone = nullptr;
  if ( !m_zones.TryGet( slot, &zone ) )
  {
    const ae::Int3 size = Z::GetSize();
    zone = ae::New< aeInitializer< Z > >( AE_ALLOC_TAG_FIXME, m_initInfo );
    zone->Get().SetZoneInfo( slot * size );
    m_zones.Set( slot, zone );
  }

  ae::Int3 localPos = m_GetLocal( pos );
  zone->Get().Set( localPos, value );
}

// @NOTE: const and non-const TryGet() must be implemented seprately so the correct Z::GridType::Get() is called
template < typename Z >
typename Z::GridType* aeSparseGrid< Z >::TryGet( ae::Int3 pos )
{
  ae::Int3 slot = m_GetSlot( pos );

  aeInitializer< Z >* zone = nullptr;
  if ( m_zones.TryGet( slot, &zone ) )
  {
    ae::Int3 localPos = m_GetLocal( pos );
    return &zone->Get().Get( localPos );
  }

  return nullptr;
}

template < typename Z >
const typename Z::GridType* aeSparseGrid< Z >::TryGet( ae::Int3 pos ) const
{
  ae::Int3 slot = m_GetSlot( pos );

  aeInitializer< Z >* zone = nullptr;
  if ( m_zones.TryGet( slot, &zone ) )
  {
    ae::Int3 localPos = m_GetLocal( pos );
    return &zone->Get().Get( localPos );
  }

  return nullptr;
}

template < typename Z >
uint32_t aeSparseGrid< Z >::Length() const
{
  return m_zones.Length();
}

template < typename Z >
Z* aeSparseGrid< Z >::GetZone( uint32_t i )
{
  return &m_zones.GetValue( i )->Get();
}

template < typename Z >
const Z* aeSparseGrid< Z >::GetZone( uint32_t i ) const
{
  return &m_zones.GetValue( i )->Get();
}

template < typename Z >
Z* aeSparseGrid< Z >::GetZone( ae::Int3 pos )
{
  ae::Int3 slot = m_GetSlot( pos );
  aeInitializer< Z >* zone = nullptr;
  m_zones.TryGet( slot, &zone );
  return zone ? &zone->Get() : nullptr;
}

template < typename Z >
const Z* aeSparseGrid< Z >::GetZone( ae::Int3 pos ) const
{
  ae::Int3 slot = m_GetSlot( pos );
  aeInitializer< Z >* zone = nullptr;
  m_zones.TryGet( slot, &zone );
  return zone ? &zone->Get() : nullptr;
}

#endif
