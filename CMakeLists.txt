cmake_minimum_required(VERSION 4.0)

set(PROJECT_NAME active_site_predictor)
project(${PROJECT_NAME})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For LSPs
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CONFIGURATION_TYPES Release Debug)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# 1. Define where Cabal will build the library
set(HASKELL_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/haskell)
set(HASKELL_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/haskell_build)

if(WIN32)
    set(HASKELL_LIB_NAME "asplib.dll")
else()
    set(HASKELL_LIB_NAME "asplib.a")
endif()

set(HASKELL_LIB_PATH ${HASKELL_BUILD_DIR}/build/${LIB_NAME})

ExternalProject_Add(haskell_target
    SOURCE_DIR ${HASKELL_SRC_DIR}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ghc -no-hs-main -staticlib Main.hs -o asplib.a
    INSTALL_COMMAND ""
    BUILD_BYPRODUCTS ${HASKELL_LIB_PATH}
)

# ideally would use find_package, unsure how that's supposed to work on windows though
# TODO: Conan support?
add_subdirectory(include/glfw)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/cpp/*.cpp")
list(APPEND SOURCES "src/main.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
	include/glfw/include
	src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	glfw
)

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
