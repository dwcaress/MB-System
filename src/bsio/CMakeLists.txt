#
#

message("in bsio")

# Static library
add_library(mbbsio STATIC
           mbbs_io.c
           mbbs_iomrv1.c
           mbbs_iomrv2.c
           mbbs_memalloc.c
           mbbs_misc.c
           mbbs_swapbytes.c
           mbbs_tm.c
           mbbs_wrhdrfields.c
           mbbs_wrsampflags.c)

target_compile_definitions(mbbsio PRIVATE 
         $<$<COMPILE_LANG_AND_ID:C,GNU>:HAVE_CONFIG_H>
         $<$<COMPILE_LANG_AND_ID:C,CC>:HAVE_CONFIG_H>)


target_include_directories(mbbsio
                           PUBLIC
                           .
                           ${CMAKE_SOURCE_DIR}/src/mbio
                           ${CMAKE_SOURCE_DIR}/src/mbaux)


                           


# Shared library
add_library(mbbsioShared SHARED
           mbbs_io.c
           mbbs_iomrv1.c
           mbbs_iomrv2.c
           mbbs_memalloc.c
           mbbs_misc.c
           mbbs_swapbytes.c
           mbbs_tm.c
           mbbs_wrhdrfields.c
           mbbs_wrsampflags.c)

target_compile_definitions(mbbsioShared PRIVATE 
         $<$<COMPILE_LANG_AND_ID:C,GNU>:HAVE_CONFIG_H>
         $<$<COMPILE_LANG_AND_ID:C,CC>:HAVE_CONFIG_H>)


set_target_properties(mbbsioShared PROPERTIES OUTPUT_NAME mbbsio)

target_include_directories(mbbsioShared
                           PUBLIC
                           .
                           ${CMAKE_SOURCE_DIR}/src/mbio
                           ${CMAKE_SOURCE_DIR}/src/mbaux)


                           

