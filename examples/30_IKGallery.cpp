//------------------------------------------------------------------------------
// 30_IKGallery.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2026 John Hughes
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
// A wall of hand-authored IK specimens sharing scenario definitions and
// invariant checks with test/IKTest.cpp, so a red skeleton here is the same
// failure the unit tests report, at the same scenario and frame.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aether.h"
#include "ae/aeImGui.h"
#include "imgui_internal.h" // DockBuilder + ImGuiDockNodeFlags_NoTabBar
#include "../test/IKTest.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const ae::Tag TAG_ALL = "all";
const uint32_t kGalleryColumns = 5;
const float kGallerySpacing = 2.5f;

//------------------------------------------------------------------------------
// GallerySpecimen
//------------------------------------------------------------------------------
struct GallerySpecimen
{
	GallerySpecimen( const ae::Tag& tag ) : scenario( tag ), runner( tag, &scenario ) {}
	IKScenario scenario;
	IKScenarioRunner runner;
	IKFrameReport lastReport;
	int32_t firstFailFrame = -1;
	int32_t firstTwistFailFrame = -1;
	uint32_t failCount = 0;
	ae::Vec3 offset = ae::Vec3( 0.0f );
};

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
	ae::Window window;
	ae::GraphicsDevice render;
	ae::Input input;
	ae::TimeStep timeStep;
	ae::DebugCamera camera = ae::Axis::Z;
	ae::DebugLines debugLines = TAG_ALL;
	ae::DebugLines gridLines = TAG_ALL;
	ae::RenderTarget viewportTarget;
	aeImGui ui;
	ae::Array< GallerySpecimen* > specimens = TAG_ALL;
	uint32_t frame = 0;
	bool playing = true;
	bool stepOnce = false;
	int32_t iterationCount = 8;
	int32_t startMode = (int32_t)IKStartMode::FromBind;
	int32_t selected = 0;
	bool drawIKDebug = false;
	bool drawConstraints = true;
	float ikJointScale = 0.05f;

	auto Initialize = [&]() -> bool
	{
		AE_INFO( "Initialize" );

		window.Initialize( 1280, 720, false, true, true );
		window.SetTitle( "30_IKGallery" );
		render.Initialize( &window );
		input.Initialize( &window );
		timeStep.SetTimeStep( 1.0f / 60.0f );
		debugLines.Initialize( 64 * 1024 );
		debugLines.SetXRayEnabled( false );
		gridLines.Initialize( 4096 );
		ui.Initialize();

		for( uint32_t i = 0; i < IKScenarioCount(); i++ )
		{
			GallerySpecimen* specimen = ae::New< GallerySpecimen >( TAG_ALL, TAG_ALL );
			IKScenarioBuild( i, &specimen->scenario );
			specimen->runner.Reset();
			const uint32_t specimenColumn = i % kGalleryColumns;
			const uint32_t specimenRow = i / kGalleryColumns;
			specimen->offset = ae::Vec3( (float)specimenColumn * kGallerySpacing, (float)specimenRow * -kGallerySpacing, 0.0f );
			specimens.Append( specimen );
		}
		AE_ASSERT( specimens.Length() );
		selected = 0;
		camera.Reset( specimens[ 0 ]->offset + ae::Vec3( 2.0f * ( kGalleryColumns - 1 ), -2.0f, 1.0f ) * 0.5f, ae::Vec3( 2.5f, 6.0f, 2.5f ) );
		camera.SetDistanceLimits( 0.25f, 40.0f );

		AE_INFO( "Run" );
		return true;
	};

	auto ResetStats = [&]()
	{
		frame = 0;
		for( GallerySpecimen* specimen : specimens )
		{
			specimen->runner.Reset();
			specimen->firstFailFrame = -1;
			specimen->firstTwistFailFrame = -1;
			specimen->failCount = 0;
		}
	};

	auto Update = [&]() -> bool
	{
		const float dt = ae::Max( timeStep.GetTimeStep(), timeStep.GetDt() );
		input.Pump();
		ui.NewFrame( &render, &input, dt );

		// Fixed side-by-side layout: panel docked left, viewport right. The
		// windows have no tab bars so they can't be dragged out, re-docked,
		// or collapsed; only the splitter between them is interactive.
		const ImGuiViewport* imViewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos( imViewport->WorkPos );
		ImGui::SetNextWindowSize( imViewport->WorkSize );
		ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0.0f );
		ImGui::PushStyleVar( ImGuiStyleVar_WindowBorderSize, 0.0f );
		ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0.0f, 0.0f ) );
		ImGui::Begin( "GalleryHost", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoDocking );
		ImGui::PopStyleVar( 3 );
		const ImGuiID dockspaceId = ImGui::GetID( "GalleryDockspace" );
		if( !ImGui::DockBuilderGetNode( dockspaceId ) )
		{
			ImGui::DockBuilderAddNode( dockspaceId, ImGuiDockNodeFlags_DockSpace );
			ImGui::DockBuilderSetNodeSize( dockspaceId, imViewport->WorkSize );
			ImGuiID leftId, rightId;
			ImGui::DockBuilderSplitNode( dockspaceId, ImGuiDir_Left, 0.34f, &leftId, &rightId );
			ImGui::DockBuilderDockWindow( "IK Gallery", leftId );
			ImGui::DockBuilderDockWindow( "Viewport", rightId );
			ImGui::DockBuilderFinish( dockspaceId );
		}
		ImGui::DockSpace( dockspaceId, ImVec2( 0.0f, 0.0f ), ImGuiDockNodeFlags_NoSplit );
		ImGui::End();
		ImGuiWindowClass lockedWindow;
		lockedWindow.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
		const ImGuiWindowFlags lockedWindowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;

		ImGui::SetNextWindowClass( &lockedWindow );
		if( ImGui::Begin( "IK Gallery", nullptr, lockedWindowFlags ) )
		{
			if( ImGui::Button( playing ? "Pause" : "Play" ) )
			{
				playing = !playing;
			}
			ImGui::SameLine();
			ImGui::BeginDisabled( playing );
			if( ImGui::Button( "Step" ) )
			{
				stepOnce = true;
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			if( ImGui::Button( "Restart" ) )
			{
				ResetStats();
			}
			ImGui::SameLine();
			ImGui::Text( "Frame %u", frame );

			if( ImGui::SliderInt( "Iterations", &iterationCount, 1, 16 ) )
			{
				ResetStats();
			}
			const char* startModes[] = { "From Bind", "From Previous" };
			if( ImGui::Combo( "Start Mode", (int*)&startMode, startModes, countof( startModes ) ) )
			{
				ResetStats();
			}
			ImGui::Checkbox( "Draw IK Debug (selected)", &drawIKDebug );
			ImGui::Checkbox( "Draw Distance Constraints", &drawConstraints );
			ImGui::SliderFloat( "Joint Scale", &ikJointScale, 0.01f, 0.25f );
			if( ImGui::Button( "Focus Selected" ) )
			{
				camera.Refocus( specimens[ selected ]->offset + ae::Vec3( 0.0f, 0.0f, 0.9f ) );
			}

			ImGui::Separator();

			if( ImGui::BeginTable( "scenarios", 4, ImGuiTableFlags_RowBg ) )
			{
				ImGui::TableSetupColumn( "Scenario" );
				ImGui::TableSetupColumn( "Status", ImGuiTableColumnFlags_WidthFixed, 50.0f );
				ImGui::TableSetupColumn( "1st Fail", ImGuiTableColumnFlags_WidthFixed, 50.0f );
				ImGui::TableSetupColumn( "Fails", ImGuiTableColumnFlags_WidthFixed, 50.0f );
				ImGui::TableHeadersRow();
				for( int32_t i = 0; i < (int32_t)specimens.Length(); i++ )
				{
					const GallerySpecimen* specimen = specimens[ i ];
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::PushID( i );
					if( ImGui::Selectable( specimen->scenario.name.c_str(), selected == i, ImGuiSelectableFlags_SpanAllColumns ) )
					{
						selected = i;
					}
					ImGui::PopID();
					ImGui::TableNextColumn();
					const bool twistOnly = specimen->lastReport.pass && !specimen->lastReport.twistPass;
					if( specimen->lastReport.pass && specimen->lastReport.twistPass )
					{
						ImGui::TextColored( ImVec4( 0.3f, 1.0f, 0.3f, 1.0f ), "pass" );
					}
					else if( twistOnly )
					{
						ImGui::TextColored( ImVec4( 1.0f, 0.8f, 0.2f, 1.0f ), "twist" );
					}
					else
					{
						ImGui::TextColored( ImVec4( 1.0f, 0.3f, 0.3f, 1.0f ), "FAIL" );
					}
					ImGui::TableNextColumn();
					ImGui::Text( "%d", specimen->firstFailFrame );
					ImGui::TableNextColumn();
					ImGui::Text( "%u", specimen->failCount );
				}
				ImGui::EndTable();
			}

			ImGui::Separator();

			const GallerySpecimen* specimen = specimens[ selected ];
			const IKFrameReport& report = specimen->lastReport;
			ImGui::TextWrapped( "%s", specimen->scenario.description.c_str() );
			ImGui::Text( "bone length:     %f", report.maxBoneLengthError );
			ImGui::Text( "distance:        %f", report.maxDistanceConstraintError );
			ImGui::Text( "rotation limit:  %f", report.maxRotationLimitError );
			ImGui::Text( "twist limit:     %f", report.maxTwistLimitError );
			ImGui::Text( "effector:        %f", report.maxEffectorError );
			ImGui::Text( "effector ori:    %f", report.maxEffectorOriError );
			ImGui::Text( "static drift:    %f", report.maxStaticDrift );
			ImGui::Text( "continuity:      %f", report.maxContinuityDelta );
			ImGui::Text( "continuity ori:  %f", report.maxContinuityOriDelta );
			ImGui::Text( "mirror:          %f", report.maxMirrorError );
			if( !report.worstCheck.Empty() )
			{
				ImGui::TextColored( ImVec4( 1.0f, 0.3f, 0.3f, 1.0f ), "%s", report.worstCheck.c_str() );
			}
		}
		ImGui::End();

		// The 3D scene is rendered offscreen and displayed as a texture, so
		// the viewport participates in the dock layout like any other window
		bool viewportHovered = false;
		ImGui::SetNextWindowClass( &lockedWindow );
		ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0.0f, 0.0f ) );
		if( ImGui::Begin( "Viewport", nullptr, lockedWindowFlags | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse ) )
		{
			const ImVec2 contentSize = ImGui::GetContentRegionAvail();
			const uint32_t targetWidth = (uint32_t)ae::Max( 1.0f, contentSize.x );
			const uint32_t targetHeight = (uint32_t)ae::Max( 1.0f, contentSize.y );
			if( viewportTarget.GetWidth() != targetWidth || viewportTarget.GetHeight() != targetHeight )
			{
				viewportTarget.Initialize( targetWidth, targetHeight );
				viewportTarget.AddTexture( ae::Texture::Filter::Linear, ae::Texture::Wrap::Clamp );
				viewportTarget.AddDepth( ae::Texture::Filter::Nearest, ae::Texture::Wrap::Clamp );
			}
			ImGui::Image( (ImTextureID)(intptr_t)viewportTarget.GetTexture( 0 )->GetTexture(), contentSize, ImVec2( 0.0f, 1.0f ), ImVec2( 1.0f, 0.0f ) );
			viewportHovered = ImGui::IsWindowHovered();
		}
		ImGui::End();
		ImGui::PopStyleVar();

		if( input.GetPress( ae::Key::Space ) )
		{
			playing = !playing;
		}
		if( input.GetPress( ae::Key::F ) )
		{
			camera.Refocus( specimens[ selected ]->offset + ae::Vec3( 0.0f, 0.0f, 0.9f ) );
		}
		if( input.GetPress( ae::Key::R ) )
		{
			ResetStats();
		}
		if( !playing && input.GetPress( ae::Key::Right ) )
		{
			stepOnce = true;
		}

		camera.SetInputEnabled( viewportHovered );
		camera.Update( &input, dt );

		// Step all specimens with identical settings so the gallery reproduces
		// the unit tests exactly
		if( playing || stepOnce )
		{
			for( int32_t i = 0; i < (int32_t)specimens.Length(); i++ )
			{
				GallerySpecimen* specimen = specimens[ i ];
				const uint32_t scenarioFrame = frame % specimen->scenario.frameCount;
				const bool debugThis = drawIKDebug && ( i == selected );
				specimen->lastReport = specimen->runner.Step(
					scenarioFrame,
					(uint32_t)iterationCount,
					(IKStartMode)startMode,
					debugThis ? &debugLines : nullptr,
					ae::Matrix4::Translation( specimen->offset ),
					ikJointScale
				);
				if( !specimen->lastReport.pass )
				{
					specimen->failCount++;
					if( specimen->firstFailFrame < 0 )
					{
						specimen->firstFailFrame = (int32_t)scenarioFrame;
					}
				}
				if( !specimen->lastReport.twistPass && specimen->firstTwistFailFrame < 0 )
				{
					specimen->firstTwistFailFrame = (int32_t)scenarioFrame;
				}
			}
			frame++;
			stepOnce = false;
		}

		// Draw specimens
		for( int32_t i = 0; i < (int32_t)specimens.Length(); i++ )
		{
			const GallerySpecimen* specimen = specimens[ i ];
			const ae::Skeleton& pose = specimen->runner.SolvedPose();
			const ae::Matrix4 modelToWorld = ae::Matrix4::Translation( specimen->offset );
			const bool failed = !specimen->lastReport.pass;
			const ae::Color boneColor = failed ? ae::Color::AetherRed() : ( ( i == selected ) ? ae::Color::AetherGreen() : ae::Color::PicoBlue() );
			for( uint32_t b = 1; b < pose.GetBoneCount(); b++ )
			{
				const ae::Bone* bone = pose.GetBoneByIndex( b );
				const ae::Bone* parent = bone->parent;
				const ae::Vec3 p0 = modelToWorld.TransformPoint3x4( parent->boneToModel.GetTranslation() );
				const ae::Vec3 p1 = modelToWorld.TransformPoint3x4( bone->boneToModel.GetTranslation() );
				if( parent->parent ) // Don't draw the auto root to mount/hips connection
				{
					debugLines.AddLine( p0, p1, boneColor );
				}
				debugLines.AddOBB( modelToWorld * bone->boneToModel * ae::Matrix4::Scaling( ikJointScale * 0.4f ), boneColor );
			}
			// Distance constraints, the bracing loops that hold creatures together
			if( drawConstraints )
			{
				for( const ae::IKDistanceConstraint& constraint : specimen->scenario.distanceConstraints )
				{
					const ae::Vec3 p0 = modelToWorld.TransformPoint3x4( pose.GetBoneByIndex( constraint.idx0 )->boneToModel.GetTranslation() );
					const ae::Vec3 p1 = modelToWorld.TransformPoint3x4( pose.GetBoneByIndex( constraint.idx1 )->boneToModel.GetTranslation() );
					debugLines.AddLine( p0, p1, ae::Color::PicoOrange().ScaleA( 0.7f ) );
				}
			}
			// Targets
			ae::Map< uint32_t, ae::Vec3 > targets = TAG_ALL;
			ae::Map< uint32_t, ae::Quaternion > orientations = TAG_ALL;
			if( specimen->scenario.targetFn )
			{
				const uint32_t lastFrame = ( frame > 0 ) ? ( frame - 1 ) : 0;
				specimen->scenario.targetFn( specimen->scenario, lastFrame % specimen->scenario.frameCount, &targets, &orientations );
			}
			for( const auto& target : targets )
			{
				const ae::Vec3 p = modelToWorld.TransformPoint3x4( target.value );
				debugLines.AddSphere( p, ikJointScale * 0.5f, ae::Color::AetherGreen(), 8 );
			}
		}

		// Grid
		const uint32_t galleryRowCount = ( specimens.Length() + kGalleryColumns - 1 ) / kGalleryColumns;
		const float galleryRows = (float)galleryRowCount;
		for( float g = -2.0f; g <= 2.0f + galleryRows * kGallerySpacing; g += 0.5f )
		{
			const float xMax = 2.0f + ( kGalleryColumns - 1 ) * kGallerySpacing;
			gridLines.AddLine( ae::Vec3( -2.0f, -g, 0.0f ), ae::Vec3( xMax, -g, 0.0f ), ae::Color::PicoDarkGray() );
		}
		for( float g = -2.0f; g <= 2.0f + ( kGalleryColumns - 1 ) * kGallerySpacing; g += 0.5f )
		{
			const float yMin = -( 2.0f + galleryRows * kGallerySpacing );
			gridLines.AddLine( ae::Vec3( g, yMin, 0.0f ), ae::Vec3( g, 2.0f, 0.0f ), ae::Color::PicoDarkGray() );
		}

		if( viewportTarget.GetWidth() )
		{
			const ae::Matrix4 worldToView = ae::Matrix4::WorldToView( camera.GetPosition(), camera.GetForward(), camera.GetUp() );
			const ae::Matrix4 viewToProj = ae::Matrix4::ViewToProjection( 0.9f, viewportTarget.GetAspectRatio(), camera.GetMinDistance() * 0.5f, 100.0f );
			const ae::Matrix4 worldToProj = viewToProj * worldToView;
			viewportTarget.Activate();
			viewportTarget.Clear( window.GetFocused() ? ae::Color::AetherBlack() : ae::Color::PicoBlack() );
			debugLines.Render( worldToProj );
			gridLines.Render( worldToProj );
		}
		else
		{
			debugLines.Clear();
			gridLines.Clear();
		}

		render.Activate();
		render.Clear( ae::Color::AetherBlack() );
		ui.Render();
		render.Present();
		timeStep.Tick();
		return !input.quit;
	};

	auto Terminate = [&]() -> int32_t
	{
		AE_INFO( "Terminate" );
		for( GallerySpecimen* specimen : specimens )
		{
			ae::Delete( specimen );
		}
		specimens.Clear();
		input.Terminate();
		render.Terminate();
		window.Terminate();
		return 0;
	};

	return ae::Application( argc, argv, Initialize, Update, Terminate );
}
