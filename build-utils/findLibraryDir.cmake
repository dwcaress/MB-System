# Look for specified library, set specified var to corresponding directory
# If var is already set, then check for its existence.
# Exit if library cannot be found.
function(findLibraryDir library var)
  message("findLibraryDir() - look for ${library}")

  unset(LIBFILE CACHE)

  if (${var})
    message("${var} is already set to ${${var}}; ${library} better be there!")
    if (NOT (EXISTS ${${var}}))
      message("Could not access library directory ${${var}} specified by ${var}")
      message(FATAL_ERROR
              "Specify library ${library} directory with "
              "-D ${var}=<directory> on command line")          
    endif()
    
    find_library(LIBFILE ${library}
                 PATHS ${${var}}
                 NO_DEFAULT_PATH)
  else()
    find_library(LIBFILE ${library})
  endif()
  
  if (LIBFILE MATCHES "-NOTFOUND$")
    message("Could not find include library ${library}")
    message(FATAL_ERROR
            "Specify library ${library} directory with -D ${var}=<directory> on command line")    
  endif()
  
  message("findLibraryDir(); result ${library}: ${LIBFILE}")
  # Will return library directory component
  get_filename_component(tmp ${LIBFILE} DIRECTORY)
  set(${var} ${tmp} PARENT_SCOPE)
endfunction()
