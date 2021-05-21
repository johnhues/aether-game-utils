//------------------------------------------------------------------------------
// aeSpline.h
// Utilities for allocating objects. Provides functionality to track current and
// past allocations.
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
#ifndef AESPLINE_H
#define AESPLINE_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeMath.h"

//------------------------------------------------------------------------------
// aeSpline class
//------------------------------------------------------------------------------
class aeSpline
{
public:
  aeSpline( ae::Tag tag );
  aeSpline( ae::Tag tag, aeFloat3* controlPoints, uint32_t count );

  void AppendControlPoint( aeFloat3 p );
  void RemoveControlPoint( uint32_t index );
  void SetLooping( bool enabled );

  aeFloat3 GetControlPoint( uint32_t index ) const;
  uint32_t GetControlPointCount() const;

  aeFloat3 GetPoint( float distance ) const; // 0 <= distance <= length
  float GetMinDistance( aeFloat3 p, aeFloat3* nearestOut = nullptr );
  float GetLength() const;

  aeAABB GetAABB() const { return m_aabb; }

private:
  class Segment
  {
  public:
    void Init( aeFloat3 p0, aeFloat3 p1, aeFloat3 p2, aeFloat3 p3 );
    aeFloat3 GetPoint01( float t ) const;
    aeFloat3 GetPoint0() const;
    aeFloat3 GetPoint1() const;
    aeFloat3 GetPoint( float d ) const;
    float GetMinDistance( aeFloat3 p, aeFloat3* pOut ) const;
    float GetLength() const { return m_length; }
    aeAABB GetAABB() const { return m_aabb; }

  private:
    aeFloat3 m_a;
    aeFloat3 m_b;
    aeFloat3 m_c;
    aeFloat3 m_d;
    float m_length;
    uint32_t m_resolution;
    aeAABB m_aabb;
  };

  void m_RecalculateSegments();
  aeFloat3 m_GetControlPoint( int32_t index ) const;

  bool m_loop = false;
  ae::Array< aeFloat3 > m_controlPoints;
  ae::Array< Segment > m_segments;
  float m_length = 0.0f;
  aeAABB m_aabb;
};

#endif
