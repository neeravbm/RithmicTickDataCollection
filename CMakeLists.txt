cmake_minimum_required(VERSION 3.28)
project(RithmicTickDataCollection)

# You run C++26; tzdb only needs C++20, but leaving 26 is fine.
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Prefer the CMake variable set by the toolchain; fall back to env var.
if(DEFINED GCC_TOOLSET_ROOT)
    set(_GTS_ROOT "${GCC_TOOLSET_ROOT}")
elseif(DEFINED ENV{GCC_TOOLSET_ROOT})
    set(_GTS_ROOT "$ENV{GCC_TOOLSET_ROOT}")
endif()

if(DEFINED _GTS_ROOT)
    file(TO_CMAKE_PATH "${_GTS_ROOT}/root/usr/lib64" _GTS_LIBDIR)
    if(EXISTS "${_GTS_LIBDIR}")
        set(CMAKE_BUILD_RPATH   "${CMAKE_BUILD_RPATH};${_GTS_LIBDIR}")
        set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH};${_GTS_LIBDIR}")
    endif()
endif()


# ---------------------------
# Require a new-enough compiler (advisory; strict check below will enforce)
# ---------------------------
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "GCC version: ${CMAKE_CXX_COMPILER_VERSION}")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13)
        message(FATAL_ERROR "GCC >= 13 required for std::chrono time zones (zoned_time).")
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # VS 2022 recent toolsets are fine.
    message(STATUS "MSVC version: ${CMAKE_CXX_COMPILER_VERSION}")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Clang detected—ensure it links against a modern libstdc++/libc++ with tzdb.")
endif()

# ---------------------------
# Strict tzdb compile+link check (no fallbacks)
# ---------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CheckChronoTzdb)
check_chrono_tzdb(HAS_STD_CHRONO_TZDB)
if(NOT HAS_STD_CHRONO_TZDB)
    message(FATAL_ERROR
            "Your standard library lacks C++20 chrono time-zone support (zoned_time/current_zone). "
            "Use GCC >= 13 (e.g., gcc-toolset-14/13 on RHEL 9), a recent MSVC, or Clang with a modern libc++/libstdc++.")
endif()

# ---------------------------
# Platform/system libs
# ---------------------------
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

set(SYSTEM_LIBRARIES Threads::Threads ZLIB::ZLIB)
if(WIN32)
    list(APPEND SYSTEM_LIBRARIES ws2_32 iphlpapi psapi crypt32)
elseif(UNIX AND NOT APPLE AND NOT FREEBSD)
    list(APPEND SYSTEM_LIBRARIES dl rt) # Linux only
endif()

# ---------------------------
# Your existing deps
# ---------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(Rithmic REQUIRED)

add_subdirectory(contribs/clickhouse-cpp)
find_package(args REQUIRED)

set(TARGET_LIBRARIES
        taywee::args
        clickhouse-cpp-lib
)

# ---------------------------
# Target
# ---------------------------
add_executable(
        RithmicTickDataCollection
        main.cpp
        src/Rithmic.cpp
        src/TradeUploader.cpp
        src/Utils.cpp
)

target_include_directories(RithmicTickDataCollection PRIVATE
        include
        ${Rithmic_INCLUDE_DIRS}
        contribs/clickhouse-cpp/
        contribs/clickhouse-cpp/contrib/absl
)

# Handle either variable spelling from your FindRithmic
if(DEFINED Rithmic_LIBRARIES)
    set(_RITHMIC_LIBS ${Rithmic_LIBRARIES})
elseif(DEFINED Rithmic_LIBARIES) # original spelling
    set(_RITHMIC_LIBS ${Rithmic_LIBARIES})
endif()

target_link_libraries(RithmicTickDataCollection PRIVATE
        ${_RITHMIC_LIBS}
        ${SYSTEM_LIBRARIES}
        ${TARGET_LIBRARIES}
)

# ---------------------------
# Your original per-OS flags
# ---------------------------
if(NOT WIN32)
    add_compile_options(-O3 -D_REENTRANT -Wall -Wno-sign-compare -fno-strict-aliasing -Wpointer-arith -Winline -Wno-deprecated -Wno-write-strings)
else()
    add_compile_definitions(WinOS)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
