set(NETLIB_SOURCES
  src/hstp.cpp
  src/mftp.cpp
  src/mercury_client.cpp
  src/mercury_server.cpp

  include/hstp.hpp
  include/mftp.hpp
  include/mercury_client.hpp
  include/mercury_server.hpp
  include/netlib_global.h
)

set(NETLIB_TEST_SOURCES
  test/netlib_test.cpp
  test/hstp_test.cpp
  test/mftp_test.cpp
  test/client_test.cpp
)

set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVERY_MODE PRE_TEST)

# The library
qt_add_library(netlib ${NETLIB_SOURCES})
target_compile_definitions(netlib PRIVATE NETLIB_LIBRARY)

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
