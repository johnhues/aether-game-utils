cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# rapidjson
FetchContent_Declare(
	rapidjson
	GIT_REPOSITORY "https://github.com/Tencent/rapidjson.git"
	GIT_TAG v1.1.0
)
FetchContent_Populate(rapidjson) # Download but don't build
file(GLOB_RECURSE rapidjson_SOURCES CONFIGURE_DEPENDS "${rapidjson_SOURCE_DIR}/include/*")
add_library(rapidjson INTERFACE ${rapidjson_SOURCES}) # Header only
target_include_directories(rapidjson INTERFACE "${rapidjson_SOURCE_DIR}/include")

# openfbx
FetchContent_Declare(
	openfbx
	GIT_REPOSITORY https://github.com/nem0/OpenFBX.git
	GIT_TAG 5ce5c3490a90d346366abcda48e1d77526d09a15
)
FetchContent_Populate(openfbx) # Download but don't build
FetchContent_GetProperties(openfbx SOURCE_DIR OPENFBX_SOURCE_DIR)
file(GLOB_RECURSE OPENFBX_SOURCES CONFIGURE_DEPENDS "${OPENFBX_SOURCE_DIR}/src/*")
add_library(openfbx ${OPENFBX_SOURCES})
target_include_directories(openfbx INTERFACE "${OPENFBX_SOURCE_DIR}/src")

# Editor
file(GLOB_RECURSE AE_EDITOR_HEADERS CONFIGURE_DEPENDS "src/*.h")
file(GLOB_RECURSE AE_EDITOR_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE AE_EDITOR_RESOURCES CONFIGURE_DEPENDS "data/*")
add_bundle("Editor"
	"16_editor"
	"ae.16_editor"
	"0.0.0"
	"Icon"
	"${AE_EDITOR_HEADERS};${AE_EDITOR_SOURCES}"
	"data/Icon.icns;${AE_EDITOR_RESOURCES}"
	"ae;imgui;imguizmo;openfbx;rapidjson"
	""
)
