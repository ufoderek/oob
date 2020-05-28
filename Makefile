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

$(TEST_FILE):
	dd if=/dev/urandom of=$(TEST_FILE) bs=1M count=16
	dd if=/dev/urandom of=$(TEST_FILE) bs=1 count=128 oflag=append conv=notrunc

.PHONY: test test8 clean
test: oob$(ECC_CAP) $(TEST_FILE)
	cp -f ./$(TEST_FILE) ./$(TEST_FILE).tmp
	rm -f ./$(TEST_FILE).tmp.oob
	./oob32 --create -i ./$(TEST_FILE).tmp -j1
	./oob32 --destroy -i ./$(TEST_FILE).tmp -j1
	./oob32 --verify -i ./$(TEST_FILE).tmp -j1
	./oob32 --repair -i ./$(TEST_FILE).tmp -j1
	./oob32 --verify -i ./$(TEST_FILE).tmp -j1
	md5sum ./$(TEST_FILE) ./$(TEST_FILE).tmp

test8: oob$(ECC_CAP) $(TEST_FILE)
	cp -f ./$(TEST_FILE) ./$(TEST_FILE).tmp
	rm -f ./$(TEST_FILE).tmp.oob
	./oob32 --create -i ./$(TEST_FILE).tmp -j8
	./oob32 --destroy -i ./$(TEST_FILE).tmp -j8
	./oob32 --verify -i ./$(TEST_FILE).tmp -j8
	./oob32 --repair -i ./$(TEST_FILE).tmp -j8
	./oob32 --verify -i ./$(TEST_FILE).tmp -j8
	md5sum ./$(TEST_FILE) ./$(TEST_FILE).tmp

clean:
	rm -f linux_bch/bch.o ./$(TEST_FILE).tmp ./$(TEST_FILE).tmp.oob oob_workers.o oob_file.o bch.o oob.o oob32
