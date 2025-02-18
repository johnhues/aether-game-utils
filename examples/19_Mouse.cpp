//------------------------------------------------------------------------------
// 19_Mouse.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2025 John Hughes
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
// Headers
//------------------------------------------------------------------------------
#include "aether.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const ae::Tag TAG_FOLDER = "folder";
const ae::Vec2 kFolderIconSize = ae::Vec2( 100.0f, 85.0f );
const ae::Vec2 kRightClickMenuSize = ae::Vec2( 150.0f, -225.0f );

//------------------------------------------------------------------------------
// Shaders
//------------------------------------------------------------------------------
const char* kVertShader = R"(
	AE_UNIFORM mat4 u_worldToProj;
	AE_IN_HIGHP vec4 a_position;
	void main()
	{
		gl_Position = u_worldToProj * a_position;
	})";

const char* kFragShader = R"(
	AE_UNIFORM vec4 u_color;
	void main()
	{
		AE_COLOR = u_color;
	})";

//------------------------------------------------------------------------------
// Cursor
//------------------------------------------------------------------------------
ae::Vec4 kCursorVerts[] =
{
	ae::Vec4( 0.0f, -1.0f, 0.0f, 1.0f ),
	ae::Vec4( 1.0f, -1.0f, 0.0f, 1.0f ),
	ae::Vec4( 0.0f, 0.0f, 0.0f, 1.0f ),
};
uint16_t kCursorIndices[] =
{
	0, 1, 2
};

//------------------------------------------------------------------------------
// Quad
//------------------------------------------------------------------------------
ae::Vec4 kQuadVerts[] =
{
	ae::Vec4( -0.5f, -0.5f, 0.0f, 1.0f ),
	ae::Vec4( 0.5f, -0.5f, 0.0f, 1.0f ),
	ae::Vec4( -0.5f, 0.5f, 0.0f, 1.0f ),
	ae::Vec4( 0.5f, 0.5f, 0.0f, 1.0f ),
};
uint16_t kQuadIndices[] =
{
	0, 1, 2,
	2, 1, 3,
};

//------------------------------------------------------------------------------
// Folder class
//------------------------------------------------------------------------------
class Folder
{
public:
	ae::Vec2 pos;
	uint32_t selectionIdx = 0;
	ae::Array< Folder* > subFolders = TAG_FOLDER;
};

//------------------------------------------------------------------------------
// Program class
//------------------------------------------------------------------------------
class Program
{
public:
	void Initialize();
	void Terminate();
	bool Tick();

	void DrawRect( ae::Rect rect, ae::Color color );
	void DrawWindow();
	void DrawCursor();
	void DrawFolders();

private:
	ae::Window m_window;
	ae::GraphicsDevice m_gfx;
	ae::Input m_input;
	ae::TimeStep m_timeStep;
	ae::Shader m_shader;
	ae::VertexBuffer m_triangle, m_quad;

	ae::Matrix4 m_worldToProj;
	enum class State
	{
		Idle,
		BoxSelect,
		Dragging,
	};
	State m_state = State::Idle;
	uint32_t m_selectionIdx = 1;
	// Box select
	ae::Vec2 m_selectStart;
	ae::Rect m_selectRect;
	// Drag
	ae::Vec2 m_dragStart;
	// Right click menu
	bool m_rightClick = false;
	ae::Vec2 m_rightClickPos;
	ae::Rect m_rightClickRect;
	// Folders
	Folder m_rootFolder;
	Folder* m_currentFolder = &m_rootFolder;
	ae::Array< Folder* > m_selected = TAG_FOLDER;
};

//------------------------------------------------------------------------------
// Program member functions
//------------------------------------------------------------------------------
void Program::Initialize()
{
	m_window.Initialize( 800, 600, false, true, true );
	m_window.SetTitle( "aeOS" );
	m_gfx.Initialize( &m_window );
	m_input.Initialize( &m_window );
	m_input.SetCursorHidden( true );
	m_timeStep.SetTimeStep( 1.0f / 60.0f );

	m_shader.Initialize( kVertShader, kFragShader, nullptr, 0 );
	m_shader.SetBlending( true );

	m_triangle.Initialize( sizeof( *kCursorVerts ), sizeof( *kCursorIndices ), countof( kCursorVerts ), countof( kCursorIndices ), ae::Vertex::Primitive::Triangle, ae::Vertex::Usage::Static, ae::Vertex::Usage::Static );
	m_triangle.AddAttribute( "a_position", 4, ae::Vertex::Type::Float, 0 );
	m_triangle.UploadVertices( 0, kCursorVerts, countof( kCursorVerts ) );
	m_triangle.UploadIndices( 0, kCursorIndices, countof( kCursorIndices ) );

	m_quad.Initialize( sizeof( *kQuadVerts ), sizeof( *kQuadIndices ), countof( kQuadVerts ), countof( kQuadIndices ), ae::Vertex::Primitive::Triangle, ae::Vertex::Usage::Static, ae::Vertex::Usage::Static );
	m_quad.AddAttribute( "a_position", 4, ae::Vertex::Type::Float, 0 );
	m_quad.UploadVertices( 0, kQuadVerts, countof( kQuadVerts ) );
	m_quad.UploadIndices( 0, kQuadIndices, countof( kQuadIndices ) );

	m_rootFolder.subFolders.Append( ae::New< Folder >( TAG_FOLDER ) )->pos = ae::Vec2( 100.0f, 200.0f );
	m_rootFolder.subFolders.Append( ae::New< Folder >( TAG_FOLDER ) )->pos = ae::Vec2( 300.0f, 300.0f );
}

