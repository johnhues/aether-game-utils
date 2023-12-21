//------------------------------------------------------------------------------
// 16_IK.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2021 John Hughes
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
#include "ae/loaders.h"
#include "ae/aeImGui.h"
#include "ImGuizmo.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const ae::Tag TAG_ALL = "all";

struct Vertex
{
	ae::Vec4 pos;
	ae::Vec4 normal;
	ae::Vec4 color;
	ae::Vec2 uv;
};

//------------------------------------------------------------------------------
// Shaders
//------------------------------------------------------------------------------
const char* kVertShader = R"(
	AE_UNIFORM mat4 u_worldToProj;
	AE_UNIFORM mat4 u_normalToWorld;
	AE_UNIFORM vec4 u_color;
	AE_IN_HIGHP vec4 a_position;
	AE_IN_HIGHP vec4 a_normal;
	AE_IN_HIGHP vec4 a_color;
	AE_IN_HIGHP vec2 a_uv;
	AE_OUT_HIGHP vec3 v_normal;
	AE_OUT_HIGHP vec4 v_color;
	AE_OUT_HIGHP vec2 v_uv;
	void main()
	{
		v_normal = (u_normalToWorld * a_normal).xyz;
		v_color = a_color * u_color;
		v_uv = a_uv;
		gl_Position = u_worldToProj * a_position;
	})";

