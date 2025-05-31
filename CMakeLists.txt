# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

# Add the directory containing i2c_bus and vl53l1 components
set(EXTRA_COMPONENT_DIRS ${CMAKE_CURRENT_LIST_DIR}/components/vl53l1x/components)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
add_compile_options("-Wno-attributes")
project(lvgl_porting)
