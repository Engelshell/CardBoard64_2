cmake_minimum_required(VERSION 3.5)


set(PICO_BOARD pico CACHE STRING "Board type")
include(pico_sdk_import.cmake)
if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(PROJECT cardboard64)


project(${PROJECT} C CXX ASM)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(${PROJECT})

pico_set_program_name(${PROJECT} ${PROJECT})
pico_set_program_version(${PROJECT} "0.1")

# Create C header file with the name <pio program>.pio.h
pico_generate_pio_header(${PROJECT}  
        ${CMAKE_CURRENT_LIST_DIR}/src/bus.pio
)

# Example source
target_sources(${PROJECT} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/usbdisk.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/usb_descriptors.c
        ${CMAKE_CURRENT_LIST_DIR}/src/virtfat16.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/cart.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/cartbuspio.hpp
        )

# Example include
target_include_directories(${PROJECT} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/src
)


target_link_libraries(${PROJECT}
pico_stdlib 
hardware_dma
hardware_pio
hardware_interp
hardware_timer
hardware_clocks
tinyusb_device 
tinyusb_board 
)

pico_enable_stdio_usb(${PROJECT} 1)
pico_enable_stdio_uart(${PROJECT} 0)
# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(${PROJECT})

#url to a place
pico_set_program_url(${PROJECT} "")