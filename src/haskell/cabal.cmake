# Haskell compilation

find_program(CABAL cabal REQUIRED)

set(HASKELL_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/haskell")
set(HASKELL_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/haskell")

# get build location
execute_process(
	WORKING_DIRECTORY ${HASKELL_SRC_DIR}
	COMMAND ${CABAL} list-bin foreign-library:asplib --builddir ${HASKELL_BUILD_DIR}
	OUTPUT_VARIABLE HASKELL_LIBRARY
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (WIN32)
	get_filename_component(HASKELL_LIBRARY ${HASKELL_LIBRARY} DIRECTORY)
	set(HASKELL_LIBRARY "${HASKELL_LIBRARY}/asplib.dll")
endif()

# retrieve src files
file(GLOB_RECURSE HASKELL_SOURCES CONFIGURE_DEPENDS "${HASKELL_SRC_DIR}/*")

# build
add_custom_command(
		OUTPUT ${HASKELL_LIBRARY}
		COMMAND ${CABAL} build --builddir ${HASKELL_BUILD_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy ${HASKELL_LIBRARY} ${CMAKE_CURRENT_BINARY_DIR}
		DEPENDS ${HASKELL_SOURCES}
		WORKING_DIRECTORY ${HASKELL_SRC_DIR}
		COMMENT "Building Haskell"
)

add_custom_target(haskell_asp ALL DEPENDS ${HASKELL_LIBRARY})

# Linux needs the link to be explicitly defined
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
endif()
