# cmake/CheckChronoTzdb.cmake
# Provides: check_chrono_tzdb(<out_var>)
# Try-compiles a tiny program that uses std::chrono::zoned_time and current_zone().
# Fails (returns FALSE) if compilation OR linking fails.

function(check_chrono_tzdb OUTVAR)
    set(_src "${CMAKE_BINARY_DIR}/_tzdb_check.cpp")
    file(WRITE "${_src}" [=[
    #include <chrono>
    #include <iostream>
    int main() {
    #if !defined(__cpp_lib_chrono) || __cpp_lib_chrono < 201907
    #error "C++20 chrono tzdb (__cpp_lib_chrono >= 201907) is required"
    #endif
      using namespace std::chrono;
      auto z = zoned_time{ current_zone(), system_clock::now() };
      // Touch some members so headers are not enough; ensures full definition/link.
      auto lt = z.get_local_time();
      auto name = z.get_time_zone()->name();
      std::cout << name << " " << lt.time_since_epoch().count() << "\n";
      return 0;
    }
  ]=])

    try_compile(${OUTVAR}
            "${CMAKE_BINARY_DIR}/_tzdb_check_build"
            "${_src}"
            CMAKE_FLAGS
            "-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}"
            "-DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}"
            "-DCMAKE_CXX_EXTENSIONS=${CMAKE_CXX_EXTENSIONS}"
            OUTPUT_VARIABLE _tzdb_try_output
    )

    if(NOT ${OUTVAR})
        message(STATUS "std::chrono tzdb try-compile failed:\n${_tzdb_try_output}")
    endif()
endfunction()