void Program::Terminate()
{
	m_quad.Terminate();
	m_triangle.Terminate();
	m_shader.Terminate();
	m_input.Terminate();
	m_gfx.Terminate();
	m_window.Terminate();
}

bool Program::Tick()
{
	m_input.Pump();
	
	const ae::Vec2 currentCursorPos = ae::Vec2( m_input.mouse.position ) * m_window.GetScaleFactor();

	int32_t hoverIdx = m_currentFolder->subFolders.FindLastFn( [&]( const Folder* folder )
	{
		ae::Rect folderRect = ae::Rect::FromCenterAndSize( folder->pos, kFolderIconSize );
		return folderRect.Contains( currentCursorPos );
	} );
	Folder* hoverFolder = ( hoverIdx >= 0 ) ? m_currentFolder->subFolders[ hoverIdx ] : nullptr;
	bool hoverFolderIsSelected = ( m_selected.Find( hoverFolder ) >= 0 );

	if ( m_input.mouse.leftButton && !m_input.mousePrev.leftButton ) // Start left click
	{
		if ( m_rightClick && m_rightClickRect.Contains( currentCursorPos ) )
		{
			if ( m_selected.Length() )
			{
				for ( Folder* folder : m_selected )
				{
					if ( hoverFolder )
					{
						hoverFolder = nullptr;
						hoverFolderIsSelected = false;
					}
					m_currentFolder->subFolders.Remove( m_currentFolder->subFolders.Find( folder ) );
					ae::Delete( folder );
				}
				m_selected.Clear();
			}
			else
			{
				m_currentFolder->subFolders.Append( ae::New< Folder >( TAG_FOLDER ) )->pos = m_rightClickPos;
			}
		}
		else if ( hoverFolder )
		{
			m_state = State::Dragging;
			m_dragStart = currentCursorPos;
			if ( !hoverFolderIsSelected )
			{
				m_selected.Clear();
				m_selected.Append( hoverFolder );
				hoverFolder->selectionIdx = m_selectionIdx++;
			}
		}
		else
		{
			m_state = State::BoxSelect;
			m_selectStart = currentCursorPos;
		}
		m_rightClick = false;
	}
	else if ( !m_input.mouse.leftButton && m_input.mouse.rightButton && !m_input.mousePrev.rightButton ) // Right click
	{
		if ( !m_rightClick || !m_rightClickRect.Contains( currentCursorPos ) )
		{
			m_rightClick = true;
			m_rightClickPos = currentCursorPos;
			float offsetX = ( currentCursorPos.x > m_gfx.GetWidth() * 0.5f ) ? -kRightClickMenuSize.x : kRightClickMenuSize.x;
			float offsetY = ( currentCursorPos.y > m_gfx.GetHeight() * 0.5f ) ? kRightClickMenuSize.y : -kRightClickMenuSize.y;
			m_rightClickRect = ae::Rect::FromCenterAndSize( currentCursorPos + ae::Vec2( offsetX, offsetY ) * 0.5f, kRightClickMenuSize );

			if ( !hoverFolder || !hoverFolderIsSelected )
			{
				m_selected.Clear();
			}
			if ( hoverFolder && !hoverFolderIsSelected )
			{
				m_selected.Append( hoverFolder );
				hoverFolder->selectionIdx = m_selectionIdx++;
			}
		}
		m_state = State::Idle;
	}

	m_gfx.Activate();
	m_worldToProj = ae::Matrix4::Scaling( 2.0f / m_gfx.GetWidth(), 2.0f / m_gfx.GetHeight(), 1.0f )
		* ae::Matrix4::Translation( m_gfx.GetWidth() * -0.5f, m_gfx.GetHeight() * -0.5f, 0.0f );

	DrawWindow();

	std::sort( m_currentFolder->subFolders.begin(), m_currentFolder->subFolders.end(), []( const Folder* a, const Folder* b )
	{
		return a->selectionIdx < b->selectionIdx;
	} );
	DrawFolders();

	if ( m_input.mouse.leftButton ) // Holding mouse left
	{
		if ( m_state == State::BoxSelect )
		{
			m_selectRect = ae::Rect();
			m_selectRect.ExpandPoint( m_selectStart );
			m_selectRect.ExpandPoint( currentCursorPos );
			DrawRect( m_selectRect, ae::Color::PicoBlue().ScaleA( 0.5f ) );
		}
	}
	else if ( m_input.mousePrev.leftButton ) // Release mouse left
	{
		if ( m_state == State::BoxSelect )
		{
			m_selected.Clear();
			for ( Folder* folder : m_currentFolder->subFolders )
			{
				ae::Rect folderRect = ae::Rect::FromCenterAndSize( folder->pos, kFolderIconSize );
				if ( m_selectRect.GetIntersection( folderRect ) )
				{
					m_selected.Append( folder );
					folder->selectionIdx = m_selectionIdx++;
				}
			}
		}
		else if ( m_state == State::Dragging )
		{
			const ae::Vec2 dragOffset = currentCursorPos - m_dragStart;
			for ( Folder* folder : m_selected )
			{
				folder->pos += dragOffset;
			}
		}
		m_state = State::Idle;
	}

	if ( m_rightClick )
	{
		ae::Rect shadowRect = ae::Rect::FromCenterAndSize( m_rightClickRect.GetCenter() + ae::Vec2( 5.0f, -5.0f ), m_rightClickRect.GetSize() );
		DrawRect( shadowRect, ae::Color::PicoDarkGray() );
		DrawRect( m_rightClickRect, ae::Color::PicoLightGray() );
	}

	DrawCursor();

	m_gfx.Present();
	m_timeStep.Tick();
	return !m_input.quit;
}

