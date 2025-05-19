
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)

set(FREERTOS_KERNEL_PATH "C:/Dev/FreeRTOS-Kernel")
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(AlertaEnchente C CXX ASM)

pico_sdk_init()

add_executable(AlertaEnchente AlertaEnchente.c 
        ${CMAKE_CURRENT_LIST_DIR}/libs/ssd1306.c
        ${CMAKE_CURRENT_LIST_DIR}/libs/init_config.c
        ${CMAKE_CURRENT_LIST_DIR}/libs/matriz.c)

pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/pio_matrix.pio)

pico_set_program_name(AlertaEnchente "AlertaEnchente")
pico_set_program_version(AlertaEnchente "0.1")

pico_enable_stdio_uart(AlertaEnchente 0)
pico_enable_stdio_usb(AlertaEnchente 1)

target_link_libraries(AlertaEnchente
        pico_stdlib)

target_include_directories(AlertaEnchente PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(AlertaEnchente 
        hardware_gpio
        hardware_i2c
        hardware_adc
        hardware_pwm
        hardware_pio
        hardware_clocks
        FreeRTOS-Kernel 
        FreeRTOS-Kernel-Heap4)

pico_add_extra_outputs(AlertaEnchente)