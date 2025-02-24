# Determine source list

# The binary
qt_add_executable(mercury
  MANUAL_FINALIZATION
  main.cpp
  mainwindow.cpp mainwindow.hpp mainwindow.ui
  settingswindow.h settingswindow.cpp settingswindow.ui
  hoststreamwindow.h hoststreamwindow.cpp hoststreamwindow.ui
  joinstreamwindow.h joinstreamwindow.cpp joinstreamwindow.ui
  streampreviewwindow.h streampreviewwindow.cpp

  screenlistmodel.h screenlistmodel.cpp
  windowlistmodel.h windowlistmodel.cpp
  audioinputlistmodel.h audioinputlistmodel.cpp


)

# Qt set up
target_link_libraries(mercury PRIVATE Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets)

# Prevents console window on windows (Qt stuff)
set_target_properties(mercury PROPERTIES
    WIN32_EXECUTABLE ON
)

# Add networking library
target_link_libraries(mercury PRIVATE netlib)

# Add dependency on assets in case we want any
add_dependencies(mercury assets)

# Install to destination
include(GNUInstallDirs)
install(TARGETS mercury
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(mercury)

