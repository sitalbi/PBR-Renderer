file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE APP_HEADERS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

file(GLOB_RECURSE IMGUI_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/extern/imgui/*.cpp"
)

file(GLOB_RECURSE IMGUIZMO_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/extern/imguizmo/*.cpp"
)

add_executable(renderer_app)

target_sources(renderer_app
        PRIVATE
        ${APP_SOURCES}
        ${APP_HEADERS}
        ${IMGUI_SOURCES}
        ${IMGUIZMO_SOURCES}
)

target_include_directories(renderer_app
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_SOURCE_DIR}/extern/imgui"
        "${CMAKE_SOURCE_DIR}/extern/stb"
        "${CMAKE_SOURCE_DIR}/extern/magic_enum"
        "${CMAKE_SOURCE_DIR}/extern/imguizmo"
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