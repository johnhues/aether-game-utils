//------------------------------------------------------------------------------
// TestConfig.h
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
#ifndef TESTCONFIG_H
#define TESTCONFIG_H
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include <cstdint>

//------------------------------------------------------------------------------
// Config defines
//------------------------------------------------------------------------------
#define AE_ASSERT_IMPL( msgStr ) throw "assert" // Throw exceptions so unit tests can test asserts
#define AE_MEMORY_CHECKS 1 // Enable strict memory checks for unit tests

#define AE_LOG_FUNCTION_CONFIG TestLogger
namespace ae { enum class LogSeverity; }
void TestLogger( ae::LogSeverity severity, const char* filePath, uint32_t line, const char** tags, uint32_t tagCount, const char* message );

#endif // TESTCONFIG_H
