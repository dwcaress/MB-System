# Find specified C include subdirectory directory and write full path name
# to var. If var already exists, then check for that directory's existence.
# Otherwise try to find directory 
# If not found, exit with error
function(findCIncludeDir directory var)
  if (${var})
    message("${var} is already set to ${${var}}; ${directory} better be there!")
    if (NOT(EXISTS ${${var}}))
      message("${${var}} doesn't exist")
      message(FATAL_ERROR
              "Specify ${directory} C header files directory with -D ${var}=<directory> on command line")        
    endif()
    return()
    
  endif()
  message("${var} not yet set")  
  message("***findCIncludeDir(): look for ${directory} in ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES} ${CMAKE_SYSTEM_PREFIX_PATH} ${CMAKE_SYSTEM_APPBUNDLE_PATH}")

  # Find specified file
  find_file(${var} ${directory}
            PATHS ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES}
            ${CMAKE_SYSTEM_PREFIX_PATH}
	    ${CMAKE_SYSTEM_APPBUNDLE_PATH}
              NO_DEFAULT_PATH)

  message("***result: ${directory}: ${${var}}")
  if (${var} MATCHES "-NOTFOUND$")
    # Couldn't find directory
    message("Could not find include directory for ${directory}")
    message(FATAL_ERROR
            "Specify ${directory} location with -D ${var}=<directory> on command line")  
  endif()

  # Set value in parent scope
  set(${var} ${${var}} PARENT_SCOPE)
  
endfunction()
