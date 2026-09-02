cmake_minimum_required(VERSION 3.13)

project(pico_keyboard_wifi C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Raspberry Pi Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

pico_sdk_init()

add_executable(keyboard_wifi
    main.c
)

# Link USB Host library
target_link_libraries(keyboard_wifi
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_core
    pico_lwip_driver
    tinyusb_host
    tinyusb_board
)

# Enable USB Host mode
target_compile_definitions(keyboard_wifi PRIVATE
    CFG_TUSB_HOST=1
    CFG_TUH_HID=1
    CFG_TUSB_OS=OPT_OS_PICO
)

# USB Host pin configuration for GPIO 0 (D+) and GPIO 1 (D-)
target_compile_definitions(keyboard_wifi PRIVATE
    PICO_DEFAULT_USB_HOST_PIN_DP=0
    PICO_DEFAULT_USB_HOST_PIN_DM=1
)

pico_add_extra_outputs(keyboard_wifi)
