# Determine source list

# The binary
qt_add_executable(mercury
  MANUAL_FINALIZATION
  src/main.cpp
  src/mainwindow.cpp src/mainwindow.hpp src/mainwindow.ui
  src/settingswindow.h src/settingswindow.cpp src/settingswindow.ui
  src/hoststreamwindow.h src/hoststreamwindow.cpp src/hoststreamwindow.ui
  src/joinstreamwindow.h src/joinstreamwindow.cpp src/joinstreamwindow.ui
  src/utils.h src/utils.cpp
)

# Qt set up
target_link_libraries(mercury PRIVATE Qt6::Widgets Qt6::Network)

# Set include dir
target_include_directories(mercury
  PUBLIC ${PROJECT_SOURCE_DIR}/app/include
  PUBLIC ${PROJECT_SOURCE_DIR}/app/ui
)

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

