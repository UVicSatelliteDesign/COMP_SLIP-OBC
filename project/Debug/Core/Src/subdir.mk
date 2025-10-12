################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/adc.c \
../Core/Src/camera.c \
../Core/Src/camera_bsp.c \
../Core/Src/camera_recovery.c \
../Core/Src/data_logging.c \
../Core/Src/dcmi.c \
../Core/Src/dma.c \
../Core/Src/flash_interface.c \
../Core/Src/freertos.c \
../Core/Src/gpio.c \
../Core/Src/i2c.c \
../Core/Src/main.c \
../Core/Src/obc.c \
../Core/Src/obc_interface.c \
../Core/Src/ov5640.c \
../Core/Src/ov5640_reg.c \
../Core/Src/retarget.c \
../Core/Src/sdmmc.c \
../Core/Src/spi.c \
../Core/Src/stm32h7xx_hal_msp.c \
../Core/Src/stm32h7xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32h7xx.c \
../Core/Src/ttc.c \
../Core/Src/ttc_interface.c \
../Core/Src/usart.c 

OBJS += \
./Core/Src/adc.o \
./Core/Src/camera.o \
./Core/Src/camera_bsp.o \
./Core/Src/camera_recovery.o \
./Core/Src/data_logging.o \
./Core/Src/dcmi.o \
./Core/Src/dma.o \
./Core/Src/flash_interface.o \
./Core/Src/freertos.o \
./Core/Src/gpio.o \
./Core/Src/i2c.o \
./Core/Src/main.o \
./Core/Src/obc.o \
./Core/Src/obc_interface.o \
./Core/Src/ov5640.o \
./Core/Src/ov5640_reg.o \
./Core/Src/retarget.o \
./Core/Src/sdmmc.o \
./Core/Src/spi.o \
./Core/Src/stm32h7xx_hal_msp.o \
./Core/Src/stm32h7xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32h7xx.o \
./Core/Src/ttc.o \
./Core/Src/ttc_interface.o \
./Core/Src/usart.o 

C_DEPS += \
./Core/Src/adc.d \
./Core/Src/camera.d \
./Core/Src/camera_bsp.d \
./Core/Src/camera_recovery.d \
./Core/Src/data_logging.d \
./Core/Src/dcmi.d \
./Core/Src/dma.d \
./Core/Src/flash_interface.d \
./Core/Src/freertos.d \
./Core/Src/gpio.d \
./Core/Src/i2c.d \
./Core/Src/main.d \
./Core/Src/obc.d \
./Core/Src/obc_interface.d \
./Core/Src/ov5640.d \
./Core/Src/ov5640_reg.d \
./Core/Src/retarget.d \
./Core/Src/sdmmc.d \
./Core/Src/spi.d \
./Core/Src/stm32h7xx_hal_msp.d \
./Core/Src/stm32h7xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32h7xx.d \
./Core/Src/ttc.d \
./Core/Src/ttc_interface.d \
./Core/Src/usart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx -DUSE_PWR_LDO_SUPPLY -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -I/Users/rowanstovin/STM32Cube/Repository/STM32Cube_FW_H7_V1.12.1/Drivers/STM32H7xx_HAL_Driver/Inc -I/Users/rowanstovin/STM32Cube/Repository/STM32Cube_FW_H7_V1.12.1/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I/Users/rowanstovin/STM32Cube/Repository/STM32Cube_FW_H7_V1.12.1/Middlewares/Third_Party/FreeRTOS/Source/include -I/Users/rowanstovin/STM32Cube/Repository/STM32Cube_FW_H7_V1.12.1/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I/Users/rowanstovin/STM32Cube/Repository/STM32Cube_FW_H7_V1.12.1/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I/Users/rowanstovin/STM32Cube/Repository/STM32Cube_FW_H7_V1.12.1/Middlewares/Third_Party/FatFs/src -I/Users/rowanstovin/STM32Cube/Repository/STM32Cube_FW_H7_V1.12.1/Drivers/CMSIS/Device/ST/STM32H7xx/Include -I/Users/rowanstovin/STM32Cube/Repository/STM32Cube_FW_H7_V1.12.1/Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/adc.cyclo ./Core/Src/adc.d ./Core/Src/adc.o ./Core/Src/adc.su ./Core/Src/camera.cyclo ./Core/Src/camera.d ./Core/Src/camera.o ./Core/Src/camera.su ./Core/Src/camera_bsp.cyclo ./Core/Src/camera_bsp.d ./Core/Src/camera_bsp.o ./Core/Src/camera_bsp.su ./Core/Src/camera_recovery.cyclo ./Core/Src/camera_recovery.d ./Core/Src/camera_recovery.o ./Core/Src/camera_recovery.su ./Core/Src/data_logging.cyclo ./Core/Src/data_logging.d ./Core/Src/data_logging.o ./Core/Src/data_logging.su ./Core/Src/dcmi.cyclo ./Core/Src/dcmi.d ./Core/Src/dcmi.o ./Core/Src/dcmi.su ./Core/Src/dma.cyclo ./Core/Src/dma.d ./Core/Src/dma.o ./Core/Src/dma.su ./Core/Src/flash_interface.cyclo ./Core/Src/flash_interface.d ./Core/Src/flash_interface.o ./Core/Src/flash_interface.su ./Core/Src/freertos.cyclo ./Core/Src/freertos.d ./Core/Src/freertos.o ./Core/Src/freertos.su ./Core/Src/gpio.cyclo ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/i2c.cyclo ./Core/Src/i2c.d ./Core/Src/i2c.o ./Core/Src/i2c.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/obc.cyclo ./Core/Src/obc.d ./Core/Src/obc.o ./Core/Src/obc.su ./Core/Src/obc_interface.cyclo ./Core/Src/obc_interface.d ./Core/Src/obc_interface.o ./Core/Src/obc_interface.su ./Core/Src/ov5640.cyclo ./Core/Src/ov5640.d ./Core/Src/ov5640.o ./Core/Src/ov5640.su ./Core/Src/ov5640_reg.cyclo ./Core/Src/ov5640_reg.d ./Core/Src/ov5640_reg.o ./Core/Src/ov5640_reg.su ./Core/Src/retarget.cyclo ./Core/Src/retarget.d ./Core/Src/retarget.o ./Core/Src/retarget.su ./Core/Src/sdmmc.cyclo ./Core/Src/sdmmc.d ./Core/Src/sdmmc.o ./Core/Src/sdmmc.su ./Core/Src/spi.cyclo ./Core/Src/spi.d ./Core/Src/spi.o ./Core/Src/spi.su ./Core/Src/stm32h7xx_hal_msp.cyclo ./Core/Src/stm32h7xx_hal_msp.d ./Core/Src/stm32h7xx_hal_msp.o ./Core/Src/stm32h7xx_hal_msp.su ./Core/Src/stm32h7xx_it.cyclo ./Core/Src/stm32h7xx_it.d ./Core/Src/stm32h7xx_it.o ./Core/Src/stm32h7xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32h7xx.cyclo ./Core/Src/system_stm32h7xx.d ./Core/Src/system_stm32h7xx.o ./Core/Src/system_stm32h7xx.su ./Core/Src/ttc.cyclo ./Core/Src/ttc.d ./Core/Src/ttc.o ./Core/Src/ttc.su ./Core/Src/ttc_interface.cyclo ./Core/Src/ttc_interface.d ./Core/Src/ttc_interface.o ./Core/Src/ttc_interface.su ./Core/Src/usart.cyclo ./Core/Src/usart.d ./Core/Src/usart.o ./Core/Src/usart.su

.PHONY: clean-Core-2f-Src

