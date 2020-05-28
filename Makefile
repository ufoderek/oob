GCC=gcc
CFLAGS=-O2 -march=native
TEST_FILE=test_file

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

%.o: %.c
	$(GCC) $(CFLAGS) $(DEFS) $(INCS) -c $< -o $@

.PHONY: test test8 clean
test: oob$(ECC_CAP)
	cp -f ./$(TEST_FILE) ./$(TEST_FILE).tmp
	rm -f ./$(TEST_FILE).tmp.oob
	./oob32 --create -d ./$(TEST_FILE).tmp -o ./$(TEST_FILE).tmp.oob -j1
	./oob32 --break -d ./$(TEST_FILE).tmp -o ./$(TEST_FILE).tmp.oob -j1
	./oob32 --verify -d ./$(TEST_FILE).tmp -o ./$(TEST_FILE).tmp.oob -j1
	./oob32 --repair -d ./$(TEST_FILE).tmp -o ./$(TEST_FILE).tmp.oob -j1
	./oob32 --verify -d ./$(TEST_FILE).tmp -o ./$(TEST_FILE).tmp.oob -j1
	md5sum ./$(TEST_FILE) ./$(TEST_FILE).tmp

test8: oob$(ECC_CAP)
	cp -f ./$(TEST_FILE) ./$(TEST_FILE).tmp
	rm -f ./$(TEST_FILE).tmp.oob
	./oob32 --create -d ./$(TEST_FILE).tmp -o ./$(TEST_FILE).tmp.oob -j8
	./oob32 --break -d ./$(TEST_FILE).tmp -o ./$(TEST_FILE).tmp.oob -j8
	./oob32 --verify -d ./$(TEST_FILE).tmp -o ./$(TEST_FILE).tmp.oob -j8
	./oob32 --repair -d ./$(TEST_FILE).tmp -o ./$(TEST_FILE).tmp.oob -j8
	./oob32 --verify -d ./$(TEST_FILE).tmp -o ./$(TEST_FILE).tmp.oob -j8
	md5sum ./$(TEST_FILE) ./$(TEST_FILE).tmp

clean:
	rm -f linux_bch/bch.o bch.o oob.o oob32
