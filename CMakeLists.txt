cmake_minimum_required(VERSION 3.16)
project(uranium VERSION 1.0.0 LANGUAGES CXX)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
option(URANIUM_BUILD_SHARED "Build render, viz and equation as shared libraries" OFF)
option(URANIUM_BUILD_EXAMPLES "Build the example programs" ON)
option(URANIUM_BUILD_TESTS "Build the test programs" ON)
set(URANIUM_LIB_TYPE STATIC)
if(URANIUM_BUILD_SHARED OR BUILD_SHARED_LIBS)
  set(URANIUM_LIB_TYPE SHARED)
endif()
set(URANIUM_WARNING_FLAGS "")
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  set(URANIUM_WARNING_FLAGS -Wall -Wextra)
endif()
function(uranium_apply_warnings target)
  if(URANIUM_WARNING_FLAGS)
    target_compile_options(${target} PRIVATE ${URANIUM_WARNING_FLAGS})
  endif()
endfunction()
function(uranium_apply_api target)
  if(URANIUM_LIB_TYPE STREQUAL "SHARED")
    target_compile_definitions(${target} PUBLIC URANIUM_SHARED PRIVATE URANIUM_BUILDING_LIBRARY)
  endif()
  set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE ON)
  if(URANIUM_WARNING_FLAGS)
    target_compile_options(${target} PRIVATE ${URANIUM_WARNING_FLAGS})
  endif()
endfunction()
add_library(uranium_math INTERFACE)
add_library(uranium::math ALIAS uranium_math)
target_include_directories(uranium_math INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_features(uranium_math INTERFACE cxx_std_17)
set_target_properties(uranium_math PROPERTIES EXPORT_NAME math)
add_library(uranium_physics INTERFACE)
add_library(uranium::physics ALIAS uranium_physics)
target_link_libraries(uranium_physics INTERFACE uranium_math)
target_compile_features(uranium_physics INTERFACE cxx_std_17)
set_target_properties(uranium_physics PROPERTIES EXPORT_NAME physics)
add_library(uranium_render ${URANIUM_LIB_TYPE}
    src/render/font.cpp
    src/render/raster.cpp
    src/render/svg.cpp
    src/render/stb_impl.cpp
    src/render/transform.cpp)
add_library(uranium::render ALIAS uranium_render)
target_include_directories(uranium_render
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_features(uranium_render PUBLIC cxx_std_17)
target_compile_definitions(uranium_render PRIVATE
    URANIUM_FONT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts"
    URANIUM_INSTALL_FONT_DIR="${CMAKE_INSTALL_FULL_DATADIR}/uranium/fonts")
uranium_apply_api(uranium_render)
set_target_properties(uranium_render PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

add_library(uranium_viz ${URANIUM_LIB_TYPE}
    src/viz/axes.cpp
    src/viz/figure.cpp
    src/viz/theme.cpp)
add_library(uranium::viz ALIAS uranium_viz)
target_link_libraries(uranium_viz PUBLIC uranium_render uranium_math)
target_include_directories(uranium_viz
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_features(uranium_viz PUBLIC cxx_std_17)
uranium_apply_api(uranium_viz)
set_target_properties(uranium_viz PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

add_library(uranium_equation ${URANIUM_LIB_TYPE}
    src/equation/equation.cpp)
add_library(uranium::equation ALIAS uranium_equation)
target_link_libraries(uranium_equation PUBLIC uranium_render uranium_math)
target_include_directories(uranium_equation
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_features(uranium_equation PUBLIC cxx_std_17)
uranium_apply_api(uranium_equation)
set_target_properties(uranium_equation PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

add_library(uranium INTERFACE)
add_library(uranium::uranium ALIAS uranium)
target_link_libraries(uranium INTERFACE
    uranium::math uranium::physics uranium::render uranium::viz uranium::equation)
set_target_properties(uranium PROPERTIES EXPORT_NAME uranium)
if(URANIUM_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
if(URANIUM_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
install(DIRECTORY include/uranium DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS
    uranium_math
    uranium_physics
    uranium_render
    uranium_viz
    uranium_equation
    uranium
    EXPORT UraniumTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT UraniumTargets
    FILE UraniumTargets.cmake
    NAMESPACE uranium::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/uranium)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/UraniumConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/uraniumConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/uranium)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/uraniumConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/uraniumConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/uraniumConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/uranium)

install(DIRECTORY assets/fonts/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/uranium/fonts)
