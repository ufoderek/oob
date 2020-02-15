GCC=gcc
CFLAGS=-O2 -march=native

BCH_M=13
ECC_CAP=32
DATA_BYTES=512

BCH_CONST_PARAMS=-DCONFIG_BCH_CONST_PARAMS -DCONFIG_BCH_CONST_M=$(BCH_M) -DCONFIG_BCH_CONST_T=$(ECC_CAP) -DDATA_BYTES=$(DATA_BYTES)
DEFS=$(BCH_CONST_PARAMS)

INC_COMPAT=-Ilinux_bch/include
INCS=$(INC_COMPAT)
LIBS=-lpthread

oob$(ECC_CAP): linux_bch/bch.o bch.o oob.o oob_workers.o oob_file.o
	$(GCC) $(CFLAGS) $(LIBS) $^ -o $@

linux_bch/bch.o: linux_bch/bch.c
	$(GCC) $(CFLAGS) $(DEFS) $(INCS) -c $< -o $@

%.o: %.c
	$(GCC) $(CFLAGS) $(DEFS) $(INCS) -c $< -o $@

#oob.o: oob.c
#	$(GCC) $(CFLAGS) $(DEFS) $(INCS) -c $< -o $@

#oob_workers.o: oob_workers.c
#	$(GCC) $(CFLAGS) $(DEFS) $(INCS) -c $< -o $@

.PHONY: test test8 clean
test:
	cp -f ~/bk.jpg ~/test.jpg
	./oob32 --create -d ~/test.jpg -o ~/test.jpg.oob -j1
	./oob32 --break -d ~/test.jpg -o ~/test.jpg.oob -j1
	./oob32 --verify -d ~/test.jpg -o ~/test.jpg.oob -j1

test8:
	cp -f ~/bk.jpg ~/test.jpg
	./oob32 --create -d ~/test.jpg -o ~/test.jpg.oob -j8
	./oob32 --break -d ~/test.jpg -o ~/test.jpg.oob -j8
	./oob32 --verify -d ~/test.jpg -o ~/test.jpg.oob -j8

clean:
	rm -f linux_bch/bch.o bch.o oob.o oob32
