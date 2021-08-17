//------------------------------------------------------------------------------
// aeImGui.h
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
#include "aether.h"
#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h" // For advanced imgui features like docking
#if _AE_WINDOWS_
	#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#elif _AE_APPLE_
  #define GL_SILENCE_DEPRECATION
	#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM <OpenGL/gl3.h>
#elif _AE_LINUX_
	#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM <GLES3/gl3.h>
#endif
#include "imgui_impl_opengl3.h"

#include "aeInput.h"
#include "aeMeta.h"
#include "aeRender.h"

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
    
    if ( m_pendingRender )
    {
      ImGui::Render();
      m_pendingRender = false;
    }

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
    
    m_pendingRender = true;
	}

	void Render()
	{
		AE_ASSERT( m_init );
		ImGui::Render();
		if ( !m_headless )
		{
			ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
		}
    m_pendingRender = false;
	}

	void Discard()
	{
		AE_ASSERT( m_init );
		ImGui::Render();
		m_pendingRender = false;
	}

	template < uint32_t N >
	static bool InputText( const char* label, ae::Str< N >* str, ImGuiInputTextFlags flags = 0 )
	{
		IM_ASSERT( ( flags & ImGuiInputTextFlags_CallbackEdit ) == 0 );
		flags |= ImGuiInputTextFlags_CallbackEdit;
		char* buffer = const_cast<char*>( str->c_str() );
		size_t maxSize = ae::Str< N >::MaxLength() + 1;
		return ImGui::InputText( label, buffer, maxSize, flags, aeImGui::m_StringCallback< N >, (void*)str );
	}

	static void BeginGroupPanel( const char* name, const ImVec2& size )
	{
		ImGui::BeginGroup();

		auto cursorPos = ImGui::GetCursorScreenPos();
		auto itemSpacing = ImGui::GetStyle().ItemSpacing;
		ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 0.0f, 0.0f ) );
		ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0.0f, 0.0f ) );

		auto frameHeight = ImGui::GetFrameHeight();
		ImGui::BeginGroup();

		ImVec2 effectiveSize = size;
		if ( size.x < 0.0f )
			effectiveSize.x = ImGui::GetContentRegionAvailWidth();
		else
			effectiveSize.x = size.x;
		ImGui::Dummy( ImVec2( effectiveSize.x, 0.0f ) );

		ImGui::Dummy( ImVec2( frameHeight * 0.5f, 0.0f ) );
		ImGui::SameLine( 0.0f, 0.0f );
		ImGui::BeginGroup();
		ImGui::Dummy( ImVec2( frameHeight * 0.5f, 0.0f ) );
		ImGui::SameLine( 0.0f, 0.0f );
		ImGui::TextUnformatted( name );
		auto labelMin = ImGui::GetItemRectMin();
		auto labelMax = ImGui::GetItemRectMax();
		ImGui::SameLine( 0.0f, 0.0f );
		ImGui::Dummy( ImVec2( 0.0, frameHeight + itemSpacing.y ) );
		ImGui::BeginGroup();

		//ImGui::GetWindowDrawList()->AddRect(labelMin, labelMax, IM_COL32(255, 0, 255, 255));

		ImGui::PopStyleVar( 2 );

#if IMGUI_VERSION_NUM >= 17301
		ImGui::GetCurrentWindow()->ContentRegionRect.Max.x -= frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->WorkRect.Max.x -= frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->InnerRect.Max.x -= frameHeight * 0.5f;
#else
		ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x -= frameHeight * 0.5f;
