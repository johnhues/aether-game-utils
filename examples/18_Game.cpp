//------------------------------------------------------------------------------
//! 18_Game.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2025 John Hughes
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and /or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "18_SmallEngine.h"
#include "ae/MeshEditorPlugin.h"

//------------------------------------------------------------------------------
// Player
//------------------------------------------------------------------------------
class Player : public ae::Inheritor< Component, Player >
{
public:
	void Initialize( class SmallEngine* engine ) override;
	void Update( class SmallEngine* engine ) override;

	bool enabled = true;
	ae::Matrix4 transform = ae::Matrix4::Identity(); // Initial spawn transform
	ae::Vec3 position = ae::Vec3( 0.0f );
	ae::Vec3 velocity = ae::Vec3( 0.0f );
	float yaw = 0.0f;
	float pitch = 0.0f;
	uint32_t lookTouchId = 0;
	uint32_t moveTouchId = 0;
};

//------------------------------------------------------------------------------
// Mesh
//------------------------------------------------------------------------------
class Mesh : public ae::Inheritor< Component, Mesh >
{
public:
	void Initialize( class SmallEngine* engine ) override;
	void Render( class SmallEngine* engine ) override;
	const SmallEngine::MeshResource* meshResource = nullptr; // Set in Initialize()

	ae::Str128 name;
	ae::Matrix4 transform = ae::Matrix4::Identity();
};

//------------------------------------------------------------------------------
// PointLight
//------------------------------------------------------------------------------
class PointLight : public ae::Inheritor< Component, PointLight >
{
public:
	void Update( class SmallEngine* engine ) override;

	ae::Matrix4 transform = ae::Matrix4::Identity();
	float r = 1.0f;
	float g = 1.0f;
	float b = 1.0f;
	float intensity = 1.0f;
};

//------------------------------------------------------------------------------
// Player
//------------------------------------------------------------------------------
AE_REGISTER_CLASS( Player );
AE_REGISTER_NAMESPACECLASS_ATTRIBUTE( (Player), (ae, EditorTypeAttribute), {} );
AE_REGISTER_CLASS_VAR( Player, enabled );
AE_REGISTER_CLASS_VAR( Player, transform );

void Player::Initialize( SmallEngine* engine )
{
	const ae::Vec4 forward = transform.GetColumn( 1 );
	position = transform.GetTranslation();
	yaw = atan2f( forward.y, forward.x );
}

void Player::Update( SmallEngine* engine )
{
	if( !enabled )
	{
		return;
	}
	const float dt = engine->timeStep.GetDt();
	ae::Input& input = engine->input;
	ae::Window& window = engine->window;

	// Camera
	const ae::Array< ae::Touch, ae::kMaxTouches > newTouches = input.GetNewTouches();
	const float displaySize = ae::Min( window.GetWidth(), window.GetHeight() );
	const ae::Vec3 forward( cosf( yaw ) * cosf( pitch ), sinf( yaw ) * cosf( pitch ), sinf( pitch ) );
	const ae::Vec3 right( forward.y, -forward.x, 0.0f );
	const ae::Touch* lookTouch = input.GetTouchById( lookTouchId );
	const ae::Touch* moveTouch = input.GetTouchById( moveTouchId );
	if( input.GetMouseCaptured() ) { yaw -= input.mouse.movement.x * 0.001f; pitch += input.mouse.movement.y * 0.001f; }
	if( !lookTouch ){ const int32_t idx = newTouches.FindFn( [&]( const ae::Touch& t ){ return moveTouch || ( t.startPosition.x >= window.GetWidth() / 2.0f ); } ); if( idx >= 0 ) { lookTouchId = newTouches[ idx ].id; } }
	yaw -= input.gamepads[ 0 ].rightAnalog.x * 2.0f * dt;
	pitch += input.gamepads[ 0 ].rightAnalog.y * 2.0f * dt;
	if( lookTouch )
	{
		const ae::Vec2 touchDir = ae::Vec2( lookTouch->movement ) / ( displaySize * 0.35f );
		yaw -= touchDir.x;
		pitch += touchDir.y;
	}
	pitch = ae::Clip( pitch, -1.0f, 1.0f );

	// Movement input
	if( !moveTouch ){ const int32_t idx = newTouches.FindFn( [&]( const ae::Touch& t ){ return t.startPosition.x < window.GetWidth() / 2.0f; } ); if( idx >= 0 ) { moveTouchId = newTouches[ idx ].id; } }
	ae::Vec3 dir = ae::Vec3( 0.0f );
	dir += ( forward * ( input.Get( ae::Key::W ) - input.Get( ae::Key::S ) ) + right * ( input.Get( ae::Key::D ) - input.Get( ae::Key::A ) ) ).SafeNormalizeCopy() * ( input.Get( ae::Key::LeftShift ) ? 0.333f : 1.0f );
	dir += ( forward * input.gamepads[ 0 ].leftAnalog.y - right * input.gamepads[ 0 ].leftAnalog.x );
	if( moveTouch )
	{
		const ae::Vec2 touchDir = ( ae::Vec2( moveTouch->position - moveTouch->startPosition ) / ( displaySize * 0.15f ) ).TrimCopy( 1.0f );
		dir += ( forward * touchDir.y - right * touchDir.x );
	}

	// Physics
	velocity += ae::Vec3( dir.x, dir.y, 0.0f ).TrimCopy( 1.0f ) * dt * 15.0f;
	velocity.SetXY( ae::DtSlerp( velocity.GetXY(), 2.5f, dt, ae::Vec2( 0.0f ) ) );
	position += velocity * dt;
	ae::RaycastResult raycastResult;
	engine->registry.CallFn< Mesh >( [ & ]( Mesh* m ) { raycastResult = m->meshResource ? m->meshResource->collision.Raycast( ae::RaycastParams{ .transform = m->transform, .source = position, .ray = ae::Vec3( 0.0f, 0.0f, -0.9f ) }, raycastResult ) : raycastResult; } );
	if( raycastResult.hits.Length() )
	{
		position.z = raycastResult.hits[ 0 ].position.z + 0.8f;
		velocity.z = ae::Max( 0.0f, velocity.z );
	}
	else
	{
		velocity.z -= dt * 10.0f;
	}
	ae::PushOutInfo pushOutInfo = { .sphere = ae::Sphere( position, 0.6f ), .velocity = velocity };
	engine->registry.CallFn< Mesh >( [ & ]( Mesh* m ) { pushOutInfo = m->meshResource ? m->meshResource->collision.PushOut( ae::PushOutParams{ .transform = m->transform }, pushOutInfo ) : pushOutInfo; } );
	position = pushOutInfo.sphere.center;

	// Update engine camera
	engine->cameraPos = position;
	engine->cameraPos.z += 0.6f;
	engine->cameraPos.z += 0.015f * cos( ae::GetTime() * 15.0 ) * ae::Clip( velocity.GetXY().Length() - velocity.z, 0.0f, 2.0f ) + ae::Max( 0.0f, velocity.z );
	engine->cameraDir = forward;
}

