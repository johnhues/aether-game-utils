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
#include "aePlatform.h"
#include "imgui.h"
#include "imgui_internal.h" // For advanced imgui features like docking
#if _AE_WINDOWS_
	#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#elif _AE_APPLE_
	#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM <OpenGL/gl3.h>
#elif _AE_LINUX_
	#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM <GLES3/gl3.h>
#endif
#include "imgui_impl_opengl3.h"

#include "aeInput.h"
#include "aeRender.h"

// this is out of aeRender for macOS/winOS
extern bool gGL41;

//------------------------------------------------------------------------------
// Imgui helpers
//------------------------------------------------------------------------------
class aeImGui
{
public:
	void Initialize()
	{
		m_Initialize();
	}

	// @NOTE: Allows imgui functions to be called in a windowless application
	void InitializeHeadless()
	{
		m_headless = true;
		m_Initialize();
	}

	void Terminate()
	{
		AE_ASSERT( m_init );
		if ( !m_headless )
		{
			ImGui_ImplOpenGL3_Shutdown();
		}
		ImGui::DestroyContext();
	}

	void NewFrame( aeRender* render, aeInput* input, float dt )
	{
		AE_ASSERT( m_init );

		const InputState* inputState = input->GetState();
		ImGuiIO& io = ImGui::GetIO();

		io.AddInputCharactersUTF8( input->GetTextInput() );
		input->SetTextMode( io.WantTextInput );

		io.DeltaTime = dt;
		
		// @TODO: Check ImGuiIO::WantCaptureMouse https://github.com/ocornut/imgui/blob/master/docs/FAQ.md#q-how-can-i-tell-whether-to-dispatch-mousekeyboard-to-dear-imgui-or-to-my-application
		io.MouseDown[ 0 ] = inputState->mouseLeft;
		io.MouseDown[ 1 ] = inputState->mouseRight;
		io.MouseDown[ 2 ] = inputState->mouseMiddle;
		io.MouseWheel += inputState->scroll;
		io.MousePos = ImVec2( inputState->mousePixelPos.x, render->GetHeight() - inputState->mousePixelPos.y );
		
		AE_STATIC_ASSERT( kKeyCount <= countof( io.KeysDown ) );
		for ( uint32_t i = 0; i < kKeyCount; i++ )
		{
			io.KeysDown[ i ] = inputState->Get( (aeKey)i );
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
		
		if ( !m_headless )
		{
			ImGui_ImplOpenGL3_NewFrame();
		}
		ImGui::NewFrame();
		
		// have to alias this, since it's not passed into Render() call
		m_render = render;
	}

	void Render()
	{
		AE_ASSERT( m_init );
		ImGui::Render();
		if ( !m_headless )
		{
			// TODO: this should only be enabled if fbo in GL is sRGB
			//m_render->EnableSRGBWrites( true );
			ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
			//m_render->EnableSRGBWrites( false );
			m_render = nullptr;
		}
	}
private:
	void m_Initialize()
	{
		AE_ASSERT( !m_init );

		ImGui::CreateContext();
		// ImGuiIO& io = ImGui::GetIO(); (void)io;
		// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		ImGui::StyleColorsDark();

		if ( m_headless )
		{
			ImGuiIO& io = ImGui::GetIO();
			unsigned char* pixels;
			int width, height;
			io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
		}
		else
		{
			ImGui_ImplOpenGL3_Init( gGL41 ? "#version 410 core" : "#version 330 core" );
		}

		ImGuiIO& io = ImGui::GetIO();
		io.KeyMap[ ImGuiKey_Tab ] = (uint32_t)aeKey::Tab;
		io.KeyMap[ ImGuiKey_LeftArrow ] = (uint32_t)aeKey::Left;
		io.KeyMap[ ImGuiKey_RightArrow ] = (uint32_t)aeKey::Right;
		io.KeyMap[ ImGuiKey_UpArrow ] = (uint32_t)aeKey::Up;
		io.KeyMap[ ImGuiKey_DownArrow ] = (uint32_t)aeKey::Down;
		io.KeyMap[ ImGuiKey_PageUp ] = (uint32_t)aeKey::PageUp;
		io.KeyMap[ ImGuiKey_PageDown ] = (uint32_t)aeKey::PageDown;
		io.KeyMap[ ImGuiKey_Home ] = (uint32_t)aeKey::Home;
		io.KeyMap[ ImGuiKey_End ] = (uint32_t)aeKey::End;
		io.KeyMap[ ImGuiKey_Insert ] = (uint32_t)aeKey::Insert;
		io.KeyMap[ ImGuiKey_Delete ] = (uint32_t)aeKey::Delete;
		io.KeyMap[ ImGuiKey_Backspace ] = (uint32_t)aeKey::Backspace;
		io.KeyMap[ ImGuiKey_Space ] = (uint32_t)aeKey::Space;
		io.KeyMap[ ImGuiKey_Enter ] = (uint32_t)aeKey::Enter;
		io.KeyMap[ ImGuiKey_Escape ] = (uint32_t)aeKey::Escape;
		io.KeyMap[ ImGuiKey_KeyPadEnter ] = (uint32_t)aeKey::NumPadEnter;
		io.KeyMap[ ImGuiKey_A ] = (uint32_t)aeKey::A;
		io.KeyMap[ ImGuiKey_C ] = (uint32_t)aeKey::C;
		io.KeyMap[ ImGuiKey_V ] = (uint32_t)aeKey::V;
		io.KeyMap[ ImGuiKey_X ] = (uint32_t)aeKey::X;
		io.KeyMap[ ImGuiKey_Y ] = (uint32_t)aeKey::Y;
		io.KeyMap[ ImGuiKey_Z ] = (uint32_t)aeKey::Z;

		m_init = true;
	}

	bool m_init = false;
	bool m_headless = false;
	aeRender* m_render = nullptr;
};

#endif
