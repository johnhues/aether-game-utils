//------------------------------------------------------------------------------
// aeTesting.cpp
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
#include "ae/aeTesting.h"
namespace ae {

//------------------------------------------------------------------------------
// LifetimeTester static members
//------------------------------------------------------------------------------
const uint32_t LifetimeTester::kConstructed = 0xABABABAB;
const uint32_t LifetimeTester::kMoved = 0xBCBCBCBC;

int32_t LifetimeTester::ctorCount = 0;
int32_t LifetimeTester::copyCount = 0;
int32_t LifetimeTester::moveCount = 0;
int32_t LifetimeTester::copyAssignCount = 0;
int32_t LifetimeTester::moveAssignCount = 0;
int32_t LifetimeTester::dtorCount = 0;
int32_t LifetimeTester::currentCount = 0;

//------------------------------------------------------------------------------
// LifetimeTester member functions
//------------------------------------------------------------------------------
LifetimeTester::LifetimeTester()
{
  check = kConstructed;
  value = 0;
  
  ctorCount++;
  currentCount++;
}

LifetimeTester::LifetimeTester( const LifetimeTester& o ) // copy ctor
{
  AE_ASSERT( o.check == kConstructed );
  check = o.check;
  value = o.value;
  
  copyCount++;
  currentCount++;
}

LifetimeTester::LifetimeTester( LifetimeTester&& o ) noexcept // move ctor
{
  AE_ASSERT( o.check == kConstructed );
  check = o.check;
  value = o.value;
  o.check = kMoved;
  
  moveCount++;
  currentCount++;
}

LifetimeTester& LifetimeTester::operator=( const LifetimeTester& o ) // copy assignment
{
  AE_ASSERT( check == kConstructed || check == kMoved );
  AE_ASSERT( o.check == kConstructed );
  check = o.check;
  value = o.value;
  
  copyAssignCount++;
  
  return *this;
}

LifetimeTester& LifetimeTester::operator=( LifetimeTester&& o ) noexcept // move assignment
{
  AE_ASSERT( check == kConstructed || check == kMoved );
  AE_ASSERT( o.check == kConstructed );
  check = o.check;
  value = o.value;
  o.check = kMoved;
  
  moveAssignCount++;
  
  return *this;
}

LifetimeTester::~LifetimeTester()
{
  AE_ASSERT( check == kConstructed || check == kMoved );

  dtorCount++;
  currentCount--;
}

void LifetimeTester::ClearStats()
{
  ctorCount = 0;
  copyCount = 0;
  moveCount = 0;
  copyAssignCount = 0;
  moveAssignCount = 0;
  dtorCount = 0;
  currentCount = 0;
}

} // ae end
