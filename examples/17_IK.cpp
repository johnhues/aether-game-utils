//------------------------------------------------------------------------------
// 16_IK.cpp
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

	window.Initialize( 800, 600, false, true, true );
	window.SetTitle( "17_IK" );
	window.SetAlwaysOnTop( ( ae::GetScreens().FindFn( []( const ae::Screen& s ){ return s.isExternal; } ) >= 0 ) && ae::IsDebuggerAttached() );
	render.Initialize( &window );
	input.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	fileSystem.Initialize( "data", "ae", "ik" );
	camera.Reset( ae::Vec3( 0.0f, 0.0f, 1.0f ), ae::Vec3( 0.0f, 3.5f, 0.4f ) );
	camera.SetDistanceLimits( 0.01f, 25.0f );
	debugLines.Initialize( 20 * 1024 );
	debugLines.SetXRayEnabled( true );
	gridLines.Initialize( 4096 );
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
		if( !fileSystem.Read( ae::FileSystem::Root::Data, fileName, fileData.Data(), fileData.Length() ) )
		{
			AE_ERR( "Error reading fbx file: '#'", fileName );
			return -1;
		}
		
		ae::FbxLoader fbxLoader = TAG_ALL;
		if( !fbxLoader.Initialize( fileData.Data(), fileData.Length() ) )
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
		if( !fbxLoader.Load( fbxLoader.GetMeshName( 0 ), params ) )
		{
			AE_ERR( "Error loading fbx file data: '#'", fileName );
			return -1;
		}
	}

	ae::Skeleton currentPose = TAG_ALL;
	ae::Map< uint32_t, ae::Vec3 > targets = TAG_ALL;
	ae::Map< uint32_t, ae::Quaternion > targetOrientations = TAG_ALL;
	ae::Array< ae::IKConstraints > ikConstraints = TAG_ALL; // @TODO: Clean up. Currently not used
	ae::Map< uint32_t, ae::IKRotationConstraint > rotationConstraints = TAG_ALL;
	ae::Array< ae::IKDistanceConstraint > distanceConstraints = TAG_ALL;
	const ae::Bone* headBone = skin.GetBindPose().GetBoneByName( "QuickRigCharacter_Head" );
	const ae::Bone* neckBone = skin.GetBindPose().GetBoneByName( "QuickRigCharacter_Neck" );
	const ae::Bone* hipsBone = skin.GetBindPose().GetBoneByName( "QuickRigCharacter_Hips" );
	const ae::Bone* spineBone = skin.GetBindPose().GetBoneByName( "QuickRigCharacter_Spine" );
	const ae::Bone* spine1Bone = skin.GetBindPose().GetBoneByName( "QuickRigCharacter_Spine1" );
	const ae::Bone* spine2Bone = skin.GetBindPose().GetBoneByName( "QuickRigCharacter_Spine2" );
	const ae::Bone* rightShoulderBone = skin.GetBindPose().GetBoneByName( "QuickRigCharacter_RightShoulder" );
	const ae::Bone* rightArmBone = skin.GetBindPose().GetBoneByName( "QuickRigCharacter_RightArm" );
	const ae::Bone* rightForearmBone = skin.GetBindPose().GetBoneByName( "QuickRigCharacter_RightForeArm" );
	const ae::Bone* rightHandBone = skin.GetBindPose().GetBoneByName( "QuickRigCharacter_RightHand" );
	const ae::Bone* leftShoulderBone = skin.GetBindPose().GetBoneByName( "QuickRigCharacter_LeftShoulder" );
	const ae::Bone* leftArmBone = skin.GetBindPose().GetBoneByName( "QuickRigCharacter_LeftArm" );
	const ae::Bone* rightUpLegBone = skin.GetBindPose().GetBoneByName( "QuickRigCharacter_RightUpLeg" );
	const ae::Bone* leftUpLegBone = skin.GetBindPose().GetBoneByName( "QuickRigCharacter_LeftUpLeg" );
	{
		// Joint setup
		ikConstraints.Reserve( skin.GetBindPose().GetBoneCount() );
		for( uint32_t i = 0; i < skin.GetBindPose().GetBoneCount(); i++ )
		{
			ae::IKConstraints constraints;
			const ae::Bone* bone = skin.GetBindPose().GetBoneByIndex( i );
			if( strncmp( "QuickRigCharacter_Right", bone->name.c_str(), strlen("QuickRigCharacter_Right") ) == 0 )
			{
				constraints.twistAxis = ae::Axis::NegativeX;
				constraints.horizontalAxis = ae::Axis::NegativeZ;
				constraints.bendAxis = ae::Axis::Y;
			}
			else
			{
				constraints.twistAxis = ae::Axis::X;
				constraints.horizontalAxis = ae::Axis::NegativeZ;
				constraints.bendAxis = ae::Axis::NegativeY;
			}
			ikConstraints.Append( constraints );
		}
		// Rotation constraints
		{
			rotationConstraints.Set( rightArmBone->index, { .rotationLimits={ 0.23f, 0.23f, 0.23f, 0.23f } } );
			rotationConstraints.Set( rightForearmBone->index, { .rotationLimits={ 1.5f, 0.23f, 0.23f, 1.5f } } );
			rotationConstraints.Set( rightHandBone->index, { .rotationLimits={ 0.23f, 0.23f, 1.5f, 0.23f } } );
		}
		// Distance constraints
		{
			// Head
			distanceConstraints.Append( { (int32_t)headBone->index, (int32_t)rightShoulderBone->index, 0.85f, 1.15f } );
			distanceConstraints.Append( { (int32_t)headBone->index, (int32_t)leftShoulderBone->index, 0.85f, 1.15f } );
			distanceConstraints.Append( { (int32_t)neckBone->index, (int32_t)rightArmBone->index, 0.85f, 1.15f } );
			distanceConstraints.Append( { (int32_t)neckBone->index, (int32_t)leftArmBone->index, 0.85f, 1.15f } );
			// Collarbone
			distanceConstraints.Append( { (int32_t)rightShoulderBone->index, (int32_t)leftShoulderBone->index } );
			distanceConstraints.Append( { (int32_t)rightArmBone->index, (int32_t)leftArmBone->index, 1.0f, 1.1f } );
			distanceConstraints.Append( { (int32_t)spine2Bone->index, (int32_t)rightArmBone->index, 1.0f, 1.25f } );
			distanceConstraints.Append( { (int32_t)spine2Bone->index, (int32_t)leftArmBone->index, 1.0f, 1.25f } );
			// Spine
			distanceConstraints.Append( { (int32_t)hipsBone->index, (int32_t)spine1Bone->index, 1.0f, 1.05f } );
			distanceConstraints.Append( { (int32_t)spineBone->index, (int32_t)spine2Bone->index, 1.0f, 1.05f } );
			distanceConstraints.Append( { (int32_t)spine1Bone->index, (int32_t)neckBone->index, 1.0f, 1.05f } );
			distanceConstraints.Append( { (int32_t)spine2Bone->index, (int32_t)headBone->index, 1.0f, 1.05f } );
			// Torso
			distanceConstraints.Append( { (int32_t)leftArmBone->index, (int32_t)leftUpLegBone->index, 0.9f, 1.1f } );
			distanceConstraints.Append( { (int32_t)rightArmBone->index, (int32_t)rightUpLegBone->index, 0.9f, 1.1f } );
			distanceConstraints.Append( { (int32_t)spine1Bone->index, (int32_t)leftArmBone->index, 0.9f, 1.1f } );
			distanceConstraints.Append( { (int32_t)spine1Bone->index, (int32_t)rightArmBone->index, 0.9f, 1.1f } );
			distanceConstraints.Append( { (int32_t)spine2Bone->index, (int32_t)leftUpLegBone->index, 0.9f, 1.1f } );
			distanceConstraints.Append( { (int32_t)spine2Bone->index, (int32_t)rightUpLegBone->index, 0.9f, 1.1f } );
			// Hips
			distanceConstraints.Append( { (int32_t)rightUpLegBone->index, (int32_t)leftUpLegBone->index } );
			distanceConstraints.Append( { (int32_t)spineBone->index, (int32_t)rightUpLegBone->index , 1.0f, 1.2f} );
			distanceConstraints.Append( { (int32_t)spineBone->index, (int32_t)leftUpLegBone->index, 1.0f, 1.2f } );
			distanceConstraints.Append( { (int32_t)spine1Bone->index, (int32_t)rightUpLegBone->index, 0.9f, 1.0f } );
			distanceConstraints.Append( { (int32_t)spine1Bone->index, (int32_t)leftUpLegBone->index, 0.9f, 1.0f } );
		}
	}

	const ae::Matrix4 skeletonTransform = ae::Matrix4::RotationY( ae::Pi ) * ae::Matrix4::RotationX( ae::Pi * -0.5f );
	const ae::Matrix4 testJoint0Bind = ae::Matrix4::Translation( -1.25f, 0.5f, 0.0f ) * ae::Matrix4::Scaling( 0.2f );
	const ae::Matrix4 testJoint1Bind = ae::Matrix4::Translation( 0.0f, 0.0f, 0.8f ) * ae::Matrix4::Scaling( 0.2f );
	ae::Matrix4 testJoint0, testJoint1;
	auto SetDefault = [&]()
	{
		currentPose.Initialize( &skin.GetBindPose() );
		currentPose.SetTransform( currentPose.GetRoot(), skeletonTransform );
		targets.Clear();
		targetOrientations.Clear();
		testJoint0 = testJoint0Bind;
		testJoint1 = testJoint1Bind;
	};
	SetDefault();
	ImGuizmo::OPERATION gizmoOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE gizmoMode = ImGuizmo::LOCAL;
	bool drawMesh = true;
	bool drawSkeleton = true;
	bool autoIK = true;
	bool fromBindPose = true;
	bool drawIK = false;
	int32_t iterCount = 5;
	float ikJointScale = 0.1f;
	bool demoWindow = false;
	enum class TestJointId
	{
		None,
		Target,
		Zero,
		One
	};
	TestJointId selTestJoint = TestJointId::None;
	uint32_t selectedJointIndex = rightHandBone->index;
	ae::IKConstraints testConstraints;
	ae::IKRotationConstraint testRotationConstraints;
	testConstraints.horizontalAxis = ae::Axis::NegativeY;
	testConstraints.bendAxis = ae::Axis::Z;
	testConstraints.twistAxis = ae::Axis::X;
	testRotationConstraints.rotationLimits[ 0 ] = 0.14f;
	testRotationConstraints.rotationLimits[ 1 ] = 0.52f;
	testRotationConstraints.rotationLimits[ 2 ] = 0.38f;
	testRotationConstraints.rotationLimits[ 3 ] = 0.24f;
	auto GetSelectedTransform = [&]( bool allowTarget ) -> ae::Matrix4
	{
		switch( selTestJoint )
		{
			case TestJointId::Zero: return testJoint0;
			case TestJointId::One: return testJoint1;
			default: break;
		}
		ae::Matrix4 transform = currentPose.GetBoneByIndex( selectedJointIndex )->boneToModel;
		if( const ae::Vec3* target = allowTarget ? targets.TryGet( selectedJointIndex ) : nullptr )
		{
			transform.SetTranslation( *target );
		}
		return transform;
	};
	auto GetFocusPos = [&]() -> ae::Vec3
	{
		return GetSelectedTransform( true ).GetTranslation();
	};
	
	AE_INFO( "Run" );
	while( !input.quit )
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
		ImGui::SetNextWindowSize( ImVec2( 350, io.DisplaySize.y ), ImGuiCond_FirstUseEver );
		if( ImGui::Begin( "Options" ) )
		{
			ImGui::Checkbox( "Draw Mesh", &drawMesh );
			ImGui::Checkbox( "Draw Skeleton", &drawSkeleton );
			ImGui::Checkbox( "Draw IK", &drawIK );
			ImGui::Checkbox( "Auto IK", &autoIK );
			ImGui::SameLine();
			ImGui::BeginDisabled( autoIK );
			if( ImGui::Button( "Step" ) )
			{
				shouldStep = true;
			}
			ImGui::EndDisabled();
			ImGui::Checkbox( "From Bind Pose", &fromBindPose );
			ImGui::SliderInt( "Iterations", &iterCount, 0, 10 );
			ImGui::SliderFloat( "Joint Scale", &ikJointScale, 0.01f, 1.0f );

			ImGui::Separator();

			ImGui::RadioButton( "Selection", (int*)&gizmoOperation, 0 );
			ImGui::SameLine();
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

			if( ImGui::Button( "Reset" ) )
			{
				SetDefault();
			}
			ImGui::SameLine();
			if( ImGui::Button( "Focus" ) )
			{
				camera.Refocus( GetFocusPos() );
			}
			ImGui::SameLine();
			if( ImGui::Button( "Demo Window" ) )
			{
				demoWindow = !demoWindow;
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
					if( ImGui::Selectable( bone->name.c_str(), selectedJointIndex == i ) )
					{
						selectedJointIndex = i;
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
					ImGui::Text( "Selected Joint: %s", currentPose.GetBoneByIndex( selectedJointIndex )->name.c_str() );
					return &ikConstraints[ selectedJointIndex ];
				}
				ImGui::Text( "Selected Joint: %s", jointNames[ (int)selTestJoint ] );
				return &testConstraints;
			}();
			const char* axisNames[] = { "None", "X", "Y", "Z", "NegativeX", "NegativeY", "NegativeZ" };
			ImGui::ListBox( "Horizontal", (int*)&constraints->horizontalAxis, axisNames, countof(axisNames), 3 );
			ImGui::ListBox( "Vertical", (int*)&constraints->bendAxis, axisNames, countof(axisNames), 3 );
			ImGui::ListBox( "Primary", (int*)&constraints->twistAxis, axisNames, countof(axisNames), 3 );
			if( selTestJoint == TestJointId::None )
			{
				ae::IKRotationConstraint* rotationConstraint = rotationConstraints.TryGet( selectedJointIndex );
				if( rotationConstraint )
				{
					ImGui::SliderFloat( "R0", &rotationConstraint->rotationLimits[ 0 ], 0.0f, ae::HalfPi );
					ImGui::SliderFloat( "R1", &rotationConstraint->rotationLimits[ 1 ], 0.0f, ae::HalfPi );
					ImGui::SliderFloat( "R2", &rotationConstraint->rotationLimits[ 2 ], 0.0f, ae::HalfPi );
					ImGui::SliderFloat( "R3", &rotationConstraint->rotationLimits[ 3 ], 0.0f, ae::HalfPi );
					ImGui::SliderFloat( "T0", &rotationConstraint->twistLimits[ 0 ], -ae::Pi, 0.0f );
					ImGui::SliderFloat( "T1", &rotationConstraint->twistLimits[ 1 ], 0.0f, ae::Pi );
				}
			}
		}
		if( input.GetPress( ae::Key::V ) )
		{
			drawMesh = !drawMesh;
		}
		if( input.GetPress( ae::Key::S ) )
		{
			drawSkeleton = !drawSkeleton;
		}
		if( input.GetPress( ae::Key::Q ) )
		{
			gizmoOperation = ImGuizmo::OPERATION( 0 );
		}
		if( input.GetPress( ae::Key::W ) )
		{
			if( gizmoOperation != ImGuizmo::TRANSLATE ) { gizmoOperation = ImGuizmo::TRANSLATE; }
			else { gizmoMode = ( gizmoMode == ImGuizmo::WORLD ) ? ImGuizmo::LOCAL : ImGuizmo::WORLD; }
		}
		if( input.GetPress( ae::Key::E ) )
		{
			if( gizmoOperation != ImGuizmo::ROTATE ) { gizmoOperation = ImGuizmo::ROTATE; }
			else { gizmoMode = ( gizmoMode == ImGuizmo::WORLD ) ? ImGuizmo::LOCAL : ImGuizmo::WORLD; }
		}
		if( input.GetPress( ae::Key::R ) )
		{
			gizmoOperation = ImGuizmo::SCALE;
		}
		if( input.GetPress( ae::Key::Space ) )
		{
			SetDefault();
		}
		if( input.GetPress( ae::Key::F ) )
		{
			camera.Refocus( GetFocusPos() );
		}
		if( input.GetPress( ae::Key::I ) )
		{
			autoIK = !autoIK;
		}
		if( !autoIK && input.GetPress( ae::Key::Space ) )
		{
			shouldStep = true;
		}
		ImGui::End();

		if( demoWindow )
		{
			ImGui::ShowDemoWindow( &demoWindow );
		}
		
		camera.SetInputEnabled( !ImGui::GetIO().WantCaptureMouse && !ImGuizmo::IsUsing() );
		camera.Update( &input, dt );
		
		if( fromBindPose && autoIK )
		{
			currentPose.Initialize( &skin.GetBindPose() );
			currentPose.SetTransform( currentPose.GetRoot(), skeletonTransform );
		}
		if( ( autoIK || shouldStep ) && ( drawSkeleton || drawMesh || drawIK ) )
		{
			ae::IK ik = TAG_ALL;
			ik.rootBoneIndex = hipsBone->index;
			ik.targets = targets;
			ik.targetOrientations = targetOrientations;
			ik.distanceConstraints = distanceConstraints;
			ik.rotationConstraints = rotationConstraints;
			ik.bindPose = &skin.GetBindPose();
			ik.pose.Initialize( &currentPose );
			ik.debugLines = drawIK ? &debugLines : nullptr;
			ik.debugJointScale = ikJointScale;
			ik.Run( ( autoIK ? iterCount : 1 ), &currentPose );
		}
		
		// Update mesh
		skin.ApplyPoseToMesh( &currentPose, vertices->pos.data, vertices->normal.data, sizeof(Vertex), sizeof(Vertex), true, true, vertexData.GetMaxVertexCount() );
		vertexData.UploadVertices( 0, vertices, vertexData.GetMaxVertexCount() );
		
		// Debug
		if( drawSkeleton )
		{
			for( uint32_t i = 0; i < currentPose.GetBoneCount(); i++ )
			{
				const ae::Matrix4& t = currentPose.GetBoneByIndex( i )->boneToModel;
				ae::Vec3 p = t.GetTranslation();
				ae::Vec3 xAxis = t.GetAxis( 0 );
				ae::Vec3 yAxis = t.GetAxis( 1 );
				ae::Vec3 zAxis = t.GetAxis( 2 );
				debugLines.AddLine( p, p + xAxis * 0.05f, ae::Color::Red() );
				debugLines.AddLine( p, p + yAxis * 0.05f, ae::Color::Green() );
				debugLines.AddLine( p, p + zAxis * 0.05f, ae::Color::Blue() );
			}

			for( uint32_t i = 0; i < currentPose.GetBoneCount(); i++ )
			{
				const ae::Bone* bone = currentPose.GetBoneByIndex( i );
				const ae::Bone* parent = bone->parent;
				if( parent )
				{
					debugLines.AddLine( parent->boneToModel.GetTranslation(), bone->boneToModel.GetTranslation(), ae::Color::PicoBlue() );
					debugLines.AddOBB( bone->boneToModel * ae::Matrix4::Scaling( 0.05f ), ae::Color::PicoBlue() );
				}
			}

			if( selectedJointIndex < currentPose.GetBoneCount() )
			{
				const ae::Bone* childBone = currentPose.GetBoneByIndex( selectedJointIndex );
				// @TODO: Show selected
			}
		}

		ae::IK ik = TAG_ALL;
		ik.debugLines = &debugLines;
		ik.debugJointScale = ikJointScale;
		const ae::Vec3 testJointClipped = testJoint1.GetTranslation() + ik.ClipJoint(
			( testJoint0Bind.GetTranslation() - testJoint1Bind.GetTranslation() ).Length(),
			testJoint0.GetTranslation(),
			testJoint0.GetRotation(),
			testJoint1.GetTranslation(),
			testConstraints,
			testRotationConstraints,
			ae::Color::PicoBlue()
		);

		// Joint limits
		debugLines.AddOBB( testJoint0, ae::Color::PicoBlue() );
		debugLines.AddOBB( testJoint1, ae::Color::PicoBlue() );
		debugLines.AddLine( testJoint0.GetTranslation(), testJointClipped, ae::Color::PicoBlue() );

		// Add grid
		gridLines.AddLine( ae::Vec3( -2, 0, 0 ), ae::Vec3( 2, 0, 0 ), ae::Color::Red() );
		gridLines.AddLine( ae::Vec3( 0, -2, 0 ), ae::Vec3( 0, 2, 0 ), ae::Color::Green() );
		for( float i = -2; i <= 2.00001f; i += 0.5f )
		{
			if( ae::Abs( i ) < 0.0001f ) { continue; }
			gridLines.AddLine( ae::Vec3( i, -2, 0 ), ae::Vec3( i, 2, 0 ), ae::Color::PicoLightGray() );
			gridLines.AddLine( ae::Vec3( -2, i, 0 ), ae::Vec3( 2, i, 0 ), ae::Color::PicoLightGray() );
		}

		// Start frame
		const ae::Matrix4 worldToView = ae::Matrix4::WorldToView( camera.GetPosition(), camera.GetForward(), camera.GetUp() );
		const ae::Matrix4 viewToProj = ae::Matrix4::ViewToProjection( 0.9f, render.GetAspectRatio(), camera.GetMinDistance() * 0.5f, 50.0f );
		const ae::Matrix4 worldToProj = viewToProj * worldToView;
		const ae::Matrix4 projToWorld = worldToProj.GetInverse();

		// Mouse picking
		if( window.GetFocused() && !ImGui::GetIO().WantCaptureMouse && !ImGuizmo::IsUsing() )
		{
			const ae::Vec4 ndcPos = ae::Vec4( input.mouse.position.x / (float)window.GetWidth() * 2.0f - 1.0f, input.mouse.position.y / (float)window.GetHeight() * 2.0f - 1.0f, 0.0f, 1.0f );
			ae::Vec4 worldPos = projToWorld * ndcPos;
			worldPos /= worldPos.w;
			const ae::Vec3 cameraPos = camera.GetPosition();
			const ae::Vec3 cameraDir = ( worldPos.GetXYZ() - cameraPos ).SafeNormalizeCopy();
			for( uint32_t i = 0; i < currentPose.GetBoneCount(); i++ )
			{
				const ae::Bone* bone = currentPose.GetBoneByIndex( i );
				const ae::Vec3 pos = bone->boneToModel.GetTranslation();
				if( ae::Sphere( pos, ikJointScale * 0.5f ).IntersectRay( cameraPos, cameraDir * 1000.0f ) )
				{
					const bool selected = ( selectedJointIndex == i && selTestJoint == TestJointId::None );
					const ae::Color color = selected ? ae::Color::AetherGreen() : ae::Color::AetherRed();
					debugLines.AddCircle( pos, -cameraDir, ikJointScale * 0.5f, color, 16 );
					if( input.mouse.leftButton && input.mousePrev.leftButton )
					{
						selectedJointIndex = i;
						selTestJoint = TestJointId::None;
					}
					else if( input.mouse.rightButton && input.mousePrev.rightButton )
					{
						selectedJointIndex = i;
						selTestJoint = TestJointId::None;
						targets.Remove( selectedJointIndex );
						targetOrientations.Remove( selectedJointIndex );
					}
				}
			}
		}
		
		if( gizmoOperation && camera.GetMode() == ae::DebugCamera::Mode::None )
		{
			ae::Matrix4 gizmoTransform = GetSelectedTransform( gizmoOperation != ImGuizmo::ROTATE );
			if( ImGuizmo::Manipulate(
				worldToView.data,
				viewToProj.data,
				gizmoOperation,
				( gizmoOperation == ImGuizmo::SCALE ) ? ImGuizmo::LOCAL : gizmoMode,
				gizmoTransform.data
			) )
			{
				switch( selTestJoint )
				{
					case TestJointId::Zero: testJoint0 = gizmoTransform; break;
					case TestJointId::One: testJoint1 = gizmoTransform; break;
					default:
					{
						if( gizmoOperation == ImGuizmo::TRANSLATE )
						{
							targets.Set( selectedJointIndex, gizmoTransform.GetTranslation() );
						}
						else if( gizmoOperation == ImGuizmo::ROTATE )
						{
							targetOrientations.Set( selectedJointIndex, gizmoTransform.GetRotation() );
						}
						break;
					}
				}
			}
		}
		
		render.Activate();
		render.Clear( window.GetFocused() ? ae::Color::AetherBlack() : ae::Color::PicoBlack() );
		
		// Render mesh
		if( drawMesh )
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
