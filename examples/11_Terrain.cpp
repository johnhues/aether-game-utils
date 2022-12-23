//------------------------------------------------------------------------------
// 11_Terrain.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2020 John Hughes
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
#include "ae/aeImGui.h"
#include "ae/aeTerrain.h"
#include "ae/SpriteRenderer.h"
#include "ImGuizmo.h"

const char* kFileName = "objects.dat";
const uint32_t kCurrentFileVersion = 3;
const uint32_t kMinSupportedFileVersion = 1;
const ae::Tag TAG_EXAMPLE = ae::Tag( "example" );

//------------------------------------------------------------------------------
// Terrain Shader
//------------------------------------------------------------------------------
const char* kTerrainVertShader = R"(
  AE_UNIFORM mat4 u_worldToProj;
  AE_IN_HIGHP vec3 a_position;
  AE_IN_HIGHP vec3 a_normal;
  AE_IN_HIGHP vec4 a_materials;
  AE_OUT_HIGHP vec3 v_normal;
  AE_OUT_HIGHP vec4 v_materials;
  void main()
  {
    v_normal = a_normal;
    v_materials = a_materials;
    gl_Position = u_worldToProj * vec4( a_position, 1.0 );
  })";

const char* kTerrainFragShader = R"(
  AE_UNIFORM vec4 u_topColor;
  AE_UNIFORM vec4 u_sideColor;
  AE_UNIFORM vec4 u_pathColor;
  AE_IN_HIGHP vec3 v_normal;
  AE_IN_HIGHP vec4 v_materials;
  void main()
  {
    float top = max( 0.0, v_normal.z );
    top *= top;
    top *= top;
    vec4 color = mix( u_sideColor, u_topColor, top ) * v_materials.r;
    color += u_pathColor * v_materials.g;
    
    float light = dot( normalize( v_normal ), normalize( vec3( 1.0 ) ) );
    light = max( 0.0, light );
    light = mix( 0.8, 4.0, light );
    
    AE_COLOR = vec4( color.rgb * vec3( light ), color.a );
  })";

//------------------------------------------------------------------------------
// Grid Shader
//------------------------------------------------------------------------------
const char* kGridVertexStr = R"(
    AE_UNIFORM_HIGHP mat4 u_screenToWorld;
    AE_IN_HIGHP vec4 a_position;
    AE_OUT_HIGHP vec3 v_worldPos;
    void main()
    {
      v_worldPos = vec3( u_screenToWorld * a_position );
      gl_Position = a_position;
    })";

const char* kGridFragStr = R"(
    AE_IN_HIGHP vec3 v_worldPos;
    void main()
    {
      int x = int( floor( v_worldPos.x ) ) % 2;
      int y = int( floor( v_worldPos.y ) ) % 2;
      AE_COLOR.rgb = mix( vec3( 0.3 ), vec3( 0.35 ), int( x != y ) );
      float gridX = mod( v_worldPos.x + 16.0, 32.0 ) - 16.0;
      float gridY = mod( v_worldPos.y + 16.0, 32.0 ) - 16.0;
      if ( abs( gridX ) < 0.05 || abs( gridY ) < 0.05 ) { AE_COLOR.rgb = vec3( 0.25 ); }
      AE_COLOR.a = 1.0;
    })";

class Grid
{
public:
  void Initialize()
  {
    struct BGVertex
    {
      ae::Vec4 pos;
    };
    
    BGVertex bgVertices[ aeQuadVertCount ];
    uint8_t bgIndices[ aeQuadIndexCount ];
    for ( uint32_t i = 0; i < aeQuadVertCount; i++ )
    {
      bgVertices[ i ].pos = ae::Vec4( aeQuadVertPos[ i ] * 2.0f, 1.0f );
    }
    for ( uint32_t i = 0; i < aeQuadIndexCount; i++ )
    {
      bgIndices[ i ] = aeQuadIndices[ i ];
    }

    m_bgVertexData.Initialize( sizeof( BGVertex ), sizeof( uint8_t ), countof( bgVertices ), countof( bgIndices ), ae::Vertex::Primitive::Triangle, ae::Vertex::Usage::Static, ae::Vertex::Usage::Static );
    m_bgVertexData.AddAttribute( "a_position", 4, ae::Vertex::Type::Float, offsetof( BGVertex, pos ) );
    m_bgVertexData.SetVertices( bgVertices, countof( bgVertices ) );
    m_bgVertexData.SetIndices( bgIndices, countof( bgIndices ) );

    m_gridShader.Initialize( kGridVertexStr, kGridFragStr, nullptr, 0 );
  }

