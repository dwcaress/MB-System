set(MB_HOME ${CMAKE_CURRENT_SOURCE_DIR})
message("MB_HOME: ${MB_HOME}")

message("CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

message("Add build-utils/ subdirectory to module CMAKE_MODULE_PATH")
list(APPEND CMAKE_MODULE_PATH ${MB_HOME}/)

message("CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

message("Call foo()")
foo()

# Test function
function(foo)
  message("this is foo!")
endfunction()
