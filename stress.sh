set -e

while true
do
	rm -f test.tar.xz
	rm -f test.tar.xz.oob
	rm -f test.tar.xz.fix
	rm -f test.tar.xz.oob.fix
	cp test.tar.xz.gold test.tar.xz
	./oob generate test.tar.xz test.tar.xz.oob
	./oob verify test.tar.xz test.tar.xz.oob
	./bitflip test.tar.xz test.tar.xz.oob 2> /dev/null
	./oob correct test.tar.xz test.tar.xz.oob 2> /dev/null
	diff -s test.tar.xz.gold test.tar.xz.fix
done
