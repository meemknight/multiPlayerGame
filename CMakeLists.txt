#https://github.com/meemknight/cmakeSetup

cmake_minimum_required(VERSION 3.16)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug$<$<CONFIG:Debug>:Debug>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")

project(multyPlayer)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/glfw-3.3.2)
add_subdirectory(thirdparty/glad)
add_subdirectory(thirdparty/stb_image)
add_subdirectory(thirdparty/stb_truetype)
add_subdirectory(thirdparty/gl2d)
add_subdirectory(thirdparty/raudio)
add_subdirectory(thirdparty/glm)
add_subdirectory(thirdparty/imgui-docking)
add_subdirectory(thirdparty/glUI)
add_subdirectory(thirdparty/enet-1.3.17)
add_subdirectory(thirdparty/safeSave)


# Define MY_SOURCES to be a list of all the source files for my game B
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE MY_SOURCES_COMMON CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/common/*.cpp")

add_executable(multyPlayer)

set_property(TARGET multyPlayer PROPERTY CXX_STANDARD 17)

target_compile_definitions(multyPlayer PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/") # This is useful to get an ASSETS_PATH in your IDE during development but you should comment this if you compile a release version and uncomment the next line
#target_compile_definitions(multyPlayer PUBLIC RESOURCES_PATH="./resources/") # Uncomment this line to setup the ASSETS_PATH macro to the final assets directory when you share the game

target_sources(multyPlayer PRIVATE ${MY_SOURCES} ${MY_SOURCES_COMMON})

if(MSVC) # If using the VS compiler...

	target_compile_definitions(multyPlayer PUBLIC _CRT_SECURE_NO_WARNINGS)
	set_target_properties(multyPlayer PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
	#set_property(TARGET multyPlayer PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug<$<CONFIG:Debug>:Debug>")
	#set_property(TARGET multyPlayer PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")


	#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D _DEBUG /W3
	#/MD /Od /Zi /EHsc")
	#   set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /W3 /GL /Od
	#/Oi /Gy /Zi /EHsc")

	#foreach(flag_var
	#	CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
	#	CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
	#STRING (REGEX REPLACE "[/|-]RTC(su|[1suc])" "" ${flag_var} "${${flag_var}}")
	#endforeach(flag_var) #remiove basic runtime error checks
	#foreach(flag_var
	#		CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
	#		CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
	#	STRING (REGEX REPLACE "/GR" "" ${flag_var} "${${flag_var}}")
	#endforeach(flag_var)#remove rtti
	#
	#foreach(flag_var
	#		CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_RELEASE
	#		CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
	#	STRING (REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
	#endforeach(flag_var)#staticalli link
	#foreach(flag_var
	#		CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG
	#		CMAKE_CXX_FLAGS_MINSIZEREL)
	#	STRING (REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
	#endforeach(flag_var)#staticalli link




endif()

target_include_directories(multyPlayer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_include_directories(multyPlayer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/common/")
target_include_directories(multyPlayer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/gameLayer/")
target_include_directories(multyPlayer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/platform/")


target_link_libraries(multyPlayer PRIVATE glm glfw glad stb_image stb_truetype gl2d raudio imgui glui enet safeSave)

# #########################

project(levelBuilder)
add_executable(levelBuilder)


file(GLOB_RECURSE MY_SOURCES_LEVEL_BUILDER CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/levelBuilder/src/*.cpp")
file(GLOB_RECURSE MY_SOURCES_PLATFORM_CPPS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/*.cpp")


target_include_directories(levelBuilder PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_include_directories(levelBuilder PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/common/")
target_include_directories(levelBuilder PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/levelBuilder/include")
target_include_directories(levelBuilder PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/platform/")



set_property(TARGET levelBuilder PROPERTY CXX_STANDARD 17)

target_compile_definitions(levelBuilder PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/") # This is useful to get an ASSETS_PATH in your IDE during development but you should comment this if you compile a release version and uncomment the next line
#target_compile_definitions(levelBuilder PUBLIC RESOURCES_PATH="./resources/") # Uncomment this line to setup the ASSETS_PATH macro to the final assets directory when you share the game

target_sources(levelBuilder PRIVATE ${MY_SOURCES_LEVEL_BUILDER} ${MY_SOURCES_PLATFORM_CPPS} ${MY_SOURCES_COMMON})

if(MSVC) # If using the VS compiler...

	target_compile_definitions(levelBuilder PUBLIC _CRT_SECURE_NO_WARNINGS)
	set_target_properties(levelBuilder PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
	set_property(TARGET levelBuilder PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug<$<CONFIG:Debug>:Debug>")
	set_property(TARGET levelBuilder PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")

endif()

target_link_libraries(levelBuilder PRIVATE glm glfw glad stb_image stb_truetype gl2d raudio imgui glui enet safeSave)
