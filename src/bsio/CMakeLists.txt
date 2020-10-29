#
#

message("in bsio")

# Static library
add_library(bsio STATIC
           mbbs_io.c
           mbbs_iomrv1.c
           mbbs_iomrv2.c
           mbbs_memalloc.c
           mbbs_misc.c
           mbbs_swapbytes.c
           mbbs_tm.c
           mbbs_wrhdrfields.c
           mbbs_wrsampflags.c)

target_compile_definitions(bsio PRIVATE 
         $<$<COMPILE_LANG_AND_ID:C,GNU>:HAVE_CONFIG_H>
         $<$<COMPILE_LANG_AND_ID:C,CC>:HAVE_CONFIG_H>)


target_include_directories(bsio
                           PUBLIC
                           .
                           ${CMAKE_SOURCE_DIR}/src/mbio
                           ${CMAKE_SOURCE_DIR}/src/mbaux)


                           

