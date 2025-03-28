# Determine source list
set(TEST_SOURCES
test/mercury_test.cpp
)

# The binary
qt_add_executable(mercury
  MANUAL_FINALIZATION
  stream/streamwindow.cpp stream/streamwindow.hpp
  stream/streamservice.hpp
  stream/sidepane.cpp stream/sidepane.hpp
  stream/streamdisplay.cpp stream/streamdisplay.hpp
  stream/performancetab.cpp
  home/main.cpp
  home/mainwindow.cpp home/mainwindow.hpp home/mainwindow.ui
  home/settingswindow.h home/settingswindow.cpp home/settingswindow.ui
  home/hoststreamwindow.h home/hoststreamwindow.cpp home/hoststreamwindow.ui
  home/joinstreamwindow.h home/joinstreamwindow.cpp home/joinstreamwindow.ui
  home/utils.h home/utils.cpp
  config/mconfig.cpp
  config/mconfig.hpp
  home/toastnotification.h home/toastnotification.cpp

  streampreview/streampreviewwindow.h streampreview/streampreviewwindow.cpp
  streampreview/screenlistmodel.h streampreview/screenlistmodel.cpp
  streampreview/windowlistmodel.h streampreview/windowlistmodel.cpp
  streampreview/audioinputlistmodel.h streampreview/audioinputlistmodel.cpp

  singleton/videomanager.h singleton/videomanager.cpp
)

# Qt set up
target_link_libraries(mercury PRIVATE Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Multimedia Qt6::MultimediaWidgets Qt6::Charts)

# Prevents console window on windows (Qt stuff)
set_target_properties(mercury PROPERTIES
    WIN32_EXECUTABLE ON
)

# Add networking library
target_link_libraries(mercury PRIVATE netlib)

# Add dependency on assets in case we want any
add_dependencies(mercury assets)
add_dependencies(mercury config)

# Install to destination
include(GNUInstallDirs)
install(TARGETS mercury
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(mercury)

# Set up testing executable
enable_testing(true)
qt_add_executable(mercury_test ${TEST_SOURCES})
target_link_libraries(mercury_test PRIVATE Qt6::Widgets Qt6::Network Qt6::Multimedia Qt6::Test netlib)

