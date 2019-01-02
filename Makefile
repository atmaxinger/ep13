all: scanner1
	perf stat -e cycles -e instructions -e L1-dcache-load-misses -e LLC-load-misses -e branch-misses ./scanner1 /nfs/unsafe/httpd/ftp/pub/anton/lvas/effizienz-aufgabe13/llinput

scanner1: scanner-full.c
	gcc -O scanner-full.c -o scanner1

# scanner: scanner.c
# 	gcc -O scanner.c -o scanner

# scanner.c: scanner.l

# ep13.tar.gz:
#	mkdir ep13
#	ln scanner.l Makefile ep13
#	tar cfz ep13.tar.gz ep13
