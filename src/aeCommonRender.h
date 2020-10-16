//------------------------------------------------------------------------------
// aeCommonRender.h
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
#ifndef AECOMMONRENDER_H
#define AECOMMONRENDER_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeRender.h"

//------------------------------------------------------------------------------
// aeRenderInternal class
//------------------------------------------------------------------------------
class aeRenderInternal
{
public:
  virtual ~aeRenderInternal() {}

  virtual void Initialize( aeRender* render ) = 0;
  virtual void Terminate( aeRender* render ) = 0;
  virtual void StartFrame( aeRender* render ) = 0;
  virtual void EndFrame( aeRender* render ) = 0;
};

//------------------------------------------------------------------------------
// aeOpenGLRender class
//------------------------------------------------------------------------------
class aeOpenGLRender : public aeRenderInternal
{
public:
  aeOpenGLRender();

  void Initialize( aeRender* render ) override;
  void Terminate( aeRender* render ) override;
  void StartFrame( aeRender* render ) override;
  void EndFrame( aeRender* render ) override;

  // this is so imgui and the main render copy can enable srgb writes in GL
  void EnableSRGBWrites( aeRender* render, bool enable );

private:
  void* m_context;
  int32_t m_defaultFbo;
};

#endif
