cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# use c++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# use unicode
add_definitions(-D_UNICODE -DUNICODE)

add_executable(example example.cpp)
add_dependencies(example ${AE_LIBRARY_NAME})
target_link_libraries(example ${AE_LIBRARIES})

add_executable(server server.cpp)
add_dependencies(server ${AE_LIBRARY_NAME})
target_link_libraries(server ${AE_LIBRARIES})

add_executable(client client.cpp)
add_dependencies(client ${AE_LIBRARY_NAME})
target_link_libraries(client ${AE_LIBRARIES})

add_executable(replication_server ReplicationServer.cpp ReplicationCommon.h)
add_dependencies(replication_server ${AE_LIBRARY_NAME})
target_link_libraries(replication_server ${AE_LIBRARIES})

add_executable(replication_client ReplicationClient.cpp ReplicationCommon.h)
add_dependencies(replication_client ${AE_LIBRARY_NAME})
target_link_libraries(replication_client ${AE_LIBRARIES})
