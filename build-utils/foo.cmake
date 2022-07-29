set(MB_HOME ${CMAKE_CURRENT_SOURCE_DIR})
message("MB_HOME: ${MB_HOME}")

message("CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

message("Add build-utils/ subdirectory to module CMAKE_MODULE_PATH")
list(APPEND CMAKE_MODULE_PATH ${MB_HOME}/)

message("CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

message("Call foo()")
foo()

# Test function
message("call execute_process()")
execute_process(COMMAND "ls" "-l"
		WORKING_DIRECTORY "/Users/mappingauv"
		OUTPUT_VARIABLE myOutput
		ERROR_VARIABLE myError
		RESULT_VARIABLE status
		)

message("output: ${myOutput}")
message("error: ${myError}")
message("status: ${status}")
