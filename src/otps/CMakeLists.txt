message("In src/otps")

add_executable(mbotps mbotps.c otps.h)

file(WRITE otps.h
	"char *otps_location = \"${otpsDir}\";")
 
target_link_libraries(mbotps PRIVATE mbio)

install(TARGETS mbotps
	DESTINATION ${CMAKE_INSTALL_BINDIR})
