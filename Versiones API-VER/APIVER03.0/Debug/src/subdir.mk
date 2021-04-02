################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Business.cpp \
../src/Configurations.cpp \
../src/I2C_Controller.cpp \
../src/IO_Interface.cpp \
../src/Recovery_Mode.cpp \
../src/Report2Server.cpp \
../src/Rtc_Controller.cpp \
../src/Sargas_Connection.cpp \
../src/Tcp_Class.cpp \
../src/Watch_Dog.cpp \
../src/Web_Functions.cpp \
../src/main.cpp 

OBJS += \
./src/Business.o \
./src/Configurations.o \
./src/I2C_Controller.o \
./src/IO_Interface.o \
./src/Recovery_Mode.o \
./src/Report2Server.o \
./src/Rtc_Controller.o \
./src/Sargas_Connection.o \
./src/Tcp_Class.o \
./src/Watch_Dog.o \
./src/Web_Functions.o \
./src/main.o 

CPP_DEPS += \
./src/Business.d \
./src/Configurations.d \
./src/I2C_Controller.d \
./src/IO_Interface.d \
./src/Recovery_Mode.d \
./src/Report2Server.d \
./src/Rtc_Controller.d \
./src/Sargas_Connection.d \
./src/Tcp_Class.d \
./src/Watch_Dog.d \
./src/Web_Functions.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GNU C++ Compiler'
	arm-unknown-eabi-g++ -std=gnu++14 -D_DEBUG -I"C:\Users\Integra Fredy\nbworkspace\APIVER03.0\src" -IC:/nburn/nbrtos/include -IC:/nburn/platform/MODM7AE70/include -IC:/nburn/arch/cortex-m7/include -IC:/nburn/arch/cortex-m7/cpu/SAME70/include -IC:/nburn/libraries/include -O0 -g3 -Wall -c -fmessage-length=0 -fdata-sections -fno-use-cxa-atexit -ffunction-sections -gdwarf-2 -fno-exceptions -fno-rtti -Wno-write-strings -falign-functions=4 -fno-asynchronous-unwind-tables -mcpu=cortex-m7 -DMODM7AE70 -DSAME70 -Dcortex-m7 -mfpu=fpv5-d16 -D__SAME70Q21__ -mfloat-abi=softfp -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


