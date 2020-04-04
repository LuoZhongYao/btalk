target_sources(soc PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/soc.c
	${CMAKE_CURRENT_SOURCE_DIR}/flash.c
	${CMAKE_CURRENT_SOURCE_DIR}/bt.c)

target_link_options(soc INTERFACE -lpthread)
target_compile_options(soc PUBLIC -D__BLUEZ__=1)
