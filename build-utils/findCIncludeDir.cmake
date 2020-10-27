# Find C include directory with specified name
function(findCIncludeDir directory var)
  message("***findCIncludeDir(): look for ${directory} in ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES}")
  find_file(${var} ${directory}
            PATHS ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES}
            NO_DEFAULT_PATH
            REQUIRED)

message("***Found ${directory}: ${var}")
# Set value in parent scope
set(${var} ${${var}} PARENT_SCOPE)
  
endfunction()
