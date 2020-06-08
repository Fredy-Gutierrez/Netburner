################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
USER_OBJS.elf \

ELF += \
WifiApp.elf \

S19FILE += \
WifiApp.s19 \


# All Target
all: WifiApp.bin

# Tool invocations
WifiApp.bin: $(ELF)
	@echo 'Building target: $@'
	@echo 'Invoking: NB Flashpack'
	flashpack -output-target bin -C -P MODM7AE70 -cflag S:3 -cflag S:0 -cflag S:1 -O "WifiApp.bin" $(ELF)
	@echo 'Finished building target: $@'
	@echo ' '

WifiApp.elf: $(OBJS) $(USER_OBJS)
	@echo 'Invoking: GNU C/C++ Linker'
	arm-unknown-eabi-g++ $(OBJS) $(USER_OBJS) $(LIBS) -Wl,-Map="WifiApp.map" -nostartfiles -Wl,--start-group -lnetburner  -lstdc++ -Wl,--end-group -LC:/nburn/platform/MODM7AE70/original/lib -Lnblibs -Wl,-n -TC:/nburn/platform/MODM7AE70/lib/MODM7AE70.ld -Wl,-RC:/nburn/platform/MODM7AE70/lib/sys.ld -Wl,--cref -Wl,-S -Wl,--gc-sections -mfpu=fpv5-d16 -mfloat-abi=softfp -mthumb -Wl,--thumb-entry=Reset_Handler -mcpu=cortex-m7 -o"WifiApp.elf"
	@echo 'Finished building: $@'
	@echo ' '

USER_OBJS.elf: WifiApp.elf $(OBJS) $(USER_OBJS)

WifiApp.s19: $(ELF)
	@echo 'Invoking: GNU Objcopy'
	arm-unknown-eabi-objcopy --strip-all --output-target=srec $(ELF) "WifiApp.s19"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(S19FILE)$(CC_DEPS)$(C++_DEPS)$(EXECUTABLES)$(NBECLIPSE_TOOLCHAIN_TC30_TOOL_NB_FLASHPACK_OUTPUT_OUTPUTS)$(NETBURNER_COMPCODE_OUTPUT_OUTPUTS)$(OBJS)$(C_UPPER_DEPS)$(CXX_DEPS)$(ELF)$(C_DEPS)$(CPP_DEPS) WifiApp.bin
	-@echo ' '

.PHONY: all clean dependents

-include ../makefile.targets
