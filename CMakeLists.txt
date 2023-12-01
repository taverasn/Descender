cmake_minimum_required(VERSION 3.16)

project(Descender)

# Great way to reduce compile times for large files like Gateware.h 
# https://edgarluque.com/blog/cmake-precompiled-headers/
set(PRE_COMPILED
    ./Source/precompiled.h 
)
# Create list of source files.
# CMake generally discourages the use of GLOB. (it can make doing more complex operations fail)
# For students the conveinece of not editing the CMake constantly is probably more important.  
file(
	GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
	./Source/*.h
	./Source/*.cpp
    ./Shaders/*.glsl
	./gateware-main/Gateware.h
	./tinyxml2/tinyxml2.cpp
	./tinyxml2/tinyxml2.h
	./stb/stb_image.h
	./flecs-3.1.4/flecs.h
    ./flecs-3.1.4/flecs.c
	./inifile-cpp-master/include/inicpp.h
	./defaults.ini # get some syntax highlighting
)
# The FLECS library core is written in C. If it tries to use the C++ pre-compiled header it will not compile.
set_source_files_properties(./flecs-3.1.4/flecs.c PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
# Example: You can use this to remove files so later you can add them individually if needed.
#list(FILTER SOURCE_FILES EXCLUDE REGEX ".*main.*.cpp$")


# currently using unicode in some libraries on win32 but will change soon
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

if (WIN32)
	# by default CMake selects "ALL_BUILD" as the startup project 
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
		 PROPERTY VS_STARTUP_PROJECT Descender)

	add_executable (Descender ${SOURCE_FILES})
	source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${SOURCE_FILES})

	include_directories(${OPENGL_INCLUDE_DIR}) 
	link_libraries(${OPENGL_LIBRARIES})

	# add support for dds texture loading
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/freetype/include)
	# Find the libraries
	find_library(FREETYPE_LIB_D NAMES freetyped PATHS ${CMAKE_CURRENT_SOURCE_DIR}/freetype/build/Debug)
	find_library(FREETYPE_LIB_R NAMES freetype PATHS ${CMAKE_CURRENT_SOURCE_DIR}/freetype/build/Release)
	# link the ktx sdk include and lib files
	target_link_libraries(Descender debug ${FREETYPE_LIB_D} optimized ${FREETYPE_LIB_R})
	message(STATUS "Copying freetyped.dll")
	add_custom_command(TARGET Descender POST_BUILD      # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${CMAKE_CURRENT_SOURCE_DIR}/freetype/build/Debug/freetyped.dll"      # <--this is in-file
        $<TARGET_FILE_DIR:Descender>)                 # <--this is out-file path
	message(STATUS "Copying freetype.dll")
	add_custom_command(TARGET Descender POST_BUILD     # Adds a post-build event to MyTest
		COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
			"${CMAKE_CURRENT_SOURCE_DIR}/freetype/build/Release/freetype.dll"      # <--this is in-file
			$<TARGET_FILE_DIR:Descender>)                 # <--this is out-file path

	
	# shaderc_combined.lib in Vulkan requires this for debug & release (runtime shader compiling)
	# IMPORTANT! Below is the OLD way of setting the compiler options it does NOT work with pre-compiled headers!
	#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MD") // DO NOT USE
endif(WIN32)

if(UNIX AND NOT APPLE)
	# libshaderc_combined.a is required for runtime shader compiling
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lX11 -lGL")
    find_package(X11)
	find_package(OpenGL)
    link_libraries(${X11_LIBRARIES})
    include_directories(${X11_INCLUDE_DIR})
    include_directories(${OPENGL_INCLUDE_DIR}) 
	link_libraries(${OPENGL_LIBRARIES})
endif(UNIX AND NOT APPLE)

if(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fmodules -fcxx-modules")
	set(Architecture ${CMAKE_OSX_ARCHITECTURES})
	find_package(OpenGL)
	include_directories(${OPENGL_INCLUDE_DIR}) 
	link_libraries(${OPENGL_LIBRARIES})
	add_executable (Descender source/main.mm)
endif(APPLE)

# using some c++17 filesystem features to grab file timestamps
# most code other code in the project only requires c++11
# we could remove this if we had to, but most compilers can do 17 these days
target_compile_features(Descender PUBLIC cxx_std_17)

# adding gateware.h and other librairies as a precompiled headers to reduce compile times
target_precompile_headers(Descender PRIVATE ${PRE_COMPILED})


set_source_files_properties( ${SHADER_FILES} PROPERTIES
        VS_SHADER_TYPE Vertex # not all are vertex shaders, but that's ok here
        VS_SHADER_MODEL 5.1
        VS_SHADER_ENTRYPOINT main
        VS_TOOL_OVERRIDE "None" # stop VS from compiling, we will do it
)
