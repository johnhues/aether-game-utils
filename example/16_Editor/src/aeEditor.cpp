//------------------------------------------------------------------------------
// EditorClient.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2021 John Hughes
// Created by John Hughes on 11/22/21.
//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeEditor.h"
#include "Level.h"
#include "Resource.h"
#include "ModelComponent.h" // @TODO: Remove

// Level
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include <unistd.h>
namespace ae {

enum class EditorMsg : uint8_t
{
  None,
  Modification,
  Load
};

//------------------------------------------------------------------------------
// Level member functions
//------------------------------------------------------------------------------
void Level::Save( const Registry* registry )
{
  uint32_t typeCount = registry->GetTypeCount();
  for ( uint32_t typeIdx = 0; typeIdx < typeCount; typeIdx++ )
  {
    const ae::Type* type = registry->GetTypeByIndex( typeIdx );
    const char* typeName = type->GetName();
    uint32_t varCount = type->GetVarCount();
    
    uint32_t componentCount = registry->GetComponentCountByIndex( typeIdx );
    for ( uint32_t compIdx = 0; compIdx < componentCount; compIdx++ )
    {
      const Component& component = registry->GetComponentByIndex( typeIdx, compIdx );
      LevelObject* levelObject = objects.TryGet( component.GetEntity() );
      if ( !levelObject )
      {
        levelObject = &objects.Set( component.GetEntity(), {} );
      }
      
      levelObject->id = component.GetEntity();
      levelObject->name = registry->GetNameByEntity( component.GetEntity() );
      
      ae::Dict& props = levelObject->components.Set( typeName, TAG_LEVEL );
      for ( uint32_t varIdx = 0; varIdx < varCount; varIdx++ )
      {
        const ae::Var* var = type->GetVarByIndex( varIdx );
        if ( var->IsArray() )
        {
          ae::Str32 key;
          uint32_t length = var->GetArrayLength( &component );
          
          key = ae::Str32::Format( "#::#", var->GetName(), "COUNT" );
          props.SetInt( key.c_str(), length );
          
          std::string value;
          for ( uint32_t arrIdx = 0; arrIdx < length; arrIdx++ )
          {
            key = ae::Str32::Format( "#::#", var->GetName(), arrIdx );
            value = var->GetObjectValueAsString( &component, arrIdx );
            props.SetString( key.c_str(), value.c_str() );
          }
        }
        else
        {
          auto value = var->GetObjectValueAsString( &component );
          props.SetString( var->GetName(), value.c_str() );
        }
      }
    }
  }
}

bool Level::Load( Registry* registry, CreateObjectFn fn ) const
{
  registry->Clear();
  
  uint32_t objectCount = objects.Length();
  // Create all components
  for ( uint32_t i = 0; i < objectCount; i++ )
  {
    const LevelObject& levelObject = objects.GetValue( i );
    Entity entity = registry->CreateEntity( levelObject.id, levelObject.name.c_str() );
    if ( fn )
    {
      fn( levelObject, entity, registry );
    }
    for ( uint32_t j = 0; j < levelObject.components.Length(); j++ )
    {
      const char* typeName = levelObject.components.GetKey( j ).c_str();
      registry->AddComponent( entity, typeName );
    }
  }
  // Serialize all components (second phase to handle references)
  for ( uint32_t i = 0; i < objectCount; i++ )
  {
    const LevelObject& levelObject = objects.GetValue( i );
    Entity entity = levelObject.id;
    for ( uint32_t j = 0; j < levelObject.components.Length(); j++ )
    {
      const char* typeName = levelObject.components.GetKey( j ).c_str();
      const ae::Type* type = ae::GetTypeByName( typeName );
      const ae::Dict& props = levelObject.components.GetValue( j );
      
      Component* component = &registry->GetComponent( entity, typeName );
      uint32_t varCount = type->GetVarCount();
      for ( uint32_t k = 0; k < varCount; k++ )
      {
        const ae::Var* var = type->GetVarByIndex( k );
        if ( var->IsArray() )
        {
          ae::Str32 key = ae::Str32::Format( "#::#", var->GetName(), "COUNT" );
          uint32_t length = props.GetInt( key.c_str(), 0 );
          length = var->SetArrayLength( component, length );
          
          for ( uint32_t arrIdx = 0; arrIdx < length; arrIdx++ )
          {
            key = ae::Str32::Format( "#::#", var->GetName(), arrIdx );
            if ( const char* value = props.GetString( key.c_str(), nullptr ) )
            {
              var->SetObjectValueFromString( component, value, arrIdx );
            }
          }
        }
        else if ( const char* value = props.GetString( var->GetName(), nullptr ) )
        {
          var->SetObjectValueFromString( component, value );
        }
      }
    }
  }
  return true;
}

bool Level::Write() const
{
  if ( !filePath.Length() )
  {
    return false;
  }

  rapidjson::Document document;
  rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
  document.SetObject();
  {
    uint32_t objectCount = objects.Length();
    rapidjson::Value jsonObjects( rapidjson::kArrayType );
    jsonObjects.Reserve( objectCount, allocator );

    for ( uint32_t objIdx = 0; objIdx < objectCount; objIdx++ )
    {
      const LevelObject& levelObject = objects.GetValue( objIdx );
      rapidjson::Value jsonObject( rapidjson::kObjectType );
      jsonObject.AddMember( "id", levelObject.id, allocator );
      if ( levelObject.name.Length() )
      {
        jsonObject.AddMember( "name", rapidjson::StringRef( levelObject.name.c_str() ), allocator );
      }
      rapidjson::Value transformJson;
      ae::Str64 transformStr = ae::ToString( levelObject.transform );
      transformJson.SetString( transformStr.c_str(), allocator );
      jsonObject.AddMember( "transform", transformJson, allocator );
      
      uint32_t componentCount = levelObject.components.Length();
      rapidjson::Value jsonComponents( rapidjson::kObjectType );
      for ( uint32_t compIdx = 0; compIdx < componentCount; compIdx++ )
      {
        const ae::Dict& levelComponent = levelObject.components.GetValue( compIdx );
        uint32_t propCount = levelComponent.Length();
        rapidjson::Value jsonComponent( rapidjson::kObjectType );
        for ( uint32_t propIdx = 0; propIdx < propCount; propIdx++ )
        {
          auto k = rapidjson::StringRef( levelComponent.GetKey( propIdx ) );
          auto v = rapidjson::StringRef( levelComponent.GetValue( propIdx ) );
          jsonComponent.AddMember( k, v, allocator );
        }
        const char* type = levelObject.components.GetKey( compIdx ).c_str();
        jsonComponents.AddMember( rapidjson::StringRef( type ), jsonComponent, allocator );
      }
      jsonObject.AddMember( "components", jsonComponents, allocator );
      jsonObjects.PushBack( jsonObject, allocator );
    }

    document.AddMember( "objects", jsonObjects, allocator );
  }
  
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter< rapidjson::StringBuffer > writer( buffer );
  document.Accept( writer );

  uint32_t writtenBytes = ae::FileSystem::Write( filePath.c_str(), buffer.GetString(), buffer.GetSize(), false );
  AE_ASSERT( writtenBytes == 0 || writtenBytes == buffer.GetSize() );
  return writtenBytes != 0;
}

bool Level::Read( const char* path )
{
  uint32_t fileSize = ae::FileSystem::GetSize( path );
  if ( !fileSize )
  {
    return false;
  }
  
  objects.Clear();
  
  char* jsonBuffer = new char[ fileSize + 1 ];
  ae::FileSystem::Read( path, jsonBuffer, fileSize );
  jsonBuffer[ fileSize ] = 0;
  
  rapidjson::Document document;
  AE_ASSERT( !document.ParseInsitu( jsonBuffer ).HasParseError() );
  AE_ASSERT( document.IsObject() );
  
  const auto& jsonObjects = document[ "objects" ];
  AE_ASSERT( jsonObjects.IsArray() );
  
  for ( const auto& jsonObject : jsonObjects.GetArray() )
  {
    Entity entity = jsonObject[ "id" ].GetUint();
    LevelObject& levelObject = objects.Set( entity, {} );
    levelObject.id = entity;
    if ( jsonObject.HasMember( "name" ) )
    {
      levelObject.name = jsonObject[ "name" ].GetString();
    }
    levelObject.transform = ae::FromString< ae::Matrix4 >( jsonObject[ "transform" ].GetString() );
    for ( const auto& componentIter : jsonObject[ "components" ].GetObject() )
    {
      const char* typeName = componentIter.name.GetString();
      ae::Dict& props = levelObject.components.Set( typeName, TAG_LEVEL );
      for ( const auto& propIter : componentIter.value.GetObject() )
      {
        props.SetString( propIter.name.GetString(), propIter.value.GetString() );
      }
    }
  }
  
  filePath = path;
  return true;
}

void EditorProgram::Initialize( uint32_t port, ae::Axis worldUp )
{
  const char* kVertShader = "\
    AE_UNIFORM mat4 u_worldToProj;\
    AE_UNIFORM vec4 u_color;\
    AE_IN_HIGHP vec4 a_position;\
    AE_IN_HIGHP vec4 a_color;\
    AE_OUT_HIGHP vec4 v_color;\
    void main()\
    {\
      v_color = a_color * u_color;\
      gl_Position = u_worldToProj * a_position;\
    }";

  const char* kFragShader = "\
    AE_IN_HIGHP vec4 v_color;\
    void main()\
    {\
      AE_COLOR = v_color;\
    }";

  struct Vertex
  {
    ae::Vec4 pos;
    ae::Vec4 color;
  };
  Vertex kCubeVerts[] =
  {
    { ae::Vec4( -0.5f, -0.5f, -0.5f, 1.0f ), ae::Color::PicoRed().GetLinearRGBA() },
    { ae::Vec4( 0.5f, -0.5f, -0.5f, 1.0f ), ae::Color::PicoOrange().GetLinearRGBA() },
    { ae::Vec4( 0.5f, 0.5f, -0.5f, 1.0f ), ae::Color::PicoYellow().GetLinearRGBA() },
    { ae::Vec4( -0.5f, 0.5f, -0.5f, 1.0f ), ae::Color::PicoPeach().GetLinearRGBA() },
    { ae::Vec4( -0.5f, -0.5f, 0.5f, 1.0f ), ae::Color::PicoGreen().GetLinearRGBA() },
    { ae::Vec4( 0.5f, -0.5f, 0.5f, 1.0f ), ae::Color::PicoPeach().GetLinearRGBA() },
    { ae::Vec4( 0.5f, 0.5f, 0.5f, 1.0f ), ae::Color::PicoPink().GetLinearRGBA() },
    { ae::Vec4( -0.5f, 0.5f, 0.5f, 1.0f ), ae::Color::PicoBlue().GetLinearRGBA() },
  };

  uint16_t kCubeIndices[] =
  {
    4, 5, 6, 4, 6, 7, // Top
    0, 3, 1, 1, 3, 2, // Bottom
    0, 4, 3, 3, 4, 7, // Left
    1, 2, 6, 6, 5, 1, // Right
    6, 2, 3, 6, 3, 7, // Back
    0, 1, 5, 0, 5, 4 // Front
  };

  AE_INFO( "Editor Initialize (port: #)", port );
  this->port = port;

  window.Initialize( 800, 600, false, true );
  window.SetTitle( "ae" );
  render.Initialize( &window );
  input.Initialize( &window );
  fileSystem.Initialize( "", "ae", "editor" );
  timeStep.SetTimeStep( 1.0f / 60.0f );
  ui.Initialize();
  camera.Reset( worldUp, ae::Vec3( 0.0f ), ae::Vec3( 10.0f ) );
  debugLines.Initialize( 512 );
  editor.Initialize( this );
  resourceManager = ae::New< ResourceManager >( TAG_RESOURCE );
  resourceManager->Initialize( &fileSystem );

  shader.Initialize( kVertShader, kFragShader, nullptr, 0 );
  shader.SetDepthTest( true );
  shader.SetDepthWrite( true );
  shader.SetBlending( true );
  shader.SetCulling( ae::Shader::Culling::CounterclockwiseFront );

  vertexData.Initialize( sizeof( *kCubeVerts ), sizeof( *kCubeIndices ), countof( kCubeVerts ), countof( kCubeIndices ), ae::VertexData::Primitive::Triangle, ae::VertexData::Usage::Static, ae::VertexData::Usage::Static );
  vertexData.AddAttribute( "a_position", 4, ae::VertexData::Type::Float, offsetof( Vertex, pos ) );
  vertexData.AddAttribute( "a_color", 4, ae::VertexData::Type::Float, offsetof( Vertex, color ) );
  vertexData.SetVertices( kCubeVerts, countof( kCubeVerts ) );
  vertexData.SetIndices( kCubeIndices, countof( kCubeIndices ) );
  vertexData.Upload();
  
  struct QuadVertex
  {
    ae::Vec4 pos;
    ae::Vec2 uv;
  };
  QuadVertex quadVerts[] =
  {
    { ae::Vec4( -0.5f, 0.5f, 0.0f, 1.0f ), ae::Vec2( 0.0f, 1.0f ) },
    { ae::Vec4( -0.5f, -0.5f, 0.0f, 1.0f ), ae::Vec2( 0.0f, 0.0f ) },
    { ae::Vec4( 0.5f, -0.5f, 0.0f, 1.0f ), ae::Vec2( 1.0f, 0.0f ) },
    { ae::Vec4( -0.5f, 0.5f, 0.0f, 1.0f ), ae::Vec2( 0.0f, 1.0f ) },
    { ae::Vec4( 0.5f, -0.5f, 0.0f, 1.0f ), ae::Vec2( 1.0f, 0.0f ) },
    { ae::Vec4( 0.5f, 0.5f, 0.0f, 1.0f ), ae::Vec2( 1.0f, 1.0f ) }
  };
  quad.Initialize( sizeof( *quadVerts ), 0, countof( quadVerts ), 0, ae::VertexData::Primitive::Triangle, ae::VertexData::Usage::Static, ae::VertexData::Usage::Static );
  quad.AddAttribute( "a_position", 4, ae::VertexData::Type::Float, offsetof( QuadVertex, pos ) );
  quad.AddAttribute( "a_uv", 2, ae::VertexData::Type::Float, offsetof( QuadVertex, uv ) );
  quad.SetVertices( quadVerts, countof( quadVerts ) );
  quad.Upload();
  
  const char* iconVertexShader = R"(
    AE_UNIFORM mat4 u_worldToProj;
    AE_IN_HIGHP vec4 a_position;
    AE_IN_HIGHP vec2 a_uv;
    AE_OUT_HIGHP vec2 v_uv;
    void main()
    {
      v_uv = a_uv;
      gl_Position = u_worldToProj * a_position;
    })";
  const char* iconFragShader = R"(
    AE_UNIFORM sampler2D u_tex;
    AE_UNIFORM vec4 u_color;
    AE_IN_HIGHP vec2 v_uv;
    void main()
    {
      AE_COLOR = AE_TEXTURE2D( u_tex, v_uv ) * u_color;
      AE_COLOR.rgb *= AE_COLOR.a;
    })";
  iconShader.Initialize( iconVertexShader, iconFragShader, nullptr, 0 );
  iconShader.SetDepthTest( true );
  iconShader.SetDepthWrite( false );
  iconShader.SetBlending( true );
  iconShader.SetBlendingPremul( true );
  iconShader.SetCulling( ae::Shader::Culling::CounterclockwiseFront );
}

