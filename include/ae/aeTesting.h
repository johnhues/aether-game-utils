//------------------------------------------------------------------------------
// aeTesting.h
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
#ifndef AE_TESTING_H
#define AE_TESTING_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"

namespace ae {

//------------------------------------------------------------------------------
// LifeTimeTester class
//------------------------------------------------------------------------------
class LifetimeTester
{
public:
  LifetimeTester(); // default
  LifetimeTester( const LifetimeTester& ); // copy
  LifetimeTester( LifetimeTester&& ) noexcept; // move
  LifetimeTester& operator=( const LifetimeTester& ); // copy assignment
  LifetimeTester& operator=( LifetimeTester&& ) noexcept; // move assignment
  ~LifetimeTester();
  
  static const uint32_t kConstructed;
  static const uint32_t kMoved;
  
  uint32_t check;
  
  static void ClearStats();
  
  static int32_t ctorCount;
  static int32_t copyCount;
  static int32_t moveCount;
  static int32_t copyAssignCount;
  static int32_t moveAssignCount;
  static int32_t dtorCount;
  static int32_t currentCount;
};

} // ae end

#endif
