cmake_minimum_required(VERSION 3.16)

project(BlokusAI DESCRIPTION "Blokus Artificial Intelligence project" LANGUAGES CXX)

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

option(ENABLE_BLOCKUS_PROJECT "Check it to generate the Blokus project." ON)
option(ENABLE_7W_PROJECT "Check it to generate the 7W project." ON)
option(BUILD_UI "Check it to generate the Blokus UI projects." OFF)

option(BUILD_ML_PROJECTS "Check it to generate the Blokus ML projects." ON)
if(BUILD_ML_PROJECTS)
	find_package(Torch REQUIRED)
endif()

# Libraries code
add_subdirectory(src)

# Executables code
add_subdirectory(apps)

# Tests code
add_subdirectory(tests)