file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE IMGUI_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/extern/imgui/*.cpp"
)

add_executable(renderer_app
    ${APP_SOURCES}
    ${IMGUI_SOURCES}
)

target_include_directories(renderer_app
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_SOURCE_DIR}/extern/imgui"
        "${CMAKE_SOURCE_DIR}/extern/stb"
        "${CMAKE_SOURCE_DIR}/extern/magic_enum"
)

target_link_libraries(renderer_app
    PRIVATE
        renderer_lib
        assimp
)

target_compile_definitions(renderer_app
    PRIVATE
        IMGUI_IMPL_OPENGL_LOADER_GLAD
        RES_DIR="${CMAKE_SOURCE_DIR}/res"
)