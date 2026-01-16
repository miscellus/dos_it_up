# Run from a shell that has DJGPP environment set (call tools\djgpp\setenv.bat first),
# or run 'make' from a cmd session where setenv.bat was executed.

SRCDIR = src
BUILD = build
DATADIR = data
BUILD_DATA = data
TOOLDIR = tools
TARGET = $(BUILD)/myprog.exe

CC = i586-pc-msdosdjgpp-gcc
PYTHON = python3
CP = cp

INCLUDE = -Iexternal
CFLAGS = -std=gnu99
# CFLAGS += -save-temps
CFLAGS += -Wall -Werror -pedantic
CFLAGS += -Wno-unused-function -Wno-error=unused-variable
CFLAGS += -march=i486
CFLAGS += -O2
# CFLAGS += -O0 -g
CFLAGS += -fomit-frame-pointer
CFLAGS += $(INCLUDE)
# CFLAGS += -mgeneral-regs-only
LDFLAGS = -Lexternal -lmikmod

ALL_SRC = Makefile $(wildcard $(SRCDIR)/*.[hc]) $(wildcard $(SRCDIR)/*.inc)

IMGS = $(wildcard $(DATADIR)/*.png)
BUILD_IMGS = $(IMGS:.png=.img)
BUILD_MUSIC = $(wildcard $(DATADIR)/*.mod)

ALLDATA = $(BUILD_IMGS) $(BUILD_MUSIC)

$(info $(ALLDATA))

SRCS = \
	$(SRCDIR)/gfx.c \
	$(SRCDIR)/timer.c \
	$(SRCDIR)/data.c \
	$(SRCDIR)/main.c \
#
OBJS = \
	$(BUILD)/gfx.o \
	$(BUILD)/timer.o \
	$(BUILD)/data.o \
	$(BUILD)/main.o \

.PHONY: all clean run

all: $(BUILD) $(TARGET)

run: all
	dosbox -c "mount c \"$(BUILD)\"" -c "c:" -c "myprog.exe" -c "pause" -c "exit"

$(BUILD):
	mkdir -p $(BUILD)

# Compile C sources
$(BUILD)/%.o: $(SRCDIR)/%.c $(BUILD_IMGS) $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble .s (use -c so gcc invokes gas)
$(BUILD)/%.s.o: $(SRCDIR)/%.s $(BUILD)
	$(CC) -c $< -o $@

$(DATADIR)/%.img: $(DATADIR)/%.png
	$(PYTHON) $(TOOLDIR)/convert_image_to_raw.py $< $@

$(BUILD)/CWSDPMI.EXE: $(BUILD)
	$(CP) external/CWSDPMI.EXE $@

# Link
$(TARGET): $(OBJS) $(ALL_SRC) $(ALLDATA) $(BUILD)/CWSDPMI.EXE
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

clean:
	rm -f $(BUILD)/*.o
	rm -f $(TARGET)

