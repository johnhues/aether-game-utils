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
	ae::DebugCamera camera = ae::Axis::Y;
	ae::DebugLines debugLines = TAG_ALL;
	ae::DebugLines gridLines = TAG_ALL;
	aeImGui ui;

	window.Initialize( 800, 600, false, true );
	window.SetTitle( "17_IK" );
	render.Initialize( &window );
	input.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	fileSystem.Initialize( "data", "ae", "ik" );
	camera.Reset( ae::Vec3( 0.0f, 1.0f, 0.0f ), ae::Vec3( 0.0f, 0.4f, 3.5f ) );
	camera.SetDistanceLimits( 1.0f, 25.0f );
	debugLines.Initialize( 4096 );
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
	ae::Matrix4 targetTransform;
	ae::Matrix4 testJointTransform;
	auto SetDefault = [&]()
	{
		testJointTransform = ae::Matrix4::Translation( ae::Vec3( 0.0f, 2.0f, 0.0f ) ) * ae::Matrix4::Scaling( 0.1f );
		targetTransform = skin.GetBindPose().GetBoneByName( rightHandBoneName )->transform;
		currentPose.Initialize( &skin.GetBindPose() );
	};
	SetDefault();
	ImGuizmo::OPERATION gizmoOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE gizmoMode = ImGuizmo::WORLD;
	bool drawMesh = true;
	bool drawSkeleton = true;
	bool autoIK = true;
	bool moveTestJoint = false;
	int selection = 1;
	auto GetSelectedTransform = [&]() -> ae::Matrix4&
	{
		if ( moveTestJoint )
		{
			return testJointTransform;
		}
		return targetTransform;
	};
	float angleLimit[ 4 ] = { ae::QuarterPi, ae::QuarterPi, ae::QuarterPi, ae::QuarterPi };
	
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
			ImGui::Checkbox( "Auto IK", &autoIK );
			ImGui::SameLine();
			ImGui::BeginDisabled( autoIK );
			if ( ImGui::Button( "Step" ) )
			{
				shouldStep = true;
			}
			ImGui::EndDisabled();
			ImGui::Checkbox( "Draw Skeleton", &drawSkeleton );

			ImGui::Separator();

			ImGui::RadioButton( "Translate", (int*)&gizmoOperation, ImGuizmo::TRANSLATE );
			ImGui::SameLine();
			ImGui::RadioButton( "Rotate", (int*)&gizmoOperation, ImGuizmo::ROTATE );

			ImGui::RadioButton( "World", (int*)&gizmoMode, ImGuizmo::WORLD );
			ImGui::SameLine();
			ImGui::RadioButton( "Local", (int*)&gizmoMode, ImGuizmo::LOCAL );

			ImGui::Separator();

			if ( ImGui::Button( "Reset" ) )
			{
				SetDefault();
			}
			ImGui::SameLine();
			if ( ImGui::Button( "Focus" ) )
			{
				camera.Refocus( GetSelectedTransform().GetTranslation() );
			}

			ImGui::Separator();

			ImGui::Checkbox( "Move Test Joint", &moveTestJoint );
			ImGui::SliderFloat( "T1", &angleLimit[ 0 ], 0.0f, ae::HalfPi );
			ImGui::SliderFloat( "T2", &angleLimit[ 1 ], 0.0f, ae::HalfPi );
			ImGui::SliderFloat( "T3", &angleLimit[ 2 ], 0.0f, ae::HalfPi );
			ImGui::SliderFloat( "T4", &angleLimit[ 3 ], 0.0f, ae::HalfPi );
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
			SetDefault();
		}
		if ( input.GetPress( ae::Key::F ) )
		{
			camera.Refocus( GetSelectedTransform().GetTranslation() );
		}
		if ( input.GetPress( ae::Key::I ) )
		{
			autoIK = !autoIK;
		}
		if ( input.Get( ae::Key::Num1 ) ) { selection = 1; }
		if ( input.Get( ae::Key::Num2 ) ) { selection = 2; }
		if ( input.Get( ae::Key::Num3 ) ) { selection = 3; }
		if ( input.Get( ae::Key::Num4 ) ) { selection = 4; }
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
				if ( b->name == anchorBoneName )
				{
					break;
				}
			}
			// ae::IKChain* chain = &ik.chains.Append();
			// chain->bones.Append( { extentBone->transform, extentBone->parent->transform.GetTranslation() } );
			ik.targetTransform = targetTransform;
			ik.pose.Initialize( &currentPose );
			for ( uint32_t idx : ik.chain )
			{
				ae::IKJoint joint;
				const ae::Bone* bone = ik.pose.GetBoneByIndex( idx );
				const bool right = memcmp( "QuickRigCharacter_Right", bone->name.c_str(), strlen( "QuickRigCharacter_Right" ) ) == 0;
				joint.primaryAxis = right ? ae::Vec3( -1, 0, 0 ) : ae::Vec3( 1, 0, 0 );
				ik.joints.Append( joint );
			}
			ik.Run( autoIK ? 10 : 1, &currentPose );

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
		skin.ApplyPoseToMesh( &currentPose, vertices->pos.data, vertices->normal.data, sizeof(Vertex), sizeof(Vertex), vertexData.GetMaxVertexCount() );
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
		}

		ae::Vec3 testJoint( 0.0f, -2.0f, 0.0f );
		ae::Vec3 jointSegment = testJointTransform.GetTranslation() - testJoint;
		ae::Vec3 jointO( 0.0f );
		ae::Plane( ae::Vec3( 0.0f ), ae::Vec3( 0, -1, 0 ) ).IntersectLine( testJoint, jointSegment, &jointO );
		const float q[ 4 ] = {
			ae::Atan( ae::Max( angleLimit[ 0 ], 0.0001f ) ) * testJoint.y,
			ae::Atan( ae::Max( angleLimit[ 1 ], 0.0001f ) ) * testJoint.y,
			ae::Atan( -ae::Max( angleLimit[ 2 ], 0.0001f ) ) * testJoint.y,
			ae::Atan( -ae::Max( angleLimit[ 3 ], 0.0001f ) ) * testJoint.y
		};

		// Joint limits
		debugLines.AddOBB( testJointTransform, ae::Color::Magenta() );
		debugLines.AddLine( testJoint, testJointTransform.GetTranslation(), ae::Color::Magenta() );
		debugLines.AddCircle( jointO, ae::Vec3( 0, 1, 0 ), 0.1f, ae::Color::Magenta(), 16 );
		debugLines.AddCircle( ae::Vec3( q[ 0 ], 0.0f, 0.0f ), ae::Vec3( 0, 1, 0 ), 0.1f, ae::Color::Magenta(), 16 );
		debugLines.AddCircle( ae::Vec3( 0.0f, 0.0f, q[ 1 ] ), ae::Vec3( 0, 1, 0 ), 0.1f, ae::Color::Magenta(), 16 );
		debugLines.AddCircle( ae::Vec3( q[ 2 ], 0.0f, 0.0f ), ae::Vec3( 0, 1, 0 ), 0.1f, ae::Color::Magenta(), 16 );
		debugLines.AddCircle( ae::Vec3( 0.0f, 0.0f, q[ 3 ] ), ae::Vec3( 0, 1, 0 ), 0.1f, ae::Color::Magenta(), 16 );
		debugLines.AddLine( testJoint, ae::Vec3( q[ 0 ], 0.0f, 0.0f ), ae::Color::Magenta() );
		debugLines.AddLine( testJoint, ae::Vec3( 0.0f, 0.0f, q[ 1 ] ), ae::Color::Magenta() );
		debugLines.AddLine( testJoint, ae::Vec3( q[ 2 ], 0.0f, 0.0f ), ae::Color::Magenta() );
		debugLines.AddLine( testJoint, ae::Vec3( 0.0f, 0.0f, q[ 3 ] ), ae::Color::Magenta() );
		for ( uint32_t i = 0; i < 16; i++ )
		{
			const ae::Vec3 ab0( q[ 0 ], 0, q[ 1 ] ); // +x +y
			const ae::Vec3 ab1( q[ 2 ], 0, q[ 1 ] ); // -x +y
			const ae::Vec3 ab2( q[ 2 ], 0, q[ 3 ] ); // -x -y
			const ae::Vec3 ab3( q[ 0 ], 0, q[ 3 ] ); // +x -y
			const float step = ( ae::HalfPi / 16 );
			const float angle = i * step;
			const ae::Vec3 p0( ae::Cos( angle ), 0, ae::Sin( angle ) );
			const ae::Vec3 p1( ae::Cos( angle + step ), 0, ae::Sin( angle + step ) );
			debugLines.AddLine( p0 * ab0, p1 * ab0, ae::Color::Green() );
			debugLines.AddLine( p0 * ab1, p1 * ab1, ae::Color::Green() );
			debugLines.AddLine( p0 * ab2, p1 * ab2, ae::Color::Green() );
			debugLines.AddLine( p0 * ab3, p1 * ab3, ae::Color::Green() );
		}

		// Add grid
		gridLines.AddLine( ae::Vec3( -2, 0, 0 ), ae::Vec3( 2, 0, 0 ), ae::Color::Red() );
		gridLines.AddLine( ae::Vec3( 0, -2, 0 ), ae::Vec3( 0, 2, 0 ), ae::Color::Green() );
		gridLines.AddLine( ae::Vec3( 0, 0, -2 ), ae::Vec3( 0, 0, 2 ), ae::Color::Blue() );
		for ( float i = -2; i <= 2.00001f; i += 0.2f )
		{
			if ( ae::Abs( i ) < 0.0001f ) { continue; }
			gridLines.AddLine( ae::Vec3( i, 0, -2 ), ae::Vec3( i, 0, 2 ), ae::Color::PicoLightGray() );
			gridLines.AddLine( ae::Vec3( -2, 0, i ), ae::Vec3( 2, 0, i ), ae::Color::PicoLightGray() );
		}
		
		// Start frame
		ae::Matrix4 worldToView = ae::Matrix4::WorldToView( camera.GetPosition(), camera.GetForward(), camera.GetLocalUp() );
		ae::Matrix4 viewToProj = ae::Matrix4::ViewToProjection( 0.9f, render.GetAspectRatio(), 0.25f, 50.0f );
		ae::Matrix4 worldToProj = viewToProj * worldToView;
		
		ImGuizmo::Manipulate(
			worldToView.data,
			viewToProj.data,
			gizmoOperation,
			gizmoMode,
			GetSelectedTransform().data
		);
		
		render.Activate();
		render.Clear( ae::Color::PicoDarkPurple() );
		
		// Render mesh
		if ( drawMesh )
		{
			ae::Matrix4 modelToWorld = ae::Matrix4::Identity();
			ae::UniformList uniformList;
			uniformList.Set( "u_worldToProj", worldToProj * modelToWorld );
			uniformList.Set( "u_normalToWorld", modelToWorld.GetNormalMatrix() );
			uniformList.Set( "u_lightDir", ae::Vec3( 0.0f, 0.0f, -1.0f ).NormalizeCopy() );
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
