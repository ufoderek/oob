GCC=gcc
CFLAGS=-O2 -march=native

BCH_M=13
ECC_CAP=32
DATA_BYTES=512

BCH_CONST_PARAMS=-DCONFIG_BCH_CONST_PARAMS -DCONFIG_BCH_CONST_M=$(BCH_M) -DCONFIG_BCH_CONST_T=$(ECC_CAP) -DDATA_BYTES=$(DATA_BYTES)
DEFS=$(BCH_CONST_PARAMS)

INC_COMPAT=-Ilinux_bch/include
INCS=$(INC_COMPAT)

oob$(ECC_CAP): linux_bch/bch.o bch.o oob.o
	$(GCC) $(CFLAGS) $^ -o $@

linux_bch/bch.o: linux_bch/bch.c
	$(GCC) $(CFLAGS) $(DEFS) $(INCS) -c $< -o $@

bch.o: bch.c
	$(GCC) $(CFLAGS) $(DEFS) $(INCS) -c $< -o $@

oob.o: oob.c
	$(GCC) $(CFLAGS) $(DEFS) $(INCS) -c $< -o $@

.PHONY: clean
clean:
	rm -f linux_bch/bch.o bch.o oob.o oob32
