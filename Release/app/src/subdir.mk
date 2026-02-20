################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../app/src/app.c \
../app/src/display.c \
../app/src/flash.c \
../app/src/logger.c \
../app/src/sht85.c \
../app/src/task_actuator.c \
../app/src/task_actuator_interface.c \
../app/src/task_adc.c \
../app/src/task_adc_interface.c \
../app/src/task_menu.c \
../app/src/task_menu_interface.c \
../app/src/task_sensor.c \
../app/src/task_sht85.c \
../app/src/task_sht85_interface.c \
../app/src/task_system.c \
../app/src/task_system_interface.c 

OBJS += \
./app/src/app.o \
./app/src/display.o \
./app/src/flash.o \
./app/src/logger.o \
./app/src/sht85.o \
./app/src/task_actuator.o \
./app/src/task_actuator_interface.o \
./app/src/task_adc.o \
./app/src/task_adc_interface.o \
./app/src/task_menu.o \
./app/src/task_menu_interface.o \
./app/src/task_sensor.o \
./app/src/task_sht85.o \
./app/src/task_sht85_interface.o \
./app/src/task_system.o \
./app/src/task_system_interface.o 

C_DEPS += \
./app/src/app.d \
./app/src/display.d \
./app/src/flash.d \
./app/src/logger.d \
./app/src/sht85.d \
./app/src/task_actuator.d \
./app/src/task_actuator_interface.d \
./app/src/task_adc.d \
./app/src/task_adc_interface.d \
./app/src/task_menu.d \
./app/src/task_menu_interface.d \
./app/src/task_sensor.d \
./app/src/task_sht85.d \
./app/src/task_sht85_interface.d \
./app/src/task_system.d \
./app/src/task_system_interface.d 


# Each subdirectory must supply rules for building sources it contributes
app/src/%.o app/src/%.su app/src/%.cyclo: ../app/src/%.c app/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../app/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-app-2f-src

clean-app-2f-src:
	-$(RM) ./app/src/app.cyclo ./app/src/app.d ./app/src/app.o ./app/src/app.su ./app/src/display.cyclo ./app/src/display.d ./app/src/display.o ./app/src/display.su ./app/src/flash.cyclo ./app/src/flash.d ./app/src/flash.o ./app/src/flash.su ./app/src/logger.cyclo ./app/src/logger.d ./app/src/logger.o ./app/src/logger.su ./app/src/sht85.cyclo ./app/src/sht85.d ./app/src/sht85.o ./app/src/sht85.su ./app/src/task_actuator.cyclo ./app/src/task_actuator.d ./app/src/task_actuator.o ./app/src/task_actuator.su ./app/src/task_actuator_interface.cyclo ./app/src/task_actuator_interface.d ./app/src/task_actuator_interface.o ./app/src/task_actuator_interface.su ./app/src/task_adc.cyclo ./app/src/task_adc.d ./app/src/task_adc.o ./app/src/task_adc.su ./app/src/task_adc_interface.cyclo ./app/src/task_adc_interface.d ./app/src/task_adc_interface.o ./app/src/task_adc_interface.su ./app/src/task_menu.cyclo ./app/src/task_menu.d ./app/src/task_menu.o ./app/src/task_menu.su ./app/src/task_menu_interface.cyclo ./app/src/task_menu_interface.d ./app/src/task_menu_interface.o ./app/src/task_menu_interface.su ./app/src/task_sensor.cyclo ./app/src/task_sensor.d ./app/src/task_sensor.o ./app/src/task_sensor.su ./app/src/task_sht85.cyclo ./app/src/task_sht85.d ./app/src/task_sht85.o ./app/src/task_sht85.su ./app/src/task_sht85_interface.cyclo ./app/src/task_sht85_interface.d ./app/src/task_sht85_interface.o ./app/src/task_sht85_interface.su ./app/src/task_system.cyclo ./app/src/task_system.d ./app/src/task_system.o ./app/src/task_system.su ./app/src/task_system_interface.cyclo ./app/src/task_system_interface.d ./app/src/task_system_interface.o ./app/src/task_system_interface.su

.PHONY: clean-app-2f-src

