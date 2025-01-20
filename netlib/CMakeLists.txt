set(NETLIB_SOURCES
  src/mnet.cpp
)

# The library
qt_add_library(netlib ${NETLIB_SOURCES})

# Set include dir
target_include_directories(netlib
  PUBLIC ${PROJECT_SOURCE_DIR}/netlib/include
)
