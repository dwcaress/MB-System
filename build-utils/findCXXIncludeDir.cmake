
# Find C++ include directory with specified name
function(findCXXIncludeDir directory var)
  message("***findCXXIncludeDir(): look for ${directory}")
  find_file(${var} ${directory}
            PATHS ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}
            NO_DEFAULT_PATH
            REQUIRED)

  message("***Found ${directory}: ${var}")  
  set(${var} ${${var}} PARENT_SCOPE)
  
endfunction()


