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

ae::Vec2 GetNearestPointOnEllipse( ae::Vec2 halfSize, ae::Vec2 center, ae::Vec2 p )
{
	// https://stackoverflow.com/a/46007540/2423134
	// https://blog.chatfield.io/simple-method-for-distance-to-ellipse/
	// https://github.com/0xfaded/ellipse_demo/issues/1
	const float px = ae::Abs( p[ 0 ] );
	const float py = ae::Abs( p[ 1 ] );
	const float a = ae::Abs( halfSize.x );
	const float b = ae::Abs( halfSize.y );

	float tx = 0.707f;
	float ty = 0.707f;
	// Only 3 iterations should be needed for high quality results
	for( uint32_t i = 0; i < 3; i++ )
	{
		const float x = a * tx;
		const float y = b * ty;
		const float ex = ( a * a - b * b ) * ( tx * tx * tx ) / a;
		const float ey = ( b * b - a * a ) * ( ty * ty * ty ) / b;
		const float rx = x - ex;
		const float ry = y - ey;
		const float qx = px - ex;
		const float qy = py - ey;
		const float r = hypotf( ry, rx );
		const float q = hypotf( qy, qx );
		tx = ae::Min( 1.0f, ae::Max( 0.0f, ( qx * r / q + ex ) / a ) );
		ty = ae::Min( 1.0f, ae::Max( 0.0f, ( qy * r / q + ey ) / b ) );
		const float t = hypotf( ty, tx );
		tx /= t;
		ty /= t;
	}

	return ae::Vec2( copysignf( a * tx, p[ 0 ] ), copysignf( b * ty, p[ 1 ] ) );
}

ae::Vec3 GetAxisVector( ae::Axis axis, bool negative = true )
{
	if( negative )
	{
		switch( axis )
		{
			case ae::Axis::X: return ae::Vec3( 1, 0, 0 );
			case ae::Axis::Y: return ae::Vec3( 0, 1, 0 );
			case ae::Axis::Z: return ae::Vec3( 0, 0, 1 );
			case ae::Axis::NegativeX: return ae::Vec3( -1, 0, 0 );
			case ae::Axis::NegativeY: return ae::Vec3( 0, -1, 0 );
			case ae::Axis::NegativeZ: return ae::Vec3( 0, 0, -1 );
		}
	}
	else
	{
		switch( axis )
		{
			case ae::Axis::X:
			case ae::Axis::NegativeX:
				return ae::Vec3( 1, 0, 0 );
			case ae::Axis::Y:
			case ae::Axis::NegativeY:
				return ae::Vec3( 0, 1, 0 );
			case ae::Axis::Z:
			case ae::Axis::NegativeZ:
				return ae::Vec3( 0, 0, 1 );
		}
	}
	return ae::Vec3( 0.0f );
}

