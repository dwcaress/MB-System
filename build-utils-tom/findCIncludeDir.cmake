# Find specified C include subdirectory directory and write full path name
# to var (including specified subdirectory name).
# If var already exists, then check for that directory's existence.
# Otherwise try to find directory 
# If not found, exit with error
# NOTES:
# Source code #include always specifies header file WITHOUT leading
# subdirectory. 

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
            ${CMAKE_SYSTEM_PREFIX_PATH} /usr/local/include
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

  # Resolve any symlink
  get_filename_component(realPath ${${var}} REALPATH)
  
  # Is the target file a directory? 
  if (IS_DIRECTORY ${realPath})
    message("target ${${var}} is a directory")
    set(${var} ${${var}} PARENT_SCOPE)
    return()
  endif()

  # Target file is not a directory, presumably header file itself. So 
  # return the directory that contains it.
  message("${${var}} is not a directory or symlink; get enclosing directory")
  message("get_filename_component(tmp ${${var}} DIRECTORY)")
  get_filename_component(tmp ${${var}} DIRECTORY)
  message("set ${var} to ${tmp}")
  set(${var} ${tmp} PARENT_SCOPE)
  
endfunction()
