set(NETLIB_SOURCES
  src/hstp.cpp
  src/mftp.cpp
  src/mercury_client.cpp
  src/mercury_server.cpp
  src/logger.cpp

  include/hstp.hpp
  include/mftp.hpp
  include/mercury_client.hpp
  include/mercury_server.hpp
  include/logger.hpp
)

set(NETLIB_TESTER_SOURCES
  netlib_tester.cpp
)

# The library
qt_add_library(netlib ${NETLIB_SOURCES})

# Tester executable
qt_add_executable(netlib_tester ${NETLIB_TESTER_SOURCES})

# Qt set up
target_link_libraries(netlib PRIVATE Qt6::Widgets Qt6::Network Qt6::Multimedia)
target_link_libraries(netlib_tester PRIVATE Qt6::Widgets Qt6::Network Qt6::Multimedia)
target_link_libraries(netlib_tester PRIVATE netlib)

# Set include dir
target_include_directories(netlib
  PUBLIC ${PROJECT_SOURCE_DIR}/netlib/include
)
