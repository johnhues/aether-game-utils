//------------------------------------------------------------------------------
//! 18_SmallEngine.cpp
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
// Component
//------------------------------------------------------------------------------
AE_REGISTER_CLASS( Component );

//------------------------------------------------------------------------------
// Resources
//------------------------------------------------------------------------------
extern const char* kVertShader;
extern const char* kFragShader;
bool LoadObj( const char* fileName, const ae::FileSystem* fs, ae::VertexBuffer* vertexDataOut, ae::CollisionMesh<>* collisionOut, ae::EditorMesh* editorMeshOut );
void LoadTarga( const char* fileName, const ae::FileSystem* fs, ae::Texture2D* tex );

//------------------------------------------------------------------------------
// SmallEngineEditorPlugin
//------------------------------------------------------------------------------
class SmallEngineEditorPlugin : public ae::MeshEditorPlugin
{
public:
	SmallEngineEditorPlugin( SmallEngine* engine ) : ae::MeshEditorPlugin( TAG_SMALL_ENGINE ), m_engine( engine ) {}
	void OnEvent( const ae::EditorEvent& event ) override;
	ae::Optional< ae::EditorMesh > TryLoad( const char* resourceStr ) override;
private:
	SmallEngine* m_engine = nullptr;
};

//------------------------------------------------------------------------------
// Engine member functions
//------------------------------------------------------------------------------
SmallEngine::~SmallEngine()
{
	for( auto& pair : meshResources )
	{
		ae::Delete( pair.value );
	}
	meshResources.Clear();
}

bool SmallEngine::Initialize( int argc, char* argv[] )
{
	fs.Initialize( "data", "ae", "editor" );
	
	ae::EditorParams editorParams( argc, argv, &registry );
	// editorParams.run = true;
	editor.AddPlugin< SmallEngineEditorPlugin >( this );
	if( editor.Initialize( editorParams ) )
	{
		return false; // Exit, the editor has forked, ran, closed, and returned gracefully
	}
	window.Initialize( 1280, 720, false, true, true );
	window.SetTitle( "Press '~' to Open the Editor" );
	input.Initialize( &window );
	gfx.Initialize( &window );
	timeStep.SetTimeStep( 1.0f / 60.0f );
	debugLines.Initialize( 10 * 1024 );

	// @TODO: Move to Game.cpp
	LoadTarga( "level.tga", &fs, &defaultTexture );
	meshShader.Initialize( kVertShader, kFragShader, nullptr, 0 );
	meshShader.SetDepthTest( true );
	meshShader.SetDepthWrite( true );
	meshShader.SetCulling( ae::Culling::CounterclockwiseFront );

	return true;
}

void SmallEngine::Run()
{
	while( !input.quit )
	{
		// Update
		input.Pump();
		editor.Update();
		if( input.GetMousePressLeft() ) { input.SetMouseCaptured( true ); }
		if( input.GetPress( ae::Key::F ) ) { window.SetFullScreen( !window.GetFullScreen() ); input.SetMouseCaptured( window.GetFullScreen() ); }
		if( input.GetPress( ae::Key::Escape ) ) { input.SetMouseCaptured( false ); window.SetFullScreen( false ); }
		if( input.Get( ae::Key::Tilde ) && !input.GetPrev( ae::Key::Tilde ) ) { editor.Launch(); }
		registry.CallFn< Component >( [&]( Component* component )
		{
			if( !component->initialized )
			{
				component->Initialize( this );
				component->initialized = true;
			}
		} );
		registry.CallFn< Component >( [&]( Component* c ){ c->Update( this ); } );
		
		// Render
		gfx.Activate();
		gfx.Clear( skyColor );
		worldToView = ae::Matrix4::WorldToView( cameraPos, cameraDir, ae::Vec3( 0.0f, 0.0f, 1.0f ) );
		viewToProj = ae::Matrix4::ViewToProjection( 1.1f, gfx.GetAspectRatio(), 0.1f, 500.0f );
		worldToProj = viewToProj * worldToView;
		registry.CallFn< Component >( [&]( Component* c ){ c->Render( this ); } );
		debugLines.Render( worldToProj );
		gfx.Present();
		timeStep.Tick();
	}
}

