cmake_minimum_required(VERSION 3.24)

project(NanoShapeProject)

find_package(Qt6 REQUIRED COMPONENTS Gui Quick)
qt_standard_project_setup()

add_subdirectory(nanoshape)
add_subdirectory(example)
