# Editor {#editor}

ae::Editor is a level editor that runs as a second instance of your program, beside the game. Levels are JSON files of entities and their ae::Component variables. The game connects to the editor over a local TCP port, receives variable edits as they happen, and reloads the level when asked. It lives in `extras/Editor.cpp` and `extras/include/ae/Editor.h`, and draws its interface with Dear ImGui and ImGuizmo.

\demo{examples/18_SmallEngine,21/9}

Click the game and press `~` to open the editor beside it. Select objects in the viewport or the object list, then move, rotate and scale them with `W`, `E` and `R`. Transform and variable edits appear in the game immediately. Created and deleted objects reach the game when the level is saved and File › Game Load reloads it, which needs a page that stores files; this one does not, see [Web](#editor_web). The example is [`18_SmallEngine.cpp`](https://github.com/johnhues/aether-game-utils/blob/main/examples/18_SmallEngine.cpp) and [`18_Game.cpp`](https://github.com/johnhues/aether-game-utils/blob/main/examples/18_Game.cpp).

## Setup {#editor_setup}
Initialize the editor before anything else. In the editor instance ae::Editor::Initialize() runs the editor and returns true once it closes, so the program should exit. In the game it returns false.
```cpp
ae::EditorParams params( argc, argv, &registry );
editor.AddPlugin< MyEditorPlugin >( this );
if( editor.Initialize( params ) )
{
	return false; // The editor ran and closed
}
editor.QueueRead( "data/example.level" );
```
Call ae::Editor::Update() each frame to receive edits and level loads into the registry, and ae::Editor::Launch() to open the editor, which connects to it instead when it is already open. ae::EditorParams::port selects the port, and 0 disables the connection.

## Components {#editor_components}
Every ae::Component type with an ae::EditorTypeAttribute can be added to entities in the editor, and each of its registered variables is shown and saved.
```cpp
AE_REGISTER_CLASS( Mesh );
AE_REGISTER_NAMESPACECLASS_ATTRIBUTE( (Mesh), (ae, EditorTypeAttribute), {} );
AE_REGISTER_CLASS_VAR( Mesh, name );
AE_REGISTER_NAMESPACECLASS_VAR_ATTRIBUTE( (Mesh), name, (ae, EditorMeshResourceAttribute), {} );
```
ae::EditorRequiredAttribute adds other components alongside a component, ae::EditorDisplayNameAttribute names an entity after a variable, and ae::EditorVisibilityAttribute shows or hides it with a bool variable.

## Plugins {#editor_plugins}
An ae::EditorPlugin receives ae::EditorEvent callbacks in both instances: level and component changes in the editor, and the `Live*` events in the game. ae::MeshEditorPlugin draws components with an ae::EditorMeshResourceAttribute variable as meshes, loaded by its TryLoad(). TryLoad() can return an empty ae::Optional while a file loads, and is called again each frame until it returns a mesh. Register each plugin type with AE_REGISTER_EDITOR_PLUGIN() so ae::Editor::HotLoad() can repair it.

## Web {#editor_web}
On the web the editor is another instance on the same page. ae::Editor::Launch() asks the page for it through `InstantiateOptions.onLaunch`, see \ref aether_js, and the two instances connect by port through a stream that aether.js provides between instances on the same page. Saving writes through ae::FileSystem::Write(), which hands the file to `InstantiateOptions.onWrite`, and fails when the page does not provide it. The examples page starts launched instances on a new canvas to the left of the first, with a divider between them to drag, and does not store files.
