
# Generate some MB-System header files 
## message("Now in buildHeaders")
### buildHeaders("xyz")

function(buildHeaders)

   message("otpsDir: ${otpsDir}")

   execute_process(COMMAND "echo" "char *otps_location = \"/usr/local/opt/otps\";"
		OUTPUT_FILE "/tmp/otps.h"
		)

     execute_process(COMMAND "echo" "char *otps_location = \"/usr/local/opt/otps\";"
		OUTPUT_FILE "/tmp/part1"
		)

  execute_process(COMMAND "echo" "const char *levitusfile = \"/usr/local/share/mbsystem/LevitusAnnual82.dat\";"
		OUTPUT_FILE "/tmp/part2"
		)

  execute_process(COMMAND "cat" "/tmp/part1" "/tmp/part2"
		OUTPUT_FILE "/tmp/levitus.h")
		
endfunction()

