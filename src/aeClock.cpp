//------------------------------------------------------------------------------
// aeClock.cpp
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
#include <thread>
#include "aether.h"
#if _AE_WINDOWS_
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#endif
#include "aeClock.h"

//------------------------------------------------------------------------------
// aeTicker member functions
//------------------------------------------------------------------------------
aeTicker::aeTicker( double interval )
{
  m_interval = interval;
  m_accumulate = 0.0;
}

bool aeTicker::Tick( double dt )
{
  m_accumulate += dt;

  if ( m_accumulate >= 0.0 )
  {
    m_accumulate -= m_interval;
    return true;
  }

  return false;
}
