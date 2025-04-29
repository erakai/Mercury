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
  stream/streaminfo.cpp stream/streaminfo.hpp
  stream/volumecontrolwidget.hpp  stream/volumecontrolwidget.cpp
  stream/mutestreambutton.hpp     stream/mutestreambutton.cpp
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
  singleton/audiomanager.hpp singleton/audiomanager.cpp
        stream/annotationdisplay.cpp
        stream/annotationdisplay.hpp
        stream/painttoolwidget.cpp
        stream/painttoolwidget.hpp
        stream/reactionpanel.cpp
        stream/reactionpanel.hpp
        stream/ReactionDisplay.cpp
        stream/reactiondisplay.hpp
        config/filedownloader.hpp
        config/filedownloader.cpp
        api/mapi.hpp api/mapi.cpp
        streambrowser/streambrowser.h streambrowser/streambrowser.cpp
        stream/streamcontrolpanel.cpp
        stream/streamcontrolpanel.hpp
        stream/streamdisplaycontrols.cpp
        stream/streamdisplaycontrols.hpp
        stream/qualitycontrolbutton.cpp
)

# Qt set up
target_link_libraries(mercury PRIVATE Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Multimedia Qt6::MultimediaWidgets)

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

# Replace Info.plist inside mercury.app with template plist file
if(APPLE)
    set_target_properties(mercury PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/app/Info.plist
        MACOSX_BUNDLE_ICON_FILE mercury.icns
    )

    # Copy MercuryLogo.icns into the app bundle
    add_custom_command(TARGET mercury POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/assets/mercury.icns
        $<TARGET_BUNDLE_DIR:mercury>/Contents/Resources/mercury.icns
    )

    # Copy Info.plist manually after building
    add_custom_command(TARGET mercury POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/app/Info.plist
        $<TARGET_BUNDLE_DIR:mercury>/Contents/Info.plist
    )

    # Copy assets folder into .app/Contents/MacOS/assets
    add_custom_command(TARGET mercury POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_BUNDLE_DIR:mercury>/Contents/MacOS/assets
    )

    # Copy config folder into .app/Contents/MacOS/config
    add_custom_command(TARGET mercury POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/app/config
        $<TARGET_BUNDLE_DIR:mercury>/Contents/MacOS/config
    )
endif()

qt_finalize_executable(mercury)

# Set up testing executable
enable_testing(true)
qt_add_executable(mercury_test ${TEST_SOURCES})
target_link_libraries(mercury_test PRIVATE Qt6::Widgets Qt6::Network Qt6::Multimedia Qt6::Test netlib)



