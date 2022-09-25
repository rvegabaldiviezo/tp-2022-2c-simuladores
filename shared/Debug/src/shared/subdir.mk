################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/shared/serialization.c \
../src/shared/socket.c 

OBJS += \
./src/shared/serialization.o \
./src/shared/socket.o 

C_DEPS += \
./src/shared/serialization.d \
./src/shared/socket.d 


# Each subdirectory must supply rules for building sources it contributes
src/shared/%.o: ../src/shared/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


