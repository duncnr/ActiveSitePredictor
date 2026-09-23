# use Cabal to find location of the foreign library building.
execute_process(
    COMMAND
        "${CABAL_EXECUTABLE}"
        list-bin
        foreign-library:asplib
        --builddir "${HASKELL_BUILD_DIR}"

    WORKING_DIRECTORY "${HASKELL_SRC_DIR}"

    OUTPUT_VARIABLE CABAL_LIBRARY_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE

    RESULT_VARIABLE CABAL_RESULT
)

# stop if `cabal list-bin` failed.
if(NOT CABAL_RESULT EQUAL 0)
    message(FATAL_ERROR
        "Could not determine the location of the Haskell library."
    )
endif()

# ensure Cabal produces a real file.
if(NOT EXISTS "${CABAL_LIBRARY_PATH}")
    message(FATAL_ERROR
        "Cabal reported Haskell library at:\n"
        "${CABAL_LIBRARY_PATH}\n"
        "but that file does not exist."
    )
endif()

# create predictable lib directory.
file(MAKE_DIRECTORY "${HASKELL_LIB_DIR}")

# copy Cabal library to predictable location CMake expects.
file(COPY_FILE
    "${CABAL_LIBRARY_PATH}"
    "${HASKELL_LIBRARY}"
    ONLY_IF_DIFFERENT
)

message(STATUS
    "Haskell library copied to ${HASKELL_LIBRARY}"
)
