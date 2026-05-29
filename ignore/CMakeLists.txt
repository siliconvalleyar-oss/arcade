###############################################################################

cmake_minimum_required (VERSION 3.9.5)

project(app)

# app binary output folders
set(RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add include directories
include_directories("${PROJECT_SOURCE_DIR}")

# Include all source files into the project
file(GLOB_RECURSE ${PROJECT_NAME}_SRCFILES
	"${PROJECT_SOURCE_DIR}/src/*.cpp"
	"${PROJECT_SOURCE_DIR}/src/cmp/*.cpp"
	"${PROJECT_SOURCE_DIR}/src/man/*.cpp"
	"${PROJECT_SOURCE_DIR}/src/sys/*.cpp"
	"${PROJECT_SOURCE_DIR}/lib/picoPNG/src/*.cpp"
)

# TinyPTC source files (only Xlib target is enabled)
set(TINYPTC_SOURCES
	"${PROJECT_SOURCE_DIR}/lib/tinyPTC/src/xlib.c"
	"${PROJECT_SOURCE_DIR}/lib/tinyPTC/src/convert.c"
)

# Executable to be produced with required libraries
add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRCFILES} ${TINYPTC_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE
	"${PROJECT_SOURCE_DIR}"
	"${PROJECT_SOURCE_DIR}/src"
)
target_link_libraries(${PROJECT_NAME} X11 Xext)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
