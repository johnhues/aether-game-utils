//------------------------------------------------------------------------------
// 16_SkinnedMesh.cpp
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
#include "ae/aether.h"
#include "ae/loaders.h"

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
const char* kVertShader = "\
	AE_UNIFORM mat4 u_worldToProj;\
	AE_UNIFORM vec4 u_color;\
	AE_IN_HIGHP vec4 a_position;\
	AE_IN_HIGHP vec4 a_color;\
	AE_IN_HIGHP vec2 a_uv;\
	AE_OUT_HIGHP vec4 v_color;\
	AE_OUT_HIGHP vec2 v_uv;\
	void main()\
	{\
		v_color = a_color * u_color;\
		v_uv = a_uv;\
		gl_Position = u_worldToProj * a_position;\
	}";

const char* kFragShader = "\
	AE_UNIFORM sampler2D u_tex;\
	AE_IN_HIGHP vec4 v_color;\
	AE_IN_HIGHP vec2 v_uv;\
	void main()\
	{\
		AE_COLOR = AE_TEXTURE2D( u_tex, v_uv ) * v_color;\
	}";

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

	window.Initialize( 800, 600, false, true );
	window.SetTitle( "16_SkinnedMesh" );
	render.Initialize( &window );
	input.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	fileSystem.Initialize( "data", "johnhues", "16_SkinnedMesh" );
	camera.Initialize( ae::Axis::Y, ae::Vec3( 0.0f, 1.0f, 0.0f ), ae::Vec3( 0.0f, 0.4f, 3.5f ) );
	camera.SetDistanceLimits( 1.0f, 25.0f );
	debugLines.Initialize( 4096 );

	shader.Initialize( kVertShader, kFragShader, nullptr, 0 );
	shader.SetDepthTest( true );
	shader.SetDepthWrite( true );
	shader.SetBlending( true );
	shader.SetCulling( ae::Shader::Culling::CounterclockwiseFront );

	ae::Texture2D texture;
	{
		ae::TargaFile targaFile = TAG_ALL;
		uint32_t fileSize = fileSystem.GetSize( "character.tga" );
		AE_ASSERT( fileSize );
		ae::Scratch< uint8_t > fileData( TAG_ALL, fileSize );
		fileSystem.Read( "character.tga", fileData.Data(), fileData.Length() );
		targaFile.Load( fileData.Data(), fileData.Length() );
		texture.Initialize( targaFile.textureParams );
	}
	
	ae::Skin skin = TAG_ALL;
	ae::Animation anim = TAG_ALL;
	ae::VertexData vertexData;
	{
		ae_VertexLoaderInfo vertexInfo;
		vertexInfo.size = sizeof(Vertex);
		vertexInfo.posOffset = offsetof( Vertex, pos );
		vertexInfo.normalOffset = offsetof( Vertex, normal );
		vertexInfo.colorOffset = offsetof( Vertex, color );
		vertexInfo.uvOffset = offsetof( Vertex, uv );
		ae_ofbx_LoadSkinnedMesh( TAG_ALL, &fileSystem, "character.fbx", vertexInfo, &vertexData, &skin, &anim );
	}
	anim.loop = true;
	
	double animTime = 0.0;
	
	AE_INFO( "Run" );
	while ( !input.quit )
	{
		input.Pump();
		camera.Update( &input, timeStep.GetDt() );
		
		animTime += timeStep.GetDt() * 0.01;
		
		// Update skeleton
		ae::Skeleton currentPose = TAG_ALL;
		currentPose.Initialize( skin.GetBindPose() );
		
		// Settings update
		static int32_t s_strength10 = 10;
		if ( input.Get( ae::Key::Minus ) && !input.GetPrev( ae::Key::Minus ) )
		{
			s_strength10--;
		}
		if ( input.Get( ae::Key::Equals ) && !input.GetPrev( ae::Key::Equals ) )
		{
			s_strength10++;
		}
		static int32_t s_strength10Prev = -1;
		if ( s_strength10Prev != s_strength10 )
		{
			s_strength10 = ae::Clip( s_strength10, 0, 10 );
			AE_INFO( "Anim blend strength: #", s_strength10 / 10.0f );
			s_strength10Prev = s_strength10;
		}
		ae::Array< const ae::Bone* > mask = TAG_ALL;
		static int32_t s_maskCountPrev = -1;
		if ( input.Get( ae::Key::Space ) )
		{
			std::function< void( const ae::Bone* ) > maskFn = [&]( const ae::Bone* bone )
			{
				if ( bone )
				{
					mask.Append( bone );
					for ( bone = bone->firstChild; bone; bone = bone->nextSibling )
					{
						maskFn( bone );
					}
				}
			};
			mask.Append( currentPose.GetBoneByName( "QuickRigCharacter_Hips" ) );
			maskFn( currentPose.GetBoneByName( "QuickRigCharacter_LeftUpLeg" ) );
			maskFn( currentPose.GetBoneByName( "QuickRigCharacter_RightUpLeg" ) );
		}
		if ( s_maskCountPrev != mask.Length() )
		{
			AE_INFO( "Animation Mask" );
			for ( const ae::Bone* b : mask )
			{
				AE_INFO( "\t#", b->name );
			}
			if ( !mask.Length() )
			{
				AE_INFO( "\tNone" );
			}
			s_maskCountPrev = mask.Length();
		}
		
		// Update mesh
		anim.AnimateByTime( &currentPose, animTime, s_strength10 / 10.0f, mask.Begin(), mask.Length() );
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
				debugLines.AddOBB( bone->transform * ae::Matrix4::Scaling( 0.1f ), ae::Color::Red() );
			}
		}
		
		// Start frame
		ae::Matrix4 worldToView = ae::Matrix4::WorldToView( camera.GetPosition(), camera.GetForward(), camera.GetLocalUp() );
		ae::Matrix4 viewToProj = ae::Matrix4::ViewToProjection( 0.9f, render.GetAspectRatio(), 0.25f, 50.0f );
		ae::Matrix4 worldToProj = viewToProj * worldToView;
		render.Activate();
		render.Clear( ae::Color::PicoDarkPurple() );
		
		// Render mesh
		ae::UniformList uniformList;
		uniformList.Set( "u_worldToProj", worldToProj );
		uniformList.Set( "u_color", ae::Color::White().GetLinearRGBA() );
		uniformList.Set( "u_tex", &texture );
		vertexData.Render( &shader, uniformList );
		
		// Frame end
		debugLines.Render( worldToProj );
		render.Present();
		timeStep.Wait();
	}

	AE_INFO( "Terminate" );
	input.Terminate();
	render.Terminate();
	window.Terminate();

	return 0;
}
