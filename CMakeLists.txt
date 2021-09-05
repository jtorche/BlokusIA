cmake_minimum_required(VERSION 3.8)

project(BlokusIA DESCRIPTION "Blokus Artificial Intelligence project" LANGUAGES CXX)

# Only do these if this is the main project, and not if it is included through
# add_subdirectory
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # C++ Standard
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    # Ensure -std=c++xx instead of -std=g++xx
    set(CMAKE_CXX_EXTENSIONS OFF)

    # Nicely support folders in IDE's
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

# Libraries code
add_subdirectory(src)

# Executables code
add_subdirectory(apps)

# Tests code
add_subdirectory(tests)