# Run from a shell that has DJGPP environment set (call tools\djgpp\setenv.bat first),
# or run 'make' from a cmd session where setenv.bat was executed.

SRCDIR = src
BUILD = build
TARGET = $(BUILD)/myprog.exe

CFLAGS = -std=gnu99
# CFLAGS += -save-temps
CFLAGS += -Wall -Werror -pedantic
CFLAGS += -Wno-unused-function
CFLAGS += -march=i486
# CFLAGS += -O6
CFLAGS += -O0 -g
CFLAGS += -fomit-frame-pointer
# CFLAGS += -mgeneral-regs-only
LDFLAGS =

CC = i586-pc-msdosdjgpp-gcc
LD = i586-pc-msdosdjgpp-ld
NM = i586-pc-msdosdjgpp-nm

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

# Link
$(TARGET): $(OBJS) $(wildcard $(SRCDIR)/*.[hc])
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

clean:
	rm -f $(BUILD)/*.o
	rm -f $(TARGET)

