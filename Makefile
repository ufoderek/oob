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

%.o: %.c
	$(GCC) $(CFLAGS) $(DEFS) $(INCS) -c $< -o $@

.PHONY: test test8 clean
itest:
	cp -f ~/bk.jpg ~/test.jpg
	rm -f ~/test.jpg.oob
	./oob32 --create -d ~/test.jpg -o ~/test.jpg.oob
	./oob32 --break  -d ~/test.jpg -o ~/test.jpg.oob --inplace
	./oob32 --verify -d ~/test.jpg -o ~/test.jpg.oob
	./oob32 --repair -d ~/test.jpg -o ~/test.jpg.oob --inplace
	./oob32 --verify -d ~/test.jpg -o ~/test.jpg.oob
	md5sum ~/bk.jpg ~/test.jpg

itest8:
	cp -f ~/bk.jpg ~/test.jpg
	rm -f ~/test.jpg.oob
	./oob32 --create -d ~/test.jpg -o ~/test.jpg.oob
	./oob32 --break  -d ~/test.jpg -o ~/test.jpg.oob --inplace
	./oob32 --verify -d ~/test.jpg -o ~/test.jpg.oob
	./oob32 --repair -d ~/test.jpg -o ~/test.jpg.oob --inplace
	./oob32 --verify -d ~/test.jpg -o ~/test.jpg.oob
	md5sum ~/bk.jpg ~/test.jpg

test:
	cp -f ~/bk.jpg ~/test.jpg
	rm -f ~/test.jpg.oob
	rm -f ~/test_new.jpg ~/test_new.jpg.oob
	rm -f ~/test_new2.jpg ~/test_new2.jpg.oob
	./oob32 --create -d ~/test.jpg -o ~/test.jpg.oob
	./oob32 --break  -d ~/test.jpg -o ~/test.jpg.oob -D ~/test_new.jpg -O ~/test_new.jpg.oob
	./oob32 --verify -d ~/test_new.jpg -o ~/test_new.jpg.oob
	./oob32 --repair -d ~/test_new.jpg -o ~/test_new.jpg.oob -D ~/test_new2.jpg -O ~/test_new2.jpg.oob
	./oob32 --verify -d ~/test_new2.jpg -o ~/test_new2.jpg.oob
	md5sum ~/bk.jpg ~/test_new2.jpg

test8:
	cp -f ~/bk.jpg ~/test.jpg
	rm -f ~/test.jpg.oob
	rm -f ~/test_new.jpg ~/test_new.jpg.oob
	rm -f ~/test_new2.jpg ~/test_new2.jpg.oob
	./oob32 --create -d ~/test.jpg -o ~/test.jpg.oob -j8
	./oob32 --break  -d ~/test.jpg -o ~/test.jpg.oob -D ~/test_new.jpg -O ~/test_new.jpg.oob -j8
	./oob32 --verify -d ~/test_new.jpg -o ~/test_new.jpg.oob -j8
	./oob32 --repair -d ~/test_new.jpg -o ~/test_new.jpg.oob -D ~/test_new2.jpg -O ~/test_new2.jpg.oob -j8
	./oob32 --verify -d ~/test_new2.jpg -o ~/test_new2.jpg.oob -j8
	md5sum ~/bk.jpg ~/test_new2.jpg

clean:
	rm -f oob32
	rm -f linux_bch/bch.o bch.o oob.o oob_workers.o oob_file.o