void Program::DrawRect( ae::Rect rect, ae::Color color )
{
	ae::Vec2 pos = rect.GetCenter();
	ae::Vec2 size = rect.GetSize();
	ae::Matrix4 localToWorld = ae::Matrix4::Translation( pos.x, pos.y, 0.0f ) * ae::Matrix4::Scaling( size.x, size.y, 1.0f );
	ae::UniformList uniformList;
	uniformList.Set( "u_worldToProj", m_worldToProj * localToWorld );
	uniformList.Set( "u_color", color.GetLinearRGBA() );
	m_quad.Bind( &m_shader, uniformList );
	m_quad.Draw();
}

void Program::DrawWindow()
{
	m_gfx.Clear( ae::Color::PicoLightGray() );
	ae::Rect bgRect;
	bgRect.ExpandPoint( ae::Vec2( 20.0f ) );
	bgRect.ExpandPoint( ae::Vec2( m_gfx.GetWidth() - 20.0f, m_gfx.GetHeight() - 60.0f ) );
	DrawRect( bgRect, ae::Color::PicoWhite() );
	bgRect = ae::Rect();
	bgRect.ExpandPoint( ae::Vec2( 20.0f, m_gfx.GetHeight() - 50.0f ) );
	bgRect.ExpandPoint( ae::Vec2( m_gfx.GetWidth() - 20.0f, m_gfx.GetHeight() - 20.0f ) );
	DrawRect( bgRect, ae::Color::PicoWhite() );
}

void Program::DrawCursor()
{
	ae::UniformList uniformList;
	ae::Vec2 mousePos = ae::Vec2( m_input.mouse.position ) * m_window.GetScaleFactor();

	ae::Matrix4 localToWorld = ae::Matrix4::Translation( mousePos.x - 2.0f, mousePos.y + 4.0f, 0.0f ) * ae::Matrix4::Scaling( 30.0f );
	uniformList.Set( "u_worldToProj", m_worldToProj * localToWorld );
	uniformList.Set( "u_color", ae::Color::PicoDarkGray().GetLinearRGBA() );
	m_triangle.Bind( &m_shader, uniformList );
	m_triangle.Draw();

	localToWorld = ae::Matrix4::Translation( mousePos.x, mousePos.y, 0.0f ) * ae::Matrix4::Scaling( 20.0f );
	uniformList.Set( "u_worldToProj", m_worldToProj * localToWorld );
	uniformList.Set( "u_color", ae::Color::PicoWhite().GetLinearRGBA() );
	m_triangle.Bind( &m_shader, uniformList );
	m_triangle.Draw();
}

void Program::DrawFolders()
{
	const ae::Vec2 currentCursorPos = ae::Vec2( m_input.mouse.position ) * m_window.GetScaleFactor();
	const ae::Vec2 dragOffset = currentCursorPos - m_dragStart;
	for ( const Folder* folder : m_currentFolder->subFolders )
	{
		ae::Vec2 pos = folder->pos;
		ae::Color color = ae::Color::PicoPeach();
		if ( m_selected.Find( folder ) >= 0 )
		{
			if ( m_state == State::Dragging )
			{
				pos += dragOffset;
			}
			color = color.Lerp( ae::Color::PicoBlue(), 0.4f );
		}
		ae::Rect folderRect = ae::Rect::FromCenterAndSize( pos, kFolderIconSize );
		ae::Rect shadowRect = ae::Rect::FromCenterAndSize( folderRect.GetCenter() + ae::Vec2( 5.0f, -5.0f ), folderRect.GetSize() );
		DrawRect( shadowRect, ae::Color::PicoDarkGray() );
		DrawRect( folderRect, color );
	}
}

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
	Program program;
	program.Initialize();
#if _AE_EMSCRIPTEN_
	emscripten_set_main_loop_arg( []( void* program ) { ((Program*)program)->Tick(); }, &program, 0, 1 );
#else
	while ( program.Tick() ) {}
#endif
	program.Terminate();
	return 0;
}
