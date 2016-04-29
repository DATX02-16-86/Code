cmake_minimum_required(VERSION 3.3)
project(Demo)

# FIND OPENGL
#########################################################
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})
if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)

#########################################################

if(WIN32)
set(BIN_DIR "Win32")

include_directories(${GLUT_SRC}/include)
add_subdirectory(${GLUT_SRC} "${CMAKE_CURRENT_BINARY_DIR}/freeglut")
set(GLUT_LIBRARIES freeglut)

elseif(APPLE)
set(BIN_DIR "OSX")
find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIRS})
link_directories(${GLUT_LIBRARY_DIRS})
add_definitions(${GLUT_DEFINITIONS})
else()
set(BIN_DIR "Linux")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../Bin/${BIN_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../Bin/${BIN_DIR}")

set(GLUT_SRC "" CACHE FILEPATH "Where is glut?")

include_directories(../Libraries/Tritium/Code/Tritium/Core)
include_directories(../Libraries/Tritium/Code/Tritium)
include_directories(../Libraries)
include_directories(../Generator/Code)

link_directories(../Libraries/Tritium/Bin/Release)

add_executable(Demo ../noise/Simplex/simplex.h ../noise/Simplex/simplex.cpp Demo.cpp)
target_link_libraries(Demo Generator TritiumCore ${OTHER_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})