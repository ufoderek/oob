GCC=gcc
CFLAGS=-O2 -march=native

BCH_M=13
ECC_CAP=32

LINUX_BCH_CONST_PARAMS=-DCONFIG_BCH_CONST_PARAMS -DCONFIG_BCH_CONST_M=$(BCH_M) -DCONFIG_BCH_CONST_T=$(ECC_CAP)
DEFS=$(LINUX_BCH_CONST_PARAMS)

INC_COMPAT=-Iinclude_compat
INCS=-Iinclude $(INC_COMPAT)

oob: bch.o

#oob: bch.o
#	$(GCC) $(CFLAGS) $(LIBS) $^ -o $@

%.o: %.c
	$(GCC) $(CFLAGS) $(DEFS) $(INCS) -c $< -o $@

clean:
	rm bch.o
