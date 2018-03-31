################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/crc16.c \
../src/main.c \
../src/rs232.c \
../src/serial_master.c \
../src/stxetx.c 

OBJS += \
./src/crc16.o \
./src/main.o \
./src/rs232.o \
./src/serial_master.o \
./src/stxetx.o 

C_DEPS += \
./src/crc16.d \
./src/main.d \
./src/rs232.d \
./src/serial_master.d \
./src/stxetx.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


