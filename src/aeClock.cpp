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
// aeClock functions
//------------------------------------------------------------------------------
double aeClock::GetTime()
{
#if _AE_WINDOWS_
  static LARGE_INTEGER counterFrequency = { 0 };
  if ( !counterFrequency.QuadPart )
  {
    bool success = QueryPerformanceFrequency( &counterFrequency ) != 0;
    AE_ASSERT( success );
  }

  LARGE_INTEGER performanceCount = { 0 };
  bool success = QueryPerformanceCounter( &performanceCount ) != 0;
  AE_ASSERT( success );
  return performanceCount.QuadPart / (double)counterFrequency.QuadPart;
#else
  return std::chrono::duration_cast< std::chrono::microseconds >( std::chrono::steady_clock::now().time_since_epoch() ).count() / 1000000.0;
#endif
}

//------------------------------------------------------------------------------
// aeFixedTimeStep member functions
//------------------------------------------------------------------------------
aeFixedTimeStep::aeFixedTimeStep()
{
  m_stepCount = 0;
  m_timeStep = 0.0f;
  m_frameExcess = 0;
  m_prevFrameTime = 0.0f;

  SetTimeStep( 1.0f / 60.0f );
}

void aeFixedTimeStep::Wait()
{
  if ( m_timeStep == 0.0f )
  {
    return;
  }

  if ( m_stepCount == 0 )
  {
    m_prevFrameTime = m_timeStep;
    m_frameStart = std::chrono::steady_clock::now();
  }
  else
  {
    std::chrono::steady_clock::time_point execFinish = std::chrono::steady_clock::now();
    std::chrono::microseconds execDuration = std::chrono::duration_cast< std::chrono::microseconds >( execFinish - m_frameStart );
    
    int64_t prevFrameExcess = m_prevFrameTime - m_timeStep;
    m_frameExcess = ( m_frameExcess * 0.5f + prevFrameExcess * 0.5f ) + 0.5f;

    int64_t wait = m_timeStep - execDuration.count();
    wait -= ( m_frameExcess > 0 ) ? m_frameExcess : 0;
    if ( 1000 < wait && wait < m_timeStep )
    {
      std::this_thread::sleep_for( std::chrono::microseconds( wait ) );
    }
    std::chrono::steady_clock::time_point frameFinish = std::chrono::steady_clock::now();
    std::chrono::microseconds frameDuration = std::chrono::duration_cast< std::chrono::microseconds >( frameFinish - m_frameStart );

    m_prevFrameTime = frameDuration.count();
    m_frameStart = std::chrono::steady_clock::now();
  }
  
  m_prevFrameTimeSec = m_prevFrameTime / 1000000.0f;
  
  m_stepCount++;
}

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
