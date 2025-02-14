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

set(NETLIB_TEST_SOURCES
  test/netlib_test.cpp
  test/hstp_test.cpp
)

# The library
qt_add_library(netlib ${NETLIB_SOURCES}
    include/hstp.hpp include/logger.hpp include/mercury_client.hpp include/mercury_server.hpp include/mftp.hpp)

# Set include dir and qt
target_include_directories(netlib
  PUBLIC ${PROJECT_SOURCE_DIR}/netlib/include
)
target_link_libraries(netlib PRIVATE Qt6::Widgets Qt6::Network Qt6::Multimedia)

# Tester executable
enable_testing()
qt_add_executable(netlib_test ${NETLIB_TEST_SOURCES})

# Qt set up
target_link_libraries(netlib_test PRIVATE Qt6::Widgets Qt6::Network Qt6::Multimedia)

# Other dependencies
target_link_libraries(netlib_test PRIVATE netlib)
target_link_libraries(netlib_test PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(netlib_test)
