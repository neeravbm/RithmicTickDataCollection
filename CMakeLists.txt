cmake_minimum_required(VERSION 3.28)
project(RithmicTickDataCollection)

set(CMAKE_CXX_STANDARD 26)

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
FIND_PACKAGE(Rithmic REQUIRED)

SET(BUILD_SHARED_LIBS OFF)

IF (NOT WIN32)
    add_compile_options(-O3 -D_REENTRANT -Wall -Wno-sign-compare -fno-strict-aliasing -Wpointer-arith -Winline -Wno-deprecated -Wno-write-strings)
    set(SYSTEM_LIBRARIES z pthread rt dl)
ELSE ()
    ADD_COMPILE_DEFINITIONS(WinOS)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(SYSTEM_LIBRARIES "ws2_32;iphlpapi;psapi;crypt32")
ENDIF ()

add_subdirectory(contribs/clickhouse-cpp)

find_package(args REQUIRED)

set(TARGET_LIBRARIES
        taywee::args
        clickhouse-cpp-lib
)

add_executable(
        RithmicTickDataCollection
        main.cpp
        src/Rithmic.cpp
        src/TradeUploader.cpp
        src/Utils.cpp
)

TARGET_INCLUDE_DIRECTORIES(RithmicTickDataCollection PRIVATE include ${Rithmic_INCLUDE_DIRS} contribs/clickhouse-cpp/ contribs/clickhouse-cpp/contrib/absl)
target_link_libraries(RithmicTickDataCollection PRIVATE ${Rithmic_LIBARIES} ${SYSTEM_LIBRARIES} ${TARGET_LIBRARIES})
