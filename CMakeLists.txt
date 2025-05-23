cmake_minimum_required(VERSION 3.10)

project(XGame VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# ���� � ����������� �������
set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")
set(DEST_ASSETS_DIR "${CMAKE_BINARY_DIR}/assets")

# ���� � ����������
file(GLOB_RECURSE SOURCES
    "src/*.cpp" "src/*.c"
    "src/core/*.cpp" "src/systemc/*.cpp" "src/utils/*.cpp"
    "include/core/*.cpp" "include/systems/*.cpp" "include/utils/*.cpp"
)

# ����������� include-����������
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/lib/stb_image
    ${PROJECT_SOURCE_DIR}/lib/glm
    ${PROJECT_SOURCE_DIR}/lib/assimp/include
    ${PROJECT_SOURCE_DIR}/lib/glfw/include
    ${PROJECT_SOURCE_DIR}/lib/glad/include
)

# ���� � �����������
set(GLFW_PATH ${PROJECT_SOURCE_DIR}/lib/glfw)
set(GLAD_PATH ${PROJECT_SOURCE_DIR}/lib/glad)
set(STB_IMAGE_PATH ${PROJECT_SOURCE_DIR}/lib/stb_image)

link_directories(${GLFW_PATH}/lib-vc2022 ${GLAD_PATH}/lib)

# ������ ���������� ����
add_executable(${PROJECT_NAME} ${SOURCES})

# ������� ����������
target_link_libraries(${PROJECT_NAME}
    glfw3.lib
    opengl32.lib
    ${CMAKE_DL_LIBS}
)

# ? ����������� ������� � ����
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${ASSETS_DIR} ${DEST_ASSETS_DIR}
)

# ���� ����������� ��� MSVC
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /std:c++17)
endif()
