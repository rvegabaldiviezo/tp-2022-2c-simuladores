################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/handle_cpu.c \
../src/handle_kernel.c \
../src/memoria.c 

C_DEPS += \
./src/handle_cpu.d \
./src/handle_kernel.d \
./src/memoria.d 

OBJS += \
./src/handle_cpu.o \
./src/handle_kernel.o \
./src/memoria.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/handle_cpu.d ./src/handle_cpu.o ./src/handle_kernel.d ./src/handle_kernel.o ./src/memoria.d ./src/memoria.o

.PHONY: clean-src

