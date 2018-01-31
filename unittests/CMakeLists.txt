# cmake file

# requirements

if(ANNOTATELOOPS_SKIP_TESTS)
  message(STATUS "Testing is disabled; skipping unit tests")

  return()
endif()


find_package(GTest)

if(NOT GTEST_FOUND)
  message(WARNING "Could not find GTest; skipping unit tests")

  return()
endif()

find_package(Threads REQUIRED)

# configuration

# aggregate unit test targets under a pseudo-target
add_custom_target(unittests)

set(PRJ_TEST_NAMES Test${PRJ_NAME})

foreach(PRJ_TEST_NAME ${PRJ_TEST_NAMES})
  set(TEST_SOURCES "${PRJ_TEST_NAME}.cpp")

  add_executable(${PRJ_TEST_NAME} ${TEST_SOURCES})

  target_compile_definitions(${PRJ_TEST_NAME} PUBLIC ${LLVM_DEFINITIONS})

  target_include_directories(${PRJ_TEST_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
  target_include_directories(${PRJ_TEST_NAME} PUBLIC ${GTEST_INCLUDE_DIRS})
  target_include_directories(${PRJ_TEST_NAME} PUBLIC ${LLVM_INCLUDE_DIRS})
  target_include_directories(${PRJ_TEST_NAME} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>)
  target_include_directories(${PRJ_TEST_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)

  target_link_libraries(${PRJ_TEST_NAME} PUBLIC ${GTEST_BOTH_LIBRARIES})  
  target_link_libraries(${PRJ_TEST_NAME} PUBLIC ${CMAKE_THREAD_LIBS_INIT})  

  llvm_map_components_to_libnames(LLVM_LIBS 
    core support asmparser analysis passes)

  target_link_libraries(${PRJ_TEST_NAME} PUBLIC ${LLVM_LIBS})

  # setup dependency between unit test targets and their production code
  add_dependencies(${PRJ_TEST_NAME} ${TESTEE_LIB})
  target_link_libraries(${PRJ_TEST_NAME} PUBLIC ${TESTEE_LIB})

  # exclude unit test targets from main build
  set_target_properties(${PRJ_TEST_NAME} PROPERTIES EXCLUDE_FROM_ALL TRUE)

  add_dependencies(unittests ${PRJ_TEST_NAME})

  get_property(PRJ_UNIT_TESTS_EXE TARGET ${PRJ_TEST_NAME} PROPERTY NAME)
endforeach()


set(PRJ_UNIT_TESTS_SCRIPT "run_unit_tests.sh")
get_property(PRJ_UNIT_TESTS_TARGET TARGET unittests PROPERTY NAME)

configure_file("${CMAKE_SOURCE_DIR}/utils/scripts/${PRJ_UNIT_TESTS_SCRIPT}.in" 
  "${CMAKE_BINARY_DIR}/${PRJ_UNIT_TESTS_SCRIPT}"
  @ONLY)


set(TEST_DATA_DIR "${CMAKE_CURRENT_BINARY_DIR}/data")
file(MAKE_DIRECTORY ${TEST_DATA_DIR})

add_custom_command(TARGET unittests PRE_BUILD COMMAND 
  ${CMAKE_COMMAND} -E copy_directory 
  "${CMAKE_CURRENT_SOURCE_DIR}/data/" ${TEST_DATA_DIR})

