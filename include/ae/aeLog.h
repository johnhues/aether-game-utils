//------------------------------------------------------------------------------
// aeLog.h
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
#ifndef AELOG_H
#define AELOG_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aePlatform.h"
#include "aeLog.hpp"

//------------------------------------------------------------------------------
// Logging functions
//------------------------------------------------------------------------------
#define AE_TRACE(...) aeLogInternal( _AE_LOG_TRACE_, __FILE__, __LINE__, "", __VA_ARGS__ )
#define AE_DEBUG(...) aeLogInternal( _AE_LOG_DEBUG_, __FILE__, __LINE__, "", __VA_ARGS__ )
#define AE_LOG(...) aeLogInternal( _AE_LOG_INFO_, __FILE__, __LINE__, "", __VA_ARGS__ )
#define AE_INFO(...) aeLogInternal( _AE_LOG_INFO_, __FILE__, __LINE__, "", __VA_ARGS__ )
#define AE_WARN(...) aeLogInternal( _AE_LOG_WARN_, __FILE__, __LINE__, "", __VA_ARGS__ )
#define AE_ERR(...) aeLogInternal( _AE_LOG_ERROR_, __FILE__, __LINE__, "", __VA_ARGS__ )

//------------------------------------------------------------------------------
// Assertion functions
//------------------------------------------------------------------------------
// @TODO: Use __analysis_assume( x ); on windows to prevent warning C6011 (Dereferencing NULL pointer)
#define AE_ASSERT( _x ) do { if ( !(_x) ) { aeLogInternal( _AE_LOG_FATAL_, __FILE__, __LINE__, "AE_ASSERT( " #_x " )", "" ); aeAssert(); } } while (0)
#define AE_ASSERT_MSG( _x, ... ) do { if ( !(_x) ) { aeLogInternal( _AE_LOG_FATAL_, __FILE__, __LINE__, "AE_ASSERT( " #_x " )", __VA_ARGS__ ); aeAssert(); } } while (0)
#define AE_FAIL() do { aeLogInternal( _AE_LOG_FATAL_, __FILE__, __LINE__, "", "" ); aeAssert(); } while (0)
#define AE_FAIL_MSG( ... ) do { aeLogInternal( _AE_LOG_FATAL_, __FILE__, __LINE__, "", __VA_ARGS__ ); aeAssert(); } while (0)

//------------------------------------------------------------------------------
// Static assertion functions
//------------------------------------------------------------------------------
#define AE_STATIC_ASSERT( _x ) static_assert( _x, "static assert" )
#define AE_STATIC_ASSERT_MSG( _x, _m ) static_assert( _x, _m )
#define AE_STATIC_FAIL( _m ) static_assert( 0, _m )

#endif
