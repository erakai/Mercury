# TODO: https://doc.qt.io/qt-6/cmake-get-started.html#building-a-c-gui-application

# Determine source list
set(APP_SOURCES
main.cpp
)

# The binary
add_executable(mercury ${APP_SOURCES})

# Add networking library
target_link_libraries(mercury PRIVATE netlib)

# Add dependency on assets in case we want any
add_dependencies(mercury assets)
