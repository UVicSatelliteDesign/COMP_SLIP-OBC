################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/ov5640.c \
../Drivers/ov5640_reg.c 

OBJS += \
./Drivers/ov5640.o \
./Drivers/ov5640_reg.o 

C_DEPS += \
./Drivers/ov5640.d \
./Drivers/ov5640_reg.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/%.o Drivers/%.su Drivers/%.cyclo: ../Drivers/%.c Drivers/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers

clean-Drivers:
	-$(RM) ./Drivers/ov5640.cyclo ./Drivers/ov5640.d ./Drivers/ov5640.o ./Drivers/ov5640.su ./Drivers/ov5640_reg.cyclo ./Drivers/ov5640_reg.d ./Drivers/ov5640_reg.o ./Drivers/ov5640_reg.su

.PHONY: clean-Drivers