const char* kFragShader = R"(
	AE_UNIFORM sampler2D u_tex;
	AE_UNIFORM vec3 u_lightDir;
	AE_UNIFORM vec3 u_lightColor;
	AE_UNIFORM vec3 u_ambColor;
	AE_IN_HIGHP vec3 v_normal;
	AE_IN_HIGHP vec4 v_color;
	AE_IN_HIGHP vec2 v_uv;
	void main()
	{
		vec4 diff = AE_TEXTURE2D( u_tex, v_uv );
		float lightAmt = max(0.0, dot(normalize(v_normal), -u_lightDir));
		vec3 light = u_ambColor + lightAmt * u_lightColor;
		AE_COLOR.rgb = diff.rgb * v_color.rgb * light;
		AE_COLOR.a = diff.a * v_color.a;
		
	})";

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
	AE_INFO( "Initialize" );

	ae::Window window;
	ae::GraphicsDevice render;
	ae::Input input;
	ae::TimeStep timeStep;
	ae::Shader shader;
	ae::FileSystem fileSystem;
	ae::DebugCamera camera = ae::Axis::Z;
	ae::DebugLines debugLines = TAG_ALL;
	ae::DebugLines gridLines = TAG_ALL;
	aeImGui ui;

	window.Initialize( 800, 600, false, true );
	window.SetTitle( "17_IK" );
	window.SetAlwaysOnTop( ( ae::GetScreens().FindFn( []( const ae::Screen& s ){ return s.isExternal; } ) >= 0 ) && ae::IsDebuggerAttached() );
	render.Initialize( &window );
	input.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	fileSystem.Initialize( "data", "ae", "ik" );
	camera.Reset( ae::Vec3( 0.0f, 0.0f, 1.0f ), ae::Vec3( 0.0f, 3.5f, 0.4f ) );
	camera.SetDistanceLimits( 0.25f, 25.0f );
	debugLines.Initialize( 20 * 1024 );
	gridLines.Initialize( 4096 );
	gridLines.SetXRayEnabled( false );
	ui.Initialize();

	shader.Initialize( kVertShader, kFragShader, nullptr, 0 );
	shader.SetDepthTest( true );
	shader.SetDepthWrite( true );
	shader.SetBlending( true );
	shader.SetCulling( ae::Culling::CounterclockwiseFront );

	ae::Texture2D texture;
	{
		ae::TargaFile targaFile = TAG_ALL;
		uint32_t fileSize = fileSystem.GetSize( ae::FileSystem::Root::Data, "character.tga" );
		AE_ASSERT( fileSize );
		ae::Scratch< uint8_t > fileData( fileSize );
		fileSystem.Read( ae::FileSystem::Root::Data, "character.tga", fileData.Data(), fileData.Length() );
		targaFile.Load( fileData.Data(), fileData.Length() );
		texture.Initialize( targaFile.textureParams );
	}
	
	ae::Skin skin = TAG_ALL;
	ae::VertexBuffer vertexData;
	Vertex* vertices = nullptr;
	{
		const char* fileName = "character.fbx";
		uint32_t fileSize = fileSystem.GetSize( ae::FileSystem::Root::Data, fileName );
		AE_ASSERT_MSG( fileSize, "Could not load '#'", fileName );
		ae::Scratch< uint8_t > fileData( fileSize );
		if ( !fileSystem.Read( ae::FileSystem::Root::Data, fileName, fileData.Data(), fileData.Length() ) )
		{
			AE_ERR( "Error reading fbx file: '#'", fileName );
			return -1;
		}
		
		ae::FbxLoader fbxLoader = TAG_ALL;
		if ( !fbxLoader.Initialize( fileData.Data(), fileData.Length() ) )
		{
			AE_ERR( "Error parsing fbx file: '#'", fileName );
			return -1;
		}
		
		ae::FbxLoaderParams params;
		params.descriptor.vertexSize = sizeof(Vertex);
		params.descriptor.indexSize = 4;
		params.descriptor.posOffset = offsetof( Vertex, pos );
		params.descriptor.normalOffset = offsetof( Vertex, normal );
		params.descriptor.colorOffset = offsetof( Vertex, color );
		params.descriptor.uvOffset = offsetof( Vertex, uv );
		params.vertexData = &vertexData;
		params.skin = &skin;
		params.maxVerts = fbxLoader.GetMeshVertexCount( 0u );
		vertices = ae::NewArray< Vertex >( TAG_ALL, params.maxVerts );
		params.vertexOut = vertices;
		if ( !fbxLoader.Load( fbxLoader.GetMeshName( 0 ), params ) )
		{
			AE_ERR( "Error loading fbx file data: '#'", fileName );
			return -1;
		}
	}
	
	// const char* anchorBoneName = "QuickRigCharacter_Hips";
	// const char* headBoneName = "QuickRigCharacter_Head";
	// const char* rightHandBoneName = "QuickRigCharacter_RightHand";
	// const char* leftHandBoneName = "QuickRigCharacter_LeftHand";
	// const char* leftFootBoneName = "QuickRigCharacter_LeftFoot";
	// const char* rightFootBoneName = "QuickRigCharacter_RightFoot";

	const char* rightHandBoneName = "QuickRigCharacter_RightHand";
	const char* anchorBoneName = "QuickRigCharacter_RightShoulder";

	ae::Skeleton currentPose = TAG_ALL;
	ae::Array< ae::IKConstraints > ikConstraints = TAG_ALL;
	ikConstraints.Reserve( skin.GetBindPose().GetBoneCount() );
	for ( uint32_t i = 0; i < skin.GetBindPose().GetBoneCount(); i++ )
	{
		ae::IKConstraints constraints;
		const ae::Bone* bone = skin.GetBindPose().GetBoneByIndex( i );
		if( strncmp( "QuickRigCharacter_Right", bone->name.c_str(), strlen("QuickRigCharacter_Right") ) == 0 )
		{
			constraints.primaryAxis = ae::Axis::NegativeX;
			constraints.horizontalAxis = ae::Axis::NegativeZ;
			constraints.verticalAxis = ae::Axis::Y;
		}
		else
		{
			constraints.primaryAxis = ae::Axis::X;
			constraints.horizontalAxis = ae::Axis::NegativeZ;
			constraints.verticalAxis = ae::Axis::NegativeY;
		}
		ikConstraints.Append( constraints );
	}

	const ae::Matrix4 testJoint0Bind = ae::Matrix4::Translation( 0.0f, 0.0f, 0.0f ) * ae::Matrix4::Scaling( 0.2f );
	const ae::Matrix4 testJoint1Bind = ae::Matrix4::Translation( 0.0f, 0.0f, 2.0f ) * ae::Matrix4::Scaling( 0.2f );
	ae::Matrix4 targetTransform, testJoint0, testJoint1;
	auto SetDefault = [&]()
	{
		const ae::Matrix4 skeletonTransform = ae::Matrix4::RotationY( ae::Pi ) * ae::Matrix4::RotationX( ae::Pi * -0.5f );
		targetTransform = skeletonTransform * skin.GetBindPose().GetBoneByName( rightHandBoneName )->transform;
		testJoint0 = testJoint0Bind;
		testJoint1 = testJoint1Bind;
		currentPose.Initialize( &skin.GetBindPose() );
		currentPose.SetTransform( currentPose.GetRoot(), skeletonTransform );
	};
	SetDefault();
	ImGuizmo::OPERATION gizmoOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE gizmoMode = ImGuizmo::WORLD;
	bool drawMesh = false;
	bool drawSkeleton = true;
	bool autoIK = true;
	enum class TestJointId
	{
		None,
		Target,
		Zero,
		One
	};
	TestJointId selTestJoint = TestJointId::None;
	static uint32_t s_selectedJointIndex = skin.GetBindPose().GetBoneByName( rightHandBoneName )->index;
	ae::IKConstraints testConstraints;
	testConstraints.horizontalAxis = ae::Axis::NegativeY;
	testConstraints.verticalAxis = ae::Axis::Z;
	testConstraints.primaryAxis = ae::Axis::X;
	testConstraints.rotationLimits[ 0 ] = 0.14f;
	testConstraints.rotationLimits[ 1 ] = 0.52f;
	testConstraints.rotationLimits[ 2 ] = 0.38f;
	testConstraints.rotationLimits[ 3 ] = 0.24f;
	auto GetSelectedTransform = [&]() -> ae::Matrix4&
	{
		switch( selTestJoint )
		{
			case TestJointId::Zero: return testJoint0;
			case TestJointId::One: return testJoint1;
			default: break;
		}
		return targetTransform; // IK Handle
	};
	auto GetFocusPos = [&]() -> ae::Vec3
	{
		if( selTestJoint != TestJointId::None ) { return GetSelectedTransform().GetTranslation(); }
		return currentPose.GetBoneByIndex( s_selectedJointIndex )->transform.GetTranslation();
	};
	
	AE_INFO( "Run" );
	while ( !input.quit )
	{
		const float dt = ae::Max( timeStep.GetTimeStep(), timeStep.GetDt() );
		input.Pump();

		ImGuiIO& io = ImGui::GetIO();
		ui.NewFrame( &render, &input, dt );
		ImGuizmo::SetOrthographic( false );
		ImGuizmo::SetRect( 0, 0, io.DisplaySize.x, io.DisplaySize.y );
		ImGuizmo::AllowAxisFlip( false );
		ImGuizmo::BeginFrame();

		bool shouldStep = false;
		ImGui::SetNextWindowPos( ImVec2( 0, 0 ), ImGuiCond_FirstUseEver );
		ImGui::SetNextWindowSize( ImVec2( 200, 300 ), ImGuiCond_FirstUseEver );
		if ( ImGui::Begin( "Options" ) )
		{
			ImGui::Checkbox( "Draw Mesh", &drawMesh );
			ImGui::Checkbox( "Draw Skeleton", &drawSkeleton );
			ImGui::Checkbox( "Auto IK", &autoIK );
			ImGui::SameLine();
			ImGui::BeginDisabled( autoIK );
			if ( ImGui::Button( "Step" ) )
			{
				shouldStep = true;
			}
			ImGui::EndDisabled();

			ImGui::Separator();

			ImGui::RadioButton( "Translate", (int*)&gizmoOperation, ImGuizmo::TRANSLATE );
			ImGui::SameLine();
			ImGui::RadioButton( "Rotate", (int*)&gizmoOperation, ImGuizmo::ROTATE );
			ImGui::SameLine();
			ImGui::RadioButton( "Scale", (int*)&gizmoOperation, ImGuizmo::SCALE );

			ImGui::BeginDisabled( gizmoOperation == ImGuizmo::SCALE );
			ImGui::RadioButton( "World", (int*)&gizmoMode, ImGuizmo::WORLD );
			ImGui::SameLine();
			ImGuizmo::MODE scaleGizmoMode = ImGuizmo::LOCAL;
			ImGui::RadioButton( "Local", (int*)( ( gizmoOperation == ImGuizmo::SCALE ) ? &scaleGizmoMode : &gizmoMode ), ImGuizmo::LOCAL );
			ImGui::EndDisabled();

			ImGui::Separator();

			if ( ImGui::Button( "Reset" ) )
			{
				SetDefault();
			}
			ImGui::SameLine();
			if ( ImGui::Button( "Focus" ) )
			{
				camera.Refocus( GetFocusPos() );
			}

			ImGui::Separator();

			const char* jointNames[] = { "None", "Target", "Test 0", "Test 1" };
			ImGui::ListBox( "Test Joints", (int*)&selTestJoint, jointNames, countof(jointNames), 4 );

			if( ImGui::ListBoxHeader( "Joints" ) )
			{
				for( uint32_t i = 0; i < currentPose.GetBoneCount(); i++ )
				{
					const ae::Bone* bone = currentPose.GetBoneByIndex( i );
					ImGui::PushID( i );
					if( ImGui::Selectable( bone->name.c_str(), s_selectedJointIndex == i ) )
					{
						s_selectedJointIndex = i;
					}
					ImGui::PopID();
				}
				ImGui::ListBoxFooter();
			}
			bool constraintsModified = false;
			ae::IKConstraints* constraints = [&]()
			{
				if( selTestJoint == TestJointId::None )
				{
					ImGui::Text( "Selected Joint: %s", currentPose.GetBoneByIndex( s_selectedJointIndex )->name.c_str() );
					return &ikConstraints[ s_selectedJointIndex ];
				}
				ImGui::Text( "Selected Joint: %s", jointNames[ (int)selTestJoint ] );
				return &testConstraints;
			}();
			const char* axisNames[] = { "None", "X", "Y", "Z", "NegativeX", "NegativeY", "NegativeZ" };
			ImGui::ListBox( "Horizontal", (int*)&constraints->horizontalAxis, axisNames, countof(axisNames), 3 );
			ImGui::ListBox( "Vertical", (int*)&constraints->verticalAxis, axisNames, countof(axisNames), 3 );
			ImGui::ListBox( "Primary", (int*)&constraints->primaryAxis, axisNames, countof(axisNames), 3 );
			ImGui::SliderFloat( "T0", &constraints->rotationLimits[ 0 ], 0.0f, ae::HalfPi );
			ImGui::SliderFloat( "T1", &constraints->rotationLimits[ 1 ], 0.0f, ae::HalfPi );
			ImGui::SliderFloat( "T2", &constraints->rotationLimits[ 2 ], 0.0f, ae::HalfPi );
			ImGui::SliderFloat( "T3", &constraints->rotationLimits[ 3 ], 0.0f, ae::HalfPi );
		}
		if ( input.GetPress( ae::Key::V ) )
		{
			drawMesh = !drawMesh;
		}
		if ( input.GetPress( ae::Key::S ) )
		{
			drawSkeleton = !drawSkeleton;
		}
		if ( input.GetPress( ae::Key::W ) )
		{
			if ( gizmoOperation != ImGuizmo::TRANSLATE ) { gizmoOperation = ImGuizmo::TRANSLATE; }
			else { gizmoMode = ( gizmoMode == ImGuizmo::WORLD ) ? ImGuizmo::LOCAL : ImGuizmo::WORLD; }
		}
		if ( input.GetPress( ae::Key::E ) )
		{
			if ( gizmoOperation != ImGuizmo::ROTATE ) { gizmoOperation = ImGuizmo::ROTATE; }
			else { gizmoMode = ( gizmoMode == ImGuizmo::WORLD ) ? ImGuizmo::LOCAL : ImGuizmo::WORLD; }
		}
		if ( input.GetPress( ae::Key::R ) )
		{
			gizmoOperation = ImGuizmo::SCALE;
		}
		if ( input.GetPress( ae::Key::Space ) )
		{
			SetDefault();
		}
		if ( input.GetPress( ae::Key::F ) )
		{
			camera.Refocus( GetFocusPos() );
		}
		if ( input.GetPress( ae::Key::I ) )
		{
			autoIK = !autoIK;
		}
		if ( !autoIK && input.GetPress( ae::Key::Space ) )
		{
			shouldStep = true;
		}
		ImGui::End();
		
		camera.SetInputEnabled( !ImGui::GetIO().WantCaptureMouse && !ImGuizmo::IsUsing() );
		camera.Update( &input, dt );
		
		if ( autoIK || shouldStep )
		{
			ae::IK ik = TAG_ALL;
			const ae::Bone* extentBone = currentPose.GetBoneByName( rightHandBoneName );
			for ( auto b = extentBone; b; b = b->parent )
			{
				ik.chain.Insert( 0, b->index );
				ik.joints.Insert( 0, ikConstraints[ b->index ] );
				if ( b->name == anchorBoneName )
				{
					break;
				}
			}
			// ae::IKChain* chain = &ik.chains.Append();
			// chain->bones.Append( { extentBone->transform, extentBone->parent->transform.GetTranslation() } );
			ik.targetTransform = targetTransform;
			ik.pose.Initialize( &currentPose );
			ik.Run( autoIK ? 10 : 1, &currentPose, drawSkeleton ? &debugLines : nullptr );

			// Debug
			if ( drawSkeleton )
			{
				for ( uint32_t idx : ik.chain )
				{
					debugLines.AddOBB( currentPose.GetBoneByIndex( idx )->transform * ae::Matrix4::Scaling( 0.1f ), ae::Color::Magenta() );
				}
			}
		}
		
		// Update mesh
		skin.ApplyPoseToMesh( &currentPose, vertices->pos.data, vertices->normal.data, sizeof(Vertex), sizeof(Vertex), true, true, vertexData.GetMaxVertexCount() );
		vertexData.UploadVertices( 0, vertices, vertexData.GetMaxVertexCount() );
		
		// Debug
		if ( drawSkeleton )
		{
			for ( uint32_t i = 0; i < currentPose.GetBoneCount(); i++ )
			{
				const ae::Matrix4& t = currentPose.GetBoneByIndex( i )->transform;
				ae::Vec3 p = t.GetTranslation();
				ae::Vec3 xAxis = t.GetAxis( 0 );
				ae::Vec3 yAxis = t.GetAxis( 1 );
				ae::Vec3 zAxis = t.GetAxis( 2 );
				debugLines.AddLine( p, p + xAxis * 0.2f, ae::Color::Red() );
				debugLines.AddLine( p, p + yAxis * 0.2f, ae::Color::Green() );
				debugLines.AddLine( p, p + zAxis * 0.2f, ae::Color::Blue() );
			}

			for ( uint32_t i = 0; i < currentPose.GetBoneCount(); i++ )
			{
				const ae::Bone* bone = currentPose.GetBoneByIndex( i );
				const ae::Bone* parent = bone->parent;
				if ( parent )
				{
					debugLines.AddLine( parent->transform.GetTranslation(), bone->transform.GetTranslation(), ae::Color::PicoBlue() );
					debugLines.AddOBB( bone->transform * ae::Matrix4::Scaling( 0.05f ), ae::Color::PicoBlue() );
				}
			}

			if( s_selectedJointIndex < currentPose.GetBoneCount() )
			{
				const ae::Bone* childBone = currentPose.GetBoneByIndex( s_selectedJointIndex );
				// @TODO: Show selected
			}
		}

		const ae::Vec3 testJointClipped = [&]()
		{
			return ae::IK::ClipJoint(
				( testJoint0Bind.GetTranslation() - testJoint1Bind.GetTranslation() ).Length(),
				testJoint0.GetTranslation(),
				testJoint0.GetRotation(),
				testJoint1.GetTranslation(),
				testConstraints,
				&debugLines
			);
		}();

		// Joint limits
		debugLines.AddOBB( testJoint0, ae::Color::PicoBlue() );
		debugLines.AddOBB( testJoint1, ae::Color::PicoBlue() );
		debugLines.AddLine( testJoint0.GetTranslation(), testJointClipped, ae::Color::PicoBlue() );

		// Add grid
		gridLines.AddLine( ae::Vec3( -2, 0, 0 ), ae::Vec3( 2, 0, 0 ), ae::Color::Red() );
		gridLines.AddLine( ae::Vec3( 0, -2, 0 ), ae::Vec3( 0, 2, 0 ), ae::Color::Green() );
		for ( float i = -2; i <= 2.00001f; i += 0.5f )
		{
			if ( ae::Abs( i ) < 0.0001f ) { continue; }
			gridLines.AddLine( ae::Vec3( i, -2, 0 ), ae::Vec3( i, 2, 0 ), ae::Color::PicoLightGray() );
			gridLines.AddLine( ae::Vec3( -2, i, 0 ), ae::Vec3( 2, i, 0 ), ae::Color::PicoLightGray() );
		}

		// Start frame
		const ae::Matrix4 worldToView = ae::Matrix4::WorldToView( camera.GetPosition(), camera.GetForward(), camera.GetLocalUp() );
		const ae::Matrix4 viewToProj = ae::Matrix4::ViewToProjection( 0.9f, render.GetAspectRatio(), camera.GetMinDistance() * 0.5f, 50.0f );
		const ae::Matrix4 worldToProj = viewToProj * worldToView;
		const ae::Matrix4 projToWorld = worldToProj.GetInverse();

		// if( input.mouse.leftButton && input.mousePrev.leftButton )
		{
			const ae::Vec4 ndcPos = ae::Vec4( input.mouse.position.x / (float)window.GetWidth() * 2.0f - 1.0f, input.mouse.position.y / (float)window.GetHeight() * 2.0f - 1.0f, 0.0f, 1.0f );
			ae::Vec4 worldPos = projToWorld * ndcPos;
			worldPos /= worldPos.w;
			const ae::Vec3 rayOrigin = camera.GetPosition();
			const ae::Vec3 rayDir = ( worldPos.GetXYZ() - rayOrigin ).SafeNormalizeCopy();
			const ae::Plane plane( ae::Vec3( 0.0f ), ae::Vec3( 0, 0, 1 ) );
			ae::Vec3 intersection;
			if ( plane.IntersectRay( rayOrigin, rayDir * 1000.0f, &intersection ) )
			{
				debugLines.AddCircle( intersection, -rayDir, 0.1f, ae::Color::Red(), 16 );
			}

			// @TODO: Picking
			// for ( uint32_t i = 0; i < currentPose.GetBoneCount(); i++ )
			// {
			// 	const ae::Bone* bone = currentPose.GetBoneByIndex( i );
			// 	const ae::Bone* parent = bone->parent;
			// 	if ( parent )
			// 	{
			// 		const ae::Vec3 pos = bone->transform.GetTranslation();
			// 		const ae::Color color = ( ae::Line( rayOrigin, rayOrigin + rayDir ).GetDistance( pos ) < 0.3f ) ? ae::Color::PicoRed() : ae::Color::PicoBlue();
			// 		debugLines.AddCircle( pos, -rayDir, 0.1f, color, 16 );
			// 	}
			// }
		}
		
		ImGuizmo::Manipulate(
			worldToView.data,
			viewToProj.data,
			gizmoOperation,
			( gizmoOperation == ImGuizmo::SCALE ) ? ImGuizmo::LOCAL : gizmoMode,
			GetSelectedTransform().data
		);
		
		render.Activate();
		render.Clear( window.GetFocused() ? ae::Color::AetherBlack() : ae::Color::PicoBlack() );
		
		// Render mesh
		if ( drawMesh )
		{
			ae::Matrix4 modelToWorld = ae::Matrix4::Identity();
			ae::UniformList uniformList;
			uniformList.Set( "u_worldToProj", worldToProj * modelToWorld );
			uniformList.Set( "u_normalToWorld", modelToWorld.GetNormalMatrix() );
			uniformList.Set( "u_lightDir", ae::Vec3( 0.0f, -1.0f, 0.0f ).NormalizeCopy() );
			uniformList.Set( "u_lightColor", ae::Color::PicoPeach().GetLinearRGB() );
			uniformList.Set( "u_ambColor", ae::Vec3( 0.8f ) );
			uniformList.Set( "u_color", ae::Color::White().GetLinearRGBA() );
			uniformList.Set( "u_tex", &texture );
			vertexData.Bind( &shader, uniformList );
			vertexData.Draw();
		}
		
		// Frame end
		debugLines.Render( worldToProj );
		gridLines.Render( worldToProj );
		ui.Render();
		render.Present();
		timeStep.Tick();
	}

	AE_INFO( "Terminate" );
	ae::Delete( vertices );
	vertices = nullptr;
	input.Terminate();
	render.Terminate();
	window.Terminate();

	return 0;
}