void EditorProgram::Terminate()
{
  AE_INFO( "Terminate" );
  gameTarget.Terminate();
  editor.Terminate( this );
  debugLines.Terminate();
  ui.Terminate();
  input.Terminate();
  render.Terminate();
  window.Terminate();
}

void EditorProgram::Run()
{
  float r0 = 0.0f;
  float r1 = 0.0f;
  
  AE_INFO( "Run" );
  while ( !input.quit )
  {
    m_dt = timeStep.GetDt();
    if ( m_dt > 0.0f && m_dt < timeStep.GetTimeStep() * 4.0f )
    {
      m_dt = timeStep.GetDt();
    }
    else
    {
      m_dt = 0.00001f;
    }
  
    input.Pump();
    ui.NewFrame( &render, &input, GetDt() );
    ImGuiID mainDockSpace = ImGui::DockSpaceOverViewport( ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode );
    static bool s_once = true;
    static ImGuiID dock_left_id;
    if ( s_once )
    {
      dock_left_id = ImGui::DockBuilderSplitNode( mainDockSpace, ImGuiDir_Left, 0.2f, nullptr, &mainDockSpace );
      ImGui::DockBuilderDockWindow( "Dev", dock_left_id );
      
      ImGuiDockNode* node = ImGui::DockBuilderGetNode( dock_left_id );
      node->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
      
      s_once = false;
    }

    r0 += timeStep.GetDt() * 0.6f;
    r1 += timeStep.GetDt() * 0.75f;
    
    camera.SetInputEnabled( !ImGui::GetIO().WantCaptureMouse );
    camera.Update( &input, GetDt() );
    
    editor.Update( this );

    if ( ImGui::Begin( "Dev" ) )
    {
      editor.ShowUI( this );
      ImGui::End();
    }
    if ( const ImGuiWindow* imWin = ImGui::FindWindowByName( "Dev" ) )
    {
      m_barWidth = imWin->Size.x * ImGui::GetIO().DisplayFramebufferScale.x;
    }

    ae::Int2 oldSize( gameTarget.GetWidth(), gameTarget.GetHeight() );
    ae::Int2 targetSize = GetRenderRect().GetSize();
    if ( oldSize != targetSize )
    {
      gameTarget.Initialize( targetSize.x, targetSize.y );
      gameTarget.AddTexture( ae::Texture::Filter::Linear, ae::Texture::Wrap::Clamp );
      gameTarget.AddDepth( ae::Texture::Filter::Linear, ae::Texture::Wrap::Clamp );
    }
    gameTarget.Activate();
    gameTarget.Clear( ae::Color::PicoDarkGray() );

    m_worldToView = ae::Matrix4::WorldToView( camera.GetPosition(), camera.GetForward(), camera.GetLocalUp() );
    m_viewToProj = ae::Matrix4::ViewToProjection( GetFOV(), GetAspectRatio(), 0.25f, 500.0f );
    m_worldToProj = m_viewToProj * m_worldToView;
    m_projToWorld = m_worldToProj.GetInverse();
    
    // Update mouse ray
    {
      const float scaleFactor = window.GetScaleFactor();
      const ae::RectInt renderRectInt = GetRenderRect();
      const ae::Rect renderRect(
        renderRectInt.x / scaleFactor,
        renderRectInt.y / scaleFactor,
        renderRectInt.w / scaleFactor,
        renderRectInt.h / scaleFactor
      );
      
      ae::Vec2 mouseGameWindowPixelPos( input.mouse.position );
      mouseGameWindowPixelPos -= ae::Vec2( renderRect.GetMin() );
      
      ae::Vec2 mouseGameWindowNdcPos = mouseGameWindowPixelPos / ae::Vec2( renderRect.GetSize() );
      mouseGameWindowNdcPos *= 2.0f;
      mouseGameWindowNdcPos -= ae::Vec2( 1.0f );
      
      ae::Vec4 worldPos( mouseGameWindowNdcPos, 1.0f, 1.0f );
      worldPos = m_projToWorld * worldPos;
      worldPos /= worldPos.w;
      m_mouseRay = ( worldPos.GetXYZ() - camera.GetPosition() ).SafeNormalizeCopy();
    }
    
    if ( !editor.GetObjectCount() )
    {
      ae::UniformList uniformList;
      ae::Matrix4 modelToWorld = ae::Matrix4::RotationX( r0 ) * ae::Matrix4::RotationZ( r1 );
      uniformList.Set( "u_worldToProj", m_viewToProj * m_worldToView * modelToWorld );
      uniformList.Set( "u_color", ae::Color::White().GetLinearRGBA() );
      vertexData.Render( &shader, uniformList );
    }

    editor.Render( this );
    
    debugLines.Render( m_viewToProj * m_worldToView );

    render.Activate();
    render.Clear( ae::Color::Black() );
    
    float devNdc = ( m_barWidth / render.GetWidth() ) * 2.0f;
    ae::Rect ndcRect( devNdc - 1.0f, -1.0f, 2.0f - devNdc, 2.0f );
    gameTarget.Render2D( 0, ndcRect, 0.0f );
    ui.Render();
    render.Present();

    timeStep.Wait();
  }
}

