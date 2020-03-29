set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m4)

set(CMAKE_CROSSCOMPILING ON)
set(CMAKE_C_COMPILER_WORKS ON)
set(CMAKE_CXX_COMPILER_WORKS ON)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM-ATT_COMPILER arm-none-eabi-as)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color  -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard --specs=nosys.specs")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard --specs=nosys.specs")
