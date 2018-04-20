INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_DSTARDD dstardd)

FIND_PATH(
    DSTARDD_INCLUDE_DIRS
    NAMES dstardd/api.h
    HINTS $ENV{DSTARDD_DIR}/include
        ${PC_DSTARDD_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    DSTARDD_LIBRARIES
    NAMES gnuradio-dstardd
    HINTS $ENV{DSTARDD_DIR}/lib
        ${PC_DSTARDD_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(DSTARDD DEFAULT_MSG DSTARDD_LIBRARIES DSTARDD_INCLUDE_DIRS)
MARK_AS_ADVANCED(DSTARDD_LIBRARIES DSTARDD_INCLUDE_DIRS)

