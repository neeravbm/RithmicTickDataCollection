# cmake/gcc-toolset.cmake
# Auto-select gcc-toolset-14 (then 13) on RHEL 9.
# - No effect on non-RHEL hosts (macOS/Windows/FreeBSD/etc).
# - You can override with GCC_TOOLSET_ROOT env var.
# - Set -DGCC_TOOLSET_REQUIRED=ON to fail if none is found.
# - Set -DGCC_TOOLSET_PREFERRED="14;13" to control search order.

# Defaults
if(NOT DEFINED GCC_TOOLSET_PREFERRED)
    set(GCC_TOOLSET_PREFERRED "14;13")
endif()
if(NOT DEFINED GCC_TOOLSET_REQUIRED)
    set(GCC_TOOLSET_REQUIRED OFF)
endif()

# If the user provided an explicit root via env, honor it.
if(DEFINED ENV{GCC_TOOLSET_ROOT})
    set(_CANDIDATES "$ENV{GCC_TOOLSET_ROOT}")
else()
    # Only makes sense on RHEL-like layouts (/opt/rh).
    if(UNIX AND EXISTS "/opt/rh")
        foreach(ver IN LISTS GCC_TOOLSET_PREFERRED)
            list(APPEND _CANDIDATES "/opt/rh/gcc-toolset-${ver}")
        endforeach()
    endif()
endif()

# Find a candidate that actually contains g++
set(GCC_TOOLSET_ROOT "")
foreach(root IN LISTS _CANDIDATES)
    if(EXISTS "${root}/root/usr/bin/g++")
        set(GCC_TOOLSET_ROOT "${root}")
        break()
    endif()
endforeach()

# If nothing found, either fail (if required) or quietly no-op.
if(GCC_TOOLSET_ROOT STREQUAL "")
    if(GCC_TOOLSET_REQUIRED)
        message(FATAL_ERROR
                "Requested gcc-toolset not found. Tried: ${_CANDIDATES}. "
                "Install gcc-toolset-${GCC_TOOLSET_PREFERRED} or set GCC_TOOLSET_ROOT.")
    else()
        message(STATUS "gcc-toolset not found; using system compiler.")
        return()
    endif()
endif()

# Point CMake at the toolset compilers.
set(CMAKE_C_COMPILER   "${GCC_TOOLSET_ROOT}/root/usr/bin/gcc"  CACHE FILEPATH "" FORCE)
set(CMAKE_CXX_COMPILER "${GCC_TOOLSET_ROOT}/root/usr/bin/g++"  CACHE FILEPATH "" FORCE)

# Use toolset binutils as well (harmless if absent).
if(EXISTS "${GCC_TOOLSET_ROOT}/root/usr/bin/gcc-ar")
    set(CMAKE_AR     "${GCC_TOOLSET_ROOT}/root/usr/bin/gcc-ar"     CACHE FILEPATH "" FORCE)
endif()
if(EXISTS "${GCC_TOOLSET_ROOT}/root/usr/bin/gcc-ranlib")
    set(CMAKE_RANLIB "${GCC_TOOLSET_ROOT}/root/usr/bin/gcc-ranlib" CACHE FILEPATH "" FORCE)
endif()

# Help find headers/libs installed under the toolset prefix (non-invasive).
list(PREPEND CMAKE_PREFIX_PATH "${GCC_TOOLSET_ROOT}/root/usr")

message(STATUS "Using gcc-toolset at: ${GCC_TOOLSET_ROOT}")
message(STATUS "C compiler:  ${CMAKE_C_COMPILER}")
message(STATUS "CXX compiler: ${CMAKE_CXX_COMPILER}")
