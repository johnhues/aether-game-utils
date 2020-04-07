cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(AE_CURRENT_BUNDLE_NAME "Example")
set(AE_CURRENT_EXECUTABLE_NAME example)
set(AE_CURRENT_SOURCE_FILES example.cpp)
include(AddExecutable)

set(AE_CURRENT_BUNDLE_NAME "Sprites")
set(AE_CURRENT_EXECUTABLE_NAME sprites)
set(AE_CURRENT_SOURCE_FILES sprites.cpp)
include(AddExecutable)

set(AE_CURRENT_BUNDLE_NAME "Server")
set(AE_CURRENT_EXECUTABLE_NAME server)
set(AE_CURRENT_SOURCE_FILES server.cpp)
include(AddExecutable)

set(AE_CURRENT_BUNDLE_NAME "Client")
set(AE_CURRENT_EXECUTABLE_NAME client)
set(AE_CURRENT_SOURCE_FILES client.cpp)
include(AddExecutable)

set(AE_CURRENT_BUNDLE_NAME "Replication Server")
set(AE_CURRENT_EXECUTABLE_NAME replication_server)
set(AE_CURRENT_SOURCE_FILES ReplicationServer.cpp ReplicationCommon.h)
include(AddExecutable)

set(AE_CURRENT_BUNDLE_NAME "Replication Client")
set(AE_CURRENT_EXECUTABLE_NAME replication_client)
set(AE_CURRENT_SOURCE_FILES ReplicationClient.cpp ReplicationCommon.h)
include(AddExecutable)

set(AE_CURRENT_BUNDLE_NAME "Text Input")
set(AE_CURRENT_EXECUTABLE_NAME text_input)
set(AE_CURRENT_SOURCE_FILES 05_TextInput.cpp font.png)
include(AddExecutable)
FILE(COPY font.png DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
