cmake_minimum_required(VERSION 4.0)

set(PROJECT_NAME active_site_predictor)
project(${PROJECT_NAME})

include(ExternalProject) # ExternalProject_add

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For LSPs
set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CONFIGURATION_TYPES Release Debug)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Haskell compilation
# https://github.com/minimapletinytools/haskell-ffi-cabal-foreign-library-examples/blob/master/Makefile
find_program(CABAL cabal REQUIRED)
set(HASKELL_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/haskell)
set(HASKELL_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/haskell_build)

file(GLOB_RECURSE HASKELL_SOURCES CONFIGURE_DEPENDS "src/haskell/*")
add_custom_target(haskell_asp ALL
    COMMAND ${CABAL} build --builddir ${HASKELL_BUILD_DIR}
    DEPENDS ${HASKELL_SOURCES}
    WORKING_DIRECTORY ${HASKELL_SRC_DIR}
    COMMENT "Building Haskell"
)

if(WIN32)
	set(HASKELL_LIBRARY_NAME "${HASKELL_BUILD_DIR}/*/asplib.dll")
else()
	set(HASKELL_LIBRARY_NAME "${HASKELL_BUILD_DIR}/*/asplib.so")
endif()

file(GLOB_RECURSE HASKELL_LIBRARY ${HASKELL_LIBRARY_NAME})

# ideally would use find_package, unsure how that's supposed to work on windows though
# TODO: Conan support?
add_subdirectory(include/glfw)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/cpp/*.cpp")
list(APPEND SOURCES "src/main.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})

add_dependencies(${PROJECT_NAME} haskell_asp)

target_include_directories(${PROJECT_NAME} PRIVATE
	include/glfw/include
	src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	${HASKELL_LIBRARY}
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
