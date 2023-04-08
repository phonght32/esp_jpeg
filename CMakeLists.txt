set(srcs
    "tjpgd/src/tjpgd.c"
    "esp_jpeg.c")

set(includes 
    "." 
    "tjpgd/include")

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS ${includes})

