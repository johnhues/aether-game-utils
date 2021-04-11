//------------------------------------------------------------------------------
// aeClock.h
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
#ifndef AECLOCK_H
#define AECLOCK_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include <chrono>

//------------------------------------------------------------------------------
// aeClock
//------------------------------------------------------------------------------
namespace aeClock
{
  double GetTime();
};

//------------------------------------------------------------------------------
// aeFixedTimeStep
//------------------------------------------------------------------------------
class aeFixedTimeStep
{
public:
  aeFixedTimeStep();

  void SetTimeStep( float timeStep ) { m_timeStepSec = timeStep; m_timeStep = timeStep * 1000000.0f; }
  float GetTimeStep() const { return m_timeStepSec; }
  uint32_t GetStepCount() const { return m_stepCount; }

  float GetDT() const { return m_prevFrameTimeSec; }
  float SetDT( float sec ) { m_prevFrameTimeSec = sec; } // Useful for handling frames with high delta time, eg: timeStep.SetDT( timeStep.GetTimeStep() )

  void Wait();

private:
  uint32_t m_stepCount = 0;
  float m_timeStepSec = 0.0f;
  float m_timeStep = 0.0f;
  int64_t m_frameExcess = 0;
  float m_prevFrameTime = 0.0f;
  float m_prevFrameTimeSec = 0.0f;
  std::chrono::steady_clock::time_point m_frameStart;
};

//------------------------------------------------------------------------------
// aeTicker
//------------------------------------------------------------------------------
class aeTicker
{
public:
  aeTicker( double interval );
  bool Tick( double currentTime );

private:
  double m_interval;
  double m_accumulate;
};

#endif
