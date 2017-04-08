cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(quickplotplugin CXX C)

# Default build type
IF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE RelWithDebInfo)
ENDIF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

# Select flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall") # -Wfatal-errors
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-rtti")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -O0")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DDEBUG -O0")

# C++11: Workaround for cmake < 3.1 with no target_compile_features support.
if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  endif()
# include directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
 
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Quick REQUIRED)

get_target_property(QtGUI_location Qt5::Gui LOCATION)
message(STATUS "Qt5Gui found here: " ${QtGUI_location})
#qtquick_compiler_add_resources(RESOURCES qml.qrc)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set (CMAKE_PREFIX_PATH ${QTDIR})

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)
add_definitions(-Wno-deprecated)
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

# The Qt5Widgets_INCLUDES also includes the include directories for
# dependencies QtCore and QtGui
include_directories(${Qt5Widgets_INCLUDE_DIRS} ${Qt5Qml_INCLUDE_DIRS} ${Qt5Quick_INCLUDE_DIRS})

message(STATUS "Qt5 widgets includes: " ${Qt5Widgets_INCLUDE_DIRS})
message(STATUS "Qt5 qml includes: " ${Qt5Qml_INCLUDE_DIRS})
# We need add -DQT_WIDGETS_LIB when using QtWidgets in Qt 5.
add_definitions(${Qt5Widgets_DEFINITIONS})

add_library(quickplotplugin SHARED main.cpp qpcurve.cpp qpfixedscaleengine.cpp qpplotitem.cpp qpscaleengine.cpp qpscrollingcurve.cpp qptightscaleengine.cpp quickplotplugin.cpp)

# Add auxiliary qml files to the project
add_custom_target(main.qml)
add_custom_command(TARGET quickplotplugin PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/imports $<TARGET_FILE_DIR:quickplotplugin)
# make executable
#add_executable(${PROJECT_NAME} ${SRC} ${HEADERS} ${QRC} ${RESOURCES} ${UI})
#target_link_libraries(${PROJECT_NAME} Qt5::Quick Qt5::PrintSupport Qt5::OpenGL
#    libusbDSO libOpenHantek2xxx-5xxx libOpenHantek60xx libDemoDevice libPostprocessingDSO
#    ${OPENGL_LIBRARIES} )
target_compile_features(${PROJECT_NAME} PRIVATE cxx_range_for)

# install commands
# install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ".")

