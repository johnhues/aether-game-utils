cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(AE_CURRENT_BUNDLE_NAME "Hello World")
set(AE_CURRENT_EXECUTABLE_NAME 00_hello_world)
set(AE_CURRENT_SOURCE_FILES 00_HelloWorld.cpp)
include(AddExecutable)

set(AE_CURRENT_BUNDLE_NAME "Example")
set(AE_CURRENT_EXECUTABLE_NAME 01_example)
set(AE_CURRENT_SOURCE_FILES 01_Example.cpp)
include(AddExecutable)

set(AE_CURRENT_BUNDLE_NAME "Sprites")
set(AE_CURRENT_EXECUTABLE_NAME 02_sprites)
set(AE_CURRENT_SOURCE_FILES 02_Sprites.cpp circle.png)
set(AE_RESOURCES circle.png)
include(AddExecutable)

set(AE_CURRENT_BUNDLE_NAME "Client")
set(AE_CURRENT_EXECUTABLE_NAME 03_client)
set(AE_CURRENT_SOURCE_FILES 03_Client.cpp)
include(AddExecutable)

set(AE_CURRENT_BUNDLE_NAME "Server")
set(AE_CURRENT_EXECUTABLE_NAME 03_server)
set(AE_CURRENT_SOURCE_FILES 03_Server.cpp)
include(AddExecutable)

set(AE_CURRENT_BUNDLE_NAME "Replication Client")
set(AE_CURRENT_EXECUTABLE_NAME 04_replication_client)
set(AE_CURRENT_SOURCE_FILES 04_ReplicationClient.cpp 04_ReplicationCommon.h)
include(AddExecutable)

set(AE_CURRENT_BUNDLE_NAME "Replication Server")
set(AE_CURRENT_EXECUTABLE_NAME 04_replication_server)
set(AE_CURRENT_SOURCE_FILES 04_ReplicationServer.cpp 04_ReplicationCommon.h)
include(AddExecutable)

set(AE_CURRENT_BUNDLE_NAME "Text Input")
set(AE_CURRENT_EXECUTABLE_NAME 05_text_input)
set(AE_CURRENT_SOURCE_FILES 05_TextInput.cpp font.png)
set(AE_RESOURCES font.png)
include(AddExecutable)