//------------------------------------------------------------------------------
// Mesh
//------------------------------------------------------------------------------
AE_REGISTER_CLASS( Mesh );
AE_REGISTER_NAMESPACECLASS_ATTRIBUTE( (Mesh), (ae, EditorTypeAttribute), {} );
AE_REGISTER_CLASS_VAR( Mesh, name );
AE_REGISTER_NAMESPACECLASS_VAR_ATTRIBUTE( (Mesh), name, (ae, EditorMeshResourceAttribute), {} );
AE_REGISTER_CLASS_VAR( Mesh, transform );

void Mesh::Initialize( SmallEngine* engine )
{
	meshResource = engine->GetMeshResource( name.c_str() );
}

void Mesh::Render( SmallEngine* engine )
{
	if( meshResource )
	{
		ae::UniformList uniformList;
		engine->GetUniforms( &uniformList );
		uniformList.Set( "u_tex", &engine->defaultTexture );
		uniformList.Set( "u_color", ae::Color::White().GetLinearRGBA() );
		uniformList.Set( "u_modelToProj", engine->worldToProj * transform );
		uniformList.Set( "u_modelToWorld", transform );
		uniformList.Set( "u_normalToWorld", transform.GetNormalMatrix() );
		meshResource->vertexData.Bind( &engine->meshShader, uniformList );
		meshResource->vertexData.Draw();
	}
}

//------------------------------------------------------------------------------
// PointLight
//------------------------------------------------------------------------------
AE_REGISTER_CLASS( PointLight );
AE_REGISTER_NAMESPACECLASS_ATTRIBUTE( (PointLight), (ae, EditorTypeAttribute), {} );
AE_REGISTER_CLASS_VAR( PointLight, transform );
AE_REGISTER_CLASS_VAR( PointLight, r );
AE_REGISTER_CLASS_VAR( PointLight, g );
AE_REGISTER_CLASS_VAR( PointLight, b );
AE_REGISTER_CLASS_VAR( PointLight, intensity );

void PointLight::Update( SmallEngine* engine )
{
	engine->light.position = transform.GetTranslation();
	engine->light.color = ae::Color::RGB( r, g, b );
	engine->light.intensity = intensity;
}

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main( int argc, char* argv[] )
{
	SmallEngine engine;
	if( engine.Initialize( argc, argv ) )
	{
		ae::Str256 levelPath;
		if( engine.fs.GetRootDir( ae::FileSystem::Root::Data, &levelPath ) )
		{
			ae::FileSystem::AppendToPath( &levelPath, "example.level" );
			engine.editor.QueueRead( levelPath.c_str() );
		}
		engine.Run();
	}
	return 0;
}
