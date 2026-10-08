set(URANIUM_TEST_TARGETS
    math
    numerical
    physics
    render
    equation
    viz)

add_executable(math math.cpp)
target_link_libraries(math PRIVATE uranium::math)
add_executable(numerical numerical.cpp)
target_link_libraries(numerical PRIVATE uranium::math)
add_executable(physics physics.cpp)
target_link_libraries(physics PRIVATE uranium::physics)
add_executable(render render.cpp)
target_link_libraries(render PRIVATE uranium::render)
add_executable(equation equation.cpp)
target_link_libraries(equation PRIVATE uranium::equation)
add_executable(viz viz.cpp)
target_link_libraries(viz PRIVATE uranium::viz)
foreach(target ${URANIUM_TEST_TARGETS})
  target_compile_features(${target} PRIVATE cxx_std_17)
  if(URANIUM_WARNING_FLAGS)
    target_compile_options(${target} PRIVATE ${URANIUM_WARNING_FLAGS})
  endif()
  add_test(NAME ${target} COMMAND ${target})
endforeach()
