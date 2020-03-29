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

target_link_libraries(${target}.elf PRIVATE bluetooth webrtc)

add_custom_command(TARGET ${target}.elf
	POST_BUILD
	COMMAND ${CMAKE_OBJCOPY} ARGS -O binary ${target}.elf ${target}.bin
	)
