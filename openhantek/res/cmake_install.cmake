# Install script for directory: /opt/ucc/hantek/openhantek-uli/openhantek/res

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/images" TYPE FILE FILES
    "/opt/ucc/hantek/openhantek-uli/openhantek/res/images/actions/open.png"
    "/opt/ucc/hantek/openhantek-uli/openhantek/res/images/actions/save.png"
    "/opt/ucc/hantek/openhantek-uli/openhantek/res/images/actions/zoom.png"
    "/opt/ucc/hantek/openhantek-uli/openhantek/res/images/actions/digitalphosphor.png"
    "/opt/ucc/hantek/openhantek-uli/openhantek/res/images/actions/export-as.png"
    "/opt/ucc/hantek/openhantek-uli/openhantek/res/images/actions/print.png"
    "/opt/ucc/hantek/openhantek-uli/openhantek/res/images/actions/stop.png"
    "/opt/ucc/hantek/openhantek-uli/openhantek/res/images/actions/cursors.png"
    "/opt/ucc/hantek/openhantek-uli/openhantek/res/images/actions/start.png"
    "/opt/ucc/hantek/openhantek-uli/openhantek/res/images/actions/save-as.png"
    "/opt/ucc/hantek/openhantek-uli/openhantek/res/images/openhantek.png"
    "/opt/ucc/hantek/openhantek-uli/openhantek/res/images/config/analysis.png"
    "/opt/ucc/hantek/openhantek-uli/openhantek/res/images/config/files.png"
    "/opt/ucc/hantek/openhantek-uli/openhantek/res/images/config/colors.png"
    "/opt/ucc/hantek/openhantek-uli/openhantek/res/images/config/scope.png"
    "/opt/ucc/hantek/openhantek-uli/openhantek/res/images/config/options.png"
    "/opt/ucc/hantek/openhantek-uli/openhantek/res/images/openhantek.icns"
    "/opt/ucc/hantek/openhantek-uli/openhantek/res/images/actions/digitalphosphor.svg"
    "/opt/ucc/hantek/openhantek-uli/openhantek/res/images/openhantek.svg"
    "/opt/ucc/hantek/openhantek-uli/openhantek/res/images/config/options.svg"
    "/opt/ucc/hantek/openhantek-uli/openhantek/res/images/config/analysis.svg"
    )
endif()

