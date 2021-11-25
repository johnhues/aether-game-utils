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

#include <unistd.h>
namespace ae {

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
    { ae::Vec4( -0.5f, -0.5f, -0.5f, 1.0f ), ae::Color::White().GetLinearRGBA() },
    { ae::Vec4( 0.5f, -0.5f, -0.5f, 1.0f ), ae::Color::White().GetLinearRGBA() },
    { ae::Vec4( 0.5f, 0.5f, -0.5f, 1.0f ), ae::Color::White().GetLinearRGBA() },
    { ae::Vec4( -0.5f, 0.5f, -0.5f, 1.0f ), ae::Color::White().GetLinearRGBA() },
    { ae::Vec4( -0.5f, -0.5f, 0.5f, 1.0f ), ae::Color::White().GetLinearRGBA() },
    { ae::Vec4( 0.5f, -0.5f, 0.5f, 1.0f ), ae::Color::White().GetLinearRGBA() },
    { ae::Vec4( 0.5f, 0.5f, 0.5f, 1.0f ), ae::Color::White().GetLinearRGBA() },
    { ae::Vec4( -0.5f, 0.5f, 0.5f, 1.0f ), ae::Color::White().GetLinearRGBA() },
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
    
    ae::UniformList uniformList;

    static float h = 0.0f;
    if ( input.Get( ae::Key::C ) && !input.GetPrev( ae::Key::C ) )
    {
      h = ae::Random( 0.0f, 1.0f );
    }
    ae::Matrix4 modelToWorld = ae::Matrix4::RotationX( r0 ) * ae::Matrix4::RotationZ( r1 );
    uniformList.Set( "u_worldToProj", m_viewToProj * m_worldToView * modelToWorld );
    uniformList.Set( "u_color", ae::Color::HSV( h, 0.5, 0.75 ).GetLinearRGBA() );
    vertexData.Render( &shader, uniformList );

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
  m_sock( tag ),
  m_objects( tag )
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
  if ( m_port == 0 )
  {
    return;
  }
  m_Connect();
  
  uint32_t msgLength = 0;
  uint8_t msgBuffer[ 1024 ];
  while ( ( msgLength = m_sock.ReceiveMsg( msgBuffer, sizeof(msgBuffer) ) ) )
  {
    if ( msgLength > sizeof(msgBuffer) )
    {
      AE_INFO( "Received too large of a message (# bytes)", msgLength );
      m_sock.Disconnect();
      continue;
    }
    
    m_netObjClient.ReceiveData( msgBuffer, msgLength );
    while ( ae::NetObject* netObj = m_netObjClient.PumpCreate() )
    {
      LevelObject* obj = &m_objects.Append( {} );
      obj->netObj = netObj;
      obj->Initialize( netObj->GetInitData(), netObj->InitDataLength() );
    }
  }
  
  for ( LevelObject& object : m_objects )
  {
    if ( object.netObj->SyncDataLength() )
    {
      object.Sync( object.netObj->GetSyncData(), object.netObj->SyncDataLength() );
      object.netObj->ClearSyncData();
    }
  }
  m_objects.RemoveAllFn( [=]( LevelObject& o ) {
    if ( o.netObj->IsPendingDestroy() )
    {
      m_netObjClient.Destroy( o.netObj );
      return true;
    }
    return false; }
  );
}

