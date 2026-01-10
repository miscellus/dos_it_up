# Run from a shell that has DJGPP environment set (call tools\djgpp\setenv.bat first),
# or run 'make' from a cmd session where setenv.bat was executed.

CC = i586-pc-msdosdjgpp-gcc
AS = i586-pc-msdosdjgpp-as
LD = i586-pc-msdosdjgpp-ld
NM = i586-pc-msdosdjgpp-nm

CFLAGS = -Wall -Werror -Wno-unused-function -O6 -g -march=i386 -mgeneral-regs-only -fomit-frame-pointer
# -mgeneral-regs-only
SRCDIR = src
BUILD = build
TARGET = $(BUILD)\myprog.exe

SRCS = \
	$(SRCDIR)/gfx.c \
	$(SRCDIR)/timer.c \
	$(SRCDIR)/timer_isr.s \
	$(SRCDIR)/main.c \
#
OBJS = \
	$(SRCDIR)/gfx.o \
	$(BUILD)/timer.o \
	$(BUILD)/timer_isr.s.o \
	$(BUILD)/main.o \
#

.PHONY: all clean nmstuff

all: $(BUILD) $(TARGET)

$(BUILD):
	mkdir -p $(BUILD)

# Compile C sources
$(BUILD)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble .s (use -c so gcc invokes gas)
$(BUILD)/%.s.o: $(SRCDIR)/%.s
	$(CC) -c $< -o $@

# Link
$(TARGET): $(OBJS) $(wildcard $(SRCDIR)/*.[hc])
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	rm -f $(BUILD)/*.o
	rm -f $(TARGET)

