#include "ae/aeImGui.h"
#include "imgui_impl_opengl3.h"

static ImGuiKey ae_ToImGuiKey( ae::Key key )
{
	switch( key )
	{
		case ae::Key::A: return ImGuiKey_A;
		case ae::Key::B: return ImGuiKey_B;
		case ae::Key::C: return ImGuiKey_C;
		case ae::Key::D: return ImGuiKey_D;
		case ae::Key::E: return ImGuiKey_E;
		case ae::Key::F: return ImGuiKey_F;
		case ae::Key::G: return ImGuiKey_G;
		case ae::Key::H: return ImGuiKey_H;
		case ae::Key::I: return ImGuiKey_I;
		case ae::Key::J: return ImGuiKey_J;
		case ae::Key::K: return ImGuiKey_K;
		case ae::Key::L: return ImGuiKey_L;
		case ae::Key::M: return ImGuiKey_M;
		case ae::Key::N: return ImGuiKey_N;
		case ae::Key::O: return ImGuiKey_O;
		case ae::Key::P: return ImGuiKey_P;
		case ae::Key::Q: return ImGuiKey_Q;
		case ae::Key::R: return ImGuiKey_R;
		case ae::Key::S: return ImGuiKey_S;
		case ae::Key::T: return ImGuiKey_T;
		case ae::Key::U: return ImGuiKey_U;
		case ae::Key::V: return ImGuiKey_V;
		case ae::Key::W: return ImGuiKey_W;
		case ae::Key::X: return ImGuiKey_X;
		case ae::Key::Y: return ImGuiKey_Y;
		case ae::Key::Z: return ImGuiKey_Z;
		case ae::Key::Num0: return ImGuiKey_0;
		case ae::Key::Num1: return ImGuiKey_1;
		case ae::Key::Num2: return ImGuiKey_2;
		case ae::Key::Num3: return ImGuiKey_3;
		case ae::Key::Num4: return ImGuiKey_4;
		case ae::Key::Num5: return ImGuiKey_5;
		case ae::Key::Num6: return ImGuiKey_6;
		case ae::Key::Num7: return ImGuiKey_7;
		case ae::Key::Num8: return ImGuiKey_8;
		case ae::Key::Num9: return ImGuiKey_9;
		case ae::Key::Enter: return ImGuiKey_Enter;
		case ae::Key::Escape: return ImGuiKey_Escape;
		case ae::Key::Backspace: return ImGuiKey_Backspace;
		case ae::Key::Tab: return ImGuiKey_Tab;
		case ae::Key::Space: return ImGuiKey_Space;
		case ae::Key::Minus: return ImGuiKey_Minus;
		case ae::Key::Equals: return ImGuiKey_Equal;
		case ae::Key::LeftBracket: return ImGuiKey_LeftBracket;
		case ae::Key::RightBracket: return ImGuiKey_RightBracket;
		case ae::Key::Backslash: return ImGuiKey_Backslash;
		case ae::Key::Semicolon: return ImGuiKey_Semicolon;
		case ae::Key::Apostrophe: return ImGuiKey_Apostrophe;
		case ae::Key::Tilde: return ImGuiKey_GraveAccent;
		case ae::Key::Comma: return ImGuiKey_Comma;
		case ae::Key::Period: return ImGuiKey_Period;
		case ae::Key::Slash: return ImGuiKey_Slash;
		case ae::Key::CapsLock: return ImGuiKey_CapsLock;
		case ae::Key::F1: return ImGuiKey_F1;
		case ae::Key::F2: return ImGuiKey_F2;
		case ae::Key::F3: return ImGuiKey_F3;
		case ae::Key::F4: return ImGuiKey_F4;
		case ae::Key::F5: return ImGuiKey_F5;
		case ae::Key::F6: return ImGuiKey_F6;
		case ae::Key::F7: return ImGuiKey_F7;
		case ae::Key::F8: return ImGuiKey_F8;
		case ae::Key::F9: return ImGuiKey_F9;
		case ae::Key::F10: return ImGuiKey_F10;
		case ae::Key::F11: return ImGuiKey_F11;
		case ae::Key::F12: return ImGuiKey_F12;
		case ae::Key::PrintScreen: return ImGuiKey_PrintScreen;
		case ae::Key::ScrollLock: return ImGuiKey_ScrollLock;
		case ae::Key::Pause: return ImGuiKey_Pause;
		case ae::Key::Insert: return ImGuiKey_Insert;
		case ae::Key::Home: return ImGuiKey_Home;
		case ae::Key::PageUp: return ImGuiKey_PageUp;
		case ae::Key::Delete: return ImGuiKey_Delete;
		case ae::Key::End: return ImGuiKey_End;
		case ae::Key::PageDown: return ImGuiKey_PageDown;
		case ae::Key::Right: return ImGuiKey_RightArrow;
		case ae::Key::Left: return ImGuiKey_LeftArrow;
		case ae::Key::Down: return ImGuiKey_DownArrow;
		case ae::Key::Up: return ImGuiKey_UpArrow;
		case ae::Key::NumLock: return ImGuiKey_NumLock;
		case ae::Key::NumPadDivide: return ImGuiKey_KeypadDivide;
		case ae::Key::NumPadMultiply: return ImGuiKey_KeypadMultiply;
		case ae::Key::NumPadMinus: return ImGuiKey_KeypadSubtract;
		case ae::Key::NumPadPlus: return ImGuiKey_KeypadAdd;
		case ae::Key::NumPadEnter: return ImGuiKey_KeypadEnter;
		case ae::Key::NumPad0: return ImGuiKey_Keypad0;
		case ae::Key::NumPad1: return ImGuiKey_Keypad1;
		case ae::Key::NumPad2: return ImGuiKey_Keypad2;
		case ae::Key::NumPad3: return ImGuiKey_Keypad3;
		case ae::Key::NumPad4: return ImGuiKey_Keypad4;
		case ae::Key::NumPad5: return ImGuiKey_Keypad5;
		case ae::Key::NumPad6: return ImGuiKey_Keypad6;
		case ae::Key::NumPad7: return ImGuiKey_Keypad7;
		case ae::Key::NumPad8: return ImGuiKey_Keypad8;
		case ae::Key::NumPad9: return ImGuiKey_Keypad9;
		case ae::Key::NumPadPeriod: return ImGuiKey_KeypadDecimal;
		case ae::Key::NumPadEquals: return ImGuiKey_KeypadEqual;
		case ae::Key::LeftControl: return ImGuiKey_LeftCtrl;
		case ae::Key::LeftShift: return ImGuiKey_LeftShift;
		case ae::Key::LeftAlt: return ImGuiKey_LeftAlt;
		case ae::Key::LeftSuper: return ImGuiKey_LeftSuper;
		case ae::Key::RightControl: return ImGuiKey_RightCtrl;
		case ae::Key::RightShift: return ImGuiKey_RightShift;
		case ae::Key::RightAlt: return ImGuiKey_RightAlt;
		case ae::Key::RightSuper: return ImGuiKey_RightSuper;
		default: return ImGuiKey_None;
	}
}

