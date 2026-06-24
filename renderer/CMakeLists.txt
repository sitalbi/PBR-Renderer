add_library(renderer_lib)

file(GLOB_RECURSE RENDERER_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE RENDERER_HEADERS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

target_sources(renderer_lib
        PRIVATE
        ${RENDERER_SOURCES}
        ${RENDERER_HEADERS}
)

target_include_directories(renderer_lib
        PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_SOURCE_DIR}/extern/stb"
        "${CMAKE_SOURCE_DIR}/extern/magic_enum"
)

target_link_libraries(renderer_lib
        PUBLIC
        glad
        glfw
        glm
)

target_compile_definitions(renderer_lib
        PUBLIC
        RES_DIR="${CMAKE_SOURCE_DIR}/res"
)