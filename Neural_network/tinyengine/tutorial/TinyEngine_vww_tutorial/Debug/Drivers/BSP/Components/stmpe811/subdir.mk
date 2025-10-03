################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/Components/stmpe811/stmpe811.c 

C_DEPS += \
./Drivers/BSP/Components/stmpe811/stmpe811.d 

OBJS += \
./Drivers/BSP/Components/stmpe811/stmpe811.o 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/Components/stmpe811/%.o Drivers/BSP/Components/stmpe811/%.su Drivers/BSP/Components/stmpe811/%.cyclo: ../Drivers/BSP/Components/stmpe811/%.c Drivers/BSP/Components/stmpe811/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=c11 -g -DSTM32F746xx -c -I/Users/am/Desktop/HKU/InnoWing/project/tinyengine/TinyEngine/include -I/Users/am/Desktop/HKU/InnoWing/project/tinyengine/TinyEngine/src/kernels/int_forward_op -I../Src -I../../../TinyEngine/include -I"/Users/am/Desktop/HKU/InnoWing/project/mit-han-lab/tinyengine/tutorial/TinyEngine_vww_tutorial/Src/TinyEngine/include" -I"/Users/am/Desktop/HKU/InnoWing/project/mit-han-lab/tinyengine/tutorial/TinyEngine_vww_tutorial/Src/TinyEngine/include/arm_cmsis" -I"/Users/am/Desktop/HKU/InnoWing/project/mit-han-lab/tinyengine/tutorial/TinyEngine_vww_tutorial/Src/TinyEngine/codegen/Include" -I../Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/BSP/STM32746G-Discovery -I../Drivers/CMSIS/Include -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../TinyEngine/include -I../../TinyEngine/include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@"  -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-Components-2f-stmpe811

clean-Drivers-2f-BSP-2f-Components-2f-stmpe811:
	-$(RM) ./Drivers/BSP/Components/stmpe811/stmpe811.cyclo ./Drivers/BSP/Components/stmpe811/stmpe811.d ./Drivers/BSP/Components/stmpe811/stmpe811.o ./Drivers/BSP/Components/stmpe811/stmpe811.su

.PHONY: clean-Drivers-2f-BSP-2f-Components-2f-stmpe811

