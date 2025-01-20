# Determine source list
set(APP_SOURCES
main.cpp
)

# The binary
qt_add_executable(mercury ${APP_SOURCES})

# Qt set up
target_link_libraries(mercury PRIVATE Qt6::Widgets)

# Prevents console window on windows (Qt stuff)
set_target_properties(mercury PROPERTIES
    WIN32_EXECUTABLE ON
)

# Add networking library
target_link_libraries(mercury PRIVATE netlib)

# Add dependency on assets in case we want any
add_dependencies(mercury assets)
