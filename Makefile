SRC=AIPS.c
OBJ=$(SRC:.c=.o)
OBJ32=$(SRC:.c=.o32)
WINOBJ=$(SRC:.c=.owin)
WIN64OBJ=$(SRC:.c=.owin64)
OUT=AIPS

CC=gcc
WIN=i586-mingw32msvc
WIN64=i686-w64-mingw32

CFLAGS=-Wall -O3 -g
LDFLAGS=
WINCFLAGS=
WINLDFLAGS=
RM=rm

.PHONY: check-syntax clean veryclean help

all: $(OUT) $(OUT)32 $(OUT).exe $(OUT)64.exe

%.o:%.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o32:%.c
	$(CC) $(CFLAGS) -m32 -o $@ -c $<

%.owin:%.c
	$(WIN)-$(CC) $(CFLAGS) $(WINCFLAGS) -o $@ -c $<

%.owin64:%.c
	$(WIN64)-$(CC) $(CFLAGS) $(WINCFLAGS) -o $@ -c $<

$(OUT): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

$(OUT)32: $(OBJ32)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

$(OUT).exe: $(WINOBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

$(OUT)64.exe: $(WIN64OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

clean:
	-$(RM) $(OBJ)
	-$(RM) $(WINOBJ)
	-$(RM) $(WIN64OBJ)

veryclean: clean
	-$(RM) $(OUT)
	-$(RM) $(OUT)32
	-$(RM) $(OUT).exe
	-$(RM) $(OUT)64.exe
	-$(RM) $(OBJ)
	-$(RM) $(WINOBJ)
	-$(RM) $(WIN64OBJ)
	-$(RM) *~
	-$(RM) *#

help:
	@echo Make rules
	@echo $(OUT)		Builds Linux Binary
	@echo $(OUT)32		Builds 32-bit Linux Binary
	@echo $(OUT).exe	Builds Windows 32-bit Binary
	@echo $(OUT)64.exe	Builds Windows 64-bit Binary
	@echo all		Builds all Binaries
	@echo clean		Removes object files
	@echo veryclean		Removes object files and binaries

check-syntax:
	$(CC) $(SRC) -o null -Wall
	-$(RM) null
