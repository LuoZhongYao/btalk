add_library(soc)
target_sources(soc PRIVATE
	crc.c
	soc.c
	SysClk.c
	flash.c
	bt.c
	bt_uart.c

	I94100_BSP_V3.05.002/Library/StdDriver/src/clk.c
	I94100_BSP_V3.05.002/Library/StdDriver/src/crc.c
	I94100_BSP_V3.05.002/Library/StdDriver/src/dmic.c
	I94100_BSP_V3.05.002/Library/StdDriver/src/dpwm.c
	I94100_BSP_V3.05.002/Library/StdDriver/src/eadc.c
	I94100_BSP_V3.05.002/Library/StdDriver/src/fmc.c
	I94100_BSP_V3.05.002/Library/StdDriver/src/gpio.c
	I94100_BSP_V3.05.002/Library/StdDriver/src/i2c.c
	I94100_BSP_V3.05.002/Library/StdDriver/src/i2s.c
	I94100_BSP_V3.05.002/Library/StdDriver/src/pdma.c
	I94100_BSP_V3.05.002/Library/StdDriver/src/pwm.c
	I94100_BSP_V3.05.002/Library/StdDriver/src/rtc.c
	I94100_BSP_V3.05.002/Library/StdDriver/src/spi.c
	I94100_BSP_V3.05.002/Library/StdDriver/src/sys.c
	I94100_BSP_V3.05.002/Library/StdDriver/src/timer.c
	I94100_BSP_V3.05.002/Library/StdDriver/src/uart.c
	I94100_BSP_V3.05.002/Library/StdDriver/src/usbd.c
	I94100_BSP_V3.05.002/Library/StdDriver/src/vad.c
	I94100_BSP_V3.05.002/Library/StdDriver/src/wdt.c
	I94100_BSP_V3.05.002/Library/StdDriver/src/wwdt.c
	# I94100_BSP_V3.05.002/Library/StdDriver/src/retarget.c
	I94100_BSP_V3.05.002/Library/Device/Nuvoton/I94100/Source/system_I94100.c
	)

target_compile_definitions(soc PUBLIC -D__HXT=12000000UL)
target_include_directories(soc PUBLIC include)
target_include_directories(soc PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/I94100_BSP_V3.05.002/Library/CMSIS/Include
	${CMAKE_CURRENT_SOURCE_DIR}/I94100_BSP_V3.05.002/Library/StdDriver/inc
	${CMAKE_CURRENT_SOURCE_DIR}/I94100_BSP_V3.05.002/Library/Device/Nuvoton/I94100/Include
	)
