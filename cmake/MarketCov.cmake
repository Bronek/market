# Enable coverage_report using two options CMAKE_BUILD_TYPE=Debug CODE_COVERAGE=True e.g.
# cmake -DCMAKE_BUILD_TYPE=Debug -DCODE_COVERAGE=True (followed by usual options)

if (CODE_COVERAGE)
  if (NOT GCOVR_PATH)
    message(FATAL_ERROR "Cannot find gcovr, aborting ...")
  endif()

  set (GCOVR_ADDITIONAL_ARGS --html --html-details --exclude-noncode-lines --exclude-unreachable-branches -j ${PROCESSOR_COUNT} -s)

  setup_target_for_coverage_gcovr_html(
      NAME coverage_report
      EXECUTABLE tests
      EXECUTABLE_ARGS -r console
      EXCLUDE "tests" "third_party"
      DEPENDENCIES tests
  )
endif()
