cmake_minimum_required(VERSION 3.21)

project(ReachVariantToolCore LANGUAGES CXX)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(RVT_DIR "${CMAKE_SOURCE_DIR}/src")

if(NOT EXISTS "${RVT_DIR}/main.cpp")
   message(FATAL_ERROR "ReachVariantTool sources not found at ${RVT_DIR}")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(ZLIB REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS
   "${RVT_DIR}/ui"
   "${RVT_DIR}/ui/main_window"
   "${RVT_DIR}/ui/script_editor"
   "${RVT_DIR}/ui/widgets"
)

file(GLOB_RECURSE RVT_APP_SOURCES CONFIGURE_DEPENDS
   "${RVT_DIR}/*.cpp"
   "${RVT_DIR}/*.h"
   "${RVT_DIR}/*.ui"
   "${RVT_DIR}/Resources/*.qrc"
)

list(FILTER RVT_APP_SOURCES EXCLUDE REGEX ".*\\.bak$")

add_executable(ReachVariantTool ${RVT_APP_SOURCES})
target_include_directories(ReachVariantTool PRIVATE
   "${RVT_DIR}"
   "${RVT_DIR}/ui"
   "${RVT_DIR}/ui/main_window"
   "${RVT_DIR}/ui/script_editor"
   "${RVT_DIR}/ui/widgets"
   "${RVT_DIR}/game_variants/components/megalo"
)
target_link_libraries(ReachVariantTool PRIVATE
   Qt6::Core
   Qt6::Gui
   Qt6::Widgets
   ZLIB::ZLIB
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
   target_compile_options(ReachVariantTool PRIVATE -include climits)
endif()

if(MSVC)
   target_compile_definitions(ReachVariantTool PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

install(TARGETS ReachVariantTool
   RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

if(UNIX AND NOT APPLE)
   install(FILES
      "${RVT_DIR}/Resources/icon_128px.png"
      DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps"
      RENAME "reachvarianttool.png"
   )
   install(FILES
      "${CMAKE_SOURCE_DIR}/packaging/linux/reachvarianttool.desktop"
      DESTINATION "${CMAKE_INSTALL_DATADIR}/applications"
   )
endif()
