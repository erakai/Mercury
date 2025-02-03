set(NETLIB_SOURCES
  src/mnet.cpp
)

# The library
qt_add_library(netlib ${NETLIB_SOURCES})

# Qt set up
target_link_libraries(netlib PRIVATE Qt6::Widgets Qt6::Network)

# Set include dir
target_include_directories(netlib
  PUBLIC ${PROJECT_SOURCE_DIR}/netlib/include
)