ae::RectInt EditorProgram::GetRenderRect() const
{
  return ae::RectInt( m_barWidth, 0, render.GetWidth() - m_barWidth, render.GetHeight() );
}

float EditorProgram::GetAspectRatio() const
{
  ae::RectInt rect = GetRenderRect();
  return rect.w / (float)rect.h;
}

void _ae_EditorMain( uint16_t port, ae::Axis worldUpAxis )
{
  EditorProgram program;
  program.Initialize( port, worldUpAxis );
  program.Run();
  program.Terminate();
}

//------------------------------------------------------------------------------
// ae::EditorMain entry point
//------------------------------------------------------------------------------
int _ae_argc = 0;
char** _ae_argv = nullptr;
bool EditorMain( int argc, char *argv[] )
{
//  _ae_EditorMain( 7200, ae::Axis::Y );
//  return true;

  _ae_argc = argc;
  _ae_argv = argv;
  if ( argc > 2 && strcmp( argv[ 1 ], "ae_editor" ) == 0 )
  {
    if ( argc != 4 )
    {
      exit( -1 );
    }
    uint16_t port = std::stoi( argv[ 2 ] );
    ae::Axis worldUpAxis = ( strcmp( argv[ 3 ], "yUp" ) == 0 ) ? ae::Axis::Y : ae::Axis::Z;
    if ( port )
    {
      _ae_EditorMain( port, worldUpAxis );
    }
    return true;
  }
  return false;
}

//------------------------------------------------------------------------------
// LevelObject member functions
//------------------------------------------------------------------------------
void LevelObject::Initialize( const void* data, uint32_t length )
{
  Sync( data, length );
}

void LevelObject::Sync( const void* data, uint32_t length )
{
  if ( length == sizeof(transform) )
  {
    memcpy( &transform, data, length );
  }
}

//------------------------------------------------------------------------------
// Editor member functions
//------------------------------------------------------------------------------
EditorClient::EditorClient( const ae::Tag& tag ) :
  m_sock( tag )
{}

