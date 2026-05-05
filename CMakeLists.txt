cmake_minimum_required(VERSION 3.20) # CMake install : https://cmake.org/download/
project(CatNote LANGUAGES CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# set(CMAKE_PREFIX_PATH "F:/Qt/6.10.2/mingw_64") # Qt Kit Dir
# set(CMAKE_C_COMPILER "F:/Qt/Tools/mingw1310_64/bin/gcc.exe")
# set(CMAKE_CXX_COMPILER "F:/Qt/Tools/mingw1310_64/bin/g++.exe")
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set_source_files_properties(main.cpp PROPERTIES SKIP_AUTOMOC TRUE)
find_package(Qt6 COMPONENTS Core Gui Widgets Multimedia MultimediaWidgets OpenGLWidgets Svg REQUIRED) # Qt COMPONENTS
aux_source_directory(./src srcs)

# # 设置静态链接
# if(WIN32)
#     set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
# elseif(APPLE)
#     set(CMAKE_EXE_LINKER_FLAGS "-framework CoreFoundation -framework AppKit")
# else()
#     set(CMAKE_EXE_LINKER_FLAGS "-static")
# endif()

# Specify MSVC UTF-8 encoding   
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h" "src/*.hpp")
file(GLOB RESOURCE_FILES "src/*.qrc")

add_executable(${PROJECT_NAME}
    WIN32 # If you need a terminal for debug, please comment this statement 
    # ${srcs} 
    ${SOURCES}  # 使用SOURCES而不是srcs
    ${HEADERS}  # 也添加头文件
    ${RESOURCE_FILES}
) 
target_include_directories(CatNote PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    Qt6::OpenGLWidgets
    Qt6::Svg
)