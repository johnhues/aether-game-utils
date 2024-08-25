//------------------------------------------------------------------------------
// aeImGui.h
// Utilities for allocating objects. Provides functionality to track current and
// past allocations.
//------------------------------------------------------------------------------
// Copyright (c) 2024 John Hughes
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
#ifndef IMGUI_DEFINE_MATH_OPERATORS
	#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui.h"
#include "imgui_internal.h" // For advanced imgui features like docking
#if _AE_WINDOWS_
	#pragma warning( push )
	#pragma warning( disable : 4244 ) // conversion from 'float' to 'int32_t'
#elif _AE_APPLE_
	#define GL_SILENCE_DEPRECATION
#elif _AE_LINUX_
	#define GL_GLEXT_PROTOTYPES 1
#endif
#include "imgui_impl_opengl3.h"

//------------------------------------------------------------------------------
// Imgui helpers
//------------------------------------------------------------------------------
class aeImGui
{
public:
	void Initialize();

	// @NOTE: Allows imgui functions to be called in a windowless application
	void InitializeHeadless();

	void Terminate();

	//! Call this from the same dll as the one that called Initialize() if imGui
	//! globals are modified. This automatically happens once on Initialize().
	void StoreGlobals();

	//! This must be called in all dlls that use imGui, other than the one that
	//! called Initialize(). This must be called after Initialize() and before
	//! any imGui functions are called.
	void WriteGlobals();

	void NewFrame( ae::GraphicsDevice* render, ae::Input* input, float dt );

	void Render();

	void Discard();

	template < uint32_t N >
	static bool InputText( const char* label, ae::Str< N >* str, ImGuiInputTextFlags flags = 0 );

	static void BeginGroupPanel( const char* name, const ImVec2& size = ImVec2( -1.0f, 0.0f ) );
	static void EndGroupPanel();

private:
	void m_Initialize();

	static ImVector<ImRect>& m_GetGroupPanelLabelStack();

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
	bool m_pendingRender = false;
	// Globals
	ImGuiContext* m_globalContext = nullptr;
	ImGuiMemAllocFunc m_globalAllocFn = nullptr;
	ImGuiMemFreeFunc m_globalFreenFn = nullptr;
	void* m_globalUserData = nullptr;
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
			filter.Draw( "##filter" );
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

template < uint32_t N >
bool aeImGui::InputText( const char* label, ae::Str< N >* str, ImGuiInputTextFlags flags )
{
	IM_ASSERT( ( flags & ImGuiInputTextFlags_CallbackEdit ) == 0 );
	flags |= ImGuiInputTextFlags_CallbackEdit;
	char* buffer = const_cast<char*>( str->c_str() );
	size_t maxSize = ae::Str< N >::MaxLength() + 1;
	return ImGui::InputText( label, buffer, maxSize, flags, aeImGui::m_StringCallback< N >, (void*)str );
}

#if _AE_WINDOWS_
	#pragma warning( pop )
#endif

#endif
