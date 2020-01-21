# reference catch2 (clone https://github.com/catchorg/Catch2.git and make install)
find_package(Catch2 REQUIRED)

add_executable(
  unit_tests
  SignalTest.cpp
  test.cpp
)

target_link_libraries(
  unit_tests
  ${AE_LIBRARY_NAME}
  Catch2::Catch2
)

add_custom_command(
  TARGET unit_tests
  COMMENT "Run tests"
  POST_BUILD 
  COMMAND unit_tests
)

