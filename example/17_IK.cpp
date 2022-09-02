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
	ae::DebugCamera camera;
	ae::DebugLines debugLines;
	aeImGui ui;

	window.Initialize( 800, 600, false, true );
	window.SetTitle( "17_IK" );
	render.Initialize( &window );
	input.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	fileSystem.Initialize( "data", "ae", "ik" );
	camera.Initialize( ae::Axis::Y, ae::Vec3( 0.0f, 1.0f, 0.0f ), ae::Vec3( 0.0f, 0.4f, 3.5f ) );
	camera.SetDistanceLimits( 1.0f, 25.0f );
	debugLines.Initialize( 4096 );
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
	ae::VertexArray vertexData;
	{
		const char* fileName = "character.fbx";
		uint32_t fileSize = fileSystem.GetSize( ae::FileSystem::Root::Data, fileName );
		AE_ASSERT_MSG( fileSize, "Could not load '#'", fileName );
		ae::Scratch< uint8_t > fileData( fileSize );
		fileSystem.Read( ae::FileSystem::Root::Data, fileName, fileData.Data(), fileData.Length() );
		
		ae::VertexLoaderHelper vertexInfo;
		vertexInfo.size = sizeof(Vertex);
		vertexInfo.posOffset = offsetof( Vertex, pos );
		vertexInfo.normalOffset = offsetof( Vertex, normal );
		vertexInfo.colorOffset = offsetof( Vertex, color );
		vertexInfo.uvOffset = offsetof( Vertex, uv );
		ae::ofbxLoadSkinnedMesh( TAG_ALL, fileData.Data(), fileData.Length(), vertexInfo, &vertexData, &skin, nullptr );
	}
	
	double animTime = 0.0;
	const char* handBoneName = "QuickRigCharacter_RightHand";
	const char* armBoneName = "QuickRigCharacter_RightArm";
	ae::Matrix4 targetTransform = skin.GetBindPose()->GetBoneByName( handBoneName )->transform;
	ImGuizmo::OPERATION gizmoOperation = ImGuizmo::TRANSLATE;
	
	AE_INFO( "Run" );
	while ( !input.quit )
	{
		float dt = ae::Max( timeStep.GetTimeStep(), timeStep.GetDt() );
		input.Pump();
		
		if ( input.Get( ae::Key::R ) )
		{
			targetTransform = ae::Matrix4::Translation( skin.GetBindPose()->GetBoneByName( handBoneName )->transform.GetTranslation() );
		}
		if ( input.Get( ae::Key::W ) )
		{
			gizmoOperation = ImGuizmo::TRANSLATE;
		}
		if ( input.Get( ae::Key::E ) )
		{
			gizmoOperation = ImGuizmo::ROTATE;
		}
		
		ImGuiIO& io = ImGui::GetIO();
		ui.NewFrame( &render, &input, dt );
		ImGuizmo::SetOrthographic( false );
		ImGuizmo::SetRect( 0, 0, io.DisplaySize.x, io.DisplaySize.y );
		ImGuizmo::BeginFrame();
		
		camera.SetInputEnabled( !ImGui::GetIO().WantCaptureMouse && !ImGuizmo::IsUsing() );
		camera.Update( &input, dt );
		
		animTime += dt * 0.01;
		
		ae::Skeleton currentPose = TAG_ALL;
		{
			currentPose.Initialize( skin.GetBindPose() );

			const ae::Bone* armBone = currentPose.GetBoneByName( armBoneName );
			const ae::Vec3 targetPos = targetTransform.GetTranslation();
			ae::Array< const ae::Bone* > bones = TAG_ALL;
			ae::Array< ae::Vec3 > joints = TAG_ALL;
			ae::Array< float > jointLengths = TAG_ALL;
			for ( auto b = armBone; b; b = b->firstChild )
			{
				bones.Append( b );
				ae::Vec3 p = b->transform.GetTranslation();
				ae::Vec3 p1 = b->parent->transform.GetTranslation();
				joints.Append( p );
				jointLengths.Append( ( p1 - p ).Length() );
			}
			const ae::Vec3 rootPos = joints[ 0 ];
			const ae::Vec3 polePos = rootPos + ae::Vec3( -1.0f, 0.0f, -1.0f );
			const float targetDistance = ( targetPos - rootPos ).Length();
			float armLength = 0.0f;
			for ( float f : jointLengths ) { armLength += f; }
			
			uint32_t iterationCount = 0;
			while ( ( joints[ joints.Length() - 1 ] - targetPos ).Length() > 0.1f && iterationCount < 10 )
			{
				joints[ joints.Length() - 1 ] = targetPos;
				for ( int32_t i = joints.Length() - 2; i >= 0; i-- )
				{
					ae::Vec3 oldPos = joints[ i ];
					ae::Vec3 childPos = joints[ i + 1 ];
					float childBoneLength = jointLengths[ i + 1 ];
					ae::Vec3 childToOld = ( oldPos - childPos ).SafeNormalizeCopy() * childBoneLength;
					joints[ i ] = childPos + childToOld;
				}
				
				joints[ 0 ] = rootPos;
				for ( uint32_t i = 1; i < joints.Length(); i++ )
				{
					ae::Vec3 oldPos = joints[ i ];
					ae::Vec3 parentPos = joints[ i - 1 ];
					float boneLength = jointLengths[ i ];
					ae::Vec3 parentToOld = ( oldPos - parentPos ).SafeNormalizeCopy() * boneLength;
					joints[ i ] = parentPos + parentToOld;
				}
				
				iterationCount++;
			}
			
			ae::Array< ae::Matrix4 > finalTransforms = TAG_ALL;
			for ( uint32_t i = 0; i < joints.Length() - 1; i++ )
			{
				ae::Vec3 boneDir = joints[ i + 1 ] - joints[ i ];
				const ae::Matrix4 oldTransform = bones[ i ]->transform;
				ae::Matrix4 newTransform = ae::Matrix4::Identity();
				
				ae::Vec3 xAxis = -boneDir.SafeNormalizeCopy();
				//ae::Vec3 zAxis = xAxis.Cross( polePos - joints[ i ] );
				//ae::Vec3 yAxis = zAxis.Cross( xAxis ).SafeNormalizeCopy();
				//zAxis = xAxis.Cross( yAxis ).SafeNormalizeCopy();
				ae::Vec3 yAxis = oldTransform.GetAxis( 2 ).Cross( xAxis ).SafeNormalizeCopy();
				ae::Vec3 zAxis = xAxis.Cross( yAxis );
				yAxis = zAxis.Cross( xAxis ).SafeNormalizeCopy();
				
				newTransform.SetAxis( 0, xAxis );
				newTransform.SetAxis( 1, yAxis );
				newTransform.SetAxis( 2, zAxis );
				newTransform.SetTranslation( joints[ i ] );
				finalTransforms.Append( newTransform );
			}
			finalTransforms.Append( targetTransform ).SetTranslation( joints[ joints.Length() - 1 ] );
			AE_ASSERT( finalTransforms.Length() == bones.Length() );
			for ( const auto& t : finalTransforms )
			{
				ae::Vec3 p = t.GetTranslation();
				ae::Vec3 xAxis = t.GetAxis( 0 );
				ae::Vec3 yAxis = t.GetAxis( 1 );
				ae::Vec3 zAxis = t.GetAxis( 2 );
				debugLines.AddLine( p, p + xAxis * 0.2f, ae::Color::Red() );
				debugLines.AddLine( p, p + yAxis * 0.2f, ae::Color::Green() );
				debugLines.AddLine( p, p + zAxis * 0.2f, ae::Color::Blue() );
				debugLines.AddLine( p, polePos, ae::Color::Yellow() );
			}
			currentPose.SetTransforms( bones.Begin(), finalTransforms.Begin(), bones.Length() );
		}
		
		// Update mesh
		Vertex* meshVerts = vertexData.GetWritableVertices< Vertex >();
		skin.ApplyPoseToMesh( &currentPose, meshVerts->pos.data, meshVerts->normal.data, sizeof(Vertex), sizeof(Vertex), vertexData.GetVertexCount() );
		vertexData.Upload();
		
		// Debug
		for ( uint32_t i = 0; i < currentPose.GetBoneCount(); i++ )
		{
			const ae::Bone* bone = currentPose.GetBoneByIndex( i );
			const ae::Bone* parent = bone->parent;
			if ( parent )
			{
				debugLines.AddLine( parent->transform.GetTranslation(), bone->transform.GetTranslation(), ae::Color::Red() );
				debugLines.AddOBB( bone->transform * ae::Matrix4::Scaling( 0.05f ), ae::Color::Red() );
			}
		}
		
		// Start frame
		ae::Matrix4 worldToView = ae::Matrix4::WorldToView( camera.GetPosition(), camera.GetForward(), camera.GetLocalUp() );
		ae::Matrix4 viewToProj = ae::Matrix4::ViewToProjection( 0.9f, render.GetAspectRatio(), 0.25f, 50.0f );
		ae::Matrix4 worldToProj = viewToProj * worldToView;
		
		ImGuizmo::Manipulate(
			worldToView.data,
			viewToProj.data,
			gizmoOperation,
			ImGuizmo::WORLD,
			targetTransform.data
		);
		
		render.Activate();
		render.Clear( ae::Color::PicoDarkPurple() );
		
		// Render mesh
		ae::Matrix4 modelToWorld = ae::Matrix4::Identity();
		ae::UniformList uniformList;
		uniformList.Set( "u_worldToProj", worldToProj * modelToWorld );
		uniformList.Set( "u_normalToWorld", modelToWorld.GetNormalMatrix() );
		uniformList.Set( "u_lightDir", ae::Vec3( 0.0f, 0.0f, -1.0f ).NormalizeCopy() );
		uniformList.Set( "u_lightColor", ae::Color::PicoPeach().GetLinearRGB() );
		uniformList.Set( "u_ambColor", ae::Vec3( 0.8f ) );
		uniformList.Set( "u_color", ae::Color::White().GetLinearRGBA() );
		uniformList.Set( "u_tex", &texture );
		vertexData.Draw( &shader, uniformList );
		
		// Frame end
		debugLines.Render( worldToProj );
		ui.Render();
		render.Present();
		timeStep.Wait();
	}

	AE_INFO( "Terminate" );
	input.Terminate();
	render.Terminate();
	window.Terminate();

	return 0;
}
