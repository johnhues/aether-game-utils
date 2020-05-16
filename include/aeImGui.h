//------------------------------------------------------------------------------
// aeAlloc.h
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
#ifndef AEIMGUI_H
#define AEIMGUI_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#if _AE_WINDOWS_
	#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#elif _AE_APPLE_
	#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM <OpenGL/gl3.h>
#endif
#include "imgui_impl_opengl3.h"

//------------------------------------------------------------------------------
// Imgui helpers
//------------------------------------------------------------------------------
namespace aeImGui
{
	inline void Initialize()
	{
		ImGui::CreateContext();
		// ImGuiIO& io = ImGui::GetIO(); (void)io;
		// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		ImGui::StyleColorsDark();
		ImGui_ImplOpenGL3_Init( "#version 330" );
	}

	inline void Terminate()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui::DestroyContext();
	}

	inline void NewFrame( aeRender* render, aeInput* input )
	{
		const InputState* inputState = input->GetState();
		ImGuiIO& io = ImGui::GetIO();

		io.DeltaTime = kTimeStep;
		io.MouseDown[ 0 ] = inputState->mouseLeft;
		io.MouseDown[ 1 ] = inputState->mouseRight;
		io.MouseDown[ 2 ] = inputState->mouseMiddle;
		io.MouseWheel += inputState->scroll;
		io.MousePos = ImVec2( inputState->mousePixelPos.x, render->GetHeight() - inputState->mousePixelPos.y );
		
// 		io.KeysDown[ key ] = ( event->type == SDL_KEYDOWN );
// 		io.KeyShift = ( ( SDL_GetModState() & KMOD_SHIFT ) != 0 );
// 		io.KeyCtrl = ( ( SDL_GetModState() & KMOD_CTRL ) != 0 );
// 		io.KeyAlt = ( ( SDL_GetModState() & KMOD_ALT ) != 0 );
// #ifdef _AE_WINDOWS_
// 		io.KeySuper = false;
// #else
// 		io.KeySuper = ( ( SDL_GetModState() & KMOD_GUI ) != 0 );
// #endif
		
		ImGui::GetIO().DisplaySize = ImVec2( render->GetWidth(), render->GetHeight() );
		
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();
	}

	inline void Render()
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
	}
}

#endif