  void Render( ae::Matrix4 worldToProj )
  {
    ae::UniformList uniforms;
    uniforms.Set( "u_screenToWorld", worldToProj.GetInverse() );
    m_bgVertexData.Draw( &m_gridShader, uniforms );
  }

private:
  ae::Shader m_gridShader;
  ae::VertexArray m_bgVertexData;
};

//------------------------------------------------------------------------------
// Objects
//------------------------------------------------------------------------------
struct Object
{
  Object() = default;
  Object( const char* n, ae::Sdf* s ) : name( n ), shape( s ) {}
  void Serialize( ae::BinaryStream* stream );
  
  ae::Str16 name = "";
  // Shape
  ae::Sdf* shape = nullptr;
  // Ray
  ae::Vec3 raySrc = ae::Vec3( 0.0f );
  ae::Vec3 rayDir = ae::Vec3( 0.0f );
  float rayLength = 0.0f;
  uint32_t rayType = 0;
};

void Object::Serialize( ae::BinaryStream* stream )
{
  stream->SerializeString( name );
  
  if ( shape )
  {
    if ( stream->IsWriter() )
    {
      stream->SerializeRaw( shape->GetTransform() );
    }
    else
    {
      ae::Matrix4 transform;
      stream->SerializeRaw( transform );
      shape->SetTransform( transform );
    }
  }
  else
  {
    stream->SerializeFloat( raySrc.x );
    stream->SerializeFloat( raySrc.y );
    stream->SerializeFloat( raySrc.z );
    stream->SerializeFloat( rayDir.x );
    stream->SerializeFloat( rayDir.y );
    stream->SerializeFloat( rayDir.z );
    stream->SerializeFloat( rayLength );
    stream->SerializeUint32( rayType );
  }
}

void WriteObjects( ae::FileSystem* fileSystem, const ae::Array< Object* >& objects )
{
  ae::BinaryStream wStream = ae::BinaryStream::Writer();
  wStream.SerializeUint32( kCurrentFileVersion );

  wStream.SerializeUint32( objects.Length() );
  for ( uint32_t i = 0; i < objects.Length(); i++ )
  {
    Object* object = objects[ i ];
    
    ae::Str16 type = "";
    if ( ae::Cast< ae::SdfBox >( object->shape ) ) { type = "box"; }
    else if ( ae::Cast< ae::SdfCylinder >( object->shape ) ) { type = "cylinder"; }
    else if ( ae::Cast< ae::SdfHeightmap >( object->shape ) ) { type = "heightmap"; }
    else { type = "ray"; }
    wStream.SerializeString( type );
    
    wStream.SerializeObject( *object );
  }
  fileSystem->Write( ae::FileSystem::Root::User, kFileName, wStream.GetData(), wStream.GetOffset(), false );
}

