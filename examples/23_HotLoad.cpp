//------------------------------------------------------------------------------
// 23_HotLoad.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2023 John Hughes
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
#include "23_HotLoad.h"

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
	ae::FileSystem fileSystem;
	fileSystem.Initialize( "", "ae", "23_HotLoad" );

	// Lib path
	ae::Str256 libResourcePath;
	ae::Str64 fileName = "HotLoadable.dylib";
	fileSystem.GetAbsolutePath( ae::FileSystem::Root::Data, fileName.c_str(), &libResourcePath );

	ae::Str256 buildCmd;
	ae::Str256 postBuildCmd;
	if ( ae::IsDebuggerAttached() )
	{
		ae::Str256 cmakeBuildDir;
		fileSystem.GetAbsolutePath( ae::FileSystem::Root::Bundle, "..", &cmakeBuildDir ); // Root of CMake build dir
		ae::Str256 libBuildPath = cmakeBuildDir;
		ae::FileSystem::AppendToPath( &libBuildPath, "HotLoadable.dylib" );

		ae::HotLoader::GetCMakeBuildCommand( &buildCmd, cmakeBuildDir.c_str(), "HotLoadable" );
		ae::HotLoader::GetCopyCommand( &postBuildCmd, libResourcePath.c_str(), libBuildPath.c_str() );
	}

	Game game = { fileSystem };
	ae::HotLoader hotLoader;
	hotLoader.Initialize( buildCmd.c_str(), postBuildCmd.c_str(), libResourcePath.c_str() );
	hotLoader.CallFn< GameFn >( "Game_Initialize", &game );
	while ( hotLoader.CallFn< GameFn >( "Game_Update", &game ) )
	{
		if ( game.input.GetPress( ae::Key::R ) )
		{
			AE_INFO( "Reloading" );
			hotLoader.Reload();
		}
	};

	return 0;
}