void aeImGui::Initialize()
{
	m_Initialize();
}

// @NOTE: Allows imgui functions to be called in a windowless application
void aeImGui::InitializeHeadless()
{
	m_headless = true;
	m_Initialize();
}

void aeImGui::Terminate()
{
	AE_ASSERT( m_init );
	if( !m_headless )
	{
		ImGui_ImplOpenGL3_Shutdown();
	}
	ImGui::DestroyContext();
}

//! Call this from the same dll as the one that called Initialize() if imGui
//! globals are modified. This automatically happens once on Initialize().
void aeImGui::StoreGlobals()
{
	m_globalContext = ImGui::GetCurrentContext();
	ImGui::GetAllocatorFunctions( &m_globalAllocFn, &m_globalFreenFn, &m_globalUserData );
}

//! This must be called in all dlls that use imGui, other than the one that
//! called Initialize(). This must be called after Initialize() and before
//! any imGui functions are called.
void aeImGui::WriteGlobals()
{
	ImGui::SetCurrentContext( m_globalContext );
	ImGui::SetAllocatorFunctions( m_globalAllocFn, m_globalFreenFn, m_globalUserData );
}

void aeImGui::NewFrame( ae::GraphicsDevice* render, ae::Input* input, float dt )
{
	AE_ASSERT( m_init );

	dt = ae::Max( dt, 0.0001f );
	
	if( m_pendingRender )
	{
		ImGui::Render();
		m_pendingRender = false;
	}

	ImGuiIO& io = ImGui::GetIO();

	io.AddInputCharactersUTF8( input->GetTextInput() );
	input->SetTextMode( io.WantTextInput );

	float displayScale = 1.0f;
	float windowHeight = render->GetHeight();
	if( ae::Window* window = render->GetWindow() )
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
	io.MouseWheel += input->mouse.scrollMomentum.y * ( input->RequestsNaturalScrolling() ? -0.025f : 0.025f );
	ae::Vec2 mousePos( input->mouse.position.x, windowHeight - input->mouse.position.y );
	io.MousePos = ImVec2( mousePos.x, mousePos.y );
	
	// imgui derives KeyShift/KeyCtrl/KeyAlt/KeySuper from the per-key events below
	for( uint32_t i = 0; i < 256; i++ )
	{
		const ae::Key key = (ae::Key)i;
		const ImGuiKey imKey = ae_ToImGuiKey( key );
		if( imKey != ImGuiKey_None )
		{
			io.AddKeyEvent( imKey, input->Get( key ) );
		}
	}
	
	if( !m_headless )
	{
		ImGui_ImplOpenGL3_NewFrame();
	}
	ImGui::NewFrame();
	
	m_pendingRender = true;
}

