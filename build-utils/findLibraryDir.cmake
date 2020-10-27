# Look for specified library, set var to corresponding directory
function(findLibraryDir library var)
  message("findLibraryDir() - look for ${library}")
  unset(LIBFILE CACHE)
  find_library(LIBFILE ${library} REQUIRED)
  message("findLibraryDir(); found ${library}: ${LIBFILE}")  
  get_filename_component(tmp ${LIBFILE} DIRECTORY)
  set(${var} ${tmp} PARENT_SCOPE)
endfunction()
