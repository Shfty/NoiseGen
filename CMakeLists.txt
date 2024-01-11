cmake_minimum_required(VERSION 3.28)
project(TextTracer LANGUAGES CXX)

add_executable(
    Main

    main.cpp

    src/ShaderProgram.cpp
    src/WorleyNoise.cpp
    src/WorleyNoiseGL.cpp
)

target_include_directories(
    Main

    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

find_package(PkgConfig REQUIRED)

pkg_search_module(GL REQUIRED opengl)
pkg_search_module(GLFW REQUIRED glfw3)
pkg_search_module(GLEW REQUIRED glew)

include_directories(${GLFW_INCLUDE_DIRS})

target_link_libraries(Main ${GL_LIBRARIES})
target_link_libraries(Main ${GL_STATIC_LIBRARIES})

target_link_libraries(Main ${GLEW_LIBRARIES})
target_link_libraries(Main ${GLEW_STATIC_LIBRARIES})

target_link_libraries(Main ${GLFW_LIBRARIES})
target_link_libraries(Main ${GLFW_STATIC_LIBRARIES})
