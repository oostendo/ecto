find_package(GTest)

if(NOT GTEST_FOUND)
  return()
endif()

include_directories(${GTEST_INCLUDE_DIRS})

add_executable(ecto-test
  main.cpp
  tendril.cpp
  tendrils.cpp
  spore.cpp
  exceptions.cpp
  graph.cpp
  threadpool.cpp
  )

target_link_libraries(ecto-test
  ecto
  ${GTEST_LIBRARIES}
  ${ECTO_DEP_LIBS}
  # this is so that we can create cells that are in the test lib w/o 
  # python importing them
  ecto_test_ectomodule 
  )

add_test(ecto_gtest_units ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ecto-test)
