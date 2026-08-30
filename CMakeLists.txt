cmake_minimum_required( VERSION 4.3 )

project( Trijam299Proj )

add_executable( Trijam299 )

target_include_directories( Trijam299 PRIVATE raylib/include )
target_link_libraries( Trijam299 "${CMAKE_CURRENT_SOURCE_DIR}/raylib/lib/raylib.lib" winmm )

set_property( TARGET Trijam299 PROPERTY CXX_STANDARD 23 )
set_property( TARGET Trijam299 PROPERTY DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/run" )
set_property( TARGET Trijam299 PROPERTY CMAKE_CXX_SCAN_FOR_MODULES OFF )

# Enable Edit & Continue
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	target_compile_options(Trijam299 PUBLIC "/ZI")
	target_link_options(Trijam299 PUBLIC "/INCREMENTAL")
endif()

add_subdirectory( Trijam299 )
