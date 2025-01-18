# Example of how this works:

#[[
# imgui ===================================================================
set(IMGUI_PATH "imgui")
file(GLOB IMGUI_SOURCES ${IMGUI_PATH}/*.cpp)
set(IMGUI_SOURCES ${IMGUI_SOURCES}
                  ${IMGUI_PATH}/backends/imgui_impl_sdl2.cpp
                  ${IMGUI_PATH}/backends/imgui_impl_sdlrenderer2.cpp
)
add_library("ImGui" STATIC ${IMGUI_SOURCES})
target_include_directories("ImGui" PUBLIC ${IMGUI_PATH})
target_include_directories("ImGui" PUBLIC ${IMGUI_PATH}/backends)
target_include_directories("ImGui" ${SDL_INCLUDE})
target_link_libraries("ImGui" PRIVATE ${SDL_LINK})
]]#
