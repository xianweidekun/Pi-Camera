# CMakeLists.txt for CinePI RAW Recorder
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(cinepi_raw_recorder)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 查找依赖库
find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBCAMERA REQUIRED libcamera)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)

# 包含目录
include_directories("${PROJECT_SOURCE_DIR}/src/shared")
include_directories(${LIBCAMERA_INCLUDE_DIRS})
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2_TTF_INCLUDE_DIRS})

# 添加共享库源文件
set(SHARED_SOURCES
    src/shared/camera_controller.cpp
    src/shared/sdl_helper.cpp
)

# 添加主程序源文件
set(MAIN_SOURCE
    cinepi_raw_recorder.cpp
)

# 链接库
link_directories(${LIBCAMERA_LIBRARY_DIRS})
link_directories(${SDL2_LIBRARY_DIRS})
link_directories(${SDL2_TTF_LIBRARY_DIRS})

# 创建可执行文件
add_executable(cinepi_raw_recorder ${MAIN_SOURCE} ${SHARED_SOURCES})

# 链接依赖
target_link_libraries(cinepi_raw_recorder ${LIBCAMERA_LIBRARIES})
target_link_libraries(cinepi_raw_recorder ${SDL2_LIBRARIES})
target_link_libraries(cinepi_raw_recorder ${SDL2_TTF_LIBRARIES})

# 设置输出目录
set_target_properties(cinepi_raw_recorder PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build)
