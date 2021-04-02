################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/FatLib/FileSystemUtils.cpp \
../src/FatLib/effs_time.cpp \
../src/FatLib/ftp_f.cpp \
../src/FatLib/ramdrv_mcf.cpp 

OBJS += \
./src/FatLib/FileSystemUtils.o \
./src/FatLib/effs_time.o \
./src/FatLib/ftp_f.o \
./src/FatLib/ramdrv_mcf.o 

CPP_DEPS += \
./src/FatLib/FileSystemUtils.d \
./src/FatLib/effs_time.d \
./src/FatLib/ftp_f.d \
./src/FatLib/ramdrv_mcf.d 


# Each subdirectory must supply rules for building sources it contributes
src/FatLib/%.o: ../src/FatLib/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GNU C++ Compiler'
	arm-unknown-eabi-g++ -std=gnu++14 -I"C:\Users\Integra Fredy\nbworkspace\APIVER03\src" -IC:/nburn/nbrtos/include -IC:/nburn/platform/MODM7AE70/include -IC:/nburn/arch/cortex-m7/include -IC:/nburn/arch/cortex-m7/cpu/SAME70/include -IC:/nburn/libraries/include -O2 -Wall -c -fmessage-length=0 -fdata-sections -fno-use-cxa-atexit -ffunction-sections -gdwarf-2 -fno-exceptions -fno-rtti -Wno-write-strings -falign-functions=4 -fno-asynchronous-unwind-tables -mcpu=cortex-m7 -DMODM7AE70 -DSAME70 -Dcortex-m7 -mfpu=fpv5-d16 -D__SAME70Q21__ -mfloat-abi=softfp -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