void EditorClient::Initialize( uint16_t port, Axis worldUp )
{
  Terminate();
  m_port = port;
  m_worldUp = worldUp;
  m_sock.Disconnect();
  m_Connect();
}

void EditorClient::Launch()
{
  if ( m_port == 0 || m_sock.IsConnected() )
  {
    return;
  }
  
  if ( !fork() )
  {
    ae::Str16 portStr = ae::Str16( "#", m_port );
    const char* worldUpStr = ( m_worldUp == ae::Axis::Y ) ? "yUp" : "zUp";
    char* execArgs[] = { _ae_argv[ 0 ], (char*)"ae_editor", (char*)portStr.c_str(), (char*)worldUpStr, nullptr };
    execv( _ae_argv[ 0 ], execArgs );
    return;
  }
}

void EditorClient::Update()
{
  levelDidChange = false;
  if ( m_port == 0 )
  {
    return;
  }
  m_Connect();
  
  uint32_t msgLength = 0;
  while ( ( msgLength = m_sock.ReceiveMsg( m_msgBuffer, sizeof(m_msgBuffer) ) ) )
  {
    EditorMsg msgType = EditorMsg::None;
    ae::BinaryStream rStream = ae::BinaryStream::Reader( m_msgBuffer, sizeof(m_msgBuffer) );
    rStream.SerializeRaw( msgType );
    switch ( msgType )
    {
      case EditorMsg::Modification:
      {
        Entity entity;
        ae::Matrix4 transform;
        rStream.SerializeUint32( entity );
        rStream.SerializeRaw( transform );
        AE_ASSERT( rStream.IsValid() );
        LevelObject* levelObj = level.objects.TryGet( entity );
        if ( levelObj )
        {
          levelObj->transform = transform;
        }
        break;
      }
      case EditorMsg::Load:
      {
        ae::Str256 levelPath;
        rStream.SerializeString( levelPath );
        if ( level.Read( levelPath.c_str() ) )
        {
          levelDidChange = true;
        }
        else
        {
          AE_ERR( "Could not load level '#'", levelPath );
        }
        break;
      }
      default:
        break;
    }
  }
}

void EditorClient::Terminate()
{
  m_sock.Disconnect();
  m_port = 0;
}

void EditorClient::m_Connect()
{
  if ( !m_sock.IsConnected() )
  {
    m_sock.Connect( ae::Socket::Protocol::TCP, "localhost", m_port );
  }
}

//------------------------------------------------------------------------------
// EditorObject member functions
//------------------------------------------------------------------------------
void EditorObject::Initialize( Entity entity, ae::Matrix4 transform )
{
  this->entity = entity;
  this->m_transform = transform;
}

void EditorObject::SetTransform( const ae::Matrix4& transform, EditorProgram* program )
{
//  AE_ASSERT( entity != kInvalidEntity );
//  program->registry.GetComponent< Transform >( entity );
  if ( m_transform != transform )
  {
    m_transform = transform;
    m_dirty = true;
  }
}

ae::Matrix4 EditorObject::GetTransform( const EditorProgram* program ) const
{
//  AE_ASSERT( entity != kInvalidEntity );
//  Registry* registry = const_cast< Registry* >( &program->registry );
//  return registry->GetComponent< Transform >( entity ).transform;
  return m_transform;
}

//------------------------------------------------------------------------------
// EditorConnection member functions
//------------------------------------------------------------------------------
EditorConnection::~EditorConnection()
{
  AE_ASSERT( !sock );
}

void EditorConnection::Destroy( EditorServer* editor )
{
  editor->sock.Destroy( sock );
  sock = nullptr;
}

//------------------------------------------------------------------------------
// EditorServer member functions
//------------------------------------------------------------------------------
void EditorServer::Initialize( EditorProgram* program )
{
  level = ae::New< Level >( TAG_EDITOR );
}

void EditorServer::Terminate( EditorProgram* program )
{
  for ( EditorConnection* conn : connections )
  {
    conn->Destroy( this );
    ae::Delete( conn );
  }
  connections.Clear();
  
  ae::Delete( level );
  level = nullptr;
}

void EditorServer::Update( EditorProgram* program )
{
  if ( !sock.IsListening() )
  {
    sock.Listen( ae::Socket::Protocol::TCP, false, program->port, 8 );
  }
  while ( ae::Socket* newConn = sock.Accept() )
  {
    AE_INFO( "ae::Editor client connected from #:#", newConn->GetResolvedAddress(), newConn->GetPort() );
    EditorConnection* editorConn = connections.Append( ae::New< EditorConnection >( TAG_EDITOR ) );
    editorConn->sock = newConn;
  }
  AE_ASSERT( connections.Length() == sock.GetConnectionCount() );
  
  for ( uint32_t i = 0; i < m_objects.Length(); i++ )
  {
    EditorObject* editorObj = &m_objects.GetValue( i );
    if ( editorObj->IsDirty() )
    {
      ae::Matrix4 transform = editorObj->GetTransform( program );
      ae::BinaryStream wStream = ae::BinaryStream::Writer( m_msgBuffer, sizeof(m_msgBuffer) );
      wStream.SerializeRaw( EditorMsg::Modification );
      wStream.SerializeUint32( editorObj->entity );
      wStream.SerializeRaw( transform );
      AE_ASSERT( wStream.IsValid() );
      for ( EditorConnection* conn : connections )
      {
        conn->sock->QueueMsg( wStream.GetData(), wStream.GetOffset() );
      }
      editorObj->ClearDirty();
    }
  }
  
  for ( EditorConnection* (&conn) : connections )
  {
    if ( conn->sock->IsConnected() )
    {
      conn->sock->SendAll();
    }
    else
    {
      AE_INFO( "ae::Editor client #:# disconnected", conn->sock->GetResolvedAddress(), conn->sock->GetPort() );
      conn->Destroy( this );
      ae::Delete( conn );
      conn = nullptr;
    }
  }
  connections.RemoveAllFn( []( const EditorConnection* c ){ return !c; } );
}