#endif
		ImGui::GetCurrentWindow()->Size.x -= frameHeight;

		auto itemWidth = ImGui::CalcItemWidth();
		ImGui::PushItemWidth( ImMax( 0.0f, itemWidth - frameHeight ) );

		m_GetGroupPanelLabelStack().push_back( ImRect( labelMin, labelMax ) );
	}

	static void EndGroupPanel()
	{
		ImGui::PopItemWidth();

		auto itemSpacing = ImGui::GetStyle().ItemSpacing;

		ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 0.0f, 0.0f ) );
		ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0.0f, 0.0f ) );

		auto frameHeight = ImGui::GetFrameHeight();

		ImGui::EndGroup();

		//ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(0, 255, 0, 64), 4.0f);

		ImGui::EndGroup();

		ImGui::SameLine( 0.0f, 0.0f );
		ImGui::Dummy( ImVec2( frameHeight * 0.5f, 0.0f ) );
		ImGui::Dummy( ImVec2( 0.0, frameHeight - frameHeight * 0.5f - itemSpacing.y ) );

		ImGui::EndGroup();

		auto itemMin = ImGui::GetItemRectMin();
		auto itemMax = ImGui::GetItemRectMax();
		//ImGui::GetWindowDrawList()->AddRectFilled(itemMin, itemMax, IM_COL32(255, 0, 0, 64), 4.0f);

		auto labelRect = m_GetGroupPanelLabelStack().back();
		m_GetGroupPanelLabelStack().pop_back();

		ImVec2 halfFrame = ImVec2( frameHeight * 0.25f, frameHeight ) * 0.5f;
		ImRect frameRect = ImRect( itemMin + halfFrame, itemMax - ImVec2( halfFrame.x, 0.0f ) );
		labelRect.Min.x -= itemSpacing.x;
		labelRect.Max.x += itemSpacing.x;
		for ( int i = 0; i < 4; ++i )
		{
			switch ( i )
			{
				// left half-plane
				case 0: ImGui::PushClipRect( ImVec2( -FLT_MAX, -FLT_MAX ), ImVec2( labelRect.Min.x, FLT_MAX ), true ); break;
					// right half-plane
				case 1: ImGui::PushClipRect( ImVec2( labelRect.Max.x, -FLT_MAX ), ImVec2( FLT_MAX, FLT_MAX ), true ); break;
					// top
				case 2: ImGui::PushClipRect( ImVec2( labelRect.Min.x, -FLT_MAX ), ImVec2( labelRect.Max.x, labelRect.Min.y ), true ); break;
					// bottom
				case 3: ImGui::PushClipRect( ImVec2( labelRect.Min.x, labelRect.Max.y ), ImVec2( labelRect.Max.x, FLT_MAX ), true ); break;
			}

			ImGui::GetWindowDrawList()->AddRect(
					frameRect.Min, frameRect.Max,
					ImColor( ImGui::GetStyleColorVec4( ImGuiCol_Border ) ),
					halfFrame.x );

			ImGui::PopClipRect();
		}

		ImGui::PopStyleVar( 2 );

#if IMGUI_VERSION_NUM >= 17301
		ImGui::GetCurrentWindow()->ContentRegionRect.Max.x += frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->WorkRect.Max.x += frameHeight * 0.5f;
		ImGui::GetCurrentWindow()->InnerRect.Max.x += frameHeight * 0.5f;
#else
		ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x += frameHeight * 0.5f;
#endif
		ImGui::GetCurrentWindow()->Size.x += frameHeight;

		ImGui::Dummy( ImVec2( 0.0f, 0.0f ) );

		ImGui::EndGroup();
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
			ae::Str32 version = "#version ";
			version += ae::Str16::Format( "##0 core", ae::GLMajorVersion, ae::GLMinorVersion );
			ImGui_ImplOpenGL3_Init( version.c_str() );
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
    
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigWindowsResizeFromEdges = true;
    io.IniFilename = nullptr; // @TODO: Save layout. Currently disabled because window show states are not saved in the imgui ini file

		m_init = true;
	}

	static ImVector<ImRect>& m_GetGroupPanelLabelStack()
	{
		static ImVector< ImRect > s_labelStack;
		return s_labelStack;
	}

	template < uint32_t N >
	static int m_StringCallback( ImGuiInputTextCallbackData* data )
	{
		if ( data->EventFlag == ImGuiInputTextFlags_CallbackEdit )
		{
			ae::Str< N >* str = ( ae::Str< N >* )data->UserData;
			*str = ae::Str< N >( data->BufTextLen, 'x' ); // Set Length() of string
		}
		return 0;
	}

	bool m_init = false;
	bool m_headless = false;
	aeRender* m_render = nullptr;
  bool m_pendingRender = false;
};

//------------------------------------------------------------------------------
// Var helpers
//------------------------------------------------------------------------------
static aeStr32 aeImGui_Enum( const ae::Enum* enumType, const char* varName, const char* currentValue )
{
  aeStr32 result = currentValue;
  if ( ImGui::BeginCombo( varName, currentValue ) )
  {
    for ( uint32_t i = 0; i < enumType->Length(); i++ )
    {
      auto indexName = enumType->GetNameByIndex( i );
      if ( ImGui::Selectable( indexName.c_str(), indexName == currentValue ) )
      {
        result = indexName.c_str();
      }
    }
    ImGui::EndCombo();
  }
  return result;
}

template < typename T >
bool aeImGui_Enum( const char* varName, T* valueOut )
{
  const ae::Enum* enumType = ae::GetEnum< T >();
  auto currentValue = enumType->GetNameByValue( *valueOut );
  auto resultName = aeImGui_Enum( enumType, varName, currentValue.c_str() );
  
  T prev = *valueOut;
  if ( enumType->GetValueFromString( resultName.c_str(), valueOut ) )
  {
    return prev != *valueOut;
  }
  else
  {
    return false;
  }
}

#endif
