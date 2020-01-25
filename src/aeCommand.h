//------------------------------------------------------------------------------
// Game.h
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
#ifndef AECOMMAND_H
#define AECOMMAND_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include <cstdio>
#include <cstring>
#include <vector>
#include "aeString.h"

//------------------------------------------------------------------------------
// Internal
//------------------------------------------------------------------------------
struct gdn_cmd
{
  struct cmd
  {
    char name[ 32 ];
    void (*fn)( void*, const char* );
  };
  static std::vector<cmd>& GetDefs()
  {
    static std::vector<cmd> defs;
    return defs;
  }
};

template< typename T >
inline T gdn_meta_type_parse( const char* str )
{
  return T();
};

template<>
inline int32_t gdn_meta_type_parse( const char* str )
{
  int32_t i = 0;
  sscanf( str, "%d", &i );
  return i;
};

template<>
inline uint32_t gdn_meta_type_parse( const char* str )
{
  uint32_t i = 0;
  sscanf( str, "%u", &i );
  return i;
};

template<>
inline float gdn_meta_type_parse( const char* str )
{
  float f = 0.0f;
  sscanf( str, "%f", &f );
  return f;
};

template<>
inline bool gdn_meta_type_parse( const char* str )
{
  if ( !str[ 0 ] )
  {
    // @NOTE: Default to true when no parameter is specified
    return true;
  }
  return gdn_meta_type_parse< int32_t >( str ) != 0;
};

template<>
inline const char* gdn_meta_type_parse( const char* str )
{
  return str;
};

template< typename R >
inline void gdn_run_cmd( void* data, R (*fn)( void* ), const char* params )
{
  fn( data );
}

template< typename R, typename P >
inline void gdn_run_cmd( void* data, R (*fn)( void*, P ), const char* params )
{
  char str[ 128 ];
  if ( strlcpy( str, params, countof(str) ) >= countof(str) )
  {
    return;
  }

  char* nextToken = nullptr;
  char* token = strtok_r( str, " ", &nextToken );
  if ( !token )
  {
    return;
  }

  fn( data, gdn_meta_type_parse< P >( token ) );
}

template< typename R, typename P0, typename P1 >
inline void gdn_run_cmd( void* data, R (*fn)( void*, P0, P1 ), const char* params )
{
  char str[ 128 ];
  if ( strlcpy( str, params, countof(str) ) >= countof(str) )
  {
    return;
  }

  char* nextToken = nullptr;
  char* token0 = strtok_r( str, " ", &nextToken );
  char* token1 = strtok_r( nullptr, " ", &nextToken );
  if ( !token0 || !token1 )
  {
    return;
  }

  P0 p0 = gdn_meta_type_parse< P0 >( token0 );
  P1 p1 = gdn_meta_type_parse< P1 >( token1 );
  fn( data, p0, p1 );
}

template< typename R, typename P0, typename P1, typename P2 >
inline void gdn_run_cmd( void* data, R (*fn)( void*, P0, P1, P2 ), const char* params )
{
  char str[ 128 ];
  if ( strlcpy( str, params, countof(str) ) >= countof(str) )
  {
    return;
  }

  char* nextToken = nullptr;
  char* token0 = strtok_r( str, " ", &nextToken );
  char* token1 = strtok_r( nullptr, " ", &nextToken );
  char* token2 = strtok_r( nullptr, " ", &nextToken );
  if ( !token0 || !token1 || !token2 )
  {
    return;
  }

  P0 p0 = gdn_meta_type_parse< P0 >( token0 );
  P1 p1 = gdn_meta_type_parse< P1 >( token1 );
  P2 p2 = gdn_meta_type_parse< P2 >( token2 );
  fn( data, p0, p1, p2 );
}

template< typename R, typename P0, typename P1, typename P2, typename P3 >
inline void gdn_run_cmd( void* data, R (*fn)( void*, P0, P1, P2, P3 ), const char* params )
{
  char str[ 128 ];
  if ( strlcpy( str, params, countof(str) ) >= countof(str) )
  {
    return;
  }

  char* nextToken = nullptr;
  char* token0 = strtok_r( str, " ", &nextToken );
  char* token1 = strtok_r( nullptr, " ", &nextToken );
  char* token2 = strtok_r( nullptr, " ", &nextToken );
  char* token3 = strtok_r( nullptr, " ", &nextToken );
  if ( !token0 || !token1 || !token2 || !token3 )
  {
    return;
  }

  P0 p0 = gdn_meta_type_parse< P0 >( token0 );
  P1 p1 = gdn_meta_type_parse< P1 >( token1 );
  P2 p2 = gdn_meta_type_parse< P2 >( token2 );
  P3 p3 = gdn_meta_type_parse< P3 >( token3 );
  fn( data, p0, p1, p2, p3 );
}

//------------------------------------------------------------------------------
// External
//------------------------------------------------------------------------------
#define AE_CMD( _x )                                              \
void gdn_exec_##_x( void* data, const char* params ) { gdn_run_cmd( data, _x, params ); } \
struct gdn_reg_meta_cmd_##_x {                                          \
  gdn_reg_meta_cmd_##_x() {                                             \
    gdn_cmd::cmd cmd;                                                   \
    strcpy( cmd.name, #_x );                                            \
    cmd.fn = gdn_exec_##_x;                                             \
    gdn_cmd::GetDefs().push_back( cmd );                                     \
  }};                                                                   \
gdn_reg_meta_cmd_##_x inst_gdn_reg_meta_cmd_##_x;

inline bool ae_call_cmd( void* data, const char* cmd )
{
  const char* nameEnd = strchr( cmd, ' ' );
  aeStr32 name = nameEnd ? aeStr32( nameEnd - cmd, cmd ) : aeStr32( cmd );
  uint32_t count = gdn_cmd::GetDefs().size();
  for ( uint32_t i = 0; i < count; i++ )
  {
    if ( name == gdn_cmd::GetDefs()[ i ].name )
    {
      gdn_cmd::GetDefs()[ i ].fn( data, nameEnd ? ( nameEnd + 1 ) : "" );
      return true;
    }
  }
  return false;
}

#endif
