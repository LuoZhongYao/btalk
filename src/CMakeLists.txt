set(target btalk)
add_executable(${target}.elf)
target_sources(${target}.elf PRIVATE
	${CMAKE_SOURCE_DIR}/soc/startup_I94100.S
	${CMAKE_SOURCE_DIR}/soc/_syscalls.c
	main.c
	conn.c
	app.c
	app_echo.c
	app_l2cap.c
	app_led.c
	high_pass_filter.c
	resampler.c
	)

target_link_libraries(${target}.elf PRIVATE soc bluetooth webrtc)
target_compile_definitions(${target}.elf PRIVATE -DOS_USE_SEMIHOSTING=1
	-D__STARTUP_CLEAR_BSS=1
	-D__HEAP_SIZE=0xf000)
set_property(SOURCE ${CMAKE_SOURCE_DIR}/soc/startup_I94100.S PROPERTY LANGUAGE C)

add_custom_command(TARGET ${target}.elf
	POST_BUILD
	COMMAND ${CMAKE_OBJCOPY} ARGS -O binary ${target}.elf ${target}.bin
	)
