# Copyright (c) 2012 - 2017, Lars Bilke
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors
#    may be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# USAGE:
#
# 1. Copy this file into your cmake modules path.
#
# 2. Add the following line to your CMakeLists.txt (best inside an if-condition
#    using a CMake option() to enable it just optionally):
#      include(CodeCoverage)
#
# 3. Append necessary compiler flags for all supported source files:
#      append_coverage_compiler_flags()
#    Or for specific target:
#      append_coverage_compiler_flags_to_target(YOUR_TARGET_NAME)
#
# 3.a (OPTIONAL) Set appropriate optimization flags, e.g. -O0, -O1 or -Og
#
# 4. If you need to exclude additional directories from the report, specify them
#    using full paths in the COVERAGE_EXCLUDES variable before calling
#    setup_target_for_coverage_*().
#    Example:
#      set(COVERAGE_EXCLUDES
#          '${PROJECT_SOURCE_DIR}/src/dir1/*'
#          '/path/to/my/src/dir2/*')
#    Or, use the EXCLUDE argument to setup_target_for_coverage_*().
#    Example:
#      setup_target_for_coverage_lcov(
#          NAME coverage
#          EXECUTABLE testrunner
#          EXCLUDE "${PROJECT_SOURCE_DIR}/src/dir1/*" "/path/to/my/src/dir2/*")
#
# 4.a NOTE: With CMake 3.4+, COVERAGE_EXCLUDES or EXCLUDE can also be set
#     relative to the BASE_DIRECTORY (default: PROJECT_SOURCE_DIR)
#     Example:
#       set(COVERAGE_EXCLUDES "dir1/*")
#       setup_target_for_coverage_gcovr(
#           NAME coverage
#           EXECUTABLE testrunner
#           FORMAT html-details
#           BASE_DIRECTORY "${PROJECT_SOURCE_DIR}/src"
#           EXCLUDE "dir2/*")
#
# 5. Use the functions described below to create a custom make target which
#    runs your test executable and produces a code coverage report.
#
# 6. Build a Debug build:
#      cmake -DCMAKE_BUILD_TYPE=Debug ..
#      make
#      make my_coverage_target
#

include(CMakeParseArguments)

option(CODE_COVERAGE_VERBOSE "Verbose information" FALSE)

# Check prereqs
find_program( GCOVR_PATH gcovr PATHS ${CMAKE_SOURCE_DIR}/scripts/test)

if (DEFINED CODE_COVERAGE_GCOV_TOOL)
  set(GCOV_TOOL "${CODE_COVERAGE_GCOV_TOOL}")