void aeImGui::Render()
{
	AE_ASSERT( m_init );
	ImGui::Render();
	if( !m_headless )
	{
		ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
	}
	m_pendingRender = false;
}

void aeImGui::Discard()
{
	AE_ASSERT( m_init );
	ImGui::Render();
	m_pendingRender = false;
}

void aeImGui::BeginGroupPanel( const char* name, const ImVec2& size )
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
	if( size.x < 0.0f )
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

void aeImGui::EndGroupPanel()
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
	for( int i = 0; i < 4; ++i )
	{
		switch( i )
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

void aeImGui::m_Initialize()
{
	AE_ASSERT( !m_init );

	ImGui::CreateContext();
	// ImGuiIO& io = ImGui::GetIO(); (void)io;
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	ImGui::StyleColorsDark();

	// Convert default colors to linear color space
	ImGuiStyle* style = &ImGui::GetStyle();
	for( ImVec4& color : style->Colors )
	{
		ae::Vec3 c = ae::Color::SRGB( color.x, color.y, color.z ).GetLinearRGB();
		color.x = c.x;
		color.y = c.y;
		color.z = c.z;
	}

	if( m_headless )
	{
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
	}
	else
	{
		ae::Str32 glVersionStr = "#version ";
#if _AE_IOS_ || _AE_EMSCRIPTEN_
		glVersionStr += ae::Str16::Format( "##0 es", ae::GLMajorVersion(), ae::GLMinorVersion() );
#else
		glVersionStr += ae::Str16::Format( "##0 core", ae::GLMajorVersion(), ae::GLMinorVersion() );
#endif
		ImGui_ImplOpenGL3_Init( glVersionStr.c_str() );
	}

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigWindowsResizeFromEdges = true;
	io.IniFilename = nullptr; // @TODO: Save layout. Currently disabled because window show states are not saved in the imgui ini file

	StoreGlobals();

	m_init = true;
}

ImVector<ImRect>& aeImGui::m_GetGroupPanelLabelStack()
{
	static ImVector< ImRect > s_labelStack;
	return s_labelStack;
}
