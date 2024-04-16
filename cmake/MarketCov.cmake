# Enable coverage using two options CMAKE_BUILD_TYPE=Debug CODE_COVERAGE=True e.g.
# cmake -DCMAKE_BUILD_TYPE=Debug -DCODE_COVERAGE=ON (followed by usual options)

if (CODE_COVERAGE)
  if (NOT GCOVR_PATH)
    message(FATAL_ERROR "Cannot find gcovr, aborting ...")
  endif()

  if (DEFINED CODE_COVERAGE_REPORT_FORMAT)
    set(CODE_COVERAGE_FORMAT ${CODE_COVERAGE_REPORT_FORMAT})
  else()
    set(CODE_COVERAGE_FORMAT xml)
  endif()

  set (GCOVR_ADDITIONAL_ARGS --exclude-throw-branches -s)

  setup_target_for_coverage_gcovr(
      NAME coverage
      FORMAT ${CODE_COVERAGE_FORMAT}
      EXECUTABLE tests
      EXECUTABLE_ARGS -r console
      EXCLUDE "tests" "${CMAKE_BINARY_DIR}"
      DEPENDENCIES tests
  )
endif()