const SmallEngine::MeshResource* SmallEngine::GetMeshResource( const char* name )
{
	const int32_t idx = meshResources.GetIndex( name );
	if( idx >= 0 )
	{
		return meshResources.GetValue( idx );
	}
	MeshResource* newResource = ae::New< MeshResource >( TAG_SMALL_ENGINE );
	if( LoadObj( name, &fs, &newResource->vertexData, &newResource->collision, nullptr ) )
	{
		meshResources.Set( name, newResource );
		return newResource;
	}
	ae::Delete( newResource );
	return nullptr;
}

void SmallEngine::GetUniforms( ae::UniformList* uniformList )
{
	uniformList->Set( "u_ambientLight", skyColor.GetLinearRGB() );
	
	uniformList->Set( "u_directionalLightColor", ae::Color::PicoPeach().ScaleRGB( 1.0f ).GetLinearRGB() );
	uniformList->Set( "u_directionalLightDir", ae::Vec3( -7.0f, 5.0f, -3.0f ).NormalizeCopy() );
	
	uniformList->Set( "u_pointLightPosition", light.position );
	uniformList->Set( "u_pointLightColor", ae::Vec4( light.color.GetLinearRGB(), light.intensity ) );
	
	uniformList->Set( "u_cameraPos", cameraPos );
}

//------------------------------------------------------------------------------
// SmallEngineEditorPlugin member functions
//------------------------------------------------------------------------------
void SmallEngineEditorPlugin::OnEvent( const ae::EditorEvent& event )
{
	ae::MeshEditorPlugin::OnEvent( event );
	if( event.type == ae::EditorEventType::LevelUnload )
	{
		m_engine->registry.CallFn< Component >( [&]( Component* c ){ c->Terminate( m_engine ); } );
		m_engine->registry.Clear();
	}
}

ae::Optional< ae::EditorMesh > SmallEngineEditorPlugin::TryLoad( const char* resourceStr )
{
	ae::EditorMesh result = TAG_SMALL_ENGINE;
	LoadObj( resourceStr, &m_engine->fs, nullptr, nullptr, &result );
	return result;
}

//------------------------------------------------------------------------------
// Resources
//------------------------------------------------------------------------------
const char* kVertShader = R"(
	AE_UNIFORM mat4 u_modelToProj;
	AE_UNIFORM mat4 u_modelToWorld;
	AE_UNIFORM mat4 u_normalToWorld;
	AE_UNIFORM vec4 u_color;
	AE_IN_HIGHP vec4 a_position;
	AE_IN_HIGHP vec4 a_normal;
	AE_IN_HIGHP vec2 a_uv;
	AE_IN_HIGHP vec4 a_color;
	AE_OUT_HIGHP vec3 v_worldPos;
	AE_OUT_HIGHP vec3 v_normal;
	AE_OUT_HIGHP vec2 v_uv;
	AE_OUT_HIGHP vec4 v_color;
	void main()
	{
		v_worldPos = ( u_modelToWorld * a_position ).xyz;
		v_normal = ( u_normalToWorld * a_normal ).xyz;
		v_uv = a_uv;
		v_color = a_color * u_color;
		gl_Position = u_modelToProj * a_position;
	})";

