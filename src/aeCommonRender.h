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

// SDL
#if _AE_WINDOWS_
#include <SDL.h>
#include "SDL_syswm.h"
#elif _AE_EMSCRIPTEN_
#include <SDL/SDL.h>
#elif _AE_LINUX_
#include <SDL.h>
#else
#include <SDL.h>
#endif

// OpenGL
#if _AE_WINDOWS_
  #define GLEW_STATIC 1
  #include <GL\glew.h>
#elif _AE_EMSCRIPTEN_
  #include <GLES2/gl2.h>
#elif _AE_LINUX_
  #include <GL/gl.h>
  #include <GLES3/gl3.h>
#else
  #include <OpenGL/gl3.h>
#endif

// Vulkan
#if _AE_WINDOWS_
  //#pragma comment(linker, "/subsystem:console")
  //#ifndef WIN32_LEAN_AND_MEAN
  //  #define WIN32_LEAN_AND_MEAN
  //#endif
  #ifndef VK_USE_PLATFORM_WIN32_KHR
    #define VK_USE_PLATFORM_WIN32_KHR
  #endif
#elif _AE_APPLE_
  #include <MoltenVK/mvk_vulkan.h>
#else
  #include "vulkan/vk_sdk_platform.h"
#endif

#include <vulkan/vulkan.h>

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

private:
  void* m_context;
  int32_t m_defaultFbo;
};

//------------------------------------------------------------------------------
// aeVulkanRender class
//------------------------------------------------------------------------------
class aeVulkanRender : public aeRenderInternal
{
public:
  aeVulkanRender();

  void Initialize( aeRender* render ) override;
  void Terminate( aeRender* render ) override;
  void StartFrame( aeRender* render ) override;
  void EndFrame( aeRender* render ) override;

private:
  struct SwapChainBuffer
  {
    VkImage image = 0;
    VkImageView view = 0;
  };

  VkInstance m_inst = 0;
  VkDebugReportCallbackEXT m_debugCallback = 0;
  VkSurfaceKHR m_surface = 0;
  VkDevice m_device = 0;
  VkCommandPool m_cmdPool = 0;
  VkCommandBuffer m_cmd = 0;
  VkQueue m_graphicsQueue = 0;
  VkQueue m_presentQueue = 0;
  VkSwapchainKHR m_swapChain = 0;
  aeArray< SwapChainBuffer > m_swapChainBuffers;

  VkImage m_depthImage = 0;
  VkDeviceMemory m_depthMem = 0;
  VkImageView m_depthView = 0;

  uint32_t m_currentBuffer = 0; // @TODO: Does this need to be a member var?
};

#endif
