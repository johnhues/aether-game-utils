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
	#include "GL/glew.h"
  #define IMGUI_IMPL_OPENGL_LOADER_GLEW
#elif _AE_APPLE_
  #define GL_SILENCE_DEPRECATION
#elif _AE_LINUX_
  #define GL_GLEXT_PROTOTYPES 1
#endif
#include "imgui_impl_opengl3.h"

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

  void NewFrame( ae::GraphicsDevice* render, ae::Input* input, float dt )
  {
	AE_ASSERT( m_init );
	
	if ( m_pendingRender )
	{
	  ImGui::Render();
	  m_pendingRender = false;
	}

	ImGuiIO& io = ImGui::GetIO();

	io.AddInputCharactersUTF8( input->GetTextInput() );
	input->SetTextMode( io.WantTextInput );

	float displayScale = 1.0f;
	float windowHeight = render->GetHeight();
	if ( ae::Window* window = render->GetWindow() )
	{
	  displayScale = window->GetScaleFactor();
	  windowHeight = window->GetHeight();
	}

	io.DisplaySize = ImVec2( render->GetWidth() / displayScale, render->GetHeight() / displayScale );
	io.DisplayFramebufferScale = ImVec2( displayScale, displayScale );
	io.DeltaTime = dt;
	
	// @TODO: Check ImGuiIO::WantCaptureMouse https://github.com/ocornut/imgui/blob/master/docs/FAQ.md#q-how-can-i-tell-whether-to-dispatch-mousekeyboard-to-dear-imgui-or-to-my-application
	io.MouseDown[ 0 ] = input->mouse.leftButton;
	io.MouseDown[ 1 ] = input->mouse.rightButton;
	io.MouseDown[ 2 ] = input->mouse.middleButton;
	io.MouseWheel += input->mouse.scroll.y;
	ae::Vec2 mousePos( input->mouse.position.x, windowHeight - input->mouse.position.y );
	io.MousePos = ImVec2( mousePos.x, mousePos.y );
	
		// AE_STATIC_ASSERT( kKeyCount <= countof( io.KeysDown ) );
	for ( uint32_t i = 0; i < 256; i++ )
	{
	  io.KeysDown[ i ] = input->Get( (ae::Key)i );
	}
	io.KeyShift = input->Get( ae::Key::LeftShift ) || input->Get( ae::Key::RightShift );
	io.KeyCtrl = input->Get( ae::Key::LeftControl ) || input->Get( ae::Key::RightControl );
	io.KeyAlt = input->Get( ae::Key::LeftAlt ) || input->Get( ae::Key::RightAlt );
	io.KeySuper = input->Get( ae::Key::LeftSuper ) || input->Get( ae::Key::RightSuper );
	
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

  static void BeginGroupPanel( const char* name, const ImVec2& size = ImVec2( -1.0f, 0.0f ) )
  {
	ImGui::PushID( name );
	ImGui::BeginGroup();

	auto cursorPos = ImGui::GetCursorScreenPos();
	auto itemSpacing = ImGui::GetStyle().ItemSpacing;
	ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 0.0f, 0.0f ) );
	ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0.0f, 0.0f ) );

	auto frameHeight = ImGui::GetFrameHeight();
	ImGui::BeginGroup();

	ImVec2 effectiveSize = size;
	if ( size.x < 0.0f )
	  effectiveSize.x = ImGui::GetContentRegionAvail().x;
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
	
	ImGui::PopID();
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
#if _AE_WINDOWS_
		GLenum err = glewInit();
		AE_ASSERT( err == GLEW_OK, "Error: #", glewGetErrorString( err ) );
#endif
	  ae::Str32 version = "#version ";
	  version += ae::Str16::Format( "##0 core", ae::GLMajorVersion, ae::GLMinorVersion );
	  ImGui_ImplOpenGL3_Init( version.c_str() );
	}

	ImGuiIO& io = ImGui::GetIO();
	io.KeyMap[ ImGuiKey_Tab ] = (uint32_t)ae::Key::Tab;
	io.KeyMap[ ImGuiKey_LeftArrow ] = (uint32_t)ae::Key::Left;
	io.KeyMap[ ImGuiKey_RightArrow ] = (uint32_t)ae::Key::Right;
	io.KeyMap[ ImGuiKey_UpArrow ] = (uint32_t)ae::Key::Up;
	io.KeyMap[ ImGuiKey_DownArrow ] = (uint32_t)ae::Key::Down;
	io.KeyMap[ ImGuiKey_PageUp ] = (uint32_t)ae::Key::PageUp;
	io.KeyMap[ ImGuiKey_PageDown ] = (uint32_t)ae::Key::PageDown;
	io.KeyMap[ ImGuiKey_Home ] = (uint32_t)ae::Key::Home;
	io.KeyMap[ ImGuiKey_End ] = (uint32_t)ae::Key::End;
	io.KeyMap[ ImGuiKey_Insert ] = (uint32_t)ae::Key::Insert;
	io.KeyMap[ ImGuiKey_Delete ] = (uint32_t)ae::Key::Delete;
	io.KeyMap[ ImGuiKey_Backspace ] = (uint32_t)ae::Key::Backspace;
	io.KeyMap[ ImGuiKey_Space ] = (uint32_t)ae::Key::Space;
	io.KeyMap[ ImGuiKey_Enter ] = (uint32_t)ae::Key::Enter;
	io.KeyMap[ ImGuiKey_Escape ] = (uint32_t)ae::Key::Escape;
	io.KeyMap[ ImGuiKey_KeyPadEnter ] = (uint32_t)ae::Key::NumPadEnter;
	
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
  ae::GraphicsDevice* m_render = nullptr;
  bool m_pendingRender = false;
};

//------------------------------------------------------------------------------
// Var helpers
//------------------------------------------------------------------------------
static ae::Str32 aeImGui_Enum( const ae::Enum* enumType, const char* varName, const char* currentValue, uint32_t showSearchCount = 16 )
{
  ae::Str32 result = currentValue;
  if ( ImGui::BeginCombo( varName, currentValue ) )
  {
	uint32_t count = enumType->Length();
	static ImGuiTextFilter filter;
	bool search = ( count >= showSearchCount );
	if ( search )
	{
		filter.Draw( "" );
	}

	for ( uint32_t i = 0; i < count; i++ )
	{
	  auto indexName = enumType->GetNameByIndex( i );
	  bool show = !search || filter.PassFilter( indexName.c_str() );
	  if ( show && ImGui::Selectable( indexName.c_str(), indexName == currentValue ) )
	  {
		result = indexName.c_str();
	  }
	}
	ImGui::EndCombo();
  }
  return result;
}

template < typename T >
bool aeImGui_Enum( const char* varName, T* valueOut, uint32_t showSearchCount = 16 )
{
  const ae::Enum* enumType = ae::GetEnum< T >();
  auto currentValue = enumType->GetNameByValue( *valueOut );
  auto resultName = aeImGui_Enum( enumType, varName, currentValue.c_str(), showSearchCount );
  
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