const char* kFragShader = R"(
	// Fragment shader implementing a simple PBR-like lighting model with a single
	// directional light and ambient term. Uses GGX microfacet BRDF (NDF + geometry +
	// Fresnel) for specular, and a Lambertian diffuse term.

	// Global uniforms
	AE_UNIFORM vec3 u_ambientLight;                // Ambient light color (linear)
	
	AE_UNIFORM vec3 u_directionalLightColor;      // Directional light color (linear)
	AE_UNIFORM vec3 u_directionalLightDir;        // Directional light direction (points from surface toward light)
	
	AE_UNIFORM vec3 u_pointLightPosition;         // Point light world position
	AE_UNIFORM vec4 u_pointLightColor;            // Point light color + intensity
	
	AE_UNIFORM vec3 u_cameraPos;                  // Camera/world-space position
	uniform sampler2D u_tex;                      // Diffuse / albedo texture
	
	// Interpolated inputs from vertex shader
	AE_IN_HIGHP vec3 v_normal;                    // World-space normal
	AE_IN_HIGHP vec2 v_uv;                        // UV coordinates
	AE_IN_HIGHP vec4 v_color;                     // Vertex color (rgba)
	AE_IN_HIGHP vec3 v_worldPos;                  // World-space position of fragment

	// Normal Distribution Function (GGX / Trowbridge-Reitz)
	// Returns the microfacet normal distribution for given N,H and roughness.
	float DistributionGGX(vec3 N, vec3 H, float roughness)
	{
		float a = roughness * roughness;
		float a2 = a * a;
		float NdotH = max(dot(N, H), 0.0);
		float NdotH2 = NdotH * NdotH;
		float denom = (NdotH2 * (a2 - 1.0) + 1.0);
		denom = 3.14159265359 * denom * denom; //  PI * denom^2
		return a2 / denom;
	}

	// Schlick-GGX approximation for the geometry term (per direction)
	float GeometrySchlickGGX(float NdotV, float roughness)
	{
		float r = (roughness + 1.0);
		float k = (r * r) / 8.0;
		return NdotV / (NdotV * (1.0 - k) + k);
	}

	// Smith's method to combine geometry terms for both view and light directions
	float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
	{
		float NdotV = max(dot(N, V), 0.0);
		float NdotL = max(dot(N, L), 0.0);
		return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
	}

	// Fresnel Schlick approximation: returns reflectance at grazing angle
	vec3 FresnelSchlick(float cosTheta, vec3 F0)
	{
		return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
	}

	// Calculate directional light contribution
	vec3 CalculateDirectionalLight(vec3 N, vec3 V, vec3 lightDir, vec3 lightColor, vec3 albedo, float roughness, float metallic, vec3 F0)
	{
		vec3 L = -lightDir;                  // light direction (towards light)
		vec3 H = normalize(V + L);           // half-vector between V and L

		// Compute specular BRDF components (GGX)
		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

		// Specular numerator and denominator (microfacet specular term)
		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
		vec3 specular = numerator / denominator;

		// kS is the specular reflectance, kD is the diffuse portion (energy conservation)
		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic; // metals have no Lambertian diffuse

		// Compute final lighting contribution from the directional light
		float NdotL = max(dot(N, L), 0.0);
		vec3 radiance = lightColor; // directional light radiance
		// Diffuse term (Lambert / PI) + specular
		return (kD * albedo / 3.14159265359 + specular) * radiance * NdotL;
	}

	// Calculate point light contribution
	vec3 CalculatePointLight(vec3 N, vec3 V, vec3 lightPos, vec3 lightColor, float lightIntensity, vec3 worldPos, vec3 albedo, float roughness, float metallic, vec3 F0)
	{
		vec3 L = normalize(lightPos - worldPos);  // light direction (towards light)
		vec3 H = normalize(V + L);                // half-vector between V and L
		float distance = length(lightPos - worldPos);
		float attenuation = lightIntensity / (distance * distance); // inverse square falloff

		// Compute specular BRDF components (GGX)
		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

		// Specular numerator and denominator (microfacet specular term)
		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
		vec3 specular = numerator / denominator;

		// kS is the specular reflectance, kD is the diffuse portion (energy conservation)
		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic; // metals have no Lambertian diffuse

		// Compute final lighting contribution from the point light
		float NdotL = max(dot(N, L), 0.0);
		vec3 radiance = lightColor * attenuation; // point light radiance with attenuation
		// Diffuse term (Lambert / PI) + specular
		return (kD * albedo / 3.14159265359 + specular) * radiance * NdotL;
	}

	void main()
	{
		// Normalize interpolated normal and compute view direction
		vec3 N = normalize(v_normal);
		vec3 V = normalize(u_cameraPos - v_worldPos);     // view direction (towards camera)

		// Material parameters (could be uniforms or textures)
		float roughness = 0.5;    // [0..1] surface roughness
		float metallic = 0.0;     // [0..1] metallic factor
		// Sample albedo texture and modulate by vertex color; tiling applied (v_uv * 10.0)
		vec3 albedo = v_color.rgb;// * AE_TEXTURE2D(u_tex, v_uv * 10.0).rgb;

		// Base reflectivity at normal incidence for dielectrics ~= 0.04
		vec3 F0 = vec3(0.04);
		// Metals use albedo as F0; non-metals keep low F0
		F0 = mix(F0, albedo, metallic);

		// Calculate directional light contribution
		vec3 Lo = CalculateDirectionalLight(N, V, u_directionalLightDir, u_directionalLightColor, albedo, roughness, metallic, F0);

		// Calculate point light contribution
		Lo += CalculatePointLight(N, V, u_pointLightPosition, u_pointLightColor.rgb, u_pointLightColor.a, v_worldPos, albedo, roughness, metallic, F0);

		// Ambient term (simple ambient multiply by albedo)
		vec3 ambient = u_ambientLight * albedo;

		// Final color = ambient + direct lighting
		vec3 color = ambient + Lo;

		// Output final color and preserve vertex alpha
		AE_COLOR.rgb = color;
		AE_COLOR.a = v_color.a;
	})";

