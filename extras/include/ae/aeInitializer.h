//------------------------------------------------------------------------------
// aeInitializer.h
//------------------------------------------------------------------------------
// Copyright (c) 2025 John Hughes
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
#ifndef AEINITIALIZER_H
#define AEINITIALIZER_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"

//------------------------------------------------------------------------------
// aeInitInfo class
//------------------------------------------------------------------------------
template < typename T >
class aeInitInfo
{
public:
  aeInitInfo() {}
  aeInitInfo( std::function< void( T* ) > i, std::function< void( T* ) > t ) : m_init( i ), m_term( t ) {}
  aeInitInfo( const aeInitInfo& i ) : m_init( i.m_init ), m_term( i.m_term ) {}
  
  void Initialize( T* obj ) { if ( m_init ) { m_init( obj ); } }
  void Terminate( T* obj ) { if ( m_term ) { m_term( obj ); } }

private:
  std::function< void( T* ) > m_init;
  std::function< void( T* ) > m_term;
};

//------------------------------------------------------------------------------
// aeInitializer class
//------------------------------------------------------------------------------
template < typename T >
class aeInitializer
{
public:
  aeInitializer( const aeInitInfo< T >& initInfo ) : m_initInfo( initInfo ) { m_initInfo.Initialize( &m_obj ); }
  ~aeInitializer() { m_initInfo.Terminate( &m_obj ); }

  T& Get() { return m_obj; }
  const T& Get() const { return m_obj; }

private:
  aeInitializer( aeInitializer& ) = delete;
  aeInitializer& operator= ( aeInitializer& ) = delete;

  aeInitInfo< T > m_initInfo;
  T m_obj;
};

#endif
