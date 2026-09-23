cmake_minimum_required(VERSION 4.0)

set(PROJECT_NAME active_site_predictor)
project(${PROJECT_NAME})

include(ExternalProject) # ExternalProject_add

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For LSPs
set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CONFIGURATION_TYPES Release Debug)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if(NOT WIN32)
   find_program(GHC_PKG ghc-pkg REQUIRED)

   execute_process(
      COMMAND ${GHC_PKG} field rts dynamic-library-dirs --simple-output
      OUTPUT_VARIABLE GHC_RTS_LIB_DIR
      OUTPUT_STRIP_TRAILING_WHITESPACE
      COMMAND_ERROR_IS_FATAL ANY
   )

   execute_process(
      COMMAND ${GHC_PKG} field rts hs-libraries --simple-output
      OUTPUT_VARIABLE GHC_RTS_LIB_NAME
      OUTPUT_STRIP_TRAILING_WHITESPACE
      COMMAND_ERROR_IS_FATAL ANY
   )

   file(GLOB GHC_RTS_LIBRARY "${GHC_RTS_LIB_DIR}/lib${GHC_RTS_LIB_NAME}-ghc*.so")

   if(NOT GHC_RTS_LIBRARY)
      message(FATAL_ERROR "Could not find GHC RTS library in ${GHC_RTS_LIB_DIR}")
   endif()

   message(STATUS "GHC RTS library: ${GHC_RTS_LIBRARY}")
endif()

# Haskell compilation
   # https://github.com/minimapletinytools/haskell-ffi-cabal-foreign-library-examples/blob/master/Makefile

find_program(CABAL cabal REQUIRED)

# set paths to variables
set(HASKELL_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/haskell)
set(HASKELL_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/haskell_build)
set(HASKELL_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/haskell_lib)

# determine os
if(WIN32)
   set(HASKELL_LIBRARY "${HASKELL_LIB_DIR}/asplib.dll")
else()
   set(HASKELL_LIBRARY "${HASKELL_LIB_DIR}/libasplib.so")
endif()

# retrieve src files
file(GLOB_RECURSE HASKELL_SOURCES CONFIGURE_DEPENDS "src/haskell/*")

# 
add_custom_command(
    OUTPUT "${HASKELL_LIBRARY}"

    COMMAND ${CABAL} build --builddir "${HASKELL_BUILD_DIR}"

    COMMAND "${CMAKE_COMMAND}"
        "-DCABAL_EXECUTABLE=${CABAL}"
        "-DHASKELL_SRC_DIR=${HASKELL_SRC_DIR}"
        "-DHASKELL_BUILD_DIR=${HASKELL_BUILD_DIR}"
        "-DHASKELL_LIB_DIR=${HASKELL_LIB_DIR}"
        "-DHASKELL_LIBRARY=${HASKELL_LIBRARY}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CopyHaskellLib.cmake"
 
    DEPENDS ${HASKELL_SOURCES}

    WORKING_DIRECTORY "${HASKELL_SRC_DIR}"

    COMMENT "Building Haskell"
)

add_custom_target(haskell_asp ALL DEPENDS "${HASKELL_LIBRARY}")

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

if(NOT WIN32)
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
