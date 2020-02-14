GCC=gcc
CFLAGS=-O2 -march=native

BCH_M=13
ECC_CAP=32
DATA_BYTES=512

BCH_CONST_PARAMS=-DCONFIG_BCH_CONST_PARAMS -DCONFIG_BCH_CONST_M=$(BCH_M) -DCONFIG_BCH_CONST_T=$(ECC_CAP) -DDATA_BYTES=$(DATA_BYTES)
DEFS=$(BCH_CONST_PARAMS)

INC_COMPAT=-Icompat/include
INCS=$(INC_COMPAT)

BINARY_NAME=oob$(ECC_CAP)

$(BINARY_NAME): oob.o libbch.o bch.o
	$(GCC) $(CFLAGS) $^ -o $@

oob.o: oob.c
	$(GCC) $(CFLAGS) $(DEFS) $(INCS) $< -c

libbch.o: libbch.c
	$(GCC) $(CFLAGS) $(DEFS) $(INCS) $< -c

bch.o: bch.c
	$(GCC) $(CFLAGS) $(DEFS) $(INCS) $< -c

.PHONY: clean
clean:
	rm -f bch.o libbch.o oob.o oob32
