add_executable(main
  main.cpp
)
target_link_libraries(main hardware_i2c hardware_pwm hardware_adc pico_stdlib)
pico_enable_stdio_usb(main 0)
pico_enable_stdio_uart(main 0)
pico_add_extra_outputs(main)
