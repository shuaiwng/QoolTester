cmake_minimum_required(VERSION 3.16)
project(qooltester)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(thirdparty)

# add_subdirectory(tests)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets UiTools PrintSupport)

set(TINYXML_FILE thirdparty/tinyxml2/tinyxml2.cpp)
set(SOURCE_FILES
    src/main.cpp
    src/project.cpp
    src/mainwindow.cpp
    src/utils.cpp
    src/reporter.cpp
)

set (HEADER_FILES
    src/project.h
    src/mainwindow.h
    src/utils.h
    src/reporter.h
)

set (RC_FOLDER
    assets
)


set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES} ${TINYXML_FILE} "${RC_FOLDER}/icon.rc")

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets Qt6::UiTools Qt6::PrintSupport)


add_custom_command(TARGET ${CMAKE_PROJECT_NAME} PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       "${CMAKE_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/assets")


set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
