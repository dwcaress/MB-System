message("In test/svptools")

add_executable(mbsvp_tools_test mbsvp_tools_test.cc)
target_compile_features(mbsvp_tools_test PRIVATE cxx_std_17)
target_link_libraries(mbsvp_tools_test PRIVATE mbsvptools GTest::gmock_main)
add_test(NAME mbsvp_tools_test COMMAND mbsvp_tools_test)
