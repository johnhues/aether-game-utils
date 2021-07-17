//------------------------------------------------------------------------------
// aeCommandLineArgs.cpp
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
// Headers
//------------------------------------------------------------------------------
#include "aeCommandLineArgs.h"

//------------------------------------------------------------------------------
// Helper
//------------------------------------------------------------------------------
int32_t aeCommandLineArgs_IsNumeric( const char * s )
{
    if ( s == NULL || *s == '\0' || isspace(*s) )
    {
      return 0;
    }
    char * p;
    strtod (s, &p);
    return *p == '\0';
}

//------------------------------------------------------------------------------
// aeCommandLineArgs member functions
//------------------------------------------------------------------------------
aeCommandLineArgs::aeCommandLineArgs()
{
  m_count = 0;
}

void aeCommandLineArgs::Parse( int32_t argc, char* argv[] )
{
  m_count = 0;

  uint32_t currentIdx = 0;
  for ( int32_t i = 1; i < argc; i++ )
  {
    const char* arg = argv[ i ];
    if ( !arg[ 0 ] )
    {
      continue;
    }

    if ( arg[ 0 ] != '-' || aeCommandLineArgs_IsNumeric( arg ) )
    {
      if ( m_values[ currentIdx ].Empty() )
      {
        m_values[ currentIdx ] = arg;
      }
      else
      {
        m_values[ currentIdx ].Append( " " );
        m_values[ currentIdx ].Append( arg );
      }
    }
    else
    {
      AE_ASSERT( arg[ 0 ] == '-' );
      arg++;
      if ( arg[ 0 ] != '-' && arg[ 0 ] )
      {
        bool found = false;
        for ( uint32_t j = 0; j < m_count; j++ )
        {
          if ( m_names[ j ] == arg )
          {
            currentIdx = j;
            found = true;
            break;
          }
        }

        if ( !found )
        {
          currentIdx = m_count;
          m_count++;
        }

        m_names[ currentIdx ] = arg;
        m_values[ currentIdx ] = "";
      }
    }
  }
}

ae::Dict aeCommandLineArgs::Get() const
{
  ae::Dict args = AE_ALLOC_TAG_FIXME;
  for ( uint32_t i = 0; i < m_count; i++ )
  {
    args.SetString( GetName( i ), GetValue( i ) );
  }
  return args;
}

const char* aeCommandLineArgs::GetValue( const char* name ) const
{
  for ( uint32_t i = 0; i < m_count; i++ )
  {
    if ( m_names[ i ] == name )
    {
      return m_values[ i ].c_str();
    }
  }
  return "";
}

const char* aeCommandLineArgs::GetName( uint32_t index ) const
{
  if ( index < m_count )
  {
    return m_names[ index ].c_str();
  }
  return "";
}

const char* aeCommandLineArgs::GetValue( uint32_t index ) const
{
  if ( index < m_count )
  {
    return m_values[ index ].c_str();
  }
  return "";
}

bool aeCommandLineArgs::Has( const char* name ) const
{
  for ( uint32_t i = 0; i < m_count; i++ )
  {
    if ( m_names[ i ] == name )
    {
      return true;
    }
  }
  return false;
}
