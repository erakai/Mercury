# Determine source list

# The binary
qt_add_executable(mercury
  MANUAL_FINALIZATION
  stream/streamwindow.cpp stream/streamwindow.hpp
  stream/streamservice.hpp
  stream/sidepane.cpp stream/sidepane.hpp
  stream/streamdisplay.cpp stream/streamdisplay.hpp
  home/main.cpp
  home/mainwindow.cpp home/mainwindow.hpp home/mainwindow.ui
  home/settingswindow.h home/settingswindow.cpp home/settingswindow.ui
  home/hoststreamwindow.h home/hoststreamwindow.cpp home/hoststreamwindow.ui
  home/joinstreamwindow.h home/joinstreamwindow.cpp home/joinstreamwindow.ui
  home/utils.h home/utils.cpp
  configControl/mconfig.cpp
  configControl/mconfig.hpp
  home/toastnotification.h home/toastnotification.cpp

  streampreviewwindow.h streampreviewwindow.cpp

  screenlistmodel.h screenlistmodel.cpp
  windowlistmodel.h windowlistmodel.cpp
  audioinputlistmodel.h audioinputlistmodel.cpp
)

# Qt set up
target_link_libraries(mercury PRIVATE Qt6::Widgets Qt6::Network Qt6::Multimedia Qt6::MultimediaWidgets)

# Curl set up
find_package(CURL REQUIRED)
target_link_libraries(mercury PRIVATE CURL::libcurl)

# # Set include dir
# target_include_directories(mercury
#   PUBLIC ${PROJECT_SOURCE_DIR}/app/stream
# )

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