bool ReadObjects( ae::FileSystem* fileSystem, ae::Terrain* terrain, ae::Image* heightmapImage, ae::Array< Object* >& objects )
{
  ae::Scratch< uint8_t > scratch( fileSystem->GetSize( ae::FileSystem::Root::User, kFileName ) );
  fileSystem->Read( ae::FileSystem::Root::User, kFileName, scratch.Data(), scratch.Length() );
  ae::BinaryStream rStream = ae::BinaryStream::Reader( scratch.Data(), scratch.Length() );

  uint32_t version = 0;
  rStream.SerializeUint32( version );
  if ( version < kMinSupportedFileVersion )
  {
    return false;
  }
  
  uint32_t len = 0;
  rStream.SerializeUint32( len );
  if ( !len )
  {
    return false;
  }
  
  objects.Clear();
  for ( uint32_t i = 0; i < len; i++ )
  {
    Object* object = ae::New< Object >( TAG_EXAMPLE );
    
    ae::Str16 type = "";
    rStream.SerializeString( type );
    if ( type == "box" ) { object->shape = terrain->sdf.CreateSdf< ae::SdfBox >(); }
    else if ( type == "cylinder" ) { object->shape = terrain->sdf.CreateSdf< ae::SdfCylinder >(); }
    else if ( type == "heightmap" )
    {
      ae::SdfHeightmap* heightmap = terrain->sdf.CreateSdf< ae::SdfHeightmap >();
      heightmap->SetImage( heightmapImage );
      object->shape = heightmap;
    }
    else if ( type == "ray" ) {}
    else { AE_FAIL(); }
    
    rStream.SerializeObject( *object );
    objects.Append( object );
  }
  
  return true;
}

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
  AE_INFO( "Initialize" );

  bool headless = _AE_LINUX_;
  
  ae::FileSystem fileSystem;
  ae::Window window;
  ae::GraphicsDevice render;
  ae::Input input;
  ae::DebugLines debug;
  ae::TimeStep timeStep;
  ae::Shader terrainShader;
  ae::DebugCamera camera = ae::Axis::Z;
  ae::Texture2D fontTexture;
  ae::TextRender textRender = TAG_EXAMPLE;
  class aeImGui* ui = nullptr;

  fileSystem.Initialize( "data", "ae", "terrain" );
  ui = ae::New< aeImGui >( TAG_EXAMPLE );
  if ( headless )
  {
    ui->InitializeHeadless();
  }
  else
  {
    window.Initialize( 800, 600, false, true );
    window.SetTitle( "terrain edit" );
    render.Initialize( &window );
    debug.Initialize( 64 );
    ui->Initialize();
  }

  input.Initialize( headless ? nullptr : &window );
  timeStep.SetTimeStep( 1.0f / 60.0f );
  camera.SetDistanceFromFocus( 100.0f );
  
  if ( !headless )
  {
    terrainShader.Initialize( kTerrainVertShader, kTerrainFragShader, nullptr, 0 );
    terrainShader.SetDepthTest( true );
    terrainShader.SetDepthWrite( true );

    {
      const char* fileName = "font.tga";
      uint32_t fileSize = fileSystem.GetSize( ae::FileSystem::Root::Data, fileName );
      AE_ASSERT_MSG( fileSize, "Could not load #", fileName );
      ae::Scratch< uint8_t > fileBuffer( fileSize );
      fileSystem.Read( ae::FileSystem::Root::Data, fileName, fileBuffer.Data(), fileSize );
      ae::TargaFile targa = TAG_EXAMPLE;
      targa.Load( fileBuffer.Data(), fileSize );
      fontTexture.Initialize( targa.textureParams );
    }
    textRender.Initialize( 128, 2048, &fontTexture, 8, 1.0f );
  }

  uint32_t terrainThreads = ae::Max( 1u, (uint32_t)( ae::GetMaxConcurrentThreads() * 0.75f ) );
  ae::Terrain* terrain = ae::New< ae::Terrain >( TAG_EXAMPLE );
  terrain->Initialize( terrainThreads, !headless );

  ae::Matrix4 worldToText = ae::Matrix4::Identity();
  auto drawWorldText = [&]( ae::Vec3 p, const char* str )
  {
    p = ae::Vec3::ProjectPoint( worldToText, p );
    ae::Vec2 fontSize( (float)textRender.GetFontSize() );
    textRender.Add( p, fontSize, str, ae::Color::White(), 0, 0 );
  };

  bool wireframe = false;
  static bool s_showTerrainDebug = false;

  ae::Array< Object* > objects = TAG_EXAMPLE;
  Object* currentObject = nullptr;
