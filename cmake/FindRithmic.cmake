INCLUDE(FindPackageHandleStandardArgs)

FIND_PATH(
        Rithmic_INCLUDE_DIRS
        NAMES RApiPlus.h
        HINTS ${RITHMIC_DIR}/include
        REQUIRED
        NO_DEFAULT_PATH
)

MESSAGE(Rithmic_INCLUDE_DIRS="${Rithmic_INCLUDE_DIRS}")

IF (WIN32)
    SET(SUBDIRECTORY "win10")

    IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
        SET(LIBRARY_SUFFIX "mtd64")
    ELSE ()
        SET(LIBRARY_SUFFIX "mt64")
    ENDIF ()

    #SET(LIBRARY_SUFFIX $<IF:$<CONFIG:DEBUG>,mtd64,mt64>)
    #MESSAGE(LIBRARY_SUFFIX=${LIBRARY_SUFFIX})
    #file(GENERATE OUTPUT debug_genexpr CONTENT "$<IF:$<CONFIG:DEBUG>,mtd64,mt64>")

    FIND_LIBRARY(
            RAPIPLUS_LIBRARY
            RApiPlus_${LIBRARY_SUFFIX}
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    FIND_LIBRARY(
            OMNE_STREAM_ENGINE_LIBRARY
            OmneStreamEngine_${LIBRARY_SUFFIX}
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    FIND_LIBRARY(
            OMNE_CHANNEL_LIBRARY
            OmneChannel_${LIBRARY_SUFFIX}
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    FIND_LIBRARY(
            OMNE_ENGINE_LIBRARY
            OmneEngine_${LIBRARY_SUFFIX}
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    FIND_LIBRARY(
            API_LIBRARY
            api_${LIBRARY_SUFFIX}
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    FIND_LIBRARY(
            API_POLL_STUBS_LIBRARY
            apistb_${LIBRARY_SUFFIX}
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    FIND_LIBRARY(
            KIT_LIBRARY
            kit_${LIBRARY_SUFFIX}
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    FIND_LIBRARY(
            SSL_LIBRARY
            libssl_${LIBRARY_SUFFIX}
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    FIND_LIBRARY(
            CRYPTO_LIBRARY
            libcrypto_${LIBRARY_SUFFIX}
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    FIND_LIBRARY(
            ZLIB_LIBRARIES
            zlib_${LIBRARY_SUFFIX}
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

ELSEIF (UNIX)
    SET(SUBDIRECTORY "linux-gnu-4.18-x86_64")

    FIND_LIBRARY(
            RAPIPLUS_LIBRARY
            RApiPlus-optimize
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    FIND_LIBRARY(
            OMNE_STREAM_ENGINE_LIBRARY
            OmneStreamEngine-optimize
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    FIND_LIBRARY(
            OMNE_CHANNEL_LIBRARY
            OmneChannel-optimize
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    FIND_LIBRARY(
            OMNE_ENGINE_LIBRARY
            OmneEngine-optimize
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    FIND_LIBRARY(
            API_LIBRARY
            _api-optimize
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    FIND_LIBRARY(
            API_POLL_STUBS_LIBRARY
            _apipoll-stubs-optimize
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    FIND_LIBRARY(
            KIT_LIBRARY
            _kit-optimize
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    FIND_LIBRARY(
            SSL_LIBRARY
            ssl
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    FIND_LIBRARY(
            CRYPTO_LIBRARY
            crypto
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    SET(ZLIB_USE_STATIC_LIBS ON)
    #SET(ZLIB_ROOT "/opt/homebrew/Cellar/zlib/1.3/")
    FIND_PACKAGE(ZLIB REQUIRED)

ELSE ()
    SET(SUBDIRECTORY "darwin-20.6-arm64")

    FIND_LIBRARY(
            RAPIPLUS_LIBRARY
            RApiPlus-optimize
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    FIND_LIBRARY(
            OMNE_STREAM_ENGINE_LIBRARY
            OmneStreamEngine-optimize
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    FIND_LIBRARY(
            OMNE_CHANNEL_LIBRARY
            OmneChannel-optimize
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    FIND_LIBRARY(
            OMNE_ENGINE_LIBRARY
            OmneEngine-optimize
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    FIND_LIBRARY(
            API_LIBRARY
            _api-optimize
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    FIND_LIBRARY(
            API_POLL_STUBS_LIBRARY
            _apipoll-stubs-optimize
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    FIND_LIBRARY(
            KIT_LIBRARY
            _kit-optimize
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    FIND_LIBRARY(
            SSL_LIBRARY
            ssl
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    FIND_LIBRARY(
            CRYPTO_LIBRARY
            crypto
            HINTS ${RITHMIC_DIR}/${SUBDIRECTORY}/lib
            REQUIRED
    )

    SET(ZLIB_USE_STATIC_LIBS ON)
    SET(ZLIB_ROOT "/opt/homebrew/Cellar/zlib/1.3/")
    FIND_PACKAGE(ZLIB REQUIRED)
ENDIF ()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(
        Rithmic
        REQUIRED_VARS
        Rithmic_INCLUDE_DIRS
        #ZLIB_INCLUDE_DIRS
        RAPIPLUS_LIBRARY
        OMNE_STREAM_ENGINE_LIBRARY
        OMNE_CHANNEL_LIBRARY
        OMNE_ENGINE_LIBRARY
        API_LIBRARY
        API_POLL_STUBS_LIBRARY
        KIT_LIBRARY
        SSL_LIBRARY
        CRYPTO_LIBRARY
        ZLIB_LIBRARIES
)

SET(Rithmic_LIBARIES "${RAPIPLUS_LIBRARY};${OMNE_STREAM_ENGINE_LIBRARY};${OMNE_CHANNEL_LIBRARY};${OMNE_ENGINE_LIBRARY};${API_LIBRARY};${API_POLL_STUBS_LIBRARY};${KIT_LIBRARY};${SSL_LIBRARY};${CRYPTO_LIBRARY};${ZLIB_LIBRARIES}")