elseif (DEFINED ENV{CODE_COVERAGE_GCOV_TOOL})
  set(GCOV_TOOL "$ENV{CODE_COVERAGE_GCOV_TOOL}")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang")
  if (APPLE)
    execute_process( COMMAND xcrun -f llvm-cov
      OUTPUT_VARIABLE LLVMCOV_PATH
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  else()
    find_program( LLVMCOV_PATH llvm-cov )
  endif()
  if (NOT LLVMCOV_PATH)
    message(FATAL_ERROR "Could not find llvm-cov tool! Aborting...")
  endif()
  set(GCOV_TOOL "${LLVMCOV_PATH} gcov")
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  find_program( GCOV_PATH gcov )
  set(GCOV_TOOL ${GCOV_PATH})
endif()

if(NOT GCOV_TOOL)
  message(FATAL_ERROR "Could not find gcov or llvm-cov tool! Aborting...")
endif() # NOT GCOV_TOOL

# Check supported compiler (Clang, GNU and Flang)
get_property(LANGUAGES GLOBAL PROPERTY ENABLED_LANGUAGES)
foreach(LANG ${LANGUAGES})
  if("${CMAKE_${LANG}_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang")
    if("${CMAKE_${LANG}_COMPILER_VERSION}" VERSION_LESS 3)
      message(FATAL_ERROR "Clang version must be 3.0.0 or greater! Aborting...")
    endif()
  elseif(NOT "${CMAKE_${LANG}_COMPILER_ID}" MATCHES "GNU"
         AND NOT "${CMAKE_${LANG}_COMPILER_ID}" MATCHES "(LLVM)?[Ff]lang")
    message(FATAL_ERROR "Compiler is not GNU or Flang! Aborting...")
  endif()
endforeach()

set(COVERAGE_COMPILER_FLAGS "-g --coverage"
    CACHE INTERNAL "")
if(CMAKE_CXX_COMPILER_ID MATCHES "(GNU|Clang)")
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag(-fprofile-abs-path HAVE_cxx_fprofile_abs_path)
    if(HAVE_cxx_fprofile_abs_path)
        set(COVERAGE_CXX_COMPILER_FLAGS "${COVERAGE_COMPILER_FLAGS} -fprofile-abs-path")
    endif()
    include(CheckCCompilerFlag)
    check_c_compiler_flag(-fprofile-abs-path HAVE_c_fprofile_abs_path)
    if(HAVE_c_fprofile_abs_path)
        set(COVERAGE_C_COMPILER_FLAGS "${COVERAGE_COMPILER_FLAGS} -fprofile-abs-path")
    endif()
endif()

set(CMAKE_Fortran_FLAGS_COVERAGE
    ${COVERAGE_COMPILER_FLAGS}
    CACHE STRING "Flags used by the Fortran compiler during coverage builds."
    FORCE )
set(CMAKE_CXX_FLAGS_COVERAGE
    ${COVERAGE_COMPILER_FLAGS}
    CACHE STRING "Flags used by the C++ compiler during coverage builds."
    FORCE )
set(CMAKE_C_FLAGS_COVERAGE
    ${COVERAGE_COMPILER_FLAGS}
    CACHE STRING "Flags used by the C compiler during coverage builds."
    FORCE )
set(CMAKE_EXE_LINKER_FLAGS_COVERAGE
    ""
    CACHE STRING "Flags used for linking binaries during coverage builds."
    FORCE )
set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE
    ""
    CACHE STRING "Flags used by the shared libraries linker during coverage builds."
    FORCE )
mark_as_advanced(
    CMAKE_Fortran_FLAGS_COVERAGE
    CMAKE_CXX_FLAGS_COVERAGE
    CMAKE_C_FLAGS_COVERAGE
    CMAKE_EXE_LINKER_FLAGS_COVERAGE
    CMAKE_SHARED_LINKER_FLAGS_COVERAGE )