//
//  if ( !ReadObjects( &fileSystem, terrain, &heightmapImage, objects ) )
//  {
//    ae::SdfBox* box = terrain->sdf.CreateSdf< ae::SdfBox >();
//    box->SetTransform( ae::Matrix4::Translation( camera.GetFocus() ) * ae::Matrix4::Scaling( ae::Vec3( 10.0f ) ) );
//    currentObject = objects.Append( ae::New< Object >( TAG_EXAMPLE, "Box", box ) );
//  }

  bool gizmoClickedPrev = false;
  ImGuizmo::OPERATION gizmoOperation = ImGuizmo::TRANSLATE;

  AE_INFO( "Run" );
  while ( !input.quit )
  {
    input.Pump();
    float dt = ae::Max( 0.00001f, timeStep.GetDt() );

    ui->NewFrame( &render, &input, dt );

    ImGuiIO& io = ImGui::GetIO();
    
    ImGuizmo::SetOrthographic( false );
    ImGuizmo::BeginFrame();

    if ( ImGui::Begin( "edit", nullptr ) )
    {
      // New terrain objects
      if ( ImGui::CollapsingHeader( "create" ) )
      {
        if ( ImGui::Button( "box" ) )
        {
          ae::SdfBox* box = terrain->sdf.CreateSdf< ae::SdfBox >();
          box->SetTransform(
            ae::Matrix4::Translation( camera.GetFocus() ) *
            ae::Matrix4::Scaling( ae::Vec3( 10.0f ) ) );

          currentObject = objects.Append( ae::New< Object >( TAG_EXAMPLE, "Box", box ) );
        }

        if ( ImGui::Button( "cylinder" ) )
        {
          ae::SdfCylinder* cylinder = terrain->sdf.CreateSdf< ae::SdfCylinder >();
          cylinder->SetTransform(
            ae::Matrix4::Translation( camera.GetFocus() ) *
            ae::Matrix4::Scaling( ae::Vec3( 10.0f ) ) );

          currentObject = objects.Append( ae::New< Object >( TAG_EXAMPLE, "Cylinder", cylinder ) );
        }
        
//        if ( ImGui::Button( "height map" ) )
//        {
//          ae::SdfHeightmap* heightMap = terrain->sdf.CreateSdf< ae::SdfHeightmap >();
//          heightMap->SetTransform(
//            ae::Matrix4::Translation( camera.GetFocus() ) *
//            ae::Matrix4::Scaling( ae::Vec3( 10.0f ) ) );
//          heightMap->SetImage( &heightmapImage );
//
//          currentObject = objects.Append( ae::New< Object >( TAG_EXAMPLE, "Height Map", heightMap ) );
//        }

        if ( ImGui::Button( "ray" ) )
        {
          currentObject = objects.Append( ae::New< Object >( TAG_EXAMPLE, "Ray", nullptr ) );
          currentObject->raySrc = camera.GetPosition();
          currentObject->rayDir = camera.GetForward();
          currentObject->rayLength = 100.0f;
        }

        // @TODO: Disabled because "material" is used in properties, and having both creates a conflict
        //if ( ImGui::Button( "material" ) )
        //{
        //  ae::Sdf::Box* box = terrain->sdf.CreateSdf< ae::Sdf::Box >();
        //  box->SetTransform(
        //    ae::Matrix4::Translation( camera.GetFocus() ) *
        //    ae::Matrix4::Scaling( ae::Vec3( 10.0f ) ) );
        //  box->type = ae::Sdf::Shape::Type::Material;
        //  box->materialId = 1;

        //  currentObject = objects.Append( ae::Allocate< Object >( "Material", box ) );
        //}
      }
      
      if ( ImGui::CollapsingHeader( "properties" ) )
      {
        if ( currentObject )
        {
          aeImGui::InputText( "name", &currentObject->name );

          if ( ae::Sdf* currentShape = currentObject->shape )
          {
            bool changed = false;

            ae::Matrix4 temp = currentShape->GetTransform();
            float matrixTranslation[ 3 ], matrixRotation[ 3 ], matrixScale[ 3 ];
            ImGuizmo::DecomposeMatrixToComponents( temp.data, matrixTranslation, matrixRotation, matrixScale );
            changed |= ImGui::InputFloat3( "translation", matrixTranslation );
            changed |= ImGui::InputFloat3( "rotation", matrixRotation );
            changed |= ImGui::InputFloat3( "scale", matrixScale );
            if ( changed )
            {
              ImGuizmo::RecomposeMatrixFromComponents( matrixTranslation, matrixRotation, matrixScale, temp.data );
              currentShape->SetTransform( temp );
            }

            const char* types[] = { "union", "subtraction", "smooth union", "smooth subtraction", "material" };
            changed |= ImGui::Combo( "type", (int*)&currentShape->type, types, countof( types ) );

            if ( currentShape->type == ae::Sdf::Type::SmoothUnion || currentShape->type == ae::Sdf::Type::SmoothSubtraction )
            {
              ae::Vec3 halfSize = currentShape->GetHalfSize();
              float maxLength = ae::Max( halfSize.x, halfSize.y, halfSize.z );
              changed |= ImGui::SliderFloat( "smoothing", &currentShape->smoothing, 0.0f, maxLength );
            }

            const char* materialNames[] = { "grass", "sand" };
            changed |= ImGui::Combo( "material", (int32_t*)&currentShape->materialId, materialNames, countof( materialNames ) );

            if ( auto box = ae::Cast< ae::SdfBox >( currentShape ) )
            {
              ae::Vec3 halfSize = box->GetHalfSize();
              float minLength = ae::Min( halfSize.x, halfSize.y, halfSize.z );
              changed |= ImGui::SliderFloat( "cornerRadius", &box->cornerRadius, 0.0f, minLength );
            }
            else if ( auto cylinder = ae::Cast< ae::SdfCylinder >( currentShape ) )
            {
              changed |= ImGui::SliderFloat( "top", &cylinder->top, 0.0f, 1.0f );
              changed |= ImGui::SliderFloat( "bottom", &cylinder->bottom, 0.0f, 1.0f );
            }

            if ( changed )
            {
              currentShape->Dirty();
            }
          }
          else
          {
            ImGui::InputFloat3( "source", currentObject->raySrc.data );
            if ( ImGui::InputFloat3( "direction", currentObject->rayDir.data ) )
            {
              currentObject->rayDir.SafeNormalize();
            }
            ImGui::InputFloat( "length", &currentObject->rayLength );

            const char* types[] = { "default", "fast", "voxel" };
            ImGui::Combo( "type", (int*)&currentObject->rayType, types, countof( types ) );
          }
        }
        else
        {
          ImGui::Text( "no selection" );
        }
      }

      if ( ImGui::CollapsingHeader( "objects" ) )
      {
        ImGui::BeginChild( "ChildL" );
        for ( uint32_t i = 0; i < objects.Length(); i++ )
        {
          Object* object = objects[ i ];
          ae::Str32 displayName( "(#) #", i, object->name );
          if ( ImGui::Selectable( displayName.c_str(), object == currentObject ) )
          {
            currentObject = object;
          }
        }
        ImGui::EndChild();
      }
    }
    ImGui::End();

    if ( !ImGuizmo::IsUsing() )
    {
      if ( s_showTerrainDebug )
      {
        terrain->SetDebugTextCallback( drawWorldText );
      }
      else
      {
        terrain->SetDebugTextCallback( nullptr );
      }

      // Wait for click release of gizmo before starting new terrain jobs
      ae::TerrainParams params;
      params.debug = s_showTerrainDebug ? &debug : nullptr;
      terrain->SetParams( params );
      terrain->Update( camera.GetFocus(), 512.0f );
    }

    // Camera input
    camera.SetInputEnabled( !ImGui::GetIO().WantCaptureMouse && !ImGuizmo::IsUsing() );
    camera.Update( &input, dt );

    if ( !headless )
    {
      render.Activate();
      render.Clear( ae::Color::PicoDarkPurple() );

      ae::Matrix4 worldToView = ae::Matrix4::WorldToView( camera.GetPosition(), camera.GetForward(), ae::Vec3( 0.0f, 0.0f, 1.0f ) );
      ae::Matrix4 viewToProj = ae::Matrix4::ViewToProjection( 0.525f, render.GetAspectRatio(), 0.5f, 1000.0f );
      ae::Matrix4 worldToProj = viewToProj * worldToView;
      // UI units in pixels, origin in bottom left
      ae::Matrix4 textToNdc = ae::Matrix4::Scaling( ae::Vec3( 2.0f / render.GetWidth(), 2.0f / render.GetHeight(), 1.0f ) );
      textToNdc *= ae::Matrix4::Translation( ae::Vec3( render.GetWidth() / -2.0f, render.GetHeight() / -2.0f, 0.0f ) );
      worldToText = textToNdc.GetInverse() * worldToProj;

      ae::Color top = ae::Color::SRGB8( 46, 65, 35 );
      ae::Color side = ae::Color::SRGB8( 84, 84, 74 );
      ae::Color path = ae::Color::SRGB8( 64, 64, 54 );
      ae::UniformList uniformList;
      uniformList.Set( "u_worldToProj", worldToProj );
      if ( wireframe )
      {
        uniformList.Set( "u_topColor", top.GetLinearRGBA() );
        uniformList.Set( "u_sideColor", side.GetLinearRGBA() );
        uniformList.Set( "u_pathColor", path.GetLinearRGBA() );
        terrainShader.SetBlending( false );
        terrainShader.SetCulling( ae::Culling::None );
        terrainShader.SetWireframe( true );
        terrain->Render( &terrainShader, uniformList );

        uniformList.Set( "u_topColor", top.SetA( 0.5f ).GetLinearRGBA() );
        uniformList.Set( "u_sideColor", side.SetA( 0.5f ).GetLinearRGBA() );
        uniformList.Set( "u_pathColor", path.SetA( 0.5f ).GetLinearRGBA() );
        terrainShader.SetBlending( true );
        terrainShader.SetCulling( ae::Culling::CounterclockwiseFront );
        terrainShader.SetWireframe( false );
        terrain->Render( &terrainShader, uniformList );
      }
      else
      {
        uniformList.Set( "u_topColor", top.GetLinearRGBA() );
        uniformList.Set( "u_sideColor", side.GetLinearRGBA() );
        uniformList.Set( "u_pathColor", path.GetLinearRGBA() );
        terrainShader.SetBlending( false );
        terrainShader.SetCulling( ae::Culling::CounterclockwiseFront );
        terrainShader.SetWireframe( false );
        terrain->Render( &terrainShader, uniformList );
      }

      ImGuizmo::SetRect( 0, 0, io.DisplaySize.x, io.DisplaySize.y );

      bool allowKeyboardInput = !headless && io.WantTextInput;
      if ( !allowKeyboardInput )
      {
        ae::Key delKey = _AE_OSX_ ? ae::Key::Backspace : ae::Key::Delete;
        if ( input.Get( ae::Key::Q ) && !input.GetPrev( ae::Key::Q ) )
        {
          currentObject = nullptr;
        }
        else if ( input.Get( ae::Key::W ) && !input.GetPrev( ae::Key::W ) )
        {
          gizmoOperation = ImGuizmo::TRANSLATE;
        }
        else if ( input.Get( ae::Key::E ) && !input.GetPrev( ae::Key::E ) )
        {
          gizmoOperation = ImGuizmo::ROTATE;
        }
        else if ( input.Get( ae::Key::R ) && !input.GetPrev( ae::Key::R ) )
        {
          gizmoOperation = ImGuizmo::SCALE;
        }
        else if ( currentObject && input.Get( delKey ) && !input.GetPrev( delKey ) )
        {
          objects.Remove( objects.Find( currentObject ) );
          terrain->sdf.DestroySdf( currentObject->shape );
          ae::Delete( currentObject );
          currentObject = nullptr;
        }
        
        // Camera focus
        if ( currentObject && !input.GetPrev( ae::Key::F ) && input.Get( ae::Key::F ) )
        {
          if ( currentObject->shape )
          {
            camera.Refocus( currentObject->shape->GetAABB().GetCenter() );
          }
          else
          {
            camera.Refocus( currentObject->raySrc );
          }
        }
        
        // Render mode
        if ( !input.GetPrev( ae::Key::Num1 ) && input.Get( ae::Key::Num1 ) )
        {
          wireframe = true;
          s_showTerrainDebug = true;
        }
        else if ( !input.GetPrev( ae::Key::Num2 ) && input.Get( ae::Key::Num2 ) )
        {
          wireframe = false;
          s_showTerrainDebug = true;
        }
        else if ( input.Get( ae::Key::Num3 ) && !input.GetPrev( ae::Key::Num3 ) )
        {
          wireframe = false;
          s_showTerrainDebug = false;
        }
      }

      for ( uint32_t i = 0; i < objects.Length(); i++ )
      {
        const Object* object = objects[ i ];
        if ( !object->shape )
        {
          if ( s_showTerrainDebug )
          {
            ae::Vec3 ray = object->rayDir * object->rayLength;
            if ( object->rayType == 0 || object->rayType == 1 )
            {
              if ( object->rayType )
              {
//                terrain->RaycastFast( object->raySrc, ray, true );
              }
              else
              {
                ae::RaycastParams params;
                params.source = object->raySrc;
                params.ray = ray;
                terrain->Raycast( params, nullptr );
              }
            }
            else
            {
              terrain->VoxelRaycast( object->raySrc, ray, 0 );
            }
          }
          else
          {
            ae::Vec3 endPos = object->raySrc + object->rayDir * object->rayLength;
            debug.AddLine( object->raySrc, endPos, ae::Color::PicoDarkGray() );
          }
        }
      }

      if ( currentObject )
      {
        bool gizmoClicked = ImGuizmo::IsUsing();

        if ( ae::Sdf* currentShape = currentObject->shape )
        {
          ae::Matrix4 gizmoTransform = currentShape->GetTransform();

          ImGuizmo::Manipulate(
            worldToView.data,
            viewToProj.data,
            gizmoOperation,
            ( gizmoOperation == ImGuizmo::SCALE ) ? ImGuizmo::LOCAL : ImGuizmo::WORLD,
            gizmoTransform.data
          );

          debug.AddOBB( gizmoTransform, ae::Color::Green() );
        
          if ( gizmoClicked )
          {
            // Dragging
            currentShape->SetTransform( gizmoTransform );
          }
          
          if ( gizmoClickedPrev && !gizmoClicked )
          {
            // Use ImGuizmo::IsUsing() to only update terrain when finished dragging
            currentShape->Dirty();
          }
        }
        else
        {
          ae::Matrix4 gizmoTransform = ae::Matrix4::Translation( currentObject->raySrc );

          ImGuizmo::Manipulate(
            worldToView.data,
            viewToProj.data,
            gizmoOperation,
            ( gizmoOperation == ImGuizmo::SCALE ) ? ImGuizmo::LOCAL : ImGuizmo::WORLD,
            gizmoTransform.data
          );

          if ( gizmoClicked )
          {
            // Dragging
            currentObject->raySrc = gizmoTransform.GetTranslation();
          }
        }

        gizmoClickedPrev = gizmoClicked;
      }

      debug.Render( worldToProj );
      textRender.Render( textToNdc );

      ui->Render();

      render.Present();
    }
    
    timeStep.Tick();
  }
  
  WriteObjects( &fileSystem, objects );

  AE_INFO( "Terminate" );
  ui->Terminate();
  terrain->Terminate();
  ae::Delete( terrain );
  input.Terminate();
  if ( !headless )
  {
    render.Terminate();
    window.Terminate();
  }
  
  return 0;
}
