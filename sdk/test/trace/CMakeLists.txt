foreach(
  testname
  tracer_provider_test
  span_data_test
  simple_processor_test
  tracer_test
  always_off_sampler_test
  always_on_sampler_test
  parent_or_else_sampler_test
  probability_sampler_test
  batch_span_processor_test
  attribute_utils_test)
  add_executable(${testname} "${testname}.cc")
  target_link_libraries(
    ${testname} ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
    opentelemetry_common opentelemetry_trace)
  gtest_add_tests(TARGET ${testname} TEST_PREFIX trace. TEST_LIST ${testname})
endforeach()

add_executable(sampler_benchmark sampler_benchmark.cc)
target_link_libraries(sampler_benchmark benchmark::benchmark
                      ${CMAKE_THREAD_LIBS_INIT} opentelemetry_trace)
