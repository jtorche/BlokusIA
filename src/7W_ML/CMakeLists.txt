cmake_minimum_required(VERSION 3.8)

if (BUILD_ML_PROJECTS AND ENABLE_7W_PROJECT)
	project(ML DESCRIPTION "7W ML library" LANGUAGES CXX)
	
	# Note that headers are optional, and do not affect add_library, but they will
	# not show up in IDEs unless they are listed in add_library.
	file(GLOB_RECURSE ml_headers ${CMAKE_SOURCE_DIR}/include/7W_ML/*.h)
	file(GLOB_RECURSE ml_sources *.cpp)
	
	add_library(${PROJECT_NAME} ${ml_headers} ${ml_sources})
	
	# We need this directory, and users of our library will need it too
	target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)
	
	# Enabled warnings
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W3 /wd4100 /wd4624>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>)
		
	# For nicer IDE views
	set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
	
	# Linked libraries
	target_link_libraries(${PROJECT_NAME} PRIVATE Core AI "${TORCH_LIBRARIES}")
	
endif()