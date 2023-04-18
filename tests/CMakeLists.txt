option(BUILD_TESTING "Check it to generate the Blokus tests projects." ON)

if(BUILD_TESTING)

	if(ENABLE_BLOCKUS_PROJECT)
		add_subdirectory(Blockus)
	endif()

	if(ENABLE_7W_PROJECT)
		add_subdirectory(Test7W)
	endif()
	
endif()