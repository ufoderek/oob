GCC=gcc
CFLAGS=-O2 -march=native

BCH_K=16384
BCH_M=15
ECC_CAP=128

LINUX_BCH_CONST_PARAMS=-DCONFIG_BCH_CONST_PARAMS -DCONFIG_BCH_CONST_M=$(BCH_M) -DCONFIG_BCH_CONST_T=$(ECC_CAP) -DCONFIG_BCH_CONST_K=$(BCH_K)
DEFS=$(LINUX_BCH_CONST_PARAMS)

INC_COMPAT=-Iinclude_compat
INCS=-Iinclude $(INC_COMPAT)

all: oob bitflip

oob: main.o oob.o bch.o
	$(GCC) $(CFLAGS) $(LIBS) $^ -o $@

bitflip: bitflip.o
	$(GCC) $(CFLAGS) $(LIBS) $^ -o $@

%.o: %.c
	$(GCC) $(CFLAGS) $(DEFS) $(INCS) -c $< -o $@

clean:
	rm bch.o