void EditorServer::Render( EditorProgram* program )
{
  struct LogicObj
  {
    const EditorObject* obj;
    float distanceSq;
  };
  ae::Array< LogicObj > logicObjects = TAG_EDITOR;
  
  ae::Vec3 camPos = program->camera.GetPosition();
  ae::Vec3 camUp = program->camera.GetLocalUp();
  ae::Matrix4 worldToProj = program->GetWorldToProj();
  
  uint32_t editorObjectCount = m_objects.Length();
  for ( uint32_t i = 0; i < editorObjectCount; i++ )
  {
    const EditorObject& obj = m_objects.GetValue( i );
    const ModelComponent* model = program->registry.TryGetComponent< ModelComponent >( obj.entity );
    if ( !model )
    {
      float distanceSq = ( camPos - obj.GetTransform( program ).GetTranslation() ).LengthSquared();
      logicObjects.Append( { &obj, distanceSq } );
      continue;
    }
    uint64_t seed = obj.entity * 43313;
    ae::Color color = ae::Color::HSV( ae::Random( 0.0f, 1.0f, seed ), 0.5, 0.75 );
    
    const ModelResource* modelResource = &model->GetModel( program->resourceManager );
    ae::Matrix4 objTransform = obj.GetTransform( program ) * modelResource->preTransform;
    MeshId mesh = modelResource->mesh;
    ShaderId shader = modelResource->shader;
    TextureId tex = modelResource->texture0;
    
    ae::UniformList uniformList;
    uniformList.Set( "u_localToProj", worldToProj * objTransform );
    uniformList.Set( "u_localToWorld", objTransform );
    uniformList.Set( "u_normalToWorld", objTransform.GetNormalMatrix() );
    uniformList.Set( "u_tex", &program->resourceManager->Get( tex ).texture );
    uniformList.Set( "u_color", color.GetLinearRGBA() );

    const MeshResource& meshResource = program->resourceManager->Get( mesh );
    const ShaderResource& shaderResource = program->resourceManager->Get( shader );
    meshResource.mesh.Render( &shaderResource.shader, uniformList );
  }
  
  std::sort( logicObjects.begin(), logicObjects.end(), []( const LogicObj& a, const LogicObj& b ){ return a.distanceSq > b.distanceSq; } );
  for ( const LogicObj& logicObj : logicObjects )
  {
    const EditorObject& obj = *logicObj.obj;
    uint64_t seed = obj.entity * 43313;
    ae::Color color = ae::Color::HSV( ae::Random( 0.0f, 1.0f, seed ), 0.5, 0.75 );
    
    ae::UniformList uniformList;
    ae::Vec3 objPos = obj.GetTransform( program ).GetTranslation();
    ae::Vec3 toCamera = camPos - objPos;
    ae::Matrix4 modelToWorld = ae::Matrix4::Rotation( ae::Vec3(0,0,1), ae::Vec3(0,1,0), toCamera, camUp );
    modelToWorld.SetTranslation( objPos );
    uniformList.Set( "u_worldToProj", worldToProj * modelToWorld );
    uniformList.Set( "u_tex", &program->resourceManager->Get( TextureId::Cog ).texture );
    uniformList.Set( "u_color", color.GetLinearRGBA() );
    program->quad.Render( &program->iconShader, uniformList );
  }
}