ae::Vec3 ClipJoint(
	const ae::Matrix4& bind0,
	const ae::Matrix4& bind1,
	const ae::Matrix4& j0,
	const ae::Matrix4& j0Inv,
	const ae::Matrix4& j1,
	const ae::Axis ha,
	const ae::Axis va,
	const ae::Axis pa,
	const float (&j0AngleLimits)[ 4 ],
	ae::DebugLines* debugLines = nullptr )
{
	const auto Build3D = []( ae::Axis horizontalAxis, ae::Axis verticalAxis, ae::Axis primaryAxis, float horizontalVal, float verticalVal, float primaryVal, bool negative = true ) -> ae::Vec3
	{
		ae::Vec3 result = GetAxisVector( horizontalAxis, negative ) * horizontalVal;
		result += GetAxisVector( verticalAxis, negative ) * verticalVal;
		result += GetAxisVector( primaryAxis, negative ) * primaryVal;
		return result;
	};
	const auto GetAxis = []( ae::Axis axis, const ae::Vec3 v ) -> float
	{
		switch( axis )
		{
			case ae::Axis::X: return v.x;
			case ae::Axis::Y: return v.y;
			case ae::Axis::Z: return v.z;
			case ae::Axis::NegativeX: return -v.x;
			case ae::Axis::NegativeY: return -v.y;
			case ae::Axis::NegativeZ: return -v.z;
			default: return 0.0f;
		}
	};

	const ae::Vec3 b0 = ( j0Inv * ae::Vec4( bind0.GetTranslation(), 1.0f ) ).GetXYZ();
	const ae::Vec3 b1 = ( j0Inv * ae::Vec4( bind1.GetTranslation(), 1.0f ) ).GetXYZ();
	const float b01Len = ( b0 - b1 ).Length();
	const ae::Vec2 j1Flat = [&]()
	{
		ae::Vec3 p;
		const ae::Plane plane = ae::Plane(
			Build3D( ha, va, pa, 0, 0, b01Len ),
			Build3D( ha, va, pa, 0, 0, 1 )
		);
		const ae::Vec3 j1Local = ( j0Inv * ae::Vec4( j1.GetTranslation(), 1.0f ) ).GetXYZ();
		plane.IntersectLine( ae::Vec3( 0.0f ), j1Local, &p );
		return ae::Vec2( GetAxis( ha, p ), GetAxis( va, p ) );
	}();
	const float q[ 4 ] =
	{
		b01Len * ae::Tan( ae::Clip( j0AngleLimits[ 0 ], 0.01f, ae::HalfPi - 0.01f ) ),
		b01Len * ae::Tan( ae::Clip( j0AngleLimits[ 1 ], 0.01f, ae::HalfPi - 0.01f ) ),
		b01Len * ae::Tan( -ae::Clip( j0AngleLimits[ 2 ], 0.01f, ae::HalfPi - 0.01f ) ),
		b01Len * ae::Tan( -ae::Clip( j0AngleLimits[ 3 ], 0.01f, ae::HalfPi - 0.01f ) )
	};

	const ae::Vec2 quadrantEllipse = [q, j1Flat]()
	{
		if( j1Flat.x > 0.0f && j1Flat.y > 0.0f ) { return ae::Vec2( q[ 0 ], q[ 1 ] ); } // +x +y
		if( j1Flat.x < 0.0f && j1Flat.y > 0.0f ) { return ae::Vec2( q[ 2 ], q[ 1 ] ); } // -x +y
		if( j1Flat.x < 0.0f && j1Flat.y < 0.0f ) { return ae::Vec2( q[ 2 ], q[ 3 ] ); } // -x -y
		return ae::Vec2( q[ 0 ], q[ 3 ] ); // +x -y
	}();
	const ae::Vec2 edge = GetNearestPointOnEllipse( quadrantEllipse, ae::Vec2( 0.0f ), j1Flat );
	ae::Vec2 posClipped = j1Flat;
	if( posClipped.LengthSquared() > edge.LengthSquared() )
	{
		posClipped = edge;
	}
	const ae::Vec3 resultLocal = Build3D( ha, va, pa, posClipped.x, posClipped.y, b01Len ).NormalizeCopy() * b01Len;
	const ae::Vec3 resultWorld = ( j0 * ae::Vec4( resultLocal, 1.0f ) ).GetXYZ();

	if( debugLines )
	{
		const ae::Vec3 j1FlatWorld = ( j0 * ae::Vec4( Build3D( ha, va, pa, j1Flat.x, j1Flat.y, b01Len ), 1 ) ).GetXYZ();
		const ae::Vec3 j1FlatWorldClipped = ( j0 * ae::Vec4( Build3D( ha, va, pa, posClipped.x, posClipped.y, b01Len ), 1 ) ).GetXYZ();
		debugLines->AddSphere( j1FlatWorld, 0.025f, ae::Color::Magenta(), 8 );
		debugLines->AddSphere( j1FlatWorldClipped, 0.025f, ae::Color::Magenta(), 8 );
		debugLines->AddLine( j1FlatWorld, j1FlatWorldClipped, ae::Color::Magenta() );
		debugLines->AddLine( j0.GetTranslation(), ( j0 * ae::Vec4( Build3D( ha, va, pa, q[ 0 ], 0, b01Len ), 1 ) ).GetXYZ(), ae::Color::Magenta() );
		debugLines->AddLine( j0.GetTranslation(), ( j0 * ae::Vec4( Build3D( ha, va, pa, 0, q[ 1 ], b01Len ), 1 ) ).GetXYZ(), ae::Color::Magenta() );
		debugLines->AddLine( j0.GetTranslation(), ( j0 * ae::Vec4( Build3D( ha, va, pa, q[ 2 ], 0, b01Len ), 1 ) ).GetXYZ(), ae::Color::Magenta() );
		debugLines->AddLine( j0.GetTranslation(), ( j0 * ae::Vec4( Build3D( ha, va, pa, 0, q[ 3 ], b01Len ), 1 ) ).GetXYZ(), ae::Color::Magenta() );
		for ( uint32_t i = 0; i < 16; i++ )
		{
			const ae::Vec3 q0 = Build3D( ha, va, pa, q[ 0 ], q[ 1 ], 1 ); // +x +y
			const ae::Vec3 q1 = Build3D( ha, va, pa, q[ 2 ], q[ 1 ], 1 ); // -x +y
			const ae::Vec3 q2 = Build3D( ha, va, pa, q[ 2 ], q[ 3 ], 1 ); // -x -y
			const ae::Vec3 q3 = Build3D( ha, va, pa, q[ 0 ], q[ 3 ], 1 ); // +x -y
			const float step = ( ae::HalfPi / 16 );
			const float angle = i * step;
			const ae::Vec3 l0 = Build3D( ha, va, pa, ae::Cos( angle ), ae::Sin( angle ), b01Len, false );
			const ae::Vec3 l1 = Build3D( ha, va, pa, ae::Cos( angle + step ), ae::Sin( angle + step ), b01Len, false );
			const ae::Vec4 p0 = j0 * ae::Vec4( l0 * q0, 1.0f );
			const ae::Vec4 p1 = j0 * ae::Vec4( l1 * q0, 1.0f );
			const ae::Vec4 p2 = j0 * ae::Vec4( l0 * q1, 1.0f );
			const ae::Vec4 p3 = j0 * ae::Vec4( l1 * q1, 1.0f );
			const ae::Vec4 p4 = j0 * ae::Vec4( l0 * q2, 1.0f );
			const ae::Vec4 p5 = j0 * ae::Vec4( l1 * q2, 1.0f );
			const ae::Vec4 p6 = j0 * ae::Vec4( l0 * q3, 1.0f );
			const ae::Vec4 p7 = j0 * ae::Vec4( l1 * q3, 1.0f );
			debugLines->AddLine( p0.GetXYZ(), p1.GetXYZ(), ae::Color::Magenta() );
			debugLines->AddLine( p2.GetXYZ(), p3.GetXYZ(), ae::Color::Magenta() );
			debugLines->AddLine( p4.GetXYZ(), p5.GetXYZ(), ae::Color::Magenta() );
			debugLines->AddLine( p6.GetXYZ(), p7.GetXYZ(), ae::Color::Magenta() );
		}
	}

	return resultWorld;
};

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
	const ae::Matrix4 testJoint0Bind = ae::Matrix4::Translation( 0.0f, 0.0f, 0.0f ) * ae::Matrix4::Scaling( 0.2f );
	const ae::Matrix4 testJoint1Bind = ae::Matrix4::Translation( 0.0f, 0.0f, 2.0f ) * ae::Matrix4::Scaling( 0.2f );
	ae::Matrix4 targetTransform, testJoint0, testJoint1;
	auto SetDefault = [&]()
	{
		targetTransform = skin.GetBindPose().GetBoneByName( rightHandBoneName )->transform;
		testJoint0 = testJoint0Bind;
		testJoint1 = testJoint1Bind;
		currentPose.Initialize( &skin.GetBindPose() );
		currentPose.SetTransform( currentPose.GetRoot(), ae::Matrix4::Translation( ae::Vec3( 0.0f, 0.0f, 1.0f ) ) );
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
	static uint32_t s_selectedJointIndex = 4;
	static ae::Axis s_ha = ae::Axis::NegativeZ;
	static ae::Axis s_va = ae::Axis::NegativeY;
	static ae::Axis s_pa = ae::Axis::X;
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
	float angleLimit[ 4 ] =//{ ae::Pi * 0.125f, ae::Pi * 0.125f, ae::Pi * 0.125f, ae::Pi * 0.125f };
	{
		0.14f,
		0.52f,
		0.38f,
		0.24f
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

			const char* jointNames[] = { "None", "Target", "0", "1" };
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
			const char* axisNames[] = { "None", "X", "Y", "Z", "NegativeX", "NegativeY", "NegativeZ" };
			ImGui::ListBox( "Horizontal", (int*)&s_ha, axisNames, countof(axisNames), 3 );
			ImGui::ListBox( "Vertical", (int*)&s_va, axisNames, countof(axisNames), 3 );
			ImGui::ListBox( "Primary", (int*)&s_pa, axisNames, countof(axisNames), 3 );
			ImGui::SliderFloat( "T0", &angleLimit[ 0 ], 0.0f, ae::HalfPi );
			ImGui::SliderFloat( "T1", &angleLimit[ 1 ], 0.0f, ae::HalfPi );
			ImGui::SliderFloat( "T2", &angleLimit[ 2 ], 0.0f, ae::HalfPi );
			ImGui::SliderFloat( "T3", &angleLimit[ 3 ], 0.0f, ae::HalfPi );
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
				if( const ae::Bone* parentBone = childBone->parent )
				{
					const ae::Matrix4& childTransform = childBone->transform;
					ClipJoint(
						skin.GetBindPose().GetBoneByIndex( parentBone->index )->transform,
						skin.GetBindPose().GetBoneByIndex( childBone->index )->transform,
						parentBone->transform,
						parentBone->inverseTransform,
						childTransform,
						s_ha,
						s_va,
						s_pa,
						angleLimit,
						&debugLines
					);
				}
			}
		}

		const ae::Vec3 jointClipped = ClipJoint(
			testJoint0Bind,
			testJoint1Bind,
			testJoint0,
			testJoint0.GetInverse(),
			testJoint1,
			s_ha,
			s_va,
			s_pa,
			angleLimit,
			&debugLines
		);

		// Joint limits
		debugLines.AddOBB( testJoint0, ae::Color::PicoBlue() );
		debugLines.AddOBB( testJoint1, ae::Color::PicoBlue() );
		debugLines.AddLine( testJoint0.GetTranslation(), jointClipped, ae::Color::PicoBlue() );

		// Add grid
		gridLines.AddLine( ae::Vec3( -2, 0, 0 ), ae::Vec3( 2, 0, 0 ), ae::Color::Red() );
		gridLines.AddLine( ae::Vec3( 0, -2, 0 ), ae::Vec3( 0, 2, 0 ), ae::Color::Green() );
		for ( float i = -2; i <= 2.00001f; i += 0.2f )
		{
			if ( ae::Abs( i ) < 0.0001f ) { continue; }
			gridLines.AddLine( ae::Vec3( i, -2, 0 ), ae::Vec3( i, 2, 0 ), ae::Color::PicoLightGray() );
			gridLines.AddLine( ae::Vec3( -2, i, 0 ), ae::Vec3( 2, i, 0 ), ae::Color::PicoLightGray() );
		}

		// Start frame
		const ae::Matrix4 worldToView = ae::Matrix4::WorldToView( camera.GetPosition(), camera.GetForward(), camera.GetLocalUp() );
		const ae::Matrix4 viewToProj = ae::Matrix4::ViewToProjection( 0.9f, render.GetAspectRatio(), 0.25f, 50.0f );
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
