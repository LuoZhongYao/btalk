add_library(sbc)
target_sources(sbc PRIVATE
	sbc/sbc.c
	sbc/sbc_primitives_armv6.c
	sbc/sbc_primitives.c
	sbc/sbc_primitives_iwmmxt.c
	sbc/sbc_primitives_mmx.c
	sbc/sbc_primitives_neon.c
	)
target_include_directories(sbc INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
