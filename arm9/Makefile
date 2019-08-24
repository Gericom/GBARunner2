#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

#---------------------------------------------------------------------------------
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
# DATA is a list of directories containing binary files
# all directories are relative to this makefile
#---------------------------------------------------------------------------------
BUILD		:=	build
SOURCES		:=	source source/emu source/gui source/gui/core source/fat source/save
INCLUDES	:=	include source
DATA		:=	data


#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-marm -mthumb-interwork#-mthumb

CFLAGS	:=	-g -Wall -O2 -nostdlib -nodefaultlibs \
 			-march=armv5te -mtune=arm946e-s -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-tree-loop-distribute-patterns -fno-builtin -fomit-frame-pointer \
			-ffast-math \
			$(ARCH) 

CFLAGS	+=	$(INCLUDE) -DARM9

ifdef ISNITRODEBUG
	CFLAGS	+=  -DISNITRODEBUG
endif

ifdef ARM7_DLDI
	CFLAGS	+=  -DARM7_DLDI
endif

ifdef ENABLE_WRAM_ICACHE
	CFLAGS	+=  -DENABLE_WRAM_ICACHE
endif

GIT_HASH := $(shell git rev-parse HEAD)
GIT_DATE := date#$(shell git log -1 "--date=format:%d-%m-%Y %H:%M" --pretty=format:%cd)
GIT_BRANCH := $(shell git branch | sed -n '/\* /s///p')

CFLAGS +=  -DGIT_COMMIT_HASH=$(GIT_HASH) -DGIT_COMMIT_DATE=$(GIT_DATE) -DGIT_BRANCH=$(GIT_BRANCH)


CXXFLAGS	:=	$(CFLAGS) -fno-rtti -fno-exceptions -fnon-call-exceptions

ASFLAGS	:=	-g $(ARCH) -march=armv5te -mtune=arm946e-s $(INCLUDE)

ifdef ISNITRODEBUG
	ASFLAGS	+=  -DISNITRODEBUG
endif

ifdef ARM7_DLDI
	ASFLAGS	+=  -DARM7_DLDI
endif

ifdef ENABLE_WRAM_ICACHE
	ASFLAGS	+=  -DENABLE_WRAM_ICACHE
endif

LDFLAGS	=	-specs=../ds_arm9_gbarunner.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map) -nodefaultlibs
 

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:=	
 
#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=	$(LIBNDS)
 
#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------
 
export ARM9ELF	:=	$(CURDIR)/$(TARGET).elf
export DEPSDIR := $(CURDIR)/$(BUILD)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))
 
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))
 
#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
 
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)
 
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)
 
.PHONY: $(BUILD) clean
 
#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
 
#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) *.elf *.nds* *.bin 
 
 
#---------------------------------------------------------------------------------
else
 
#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(ARM9ELF)	:	$(OFILES)
	@echo linking $(notdir $@)
	@$(LD)  $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $@

#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data 
#---------------------------------------------------------------------------------
%.ntft.o	:	%.ntft
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)
	arm-none-eabi-objcopy.exe --rename-section .rodata=.vram $(@)

#---------------------------------------------------------------------------------
%.nbfc.o	:	%.nbfc
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)
	arm-none-eabi-objcopy.exe --rename-section .rodata=.vram $(@)

#---------------------------------------------------------------------------------
%.nbfs.o	:	%.nbfs
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)
	arm-none-eabi-objcopy.exe --rename-section .rodata=.vram $(@)

#---------------------------------------------------------------------------------
%.nbfp.o	:	%.nbfp
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)
	arm-none-eabi-objcopy.exe --rename-section .rodata=.vram $(@)

-include $(DEPSDIR)/*.d
 
#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
