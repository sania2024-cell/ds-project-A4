cmake_minimum_required(VERSION 3.15)
project(PropertyFinder VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type to Release by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Find required packages
find_package(Threads REQUIRED)

# Include directories
include_directories(src)

# Source files
set(SOURCES
    src/main.cpp
    src/property.cpp
    src/property_manager.cpp
    src/csv_parser.cpp
    src/search_engine.cpp
    src/ml_predictor.cpp
    src/api_server.cpp
)

# Header files
set(HEADERS
    src/property.h
    src/property_manager.h
    src/csv_parser.h
    src/search_engine.h
    src/ml_predictor.h
    src/api_server.h
    src/utils.h
)

# Create executable
add_executable(property_finder ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(property_finder Threads::Threads)

# Compiler-specific options
if(MSVC)
    target_compile_options(property_finder PRIVATE /W4)
else()
    target_compile_options(property_finder PRIVATE -Wall -Wextra -pedantic)
endif()

# Optional: Find and link mlpack if available
find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(MLPACK mlpack)
    if(MLPACK_FOUND)
        target_include_directories(property_finder PRIVATE ${MLPACK_INCLUDE_DIRS})
        target_link_libraries(property_finder ${MLPACK_LIBRARIES})
        target_compile_definitions(property_finder PRIVATE HAVE_MLPACK)
    endif()
endif()

# Installation
install(TARGETS property_finder DESTINATION bin)

# Custom targets
add_custom_target(run
    COMMAND property_finder
    DEPENDS property_finder
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
)

add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
)

# Testing (optional)
enable_testing()
add_test(NAME property_finder_test COMMAND property_finder --test)