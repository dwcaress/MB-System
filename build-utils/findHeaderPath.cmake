# Find specified target file, which may be a C/C++ header or may be
# a directory containing headers. Set contents of var to the absolute
# directory path leading to the target.
# If var already exists, then check for $var's existence.

function(findHeaderPath target pathVar)
  if (${pathVar})
    message("${pathVar} is already set to ${${pathVar}}; ${target} better be there!")
    if (NOT(EXISTS ${${pathVar}}))
      message("${${pathVar}} doesn't exist")
      message(FATAL_ERROR
              "Specify ${target} C header files directory with -D ${pathVar}=<target> on command line")        
    endif()
    return()
    
  endif()
  message("${pathVar} not yet set")  
  message("***findHeaderPath(): look for ${target} in ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES} ${CMAKE_SYSTEM_PREFIX_PATH} ${CMAKE_SYSTEM_APPBUNDLE_PATH}")

  # Find specified file
  find_file(${pathVar} ${target}
            PATHS ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES}
            ${CMAKE_SYSTEM_PREFIX_PATH} /usr/local/include
            ${CMAKE_SYSTEM_PREFIX_PATH}
	    ${CMAKE_SYSTEM_APPBUNDLE_PATH}
            NO_DEFAULT_PATH)

  message("***result: ${target}: ${${pathVar}}")
  if (${pathVar} MATCHES "-NOTFOUND$")
    # Couldn't find target
    message("Could not find include target for ${target}")
    message(FATAL_ERROR
            "Specify ${target} location with -D ${pathVar}=<target> on command line")  

  endif()

  # Resolve any symlink
  get_filename_component(realPath ${${pathVar}} REALPATH)
  
  # Is the target file a directory? 
  if (IS_DIRECTORY ${realPath})
    message("target ${${pathVar}} is a directory")
    set(${pathVar} ${${pathVar}} PARENT_SCOPE)
    return()
  endif()

  # Target file is not a directory, presumably header file itself. So 
  # return the absolute directory that contains it.
  message("${${pathVar}} is not a directory or symlink; get enclosing directory")
  message("get_filename_component(tmp ${${pathVar}} DIRECTORY)")
  get_filename_component(tmp ${${pathVar}} DIRECTORY)
  message("set ${pathVar} to ${tmp}")
  set(${pathVar} ${tmp} PARENT_SCOPE)
  
endfunction()
