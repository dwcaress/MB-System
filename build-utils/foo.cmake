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