void EditorServer::ShowUI( EditorProgram* program )
{
  if ( m_toHide )
  {
    ImGui::SetNextItemOpen( false );
    m_toHide = false;
  }

  float dt = program->GetDt();
  
  ae::Color cursorColor = ae::Color::PicoGreen();
  ae::Vec3 mouseHover( 0.0f );
  ae::Vec3 mouseHoverNormal( 0, 1, 0 );
  Entity pickedEntity = m_PickObject( program, cursorColor, &mouseHover, &mouseHoverNormal );
  
  static float s_hold = 0.0f;
  static ae::Vec2 s_mouseMove( 0.0f );
  if ( !ImGui::GetIO().WantCaptureMouse
    && program->input.mouse.leftButton && s_hold >= 0.0f )
  {
    s_hold += dt;
    s_mouseMove += ae::Vec2( program->input.mouse.movement );
    
    if ( s_mouseMove.Length() > 4.0f )
    {
      s_hold = -1.0f;
    }
    else if ( pickedEntity )
    {
      cursorColor = ae::Color::PicoOrange();
      if ( s_hold > 0.35f )
      {
        program->camera.Refocus( mouseHover );
        s_hold = -1.0f;
      }
    }
  }
  else if ( !program->input.mouse.leftButton )
  {
    s_hold = 0.0f;
    s_mouseMove = ae::Vec2( 0.0f );
  }
  
  if ( !program->input.mouse.leftButton && program->input.mousePrev.leftButton // Release
    && !ImGui::GetIO().WantCaptureMouse && program->camera.GetMode() == ae::DebugCamera::Mode::None )
  {
    if ( m_selectRef.enabled )
    {
      uint32_t matchCount = 0;
      Component* lastMatch = nullptr;
      
      const ae::Type* refType = m_selectRef.componentVar->GetSubType();
      uint32_t componentTypesCount = program->registry.GetTypeCount();
      for ( uint32_t i = 0; i < componentTypesCount; i++ )
      {
        const ae::Type* otherType = program->registry.GetTypeByIndex( i );
        if ( !otherType->IsType( refType ) )
        {
          continue;
        }
        
        Component* otherComp = program->registry.TryGetComponent( pickedEntity, otherType->GetName() );
        if ( !otherComp )
        {
          continue;
        }
        
        matchCount++;
        lastMatch = otherComp;
      }
      
      if ( matchCount == 0 )
      {
        AE_INFO( "Invalid selection. No '#' component.", refType->GetName() );
      }
      else if ( matchCount == 1 )
      {
        m_selectRef.componentVar->SetObjectValue( m_selectRef.component, lastMatch, m_selectRef.varIdx );
        m_selectRef = SelectRef();
      }
      else
      {
        m_selectRef.pending = pickedEntity;
      }
    }
    else if ( !pickedEntity && selected )
    {
      AE_INFO( "Deselect Entity" );
      selected = kInvalidEntity;
    }
    else if ( pickedEntity != selected )
    {
      AE_INFO( "Select Entity" );
      selected = pickedEntity;
    }
  }
  
  if ( m_selectRef.enabled && m_selectRef.pending )
  {
    ImGui::OpenPopup( "ref_select_popup" );
  }
  if ( ImGui::BeginPopup( "ref_select_popup" ) )
  {
    const ae::Type* refType = m_selectRef.componentVar->GetSubType();
  
    ImGui::Text( "Select %s", refType->GetName() );
    ImGui::Separator();
    uint32_t componentTypesCount = program->registry.GetTypeCount();
    for ( uint32_t i = 0; i < componentTypesCount; i++ )
    {
      const ae::Type* otherType = program->registry.GetTypeByIndex( i );
      if ( !otherType->IsType( refType ) )
      {
        continue;
      }

      Component* otherComp = program->registry.TryGetComponent( m_selectRef.pending, otherType->GetName() );
      if ( !otherComp )
      {
        continue;
      }

      if ( ImGui::Selectable( otherType->GetName(), false ) )
      {
        m_selectRef.componentVar->SetObjectValue( m_selectRef.component, otherComp, m_selectRef.varIdx );
        m_selectRef = SelectRef();
      }
    }
    ImGui::EndPopup();
  }
  
  m_ShowEditorObject( program, selected, ae::Color::Green() );
  if ( pickedEntity )
  {
    program->debugLines.AddCircle( mouseHover + mouseHoverNormal * 0.025f, mouseHoverNormal, 0.5f, cursorColor, 8 );
    if ( selected != pickedEntity )
    {
      m_ShowEditorObject( program, pickedEntity, ae::Color::PicoDarkGray() );
    }
  }
  
  ae::Vec3 debugRefocus( 0.0f );
  program->debugLines.AddSphere( program->camera.GetFocus(), 0.1f, ae::Color::Green(), 6 );
//  if ( program->camera->GetDebugRefocusTarget( &debugRefocus ) )
//  {
//    program->debugLines.AddSphere( debugRefocus, 0.1f, ae::Color::PicoOrange(), 6 );
//  }
  
  if ( ImGui::GetIO().WantCaptureKeyboard )
  {
    // keyboard captured
  }
  else if ( selected && program->input.Get( ae::Key::F ) && !program->input.GetPrev( ae::Key::F ) )
  {
    program->camera.Refocus( m_objects.Get( selected ).GetTransform( program ).GetTranslation() );
  }
  else if ( program->input.Get( ae::Key::Q ) && !program->input.GetPrev( ae::Key::Q ) )
  {
    gizmoOperation = (ImGuizmo::OPERATION)0;
  }
  else if ( program->input.Get( ae::Key::W ) && !program->input.GetPrev( ae::Key::W ) )
  {
    if ( gizmoOperation == ImGuizmo::TRANSLATE )
    {
      gizmoMode = ( gizmoMode == ImGuizmo::LOCAL ) ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
    }
    else
    {
      gizmoOperation = ImGuizmo::TRANSLATE;
    }
  }
  else if ( program->input.Get( ae::Key::E ) && !program->input.GetPrev( ae::Key::E ) )
  {
    if ( gizmoOperation == ImGuizmo::ROTATE )
    {
      gizmoMode = ( gizmoMode == ImGuizmo::LOCAL ) ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
    }
    else
    {
      gizmoOperation = ImGuizmo::ROTATE;
    }
  }
  else if ( program->input.Get( ae::Key::R ) && !program->input.GetPrev( ae::Key::R ) )
  {
    gizmoOperation = ImGuizmo::SCALE;
  }
  else if ( program->input.Get( ae::Key::LeftMeta ) )
  {
    if ( program->input.Get( ae::Key::S ) && !program->input.GetPrev( ae::Key::S ) )
    {
      SaveLevel( program, program->input.Get( ae::Key::LeftShift ) );
    }
    else if ( program->input.Get( ae::Key::O ) && !program->input.GetPrev( ae::Key::O ) )
    {
      OpenLevel( program );
    }
    else if ( program->input.Get( ae::Key::H ) && !program->input.GetPrev( ae::Key::H ) )
    {
      if ( selected )
      {
        EditorObject& editorObject = m_objects.Get( selected );
        editorObject.hidden = !editorObject.hidden;
      }
      else
      {
        AE_INFO( "No objects selected" );
      }
    }
  }
  
  
  if ( selected && gizmoOperation )
  {
    const float scaleFactor = program->window.GetScaleFactor();
    const ae::RectInt renderRectInt = program->GetRenderRect();
    const ae::Rect renderRect(
      renderRectInt.x / scaleFactor,
      renderRectInt.y / scaleFactor,
      renderRectInt.w / scaleFactor,
      renderRectInt.h / scaleFactor
    );
    
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::Enable( program->camera.GetMode() == ae::DebugCamera::Mode::None );
    ImGuizmo::SetOrthographic( false );
    ImGuizmo::AllowAxisFlip( false );
    ImGuizmo::BeginFrame();
    ImGuizmo::SetRect( renderRect.x, renderRect.y, renderRect.w, renderRect.h );
    
    EditorObject* selectedObject = &m_objects.Get( selected );
    ae::Matrix4 transform = selectedObject->GetTransform( program );
    if ( ImGuizmo::Manipulate(
      program->GetWorldToView().data,
      program->GetViewToProj().data,
      gizmoOperation,
      ( gizmoOperation == ImGuizmo::SCALE ) ? ImGuizmo::LOCAL : gizmoMode,
      transform.data
    ) )
    {
      selectedObject->SetTransform( transform, program );
    }
  }
  
  if ( m_first )
  {
    ImGui::SetNextItemOpen( true );
  }
  if ( ImGui::TreeNode( "File" ) )
  {
    if ( ImGui::Button( "Open" ) )
    {
      OpenLevel( program );
    }
    ImGui::SameLine();
    if ( ImGui::Button( "Save" ) )
    {
      SaveLevel( program, false );
    }
    ImGui::SameLine();
    if ( ImGui::Button( "Save As" ) )
    {
      SaveLevel( program, true );
    }
    if ( ImGui::Button( "Game Load" ) )
    {
      uint8_t buffer[ kMaxEditorMessageSize ];
      ae::BinaryStream wStream = ae::BinaryStream::Writer( buffer );
      wStream.SerializeRaw( EditorMsg::Load );
      wStream.SerializeString( level->filePath );
      for ( uint32_t i = 0; i < connections.Length(); i++ )
      {
        connections[ i ]->sock->QueueMsg( wStream.GetData(), wStream.GetOffset() );
      }
    }
    ImGui::TreePop();
  }
  
  if ( m_first )
  {
    ImGui::SetNextItemOpen( true );
  }
  if ( ImGui::TreeNode( "Tools" ) )
  {
    if ( ImGui::RadioButton( "O", gizmoOperation == (ImGuizmo::OPERATION)0 ) )
    {
      gizmoOperation = (ImGuizmo::OPERATION)0;
    }
    ImGui::SameLine();
    if ( ImGui::RadioButton( "T", gizmoOperation == ImGuizmo::TRANSLATE ) )
    {
      gizmoOperation = ImGuizmo::TRANSLATE;
    }
    ImGui::SameLine();
    if ( ImGui::RadioButton( "R", gizmoOperation == ImGuizmo::ROTATE ) )
    {
      gizmoOperation = ImGuizmo::ROTATE;
    }
    ImGui::SameLine();
    if ( ImGui::RadioButton( "S", gizmoOperation == ImGuizmo::SCALE ) )
    {
      gizmoOperation = ImGuizmo::SCALE;
    }
    if ( ImGui::RadioButton( "World", gizmoMode == ImGuizmo::WORLD ) )
    {
      gizmoMode = ImGuizmo::WORLD;
    }
    ImGui::SameLine();
    if ( ImGui::RadioButton( "Local", gizmoMode == ImGuizmo::LOCAL ) )
    {
      gizmoMode = ImGuizmo::LOCAL;
    }
    ImGui::Checkbox( "Show Invisible", &m_showInvisible );
    
    ImGui::TreePop();
  }
  
  if ( ImGui::TreeNode( "Operations" ) )
  {
    if ( ImGui::Button( "Create" ) )
    {
      Entity entity = program->registry.CreateEntity();
      ae::Matrix4 transform = ae::Matrix4::Translation( program->camera.GetFocus() );
      
      EditorObject* editorObject = &m_objects.Set( entity, {} );
      editorObject->Initialize( entity, transform );
      
      selected = entity;
    }
    
    static bool s_imGuiDemo = false;
    if ( ImGui::Button( "Show ImGui Demo" ) )
    {
      s_imGuiDemo = !s_imGuiDemo;
    }
    if ( s_imGuiDemo )
    {
      ImGui::ShowDemoWindow( &s_imGuiDemo );
    }
    
    ImGui::TreePop();
  }
  
  if ( ImGui::TreeNode( "Object properties" ) )
  {
    if ( selected )
    {
      EditorObject* selectedObject = &m_objects.Get( selected );
      ImGui::Text( "Entity %u", selected );
    
      char name[ ae::Str16::MaxLength() ];
      strcpy( name, program->registry.GetNameByEntity( selected ) );
      if ( ImGui::InputText( "Name", name, countof(name), ImGuiInputTextFlags_EnterReturnsTrue ) )
      {
        AE_INFO( "Set entity name: #", name );
        program->registry.SetEntityName( selected, name );
      }
      {
        bool changed = false;
        ae::Matrix4 temp = selectedObject->GetTransform( program );
        float matrixTranslation[ 3 ], matrixRotation[ 3 ], matrixScale[ 3 ];
        ImGuizmo::DecomposeMatrixToComponents( temp.data, matrixTranslation, matrixRotation, matrixScale );
        changed |= ImGui::InputFloat3( "translation", matrixTranslation );
        changed |= ImGui::InputFloat3( "rotation", matrixRotation );
        changed |= ImGui::InputFloat3( "scale", matrixScale );
        if ( changed )
        {
          ImGuizmo::RecomposeMatrixFromComponents( matrixTranslation, matrixRotation, matrixScale, temp.data );
          selectedObject->SetTransform( temp, program );
        }
      }
      
      uint32_t componentTypesCount = program->registry.GetTypeCount();
      for ( uint32_t i = 0; i < componentTypesCount; i++ )
      {
        const ae::Type* type = program->registry.GetTypeByIndex( i );
        Component* component = program->registry.TryGetComponent( selected, type->GetName() );
        if ( !component )
        {
          continue;
        }
        
        if ( ImGui::TreeNode( type->GetName() ) )
        {
          uint32_t varCount = type->GetVarCount();
          for ( uint32_t i = 0; i < varCount; i++ )
          {
            m_ShowVar( program, component, type->GetVarByIndex( i ) );
          }
          ImGui::TreePop();
        }
      }
      
      if ( ImGui::Button( "Add Component" ) )
      {
        ImGui::OpenPopup( "add_component_popup" );
      }
      if ( ImGui::BeginPopup( "add_component_popup" ) )
      {
        const ae::Type* componentType = ae::GetType< Component >();
        uint32_t typeCount = ae::GetTypeCount();
        for ( uint32_t i = 0; i < typeCount; i++ )
        {
          const ae::Type* type = ae::GetTypeByIndex( i );
          if ( !type->IsType( componentType ) || !type->IsDefaultConstructible() )
          {
            continue;
          }
          
          if ( ImGui::Selectable( type->GetName() ) )
          {
            AE_INFO( "Create #", type->GetName() );
            program->registry.AddComponent( selected, type->GetName() );
          }
        }
        ImGui::EndPopup();
      }
    }
    else
    {
      ImGui::Text( "No Selection" );
    }
    ImGui::TreePop();
  }
  
  if ( ImGui::TreeNode( "Object List" ) )
  {
    const char* selectedTypeName = m_selectedType ? m_selectedType->GetName() : "All";
    if ( ImGui::BeginCombo( "Type", selectedTypeName, 0 ) )
    {
      if ( ImGui::Selectable( "All", !m_selectedType ) )
      {
        m_selectedType = nullptr;
      }
      if ( !m_selectedType )
      {
        ImGui::SetItemDefaultFocus();
      }
      uint32_t componentTypesCount = program->registry.GetTypeCount();
      for ( uint32_t i = 0; i < componentTypesCount; i++ )
      {
        const ae::Type* type = program->registry.GetTypeByIndex( i );
        const bool isSelected = ( m_selectedType == type );
        if ( ImGui::Selectable( type->GetName(), isSelected ) )
        {
          m_selectedType = type;
        }
        if ( isSelected )
        {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }

    if ( ImGui::BeginListBox("##listbox", ImVec2( -FLT_MIN, 16 * ImGui::GetTextLineHeightWithSpacing() ) ) )
    {
      auto& selected = this->selected;
      auto showObjInList = [&selected]( int idx, Entity entity, const char* entityName )
      {
        ImGui::PushID( idx );
        const bool isSelected = ( entity == selected );
        ae::Str16 name = entityName;
        if ( !name.Length() )
        {
          name = ae::Str16::Format( "#", entity );
        }
        if ( ImGui::Selectable( name.c_str(), isSelected ) )
        {
          selected = entity;
        }
        if ( isSelected )
        {
          ImGui::SetItemDefaultFocus();
        }
        ImGui::PopID();
      };
      
      if ( m_selectedType )
      {
        int32_t typeIndex = program->registry.GetTypeIndexByType( m_selectedType );
        AE_ASSERT( typeIndex >= 0 );
        uint32_t componentCount = program->registry.GetComponentCountByIndex( typeIndex );
        for ( uint32_t i = 0; i < componentCount; i++ )
        {
          const Component& c = program->registry.GetComponentByIndex( typeIndex, i );
          showObjInList( i, c.GetEntity(), c.GetEntityName() );
        }
      }
      else
      {
        uint32_t editorObjectCount = m_objects.Length();
        for ( uint32_t i = 0; i < editorObjectCount; i++ )
        {
          const EditorObject* editorObj = &m_objects.GetValue( i );
          showObjInList( i, editorObj->entity, program->registry.GetNameByEntity( editorObj->entity ) );
        }
      }
      ImGui::EndListBox();
    }

    ImGui::TreePop();
  }
  
  m_first = false;
}

bool EditorServer::SaveLevel( EditorProgram* program, bool saveAs )
{
  bool fileSelected = level->filePath.Length();
  if ( !fileSelected || saveAs )
  {
    fileSelected = false;
    ae::FileDialogParams params;
    params.window = &program->window;
    params.filters.Append( { "Level File", "level" } );
    params.defaultPath = level->filePath.c_str();
    auto filePath = program->fileSystem.SaveDialog( params );
    if ( filePath.c_str()[ 0 ] )
    {
      fileSelected = true;
      level->filePath = filePath.c_str();
    }
  }
  
  if ( fileSelected )
  {
    level->Save( &program->registry );
    if ( level->Write() )
    {
      AE_INFO( "Saved '#'", level->filePath );
    }
    else
    {
      AE_INFO( "Failed to save '#'", level->filePath );
    }
  }
  else
  {
    AE_INFO( "No file selected" );
  }
}

bool EditorServer::OpenLevel( EditorProgram* program )
{
  ae::FileDialogParams params;
  params.window = &program->window;
  params.filters.Append( { "Level File", "level" } );
  auto filePath = program->fileSystem.OpenDialog( params );
  if ( filePath.Length() )
  {
    AE_INFO( "Level '#'", filePath );
    AE_INFO( "Reading..." );
    if ( level->Read( filePath[ 0 ].c_str() ) )
    {
      AE_INFO( "Loading..." );
      if ( level->Load( &program->registry ) )
      {
        m_objects.Clear();
        uint32_t levelObjectCount = level->objects.Length();
        for ( uint32_t i = 0; i < levelObjectCount; i++ )
        {
          const LevelObject& levelObj = level->objects.GetValue( i );
          EditorObject& editorObj = m_objects.Set( levelObj.id, {} );
          editorObj.Initialize( levelObj.id, levelObj.transform );
        }
      
        AE_INFO( "Loaded level" );
        selected = kInvalidEntity;
        program->window.SetTitle( level->filePath.c_str() );
      }
      else
      {
        AE_INFO( "Failed to load level" );
      }
    }
    else
    {
      AE_INFO( "Failed to read level" );
    }
  }
  else
  {
    AE_INFO( "No file selected" );
  }
}

void EditorServer::SetOpen( bool isOpen )
{
  m_toHide = !isOpen;
}

void EditorServer::m_ShowVar( EditorProgram* program, Component* component, const ae::Var* var )
{
  ImGui::PushID( var->GetName() );
  if ( var->IsArray() )
  {
    uint32_t arrayLength = var->GetArrayLength( component );
    if ( arrayLength > 231 )
    {
      arrayLength = var->GetArrayLength( component );
    }
    ImGui::Text( "%s", var->GetName() );
    ImVec2 size( ImGui::GetContentRegionAvail().x, 8 * ImGui::GetTextLineHeightWithSpacing() );
    ImGui::BeginChild( "ChildL", size, true, 0 );
    for ( int i = 0; i < arrayLength; i++ )
    {
      ImGui::PushID( i );
      m_ShowVarValue( program, component, var, i );
      ImGui::PopID();
    }
    ImGui::EndChild();
    if ( var->IsArrayResizable() )
    {
      if ( ImGui::Button( "Add" ) )
      {
        var->SetArrayLength( component, arrayLength + 1 );
      }
      ImGui::SameLine();
      if ( ImGui::Button( "Remove" ) && arrayLength )
      {
        var->SetArrayLength( component, arrayLength - 1 );
      }
    }
  }
  else
  {
    m_ShowVarValue( program, component, var );
  }
  ImGui::PopID();
}

void EditorServer::m_ShowVarValue( EditorProgram* program, Component* component, const ae::Var* var, int32_t idx )
{
  switch ( var->GetType() )
  {
    case ae::Var::Type::Enum:
    {
      auto currentStr = var->GetObjectValueAsString( component, idx );
      auto valueStr = aeImGui_Enum( var->GetEnum(), var->GetName(), currentStr.c_str() );
      var->SetObjectValueFromString( component, valueStr.c_str(), idx );
      break;
    }
    case ae::Var::Type::Bool:
    {
      bool b = false;
      var->GetObjectValue( component, &b, idx );
      if ( ImGui::Checkbox( var->GetName(), &b ) )
      {
        var->SetObjectValue( component, b, idx );
      }
      break;
    }
    case ae::Var::Type::Float:
    {
      float f = 0.0f;
      var->GetObjectValue( component, &f, idx );
      if ( ImGui::InputFloat( var->GetName(), &f ) )
      {
        var->SetObjectValue( component, f, idx );
      }
      break;
    }
    case ae::Var::Type::String:
    {
      char buf[ 256 ];
      auto val = var->GetObjectValueAsString( component, idx );
      strlcpy( buf, val.c_str(), sizeof(buf) );
      ImGui::Text( "%s", var->GetName() );
      if ( ImGui::InputTextMultiline( var->GetName(), buf, sizeof(buf), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 4 ), 0 ) )
      {
        var->SetObjectValueFromString( component, buf, idx );
      }
      break;
    }
    case ae::Var::Type::Ref:
    {
      m_ShowRefVar( program, component, var, idx );
      break;
    }
    default:
      ImGui::Text( "%s (Unsupported type)", var->GetName() );
      break;
  }
}

void EditorServer::m_ShowRefVar( EditorProgram* program, Component* component, const ae::Var* var, int32_t idx )
{
  const ae::Type* componentType = ae::GetTypeFromObject( component );
  auto val = var->GetObjectValueAsString( component, idx );
  
  if ( idx < 0 )
  {
    ImGui::Text( "%s", var->GetName() );
  }
  const ae::Type* refType = var->GetSubType();
  AE_ASSERT( refType );
  if ( m_selectRef.enabled
    && m_selectRef.component == component
    && m_selectRef.componentVar == var
    && m_selectRef.varIdx == idx )
  {
    if ( ImGui::Button( "Cancel" ) )
    {
      m_selectRef = SelectRef();
    }
  }
  else
  {
    ImGui::Text( "%s", val.c_str() );
    ImGui::SameLine();
    if ( val == "NULL" )
    {
      if ( ImGui::Button( "Select" ) )
      {
        m_selectRef.enabled = true;
        m_selectRef.component = component;
        m_selectRef.componentVar = var;
        m_selectRef.varIdx = idx;
      }
    }
    else
    {
      if ( ImGui::Button( "Clear" ) )
      {
        var->SetObjectValueFromString( component, "NULL", idx );
      }
      ImGui::SameLine();
      ae::Object* selectObj = nullptr;
      if ( ImGui::Button( "Select" ) && program->serializer.StringToObjectPointer( val.c_str(), &selectObj ) )
      {
        AE_ASSERT( selectObj );
        Component* selectComp = ae::Cast< Component >( selectObj );
        AE_ASSERT( selectComp );
        selected = selectComp->GetEntity();
      }
    }
  }
}

std::string EditorProgram::Serializer::ObjectPointerToString( const ae::Object* obj ) const
{
  if ( !obj )
  {
    return std::string( "NULL" );
  }
  const ae::Type* type = ae::GetTypeFromObject( obj );
  AE_ASSERT( type->IsType< Component >() );
  Component* c = (Component*)obj;
  std::ostringstream str;
  str << c->GetEntity() << " " << type->GetName();
  return str.str();
};

bool EditorProgram::Serializer::StringToObjectPointer( const char* pointerVal, ae::Object** objOut ) const
{
  if ( strcmp( pointerVal, "NULL" ) == 0 )
  {
    *objOut = nullptr;
    return true;
  }
  Entity entity = 0;
  char typeName[ 16 ];
  typeName[ 0 ] = 0;
  if ( sscanf( pointerVal, "%u %15s", &entity, typeName ) == 2 )
  {
    *objOut = program->registry.TryGetComponent( entity, typeName );
    return true;
  }
  return false;
};

//------------------------------------------------------------------------------
// EditorPicking functions
//------------------------------------------------------------------------------
Entity EditorServer::m_PickObject( EditorProgram* program, ae::Color color, ae::Vec3* hitOut, ae::Vec3* normalOut )
{
  ae::Vec3 camPos = program->camera.GetPosition();
  ae::Vec3 mouseRay = program->GetMouseRay();
  
  ae::CollisionMesh::RaycastResult result;
  uint32_t editorObjectCount = m_objects.Length();
  for ( uint32_t i = 0; i < editorObjectCount; i++ )
  {
    const EditorObject* editorObj = &m_objects.GetValue( i );
    const ModelComponent* model = program->registry.TryGetComponent< ModelComponent >( editorObj->entity );
    if ( model )
    {
      const ModelResource* modelResource = &model->GetModel( program->resourceManager );
      const MeshResource* mesh = &model->GetMesh( program->resourceManager );
      ae::CollisionMesh::RaycastParams params;
      params.userData = editorObj;
      params.source = camPos;
      params.direction = mouseRay;
      params.transform = editorObj->GetTransform( program ) * modelResource->preTransform;
      params.hitClockwise = true;
      params.hitCounterclockwise = true;
      result = mesh->collision.Raycast( params, result );
    }
    else
    {
      float hitT = INFINITY;
      ae::Vec3 hitPos( 0.0f );
      ae::Sphere sphere( editorObj->GetTransform( program ).GetTranslation(), 0.5f );
      if ( sphere.Raycast( camPos, mouseRay, &hitT, &hitPos ) && ( !result.hitCount || hitT < result.hits[ 0 ].distance ) )
      {
        result.hits[ 0 ].position = hitPos;
        result.hits[ 0 ].normal = ( camPos - hitPos ).SafeNormalizeCopy();
        result.hits[ 0 ].distance = hitT;
        result.hits[ 0 ].userData = editorObj;
        result.hitCount = 1;
      }
    }
  }
  if ( result.hitCount )
  {
    *hitOut = result.hits[ 0 ].position;
    *normalOut = result.hits[ 0 ].normal;
    const EditorObject* editorObj = (const EditorObject*)result.hits[ 0 ].userData;
    AE_ASSERT( editorObj );
    return editorObj->entity;
  }
  
  return kInvalidEntity;
}

void EditorServer::m_ShowEditorObject( EditorProgram* program, Entity entity, ae::Color color )
{
  if ( entity )
  {
    const EditorObject* editorObj = &m_objects.Get( entity );
    const ModelComponent* model = program->registry.TryGetComponent< ModelComponent >( entity );
    if ( model )
    {
      const ModelResource* modelResource = &model->GetModel( program->resourceManager );
      const MeshResource* meshResource = &model->GetMesh( program->resourceManager );
      ae::Matrix4 obbTransform = editorObj->GetTransform( program ) * modelResource->preTransform * model->GetMesh( program->resourceManager ).collision.GetAABB().GetTransform();
      ae::OBB obb = obbTransform;
      program->debugLines.AddOBB( obb.GetTransform(), color );
    }
    else
    {
      ae::Vec3 pos = editorObj->GetTransform( program ).GetTranslation();
      ae::Vec3 normal = program->camera.GetPosition() - pos;
      program->debugLines.AddCircle( pos, normal, 0.475f, color, 16 );
    }
  }
}

}