void EditorClient::RenderHack( ResourceManager* resourceManager, const ae::Matrix4& worldToProj )
{
  for ( const LevelObject& obj : m_objects )
  {
    ae::UniformList uniformList;
    uniformList.Set( "u_localToProj", worldToProj * obj.transform );
    uniformList.Set( "u_localToWorld", obj.transform );
    uniformList.Set( "u_normalToWorld", obj.transform.GetNormalMatrix() );
    uniformList.Set( "u_tex", &resourceManager->Get( TextureId::White ).texture );
    uniformList.Set( "u_color", ae::Color::PicoLightGray().GetLinearRGBA() );

    const MeshResource& meshResource = resourceManager->Get( MeshId::Cube );
    const ShaderResource& shaderResource = resourceManager->Get( ShaderId::Default );
    meshResource.mesh.Render( &shaderResource.shader, uniformList );
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
// Editor picking forward declarations
//------------------------------------------------------------------------------
Entity PickObject( EditorProgram* program, ae::Color color, ae::Vec3* hitOut, ae::Vec3* normalOut );
void ShowEditorObject( EditorProgram* program, Entity entity, ae::Color color );

//------------------------------------------------------------------------------
// EditorConnection member functions
//------------------------------------------------------------------------------
EditorConnection::~EditorConnection()
{
  AE_ASSERT( !sock );
  AE_ASSERT( !netObjConn );
}

void EditorConnection::Destroy( EditorServer* editor )
{
  editor->netObjectServer.DestroyConnection( netObjConn );
  editor->sock.Destroy( sock );
  sock = nullptr;
  netObjConn = nullptr;
}

//------------------------------------------------------------------------------
// EditorServer member functions
//------------------------------------------------------------------------------
void EditorServer::Initialize( EditorProgram* program )
{
  level = ae::New< Level >( TAG_EDITOR );
  //OpenLevel( program );
}

void EditorServer::Terminate( EditorProgram* program )
{
  for ( EditorConnection& conn : connections )
  {
    conn.Destroy( this );
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
    EditorConnection* editorConn = &connections.Append( {} );
    editorConn->sock = newConn;
    editorConn->netObjConn = netObjectServer.CreateConnection();
  }
  AE_ASSERT( connections.Length() == sock.GetConnectionCount() );
  
  netObjectServer.UpdateSendData();
  for ( EditorConnection& conn : connections )
  {
    if ( conn.sock->IsConnected() )
    {
      conn.sock->QueueMsg( conn.netObjConn->GetSendData(), conn.netObjConn->GetSendLength() );
      conn.sock->SendAll();
    }
    else
    {
      AE_INFO( "ae::Editor client #:# disconnected", conn.sock->GetResolvedAddress(), conn.sock->GetPort() );
      conn.Destroy( this );
    }
  }
  connections.RemoveAllFn( []( const EditorConnection& c ){ return !c.sock; } );
  
  uint32_t editorObjectCount = m_objects.Length();
  for ( uint32_t i = 0; i < editorObjectCount; i++ )
  {
    Entity entity = m_objects.GetKey( i );
    EditorObject* editorObject = &m_objects.GetValue( i );
    Transform* transform = &program->registry.GetComponent< Transform >( entity );
    editorObject->netObj->SetSyncData( &transform->transform, sizeof(transform->transform) );
  }
}

void EditorServer::Render( EditorProgram* program )
{
  ae::Matrix4 worldToProj = program->GetWorldToProj();
  uint32_t editorObjectCount = m_objects.Length();
  for ( uint32_t i = 0; i < editorObjectCount; i++ )
  {
    const EditorObject& obj = m_objects.GetValue( i );
    
    ae::UniformList uniformList;
    uniformList.Set( "u_localToProj", worldToProj * obj.transform );
    uniformList.Set( "u_localToWorld", obj.transform );
    uniformList.Set( "u_normalToWorld", obj.transform.GetNormalMatrix() );
    uniformList.Set( "u_tex", &program->resourceManager->Get( TextureId::White ).texture );
    uniformList.Set( "u_color", ae::Color::PicoLightGray().GetLinearRGBA() );

    const MeshResource& meshResource = program->resourceManager->Get( MeshId::Cube );
    const ShaderResource& shaderResource = program->resourceManager->Get( ShaderId::Default );
    meshResource.mesh.Render( &shaderResource.shader, uniformList );
  }
}

void EditorServer::ShowUI( EditorProgram* program )
{
  if ( m_toHide )
  {
    ImGui::SetNextItemOpen( false );
    m_toHide = false;
  }

  m_SetActive( program, true );
  
  float dt = program->GetDt();
  
  ae::Color cursorColor = ae::Color::PicoGreen();
  ae::Vec3 mouseHover( 0.0f );
  ae::Vec3 mouseHoverNormal( 0, 1, 0 );
  Entity pickedEntity = PickObject( program, cursorColor, &mouseHover, &mouseHoverNormal );
  
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
  
  ShowEditorObject( program, selected, ae::Color::Green() );
  if ( pickedEntity )
  {
    program->debugLines.AddCircle( mouseHover + mouseHoverNormal * 0.025f, mouseHoverNormal, 0.5f, cursorColor, 8 );
    if ( selected != pickedEntity )
    {
      ShowEditorObject( program, pickedEntity, ae::Color::PicoDarkGray() );
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
    program->camera.Refocus( program->registry.GetComponent< Transform >( selected ).GetPosition() );
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
    
    ImGuizmo::Manipulate(
      program->GetWorldToView().data,
      program->GetViewToProj().data,
      gizmoOperation,
      ( gizmoOperation == ImGuizmo::SCALE ) ? ImGuizmo::LOCAL : gizmoMode,
      program->registry.GetComponent< Transform >( selected ).transform.data
    );
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
      Transform* transform = program->registry.AddComponent< Transform >( entity );
      transform->transform = ae::Matrix4::Translation( program->camera.GetFocus() );
      selected = entity;
      
      EditorObject* editorObject = &m_objects.Set( entity, {} );
      editorObject->netObj = netObjectServer.CreateNetObject();
      editorObject->netObj->SetInitData( &transform->transform, sizeof(transform->transform) );
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
      ImGui::Text( "Entity %u", selected );
    
      char name[ ae::Str16::MaxLength() ];
      strcpy( name, program->registry.GetNameByEntity( selected ) );
      if ( ImGui::InputText( "Name", name, countof(name), ImGuiInputTextFlags_EnterReturnsTrue ) )
      {
        AE_INFO( "Set entity name: #", name );
        program->registry.SetEntityName( selected, name );
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
          if ( type->IsType< Transform >() )
          {
            bool changed = false;
            ae::Matrix4 temp = program->registry.GetComponent< Transform >( selected ).transform;
            float matrixTranslation[ 3 ], matrixRotation[ 3 ], matrixScale[ 3 ];
            ImGuizmo::DecomposeMatrixToComponents( temp.data, matrixTranslation, matrixRotation, matrixScale );
            changed |= ImGui::InputFloat3( "translation", matrixTranslation );
            changed |= ImGui::InputFloat3( "rotation", matrixRotation );
            changed |= ImGui::InputFloat3( "scale", matrixScale );
            if ( changed )
            {
              ImGuizmo::RecomposeMatrixFromComponents( matrixTranslation, matrixRotation, matrixScale, temp.data );
              program->registry.GetComponent< Transform >( selected ).transform = temp;
              
              for ( EditorConnection& conn : connections )
              {
                conn.sock->QueueMsg( &temp, sizeof(temp) );
              }
            }
          }
          else
          {
            uint32_t varCount = type->GetVarCount();
            for ( uint32_t i = 0; i < varCount; i++ )
            {
              m_ShowVar( program, component, type->GetVarByIndex( i ) );
            }
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
    static const ae::Type* s_selectedType = ae::GetType< Transform >();
    if ( ImGui::BeginCombo( "Type", s_selectedType->GetName(), 0 ) )
    {
      uint32_t componentTypesCount = program->registry.GetTypeCount();
      for ( uint32_t i = 0; i < componentTypesCount; i++ )
      {
        const ae::Type* type = program->registry.GetTypeByIndex( i );
        const bool isSelected = ( s_selectedType == type );
        if ( ImGui::Selectable( type->GetName(), isSelected ) )
        {
          s_selectedType = type;
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
      int32_t typeIndex = program->registry.GetTypeIndexByType( s_selectedType );
      if ( typeIndex >= 0 )
      {
        uint32_t componentCount = program->registry.GetComponentCountByIndex( typeIndex );
        for ( int i = 0; i < componentCount; i++ )
        {
          ImGui::PushID( i );
          const Component& c = program->registry.GetComponentByIndex( typeIndex, i );
          const bool isSelected = ( c.GetEntity() == selected );
          ae::Str16 name = c.GetEntityName();
          if ( !name.Length() )
          {
            name = ae::Str16::Format( "#", c.GetEntity() );
          }
          
          if ( ImGui::Selectable( name.c_str(), isSelected ) )
          {
            selected = c.GetEntity();
          }
          if ( isSelected )
          {
            ImGui::SetItemDefaultFocus();
          }
          ImGui::PopID();
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
      if ( level->Load( &program->registry, true ) )
      {
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

void EditorServer::m_SetActive( EditorProgram* program, bool enabled )
{
  if ( enabled && !m_active )
  {
    AE_INFO( "Editor Enable" );
    m_active = true;
    level->Load( &program->registry, true );
  }
  else if ( !enabled && m_active )
  {
    AE_INFO( "Editor Disable" );
    m_active = false;
    level->Save( &program->registry );
    
    SetOpen( false );
  }
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
Entity PickObject( EditorProgram* program, ae::Color color, ae::Vec3* hitOut, ae::Vec3* normalOut )
{
  ae::Vec3 camPos = program->camera.GetPosition();
  ae::Vec3 mouseRay = program->GetMouseRay();
  
  ae::CollisionMesh::RaycastResult result;
  uint32_t modelCount = program->registry.GetComponentCount< ModelComponent >();
  for ( uint32_t i = 0; i < modelCount; i++ )
  {
    Entity entity = program->registry.GetEntityByIndex< ModelComponent >( i );
    const ModelComponent* model = &program->registry.GetComponentByIndex< ModelComponent >( i );
    if ( model->GetVisible() )
    {
      const MeshResource* mesh = &model->GetMesh( program->resourceManager );
      ae::CollisionMesh::RaycastParams params;
      params.userData = model;
      params.source = camPos;
      params.direction = mouseRay;
      params.transform = model->GetModelToWorld( program->resourceManager );
      params.hitClockwise = true;
      params.hitCounterclockwise = true;
      result = mesh->collision.Raycast( params, result );
    }
  }
  if ( result.hitCount )
  {
    *hitOut = result.hits[ 0 ].position;
    *normalOut = result.hits[ 0 ].normal;
    const ModelComponent* model = (const ModelComponent*)result.hits[ 0 ].userData;
    AE_ASSERT( model );
    return model->GetEntity();
  }
  
  return kInvalidEntity;
}

void ShowEditorObject( EditorProgram* program, Entity entity, ae::Color color )
{
  if ( entity )
  {
    const ModelComponent* model = program->registry.TryGetComponent< ModelComponent >( entity );
    if ( model )
    {
      program->debugLines.AddOBB( model->GetOBB( program->resourceManager ).GetTransform(), color );
    }
  }
}

}
