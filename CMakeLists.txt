# We need at least 3.12 for list(JOIN....), add_compile_definitions
# 3.15 for CMAKE_CXX_COMPILER_FRONTEND_VARIANT
cmake_minimum_required(VERSION 3.15)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(PREDEFINED_TARGETS_FOLDER "CustomTargets")

project(Wbigame LANGUAGES CXX C ASM)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_UNIT_TESTS "Build The-Forge's unit tests" ON)
option(BUILD_ASSETPIPELINECMD "Build The-Forge's asset pipeline command" ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

find_package(GameNetworkingSockets)
include(TheForge)
include(TheForgeUtils)
include(SpirVTools)

add_executable(Wbigame
    "Src/Client/WBIGame.cpp"
	"${CMAKE_CURRENT_BINARY_DIR}/gen/MessageTypes.pb.cc"
	"Src/Client/ClientNetwork/ClientNetwork.cpp"
)

set_target_properties(Wbigame PROPERTIES LINK_FLAGS /MACHINE:X64)
target_include_directories(Wbigame PRIVATE "${CMAKE_CURRENT_LIST_DIR}/Vendor/TheForge"
	"${CMAKE_CURRENT_LIST_DIR}/Vendor/TheForge/Common_3" 
	"${CMAKE_CURRENT_LIST_DIR}/Vendor/TheForge/Tools"
	"${CMAKE_CURRENT_LIST_DIR}/Src"
	"${CMAKE_CURRENT_BINARY_DIR}/gen"
)
target_link_libraries(Wbigame PRIVATE GameNetworkingSockets::static Xinput9_1_0 ws2_32 OS Renderer)

tf_add_shader(Wbigame ${CMAKE_SOURCE_DIR}/src/Shaders/FSL/ShaderList.fsl)
tf_add_forge_utils(Wbigame)

add_executable(Wbiserver
	"${CMAKE_CURRENT_BINARY_DIR}/gen/MessageTypes.pb.cc"
	"Src/Server/ServerGameSystem.cpp"
	"Src/Server/ServerNetwork/ServerNetwork.cpp"
    "Src/Server/Server.cpp"
)

set_target_properties(Wbiserver PROPERTIES LINK_FLAGS /MACHINE:X64)
target_include_directories(Wbiserver PRIVATE "${CMAKE_CURRENT_LIST_DIR}/Vendor/TheForge"
	"${CMAKE_CURRENT_LIST_DIR}/Vendor/TheForge/Common_3" 
	"${CMAKE_CURRENT_LIST_DIR}/Vendor/TheForge/Tools"
	"${CMAKE_CURRENT_LIST_DIR}/Src"
	"${CMAKE_CURRENT_BINARY_DIR}/gen"
)
target_link_libraries(Wbiserver PRIVATE GameNetworkingSockets::static ws2_32 OS)

set_property(TARGET OS PROPERTY FOLDER TheForge)
set_property(TARGET Renderer PROPERTY FOLDER TheForge)
set_property(TARGET SpirVTools PROPERTY FOLDER TheForge)

set_property(TARGET -Animation_ShaderList PROPERTY FOLDER TheForgeShaders)
set_property(TARGET -Fonts_ShaderList PROPERTY FOLDER TheForgeShaders)
set_property(TARGET -ShaderList PROPERTY FOLDER TheForgeShaders)
set_property(TARGET -UI_ShaderList PROPERTY FOLDER TheForgeShaders)

set_property(TARGET ozz_animation PROPERTY FOLDER Ozz)
set_property(TARGET ozz_animation_offline PROPERTY FOLDER Ozz)
set_property(TARGET ozz_base PROPERTY FOLDER Ozz)

set(VS_STARTUP_PROJECT Wbigame)
set_property(GLOBAL PROPERTY VS_STARTUP_PROJECT Wbigame)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Wbigame)
set_property(TARGET Wbigame PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$(ProjectDir)$(Configuration)")




