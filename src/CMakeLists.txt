file(GLOB blipkit_HDR "*.h")
file(GLOB blipkit_SRC "*.c")

add_library(blipkit ${blipkit_SRC})

install(TARGETS blipkit DESTINATION lib)
install(FILES ${blipkit_HDR} DESTINATION include/BlipKit)
