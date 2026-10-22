cmake_minimum_required(VERSION 4.0)

set(PROJECT_NAME active_site_predictor)
project(${PROJECT_NAME})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For LSPs
set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CONFIGURATION_TYPES Release Debug)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Haskell compilation
add_subdirectory(src/haskell)

# TODO: Conan support?
include(FetchContent)

# External libraries
find_package(Vulkan REQUIRED)

FetchContent_Declare(
	glfw
	URL "https://github.com/glfw/glfw/archive/refs/tags/3.5.1.zip"
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
	vk-bootstrap
	URL "https://github.com/charles-lunarg/vk-bootstrap/archive/refs/tags/v1.4.365.zip"
)
FetchContent_MakeAvailable(vk-bootstrap)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/cpp/*.cpp")
list(APPEND SOURCES "src/main.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
	src/cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	${HASKELL_LIBRARY}
	Vulkan::Vulkan
	vk-bootstrap::vk-bootstrap
	glfw
)

# Haskell library linkage
add_dependencies(${PROJECT_NAME} haskell_asp)
if(NOT WIN32)
   message("\t\tGHC_RTS_LIBRARY: ${GHC_RTS_LIBRARY}")
   target_link_libraries(${PROJECT_NAME} PRIVATE
      ${GHC_RTS_LIBRARY}
   )
endif()

# for later.. copy resources (models, shaders) after build
#[[
add_custom_command(
	TARGET ${PROJECT_NAME}
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_SOURCE_DIR}/src/resource"
		"$<TARGET_FILE_DIR:${PROJECT_NAME}>/resource"
)

# runs executable after build
add_custom_command(
	TARGET ${PROJECT_NAME}
	POST_BUILD
	COMMAND $<TARGET_FILE:${PROJECT_NAME}>
	WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)
#]]
