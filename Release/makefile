################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := D:/software/ccs930/ccs/tools/compiler/ti-cgt-arm_18.12.4.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./main.obj" \
"./tm4c123gh6pm_startup_ccs.obj" \
"./Apps/CarController.obj" \
"./Apps/LMT70.obj" \
"./Apps/MyBlueTooth.obj" \
"./Apps/MyJ901.obj" \
"./Apps/Mymcu90615.obj" \
"./Apps/PIDClass.obj" \
"./Apps/PWMController.obj" \
"./Apps/bluetooth.obj" \
"./Apps/cJSON.obj" \
"./Apps/delay.obj" \
"./Apps/encoder.obj" \
"./Apps/led_pwm.obj" \
"./Apps/motor.obj" \
"./Apps/motor_timer.obj" \
"./Apps/my_eeprom.obj" \
"./Apps/my_gpio.obj" \
"./Apps/my_uart.obj" \
"./OLED/MyOLED.obj" \
"./OLED/MyOledFont.obj" \
"../tm4c123gh6pm.cmd" \
$(GEN_CMDS__FLAG) \
-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include Apps/subdir_vars.mk
-include OLED/subdir_vars.mk
-include subdir_rules.mk
-include Apps/subdir_rules.mk
-include OLED/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
demo2.out \

EXE_OUTPUTS__QUOTED += \
"demo2.out" \

BIN_OUTPUTS += \
demo2.hex \

BIN_OUTPUTS__QUOTED += \
"demo2.hex" \


# All Target
all: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "demo2.out"

# Tool invocations
demo2.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: ARM Linker'
	"D:/software/ccs930/ccs/tools/compiler/ti-cgt-arm_18.12.4.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O2 --define=ccs="ccs" --define=PART_TM4C123GH6PM --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi -z -m"demo2.map" --heap_size=0 --stack_size=512 -i"D:/software/ccs930/ccs/tools/compiler/ti-cgt-arm_18.12.4.LTS/lib" -i"D:/software/ccs930/ccs/tools/compiler/ti-cgt-arm_18.12.4.LTS/include" --reread_libs --diag_wrap=off --display_error_number --warn_sections --xml_link_info="demo2_linkInfo.xml" --rom_model -o "demo2.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

demo2.hex: $(EXE_OUTPUTS)
	@echo 'Building files: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo 'Invoking: ARM Hex Utility'
	"D:/software/ccs930/ccs/tools/compiler/ti-cgt-arm_18.12.4.LTS/bin/armhex"  -o "demo2.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(BIN_OUTPUTS__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "main.obj" "tm4c123gh6pm_startup_ccs.obj" "Apps\CarController.obj" "Apps\LMT70.obj" "Apps\MyBlueTooth.obj" "Apps\MyJ901.obj" "Apps\Mymcu90615.obj" "Apps\PIDClass.obj" "Apps\PWMController.obj" "Apps\bluetooth.obj" "Apps\cJSON.obj" "Apps\delay.obj" "Apps\encoder.obj" "Apps\led_pwm.obj" "Apps\motor.obj" "Apps\motor_timer.obj" "Apps\my_eeprom.obj" "Apps\my_gpio.obj" "Apps\my_uart.obj" "OLED\MyOLED.obj" "OLED\MyOledFont.obj" 
	-$(RM) "tm4c123gh6pm_startup_ccs.d" "Apps\CarController.d" "Apps\LMT70.d" "Apps\MyBlueTooth.d" "Apps\MyJ901.d" "Apps\Mymcu90615.d" "Apps\bluetooth.d" "Apps\cJSON.d" "Apps\delay.d" "Apps\encoder.d" "Apps\led_pwm.d" "Apps\motor.d" "Apps\motor_timer.d" "Apps\my_eeprom.d" "Apps\my_gpio.d" "Apps\my_uart.d" "OLED\MyOLED.d" "OLED\MyOledFont.d" 
	-$(RM) "main.d" "Apps\PIDClass.d" "Apps\PWMController.d" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

