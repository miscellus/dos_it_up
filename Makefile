# Run from a shell that has DJGPP environment set (call tools\djgpp\setenv.bat first),
# or run 'make' from a cmd session where setenv.bat was executed.

SRCDIR = src
BUILD = build
DIST = $(BUILD)/dist
DATADIR = data
TOOLDIR = tools
EXENAME = game.exe
TARGET = $(DIST)/$(EXENAME)

CC = i586-pc-msdosdjgpp-gcc
PYTHON = python3
CP = cp

INCLUDE = -Iexternal
CFLAGS = -std=gnu99
# CFLAGS += -save-temps
CFLAGS += -Wall -Werror -pedantic
CFLAGS += -MMD #-MP
CFLAGS += -Wno-unused-function -Wno-error=unused-variable
CFLAGS += -march=i486
CFLAGS += -O2
# CFLAGS += -O0 -g
CFLAGS += -fomit-frame-pointer
CFLAGS += $(INCLUDE)
# CFLAGS += -mgeneral-regs-only
LDFLAGS = -Lexternal -lmikmod

IMGS = $(wildcard $(DATADIR)/*.png)
BUILD_IMGS = $(IMGS:.png=.img)
BUILD_MUSIC = $(wildcard $(DATADIR)/*.mod)

ALLDATA = $(BUILD_IMGS) $(BUILD_MUSIC)

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(BUILD)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

-include $(DEPS)

.PHONY: all clean run

all: $(TARGET)

run: $(TARGET)
	dosbox -c "mount c \"$(DIST)\"" -c "c:" -c "$(EXENAME)" -c "pause" -c "exit"

$(BUILD):
	mkdir -p $@

$(DIST): | $(BUILD)
	mkdir -p $@

$(BUILD)/data.o: $(wildcard $(DATADIR)/*)

# Compile C sources
$(BUILD)/%.o: $(SRCDIR)/%.c $(BUILD_IMGS) | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble .s (use -c so gcc invokes gas)
$(BUILD)/%.s.o: $(SRCDIR)/%.s | $(BUILD)
	$(CC) -c $< -o $@

$(DATADIR)/%.img: $(DATADIR)/%.png
	$(PYTHON) $(TOOLDIR)/convert_image_to_raw.py $< $@

$(DIST)/CWSDPMI.EXE: | $(DIST)
	$(CP) external/CWSDPMI.EXE $@

# Link
$(TARGET): $(OBJS) $(ALLDATA) $(DIST)/CWSDPMI.EXE
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

clean:
	rm -rf $(BUILD)

