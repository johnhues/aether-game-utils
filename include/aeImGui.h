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
#include "ImGui.h"
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

		ImGuiIO& io = ImGui::GetIO();
		io.KeyMap[ ImGuiKey_Tab ] = aeKey::Tab;
		io.KeyMap[ ImGuiKey_LeftArrow ] = aeKey::Left;
		io.KeyMap[ ImGuiKey_RightArrow ] = aeKey::Right;
		io.KeyMap[ ImGuiKey_UpArrow ] = aeKey::Up;
		io.KeyMap[ ImGuiKey_DownArrow ] = aeKey::Down;
		io.KeyMap[ ImGuiKey_PageUp ] = aeKey::PageUp;
		io.KeyMap[ ImGuiKey_PageDown ] = aeKey::PageDown;
		io.KeyMap[ ImGuiKey_Home ] = aeKey::Home;
		io.KeyMap[ ImGuiKey_End ] = aeKey::End;
		io.KeyMap[ ImGuiKey_Insert ] = aeKey::Insert;
		io.KeyMap[ ImGuiKey_Delete ] = aeKey::Delete;
		io.KeyMap[ ImGuiKey_Backspace ] = aeKey::Backspace;
		io.KeyMap[ ImGuiKey_Space ] = aeKey::Space;
		io.KeyMap[ ImGuiKey_Enter ] = aeKey::Enter;
		io.KeyMap[ ImGuiKey_Escape ] = aeKey::Escape;
		io.KeyMap[ ImGuiKey_KeyPadEnter ] = aeKey::NumPadEnter;
		io.KeyMap[ ImGuiKey_A ] = aeKey::A;
		io.KeyMap[ ImGuiKey_C ] = aeKey::C;
		io.KeyMap[ ImGuiKey_V ] = aeKey::V;
		io.KeyMap[ ImGuiKey_X ] = aeKey::X;
		io.KeyMap[ ImGuiKey_Y ] = aeKey::Y;
		io.KeyMap[ ImGuiKey_Z ] = aeKey::Z;
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

		io.AddInputCharactersUTF8( input->GetTextInput() );
		input->SetTextMode( io.WantTextInput );

		io.DeltaTime = kTimeStep;
		
		// @TODO: Check ImGuiIO::WantCaptureMouse https://github.com/ocornut/imgui/blob/master/docs/FAQ.md#q-how-can-i-tell-whether-to-dispatch-mousekeyboard-to-dear-imgui-or-to-my-application
		io.MouseDown[ 0 ] = inputState->mouseLeft;
		io.MouseDown[ 1 ] = inputState->mouseRight;
		io.MouseDown[ 2 ] = inputState->mouseMiddle;
		io.MouseWheel += inputState->scroll;
		io.MousePos = ImVec2( inputState->mousePixelPos.x, render->GetHeight() - inputState->mousePixelPos.y );
		
		const auto& keys = inputState->keys;
		AE_STATIC_ASSERT( kKeyCount <= countof( io.KeysDown ) );
		for ( uint32_t i = 0; i < kKeyCount; i++ )
		{
			io.KeysDown[ i ] = keys[ i ];
		}
		io.KeyShift = inputState->shift;
		io.KeyCtrl = inputState->ctrl;
		//io.KeyAlt = ( ( SDL_GetModState() & KMOD_ALT ) != 0 );
//#ifdef _WIN32
//		io.KeySuper = false;
//#else
//		io.KeySuper = ( ( SDL_GetModState() & KMOD_GUI ) != 0 );
//#endif

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
