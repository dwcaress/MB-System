message("In src/otps")

add_executable(mbotps mbotps.c otps.h)

file(GENERATE OUTPUT otps.h
	CONTENT "char *otps_location = \"${otpsDir}\";")
 
target_link_libraries(mbotps PRIVATE mbio)

install(TARGETS mbotps
	DESTINATION ${CMAKE_INSTALL_BINDIR})
