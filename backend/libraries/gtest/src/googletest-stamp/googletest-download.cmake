

set(command "/opt/local/bin/cmake;-Dmake=${make};-Dconfig=${config};-P;/Users/irene/uw/diamond/src/backend/gtest/src/googletest-stamp/googletest-download-impl.cmake")
execute_process(
  COMMAND ${command}
  RESULT_VARIABLE result
  OUTPUT_FILE "/Users/irene/uw/diamond/src/backend/gtest/src/googletest-stamp/googletest-download-out.log"
  ERROR_FILE "/Users/irene/uw/diamond/src/backend/gtest/src/googletest-stamp/googletest-download-err.log"
  )
if(result)
  set(msg "Command failed: ${result}\n")
  foreach(arg IN LISTS command)
    set(msg "${msg} '${arg}'")
  endforeach()
  set(msg "${msg}\nSee also\n  /Users/irene/uw/diamond/src/backend/gtest/src/googletest-stamp/googletest-download-*.log")
  message(FATAL_ERROR "${msg}")
else()
  set(msg "googletest download command succeeded.  See also /Users/irene/uw/diamond/src/backend/gtest/src/googletest-stamp/googletest-download-*.log")
  message(STATUS "${msg}")
endif()
