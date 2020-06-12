#!/bin/bash

test_cnt=0
TEST_FILE=testfile

while true
do
	test_cnt=$((test_cnt+1))
	rm -f ./$TEST_FILE
	rm -f ./$TEST_FILE.oob
	rm -f ./$TEST_FILE.bad
	rm -f ./$TEST_FILE.bad.oob
	rm -f ./$TEST_FILE.bad.fixed
	rm -f ./$TEST_FILE.bad.fixed.oob

	MBs=$((1 + RANDOM % 512))
	bytes=$((1 + RANDOM % 1024))
	CPUs=$((4 + RANDOM % 17))
	dd if=/dev/urandom of=$TEST_FILE bs=1M count=$MBs status=none
	dd if=/dev/urandom of=$TEST_FILE bs=1 count=$bytes oflag=append conv=notrunc status=none
	file_size=$(stat -c %s $TEST_FILE)

	echo ""
	echo "TEST #$test_cnt FILE_SIZE=$file_size CPUS=$CPUs"

	./oob32 --create -i ./$TEST_FILE -j$CPUs
	./oob32 --destroy -i ./$TEST_FILE -j$CPUs
	./oob32 --verify -i ./$TEST_FILE.bad -j$CPUs
	./oob32 --repair -i ./$TEST_FILE.bad -j$CPUs
	./oob32 --verify -i ./$TEST_FILE.bad.fixed -j$CPUs
	cmp ./$TEST_FILE ./$TEST_FILE.bad.fixed && echo "SUCCESS" || exit
done
