# Run from a shell that has DJGPP environment set (call tools\djgpp\setenv.bat first),
# or run 'make' from a cmd session where setenv.bat was executed.

SRCDIR = src
BUILD = build
IMGDIR = img
TOOLDIR = tools
TARGET = $(BUILD)/myprog.exe

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

CC = i586-pc-msdosdjgpp-gcc
LD = i586-pc-msdosdjgpp-ld
NM = i586-pc-msdosdjgpp-nm
OBJCOPY = i586-pc-msdosdjgpp-objcopy
PYTHON = python3

IMGS = $(wildcard $(IMGDIR)/*.png)
IMGSRAW = $(patsubst $(IMGDIR)/%.png,$(BUILD)/%.img,$(IMGS))

$(info IMGSRAW = $(IMGSRAW))


SRCS = \
	$(SRCDIR)/gfx.c \
	$(SRCDIR)/timer.c \
	$(SRCDIR)/main.c \
#
OBJS = \
	$(SRCDIR)/gfx.o \
	$(BUILD)/timer.o \
	$(BUILD)/main.o \
#
# 	$(BUILD)/sprite_sheet_1.img.o \

.PHONY: all clean run

all: $(BUILD) $(TARGET)

run: all
	dosbox -c "mount c \"$(BUILD)\"" -c "c:" -c "myprog.exe" -c "pause" -c "exit"

$(BUILD):
	mkdir -p $(BUILD)

# Compile C sources
$(BUILD)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble .s (use -c so gcc invokes gas)
$(BUILD)/%.s.o: $(SRCDIR)/%.s
	$(CC) -c $< -o $@

$(BUILD)/%.img: $(IMGDIR)/%.png
	$(PYTHON) $(TOOLDIR)/convert_image_to_raw.py $< $@

# $(BUILD)/%.img.o: $(IMGDIR)/%.raw
# 	$(OBJCOPY) -I binary -O coff-go32 -B i386 $< $@

# Link
$(TARGET): $(OBJS) $(wildcard $(SRCDIR)/*.[hc]) $(IMGSRAW)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

clean:
	rm -f $(BUILD)/*.o
	rm -f $(TARGET)

