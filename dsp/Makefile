#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
# ifeq ($(strip $(DEVKITARM)),)
# $(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
# endif

# include $(DEVKITARM)/ds_rules

WSLENV ?= notwsl

BASEPATH := $(TEAK_LLVM)

CLANG	:= $(BASEPATH)/clang
MC  	:= $(BASEPATH)/llvm-mc
LD		:= $(BASEPATH)/ld.lld
ELF2DSP1:= "$(BASEPATH)/Elf2Dsp1.exe"

# TARGET	:=	$(shell basename $(CURDIR))

#---------------------------------------------------------------------------------
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
# DATA is a list of directories containing binary files
# all directories are relative to this makefile
#---------------------------------------------------------------------------------
BUILD		:=	build
SOURCES		:=	src src/teak src/dmg
INCLUDES	:=	include
DATA		:=


#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
#ARCH	:=	-marm -mthumb-interwork

CFLAGS	:=	-c --target=teak -march=teak -O2 -integrated-as -nostdlib -ffreestanding -fno-builtin

#CFLAGS	+=	$(INCLUDE)
CXXFLAGS	:=	$(CFLAGS) -fno-rtti -fno-exceptions

ASFLAGS	:=	--arch=teak

LDFLAGS	=	-T ../linker.x

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:=
 
#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=	
 
#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------
 
export ELFPATH	:=	$(CURDIR)/$(TARGET).elf
export DSP1PATH	:=	$(CURDIR)/$(TARGET).cdc
export DEPSDIR := $(CURDIR)/$(BUILD)

ifneq ($(OS),Windows_NT)
export winElfPath := $(shell wslpath -w $(ELFPATH))
export winDsp1Path := $(shell wslpath -w $(DSP1PATH))
else
export winElfPath := $(ELFPATH)
export winDsp1Path := $(DSP1PATH)
endif

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))
 
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
 
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I $(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I $(dir)/include) \
			-I $(CURDIR)/$(BUILD)
 
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)
 
.PHONY: $(BUILD) clean
 
#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
 
#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) *.elf *.cdc *.bin 
 
 
#---------------------------------------------------------------------------------
else

%.o: %.c
	@echo $(notdir $<)
	$(CLANG) $(CFLAGS) -o $@ $<

%.o: %.s
	@echo $(notdir $<)
	$(MC) $(ASFLAGS) --filetype=obj -o $@ $<
 
#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(DSP1PATH)	: $(ELFPATH)
	@echo creating dsp1 from $(notdir $<)
	$(ELF2DSP1) -s -o "$(winDsp1Path)" "$(winElfPath)"

$(ELFPATH)	:	$(OFILES)
	@echo linking $(notdir $@)
	@$(LD)  $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -Map symbols.map -o $@

#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data 
#---------------------------------------------------------------------------------
%.bin.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPSDIR)/*.d
 
#---------------------------------------------------------------------------------------
endif
#--------------------------------------------------------------------------------------- 