get_property(GENERATOR_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug" OR GENERATOR_IS_MULTI_CONFIG))
    message(WARNING "Code coverage results with an optimised (non-Debug) build may be misleading")
endif() # NOT (CMAKE_BUILD_TYPE STREQUAL "Debug" OR GENERATOR_IS_MULTI_CONFIG)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_Fortran_COMPILER_ID STREQUAL "GNU")
    link_libraries(gcov)
endif()

# Defines a target for running and collection code coverage information
# Builds dependencies, runs the given executable and outputs reports.
# NOTE! The executable should always have a ZERO as exit code otherwise
# the coverage generation will not complete.
#
# setup_target_for_coverage_gcovr(
#     NAME ctest_coverage                    # New target name
#     EXECUTABLE ctest -j ${PROCESSOR_COUNT} # Executable in PROJECT_BINARY_DIR
#     DEPENDENCIES executable_target         # Dependencies to build first
#     BASE_DIRECTORY "../"                   # Base directory for report
#                                            #  (defaults to PROJECT_SOURCE_DIR)
#     FORMAT "cobertura"                     # Output format, one of:
#                                            #  xml cobertura sonarqube json-summary
#                                            #  json-details coveralls csv txt
#                                            #  html-single html-nested html-details
#                                            #  (xml is an alias to cobertura;
#                                            #  if no format is set, defaults to xml;
#                                            #  if any of the supported formats is
#                                            #  set in GCOVR_ADDITIONAL_ARGS options
#                                            #  it will always take priority)
#     EXCLUDE "src/dir1/*" "src/dir2/*"      # Patterns to exclude (can be relative
#                                            #  to BASE_DIRECTORY, with CMake 3.4+)
# )
# The user can set the variable GCOVR_ADDITIONAL_ARGS to supply additional flags to the
# GCVOR command.
function(setup_target_for_coverage_gcovr)
    set(options NONE)
    set(oneValueArgs BASE_DIRECTORY NAME FORMAT)
    set(multiValueArgs EXCLUDE EXECUTABLE EXECUTABLE_ARGS DEPENDENCIES)
    cmake_parse_arguments(Coverage "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT GCOVR_PATH)
        message(FATAL_ERROR "Could not find gcovr tool! Aborting...")
    endif()

    # Set base directory (as absolute path), or default to PROJECT_SOURCE_DIR
    if(DEFINED Coverage_BASE_DIRECTORY)
        get_filename_component(BASEDIR ${Coverage_BASE_DIRECTORY} ABSOLUTE)
    else()
        set(BASEDIR ${PROJECT_SOURCE_DIR})
    endif()

    if("--txt" IN_LIST GCOVR_ADDITIONAL_ARGS)
        set(Coverage_FORMAT txt)
    elseif("--csv" IN_LIST GCOVR_ADDITIONAL_ARGS)
        set(Coverage_FORMAT csv)
    elseif(("--cobertura" IN_LIST GCOVR_ADDITIONAL_ARGS)
        OR ("--cobertura-pretty" IN_LIST GCOVR_ADDITIONAL_ARGS)
        OR ("--xml" IN_LIST GCOVR_ADDITIONAL_ARGS)
        OR ("--xml-pretty" IN_LIST GCOVR_ADDITIONAL_ARGS)
        OR ("-x" IN_LIST GCOVR_ADDITIONAL_ARGS))
        set(Coverage_FORMAT cobertura)
    elseif("--sonarqube" IN_LIST GCOVR_ADDITIONAL_ARGS)
        set(Coverage_FORMAT sonarqube)
    elseif(("--json-summary" IN_LIST GCOVR_ADDITIONAL_ARGS)
        OR ("--json-summary-pretty" IN_LIST GCOVR_ADDITIONAL_ARGS))
        set(Coverage_FORMAT json-summary)
    elseif(("--json" IN_LIST GCOVR_ADDITIONAL_ARGS)
        OR ("--json-pretty" IN_LIST GCOVR_ADDITIONAL_ARGS))
        set(Coverage_FORMAT json-details)
    elseif(("--coveralls" IN_LIST GCOVR_ADDITIONAL_ARGS)
        OR ("--coveralls-pretty" IN_LIST GCOVR_ADDITIONAL_ARGS))
        set(Coverage_FORMAT coveralls)
    elseif(("--html" IN_LIST GCOVR_ADDITIONAL_ARGS)
        OR ("--html-self-contained" IN_LIST GCOVR_ADDITIONAL_ARGS))
        set(Coverage_FORMAT html-single)
    elseif("--html-details" IN_LIST GCOVR_ADDITIONAL_ARGS)
        set(Coverage_FORMAT html-details)
    elseif("--html-nested" IN_LIST GCOVR_ADDITIONAL_ARGS)
        set(Coverage_FORMAT html-nested)
    else()
        if(NOT DEFINED Coverage_FORMAT)
            set(Coverage_FORMAT xml)
        endif()

        if ((Coverage_FORMAT STREQUAL "cobertura")
            OR (Coverage_FORMAT STREQUAL "xml"))
            list(APPEND GCOVR_ADDITIONAL_ARGS --cobertura-pretty)
            set(Coverage_FORMAT cobertura) # overwrite xml
        elseif(Coverage_FORMAT STREQUAL "sonarqube")
            list(APPEND GCOVR_ADDITIONAL_ARGS --sonarqube)
        elseif(Coverage_FORMAT STREQUAL "json-summary")
            list(APPEND GCOVR_ADDITIONAL_ARGS --json-summary-pretty)
        elseif(Coverage_FORMAT STREQUAL "json-details")
            list(APPEND GCOVR_ADDITIONAL_ARGS --json-pretty)
        elseif(Coverage_FORMAT STREQUAL "coveralls")
            list(APPEND GCOVR_ADDITIONAL_ARGS --coveralls-pretty)
        elseif(Coverage_FORMAT STREQUAL "csv")
            list(APPEND GCOVR_ADDITIONAL_ARGS --csv)
        elseif(Coverage_FORMAT STREQUAL "txt")
            list(APPEND GCOVR_ADDITIONAL_ARGS --txt)
        elseif(Coverage_FORMAT STREQUAL "html-single")
            list(APPEND GCOVR_ADDITIONAL_ARGS --html --html-self-contained)
        elseif(Coverage_FORMAT STREQUAL "html-nested")
            list(APPEND GCOVR_ADDITIONAL_ARGS --html --html-nested)
        elseif(Coverage_FORMAT STREQUAL "html-details")
            list(APPEND GCOVR_ADDITIONAL_ARGS --html --html-details)
        else()
            message(FATAL_ERROR "Unsupported output style ${Coverage_FORMAT}! Aborting...")
        endif()
    endif()

    if("--output" IN_LIST GCOVR_ADDITIONAL_ARGS)
        message(FATAL_ERROR "Unsupported --output option detected in GCOVR_ADDITIONAL_ARGS! Aborting...")
    else()
        if((Coverage_FORMAT STREQUAL "html-details")
            OR (Coverage_FORMAT STREQUAL "html-nested"))
            set(GCOVR_OUTPUT_FILE ${PROJECT_BINARY_DIR}/${Coverage_NAME}/index.html)
            set(GCOVR_CREATE_FOLDER ${PROJECT_BINARY_DIR}/${Coverage_NAME})
        elseif(Coverage_FORMAT STREQUAL "html-single")
            set(GCOVR_OUTPUT_FILE ${Coverage_NAME}.html)
        elseif((Coverage_FORMAT STREQUAL "json-summary")
            OR (Coverage_FORMAT STREQUAL "json-details")
            OR (Coverage_FORMAT STREQUAL "coveralls"))
            set(GCOVR_OUTPUT_FILE ${Coverage_NAME}.json)
        elseif(Coverage_FORMAT STREQUAL "txt")
            set(GCOVR_OUTPUT_FILE ${Coverage_NAME}.txt)
        elseif(Coverage_FORMAT STREQUAL "csv")
            set(GCOVR_OUTPUT_FILE ${Coverage_NAME}.csv)
        else()
            set(GCOVR_OUTPUT_FILE ${Coverage_NAME}.xml)
        endif()
        list(APPEND GCOVR_ADDITIONAL_ARGS --output ${GCOVR_OUTPUT_FILE})
    endif()

    # Collect excludes (CMake 3.4+: Also compute absolute paths)
    set(GCOVR_EXCLUDES "")
    foreach(EXCLUDE ${Coverage_EXCLUDE} ${COVERAGE_EXCLUDES} ${COVERAGE_GCOVR_EXCLUDES})
        if(CMAKE_VERSION VERSION_GREATER 3.4)
            get_filename_component(EXCLUDE ${EXCLUDE} ABSOLUTE BASE_DIR ${BASEDIR})
        endif()
        list(APPEND GCOVR_EXCLUDES "${EXCLUDE}")
    endforeach()
    list(REMOVE_DUPLICATES GCOVR_EXCLUDES)

    # Combine excludes to several -e arguments
    set(GCOVR_EXCLUDE_ARGS "")
    foreach(EXCLUDE ${GCOVR_EXCLUDES})
        list(APPEND GCOVR_EXCLUDE_ARGS "-e")
        list(APPEND GCOVR_EXCLUDE_ARGS "${EXCLUDE}")
    endforeach()

    # Set up commands which will be run to generate coverage data
    # Run tests
    set(GCOVR_EXEC_TESTS_CMD
        ${Coverage_EXECUTABLE} ${Coverage_EXECUTABLE_ARGS}
    )

    # Create folder
    if(DEFINED GCOVR_CREATE_FOLDER)
        set(GCOVR_FOLDER_CMD
            ${CMAKE_COMMAND} -E make_directory ${GCOVR_CREATE_FOLDER})
    else()
        set(GCOVR_FOLDER_CMD echo) # dummy
    endif()

    # Running gcovr
    set(GCOVR_CMD
        ${GCOVR_PATH}
        --gcov-executable ${GCOV_TOOL}
        --gcov-ignore-parse-errors=negative_hits.warn_once_per_file
        -r ${BASEDIR}
        ${GCOVR_ADDITIONAL_ARGS}
        ${GCOVR_EXCLUDE_ARGS}
        --object-directory=${PROJECT_BINARY_DIR}
    )

    if(CODE_COVERAGE_VERBOSE)
        message(STATUS "Executed command report")

        message(STATUS "Command to run tests: ")
        string(REPLACE ";" " " GCOVR_EXEC_TESTS_CMD_SPACED "${GCOVR_EXEC_TESTS_CMD}")
        message(STATUS "${GCOVR_EXEC_TESTS_CMD_SPACED}")

        if(NOT GCOVR_FOLDER_CMD STREQUAL "echo")
            message(STATUS "Command to create a folder: ")
            string(REPLACE ";" " " GCOVR_FOLDER_CMD_SPACED "${GCOVR_FOLDER_CMD}")
            message(STATUS "${GCOVR_FOLDER_CMD_SPACED}")
        endif()

        message(STATUS "Command to generate gcovr coverage data: ")
        string(REPLACE ";" " " GCOVR_CMD_SPACED "${GCOVR_CMD}")
        message(STATUS "${GCOVR_CMD_SPACED}")
    endif()

    add_custom_target(${Coverage_NAME}
        COMMAND ${GCOVR_EXEC_TESTS_CMD}
        COMMAND ${GCOVR_FOLDER_CMD}
        COMMAND ${GCOVR_CMD}

        BYPRODUCTS ${GCOVR_OUTPUT_FILE}
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        DEPENDS ${Coverage_DEPENDENCIES}
        VERBATIM # Protect arguments to commands
        COMMENT "Running gcovr to produce code coverage report."
    )

    # Show info where to find the report
    add_custom_command(TARGET ${Coverage_NAME} POST_BUILD
        COMMAND ;
        COMMENT "Code coverage report saved in ${GCOVR_OUTPUT_FILE} formatted as ${Coverage_FORMAT}"
    )
endfunction() # setup_target_for_coverage_gcovr

function(append_coverage_compiler_flags)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COVERAGE_COMPILER_FLAGS}" PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COVERAGE_COMPILER_FLAGS}" PARENT_SCOPE)
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} ${COVERAGE_COMPILER_FLAGS}" PARENT_SCOPE)
    message(STATUS "Appending code coverage compiler flags: ${COVERAGE_COMPILER_FLAGS}")
endfunction() # append_coverage_compiler_flags

# Setup coverage for specific library
function(append_coverage_compiler_flags_to_target name)
    separate_arguments(_flag_list NATIVE_COMMAND "${COVERAGE_COMPILER_FLAGS}")
    target_compile_options(${name} PRIVATE ${_flag_list})
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_Fortran_COMPILER_ID STREQUAL "GNU")
        target_link_libraries(${name} PRIVATE gcov)
    endif()
endfunction()
