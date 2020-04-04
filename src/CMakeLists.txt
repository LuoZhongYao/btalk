set(target btalk)
add_executable(${target}.elf)
target_sources(${target}.elf PRIVATE
	main.c
	conn.c
	app.c
	app_echo.c
	app_l2cap.c
	app_led.c
	high_pass_filter.c
	resampler.c
	)

if("${SOC}" STREQUAL "i94100")
	set_property(SOURCE ${CMAKE_SOURCE_DIR}/soc/${SOC}/startup_I94100.S PROPERTY LANGUAGE C)
	target_link_options(${target}.elf PRIVATE -Wl,--gc-sections -Wl,-T${CMAKE_SOURCE_DIR}/soc/${SOC}/gcc_arm_128k.ld)
	target_sources(${target}.elf PRIVATE
		${CMAKE_SOURCE_DIR}/soc/${SOC}/startup_I94100.S
		${CMAKE_SOURCE_DIR}/soc/${SOC}/_syscalls.c
		)
	target_compile_definitions(${target}.elf PRIVATE -DOS_USE_SEMIHOSTING=1
		-D__STARTUP_CLEAR_BSS=1
		-D__HEAP_SIZE=0x8000)
	add_custom_command(TARGET ${target}.elf
		POST_BUILD
		COMMAND ${CMAKE_OBJCOPY} ARGS -O binary ${target}.elf ${target}.bin
		)
endif()

target_link_libraries(${target}.elf PRIVATE soc bluetooth webrtc)