bool LoadObj( const char* fileName, const ae::FileSystem* fs, ae::VertexBuffer* vertexDataOut, ae::CollisionMesh<>* collisionOut, ae::EditorMesh* editorMeshOut )
{
	ae::OBJLoader objFile = TAG_SMALL_ENGINE;
	const uint32_t fileSize = fs->GetSize( ae::FileSystem::Root::Data, fileName );
	ae::Scratch< uint8_t > fileBuffer( fileSize );
	fs->Read( ae::FileSystem::Root::Data, fileName, fileBuffer.Data(), fileBuffer.Length() );
	objFile.Load( { fileBuffer.Data(), fileBuffer.Length() } );
	if( !objFile.vertices.Length() )
	{
		return false;
	}
	
	if( vertexDataOut )
	{
		vertexDataOut->Initialize(
			sizeof(*objFile.vertices.Data()), sizeof(*objFile.indices.Data()),
			objFile.vertices.Length(), objFile.indices.Length(),
			ae::Vertex::Primitive::Triangle,
			ae::Vertex::Usage::Static, ae::Vertex::Usage::Static
		);
		vertexDataOut->AddAttribute( "a_position", 4, ae::Vertex::Type::Float, offsetof( ae::OBJLoader::Vertex, position ) );
		vertexDataOut->AddAttribute( "a_normal", 4, ae::Vertex::Type::Float, offsetof( ae::OBJLoader::Vertex, normal ) );
		vertexDataOut->AddAttribute( "a_uv", 2, ae::Vertex::Type::Float, offsetof( ae::OBJLoader::Vertex, texture ) );
		vertexDataOut->AddAttribute( "a_color", 4, ae::Vertex::Type::Float, offsetof( ae::OBJLoader::Vertex, color ) );
		vertexDataOut->UploadVertices( 0, objFile.vertices.Data(), objFile.vertices.Length() );
		vertexDataOut->UploadIndices( 0, objFile.indices.Data(), objFile.indices.Length() );
	}

	objFile.InitializeCollisionMesh( collisionOut );
	
	if( editorMeshOut )
	{
		editorMeshOut->verts.Reserve( objFile.vertices.Length() );
		for( uint32_t i = 0; i < objFile.vertices.Length(); i++ )
		{
			editorMeshOut->verts.Append( objFile.vertices[ i ].position.GetXYZ() );
		}
		editorMeshOut->indices.AppendArray( objFile.indices.Data(), objFile.indices.Length() );
	}
	return true;
}

void LoadTarga( const char* fileName, const ae::FileSystem* fs, ae::Texture2D* tex )
{
	ae::TargaFile tgaFile = TAG_SMALL_ENGINE;
	const uint32_t fileSize = fs->GetSize( ae::FileSystem::Root::Data, fileName );
	ae::Scratch< uint8_t > fileBuffer( fileSize );
	fs->Read( ae::FileSystem::Root::Data, fileName, fileBuffer.Data(), fileBuffer.Length() );
	tgaFile.Load( fileBuffer.Data(), fileBuffer.Length() );
	tex->Initialize( tgaFile.textureParams );
}
