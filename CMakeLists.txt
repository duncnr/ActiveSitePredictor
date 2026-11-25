cmake_minimum_required(VERSION 4.0)

set(PROJECT_NAME active_site_predictor)
project(${PROJECT_NAME})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For LSPs
set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CONFIGURATION_TYPES Release Debug)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Haskell compilation
add_subdirectory(src/haskell)

# GLSL compilation
add_subdirectory(src/glsl)

# TODO: Conan support?
include(FetchContent)

# External libraries
find_package(Vulkan REQUIRED)

macro(FETCH NAME REPO TAG)
	FetchContent_Declare(
		${NAME}
		GIT_REPOSITORY ${REPO}
		GIT_TAG ${TAG}
		GIT_SUBMODULES ""
	)
	FetchContent_MakeAvailable(${NAME})
endmacro()

FETCH(glfw https://github.com/glfw/glfw.git 3.5.1)
FETCH(vk-bootstrap https://github.com/charles-lunarg/vk-bootstrap.git v1.4.367)
FETCH(VulkanMemoryAllocator https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git v3.5.0)
FETCH(glm https://github.com/g-truc/glm.git 1.0.3)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/cpp/*.cpp")
list(APPEND SOURCES "src/main.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
	src/cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	${HASKELL_LIBRARY}
	Vulkan::Vulkan
	glfw
	vk-bootstrap::vk-bootstrap
	VulkanMemoryAllocator
	glm
)

# Haskell library linkage
add_dependencies(${PROJECT_NAME} haskell_asp)
if(NOT WIN32)
   target_link_libraries(${PROJECT_NAME} PRIVATE
      ${GHC_RTS_LIBRARY}
   )
endif()
