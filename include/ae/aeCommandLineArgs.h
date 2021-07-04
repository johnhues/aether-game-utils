//------------------------------------------------------------------------------
// aeCommandLineArgs.h
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
#ifndef AECOMMANDLINEARGS_H
#define AECOMMANDLINEARGS_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"
#include "aeString.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const uint32_t kMaxCommandLineArgs = 16;

//------------------------------------------------------------------------------
// aeCommandLineArgs
//------------------------------------------------------------------------------
class aeCommandLineArgs
{
public:
  aeCommandLineArgs();
  void Parse( int32_t argc, char* argv[] );

  ae::Dict Get() const;
  uint32_t Count() const { return m_count; }
  const char* GetName( uint32_t index ) const;
  const char* GetValue( uint32_t index ) const;
  const char* GetValue( const char* name ) const;
  bool Has( const char* name ) const;

private:
  aeStr32 m_names[ kMaxCommandLineArgs ];
  aeStr128 m_values[ kMaxCommandLineArgs ];
  uint32_t m_count;
};

#endif
