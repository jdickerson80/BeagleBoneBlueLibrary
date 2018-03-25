cmake_minimum_required( VERSION 2.8 )

# Name the project
project( BeagleBoneBlue )

# Find all of the libraries needed to build project
find_package( Threads )

# Set the active build type
set( CMAKE_BUILD_TYPE Release )
#set( CMAKE_BUILD_TYPE Debug )

# Add compiler flags
set( CMAKE_CXX_FLAGS "-Wall" )

# Bring in the project headers
include_directories( ${CMAKE_SOURCE_DIR}/Headers/ )
include_directories( ${CMAKE_SOURCE_DIR}/Headers/Bluetooth )
include_directories( ${CMAKE_SOURCE_DIR}/Headers/IO )
include_directories( ${CMAKE_SOURCE_DIR}/Headers/Math )
include_directories( ${CMAKE_SOURCE_DIR}/Headers/MotorControl )
include_directories( ${CMAKE_SOURCE_DIR}/Headers/PRU )

# Add all of the files to the build
file( GLOB_RECURSE Headers ${CMAKE_SOURCE_DIR}/Headers/*.h )
file( GLOB_RECURSE Sources ${CMAKE_SOURCE_DIR}/Sources/*.cpp )

# Add the executable output
add_library( ${PROJECT_NAME} STATIC ${Headers} ${Sources} )

# Setup all of the link libraries
target_link_libraries( ${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} )

