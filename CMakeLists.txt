cmake_minimum_required(VERSION 3.21)

project(ReachVariantToolCore LANGUAGES C CXX)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(RVT_DIR "${CMAKE_SOURCE_DIR}/native/src/ReachVariantTool")
set(RVT_ZLIB_DIR "${CMAKE_SOURCE_DIR}/native/src/zlib")

if(NOT EXISTS "${RVT_DIR}/main.cpp")
   message(FATAL_ERROR "ReachVariantTool sources not found at ${RVT_DIR}")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

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

file(GLOB RVT_ZLIB_SOURCES CONFIGURE_DEPENDS
   "${RVT_ZLIB_DIR}/*.c"
)

add_library(rvt_zlib STATIC ${RVT_ZLIB_SOURCES})
target_include_directories(rvt_zlib PUBLIC "${RVT_ZLIB_DIR}")

if(UNIX AND NOT APPLE)
   target_compile_definitions(rvt_zlib PRIVATE HAVE_UNISTD_H)
endif()

add_executable(ReachVariantTool ${RVT_APP_SOURCES})
target_include_directories(ReachVariantTool PRIVATE
   "${RVT_DIR}"
   "${RVT_DIR}/ui"
   "${RVT_DIR}/ui/main_window"
   "${RVT_DIR}/ui/script_editor"
   "${RVT_DIR}/ui/widgets"
   "${RVT_DIR}/game_variants/components/megalo"
   "${RVT_ZLIB_DIR}"
)
target_link_libraries(ReachVariantTool PRIVATE
   Qt6::Core
   Qt6::Gui
   Qt6::Widgets
   rvt_zlib
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
   target_compile_options(ReachVariantTool PRIVATE -include climits)
endif()

if(MSVC)
   target_compile_definitions(ReachVariantTool PRIVATE _CRT_SECURE_NO_WARNINGS)
   target_compile_definitions(rvt_zlib PRIVATE _CRT_SECURE_NO_WARNINGS)
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
