//------------------------------------------------------------------------------
// aeAlloc.cpp
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
#include "aeAlloc.h"
#include "aeDict.h"

//------------------------------------------------------------------------------
// AllocationInfo members and state
//------------------------------------------------------------------------------
#if _AE_ALLOC_DISABLE
#include <map>
std::map< void*, int > allocMap;

void PushArrayAlloc( void* a )
{
  allocMap[ a ] = 0;
}

bool PopArrayAlloc( void* a )
{
  auto iter = allocMap.find( a );
  if ( iter != allocMap.end() )
  {
    allocMap.erase( iter );
    return true;
  }
  else
  {
    return false;
  }
}
#endif

namespace
{
  uint32_t g_allocations = 0;
  uint32_t g_deallocations = 0;
  uint32_t g_allocatedBytes = 0;
  uint32_t g_deallocatedBytes = 0;

  // @TODO: Use static map or something to avoid using aeDict which uses strings internally.
  //        aeMap can't be used because it allocates memory, updating aeAllocInfo.
  //aeDict g_allocationCounts;
  //aeDict g_deallocationCounts;
  //aeDict g_allocationBytes;
  //aeDict g_deallocationBytes;
}

aeAllocInfo& GetAllocInfo()
{
  static aeAllocInfo g_info;
  return g_info;
}

uint32_t aeAllocInfo::GetAllocationsTotal() const
{
  return g_allocations;
}

uint32_t aeAllocInfo::GetDeallocationsTotal() const
{
  return g_deallocations;
}

uint32_t aeAllocInfo::GetAllocationBytesTotal() const
{
  return g_allocatedBytes;
}

uint32_t aeAllocInfo::GetDeallocationBytesTotal() const
{
  return g_deallocatedBytes;
}

uint32_t aeAllocInfo::GetAllocationsLength() const
{
  return 0;// g_allocationCounts.Length();
}

const char* aeAllocInfo::GetAllocationName( uint32_t index ) const
{
  return 0;//g_allocationCounts.GetKey( index );
}

uint32_t aeAllocInfo::GetAllocationCount( uint32_t index ) const
{
  return 0;//atoi( g_allocationCounts.GetValue( index ) );
}

uint32_t aeAllocInfo::GetAllocationBytes( uint32_t index ) const
{
  return 0;//atoi( g_allocationBytes.GetValue( index ) );
}

uint32_t aeAllocInfo::GetDeallocationsLength() const
{
  return 0;//g_deallocationCounts.Length();
}

const char* aeAllocInfo::GetDeallocationName( uint32_t index ) const
{
  return 0;//g_deallocationCounts.GetKey( index );
}

uint32_t aeAllocInfo::GetDeallocationCount( uint32_t index ) const
{
  return 0;//atoi( g_deallocationCounts.GetValue( index ) );
}

uint32_t aeAllocInfo::GetDeallocationBytes( uint32_t index ) const
{
  return 0;//atoi( g_deallocationBytes.GetValue( index ) );
}

void aeAllocInfo::Alloc( const char* typeName, uint32_t bytes )
{
  char buf[ aeStr128::MaxLength() + 1 ];
  uint32_t length = aeMath::Min( (uint32_t)strlen( typeName ), aeStr128::MaxLength() );
  memcpy( buf, typeName, length );
  buf[ length ] = 0;

  g_allocations++;
  g_allocatedBytes += bytes;
  //g_allocationCounts.SetInt( buf, g_allocationCounts.GetInt( typeName, 0 ) + 1 );
  //g_allocationBytes.SetInt( buf, g_allocationBytes.GetInt( typeName, 0 ) + bytes );
}

void aeAllocInfo::Dealloc( const char* typeName, uint32_t bytes )
{
  char buf[ aeStr128::MaxLength() + 1 ];
  uint32_t length = aeMath::Min( (uint32_t)strlen( typeName ), aeStr128::MaxLength() );
  memcpy( buf, typeName, length );
  buf[ length ] = 0;

  g_deallocations++;
  g_deallocatedBytes += bytes;
  //g_deallocationCounts.SetInt( buf, g_deallocationCounts.GetInt( typeName, 0 ) + 1 );
  //g_deallocationBytes.SetInt( buf, g_deallocationBytes.GetInt( typeName, 0 ) + bytes );
}
