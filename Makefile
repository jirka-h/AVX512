CFLAGS=-Wall -Wextra -O3 -g -mavx512f

SRCS=$(wildcard *.c)
BINARIES=$(SRCS:%.c=%)

all:	$(BINARIES)

clean:
	rm -rf bin/$(BINARIES)

add_avx256f:	add_avx256f.c
	gcc $(CFLAGS) $< -o bin/$@
	objdump -S bin/$@ > ./objdump/$@.lst

add_avx512f:	add_avx512f.c
	gcc $(CFLAGS) $< -o bin/$@
	objdump -S bin/$@ > ./objdump/$@.lst

detect_avx:	detect_avx.c
	gcc $(CFLAGS) $< -o bin/$@
	objdump -S bin/$@ > ./objdump/$@.lst

div_avx256f:	div_avx256f.c
	gcc $(CFLAGS) $< -o bin/$@
	objdump -S bin/$@ > ./objdump/$@.lst

div_avx512f:	div_avx512f.c
	gcc $(CFLAGS) $< -o bin/$@
	objdump -S bin/$@ > ./objdump/$@.lst

div_plain:	div_plain.c
	gcc $(CFLAGS) $< -o bin/$@
	objdump -S bin/$@ > ./objdump/$@.lst

div_two_independent_vectors_avx256f: div_two_independent_vectors_avx256f.c
	gcc $(CFLAGS) $< -o bin/$@
	objdump -S bin/$@ > ./objdump/$@.lst

div_two_independent_vectors_avx512f: div_two_independent_vectors_avx512f.c
	gcc $(CFLAGS) $< -o bin/$@
	objdump -S bin/$@ > ./objdump/$@.lst

div_four_independent_vectors_avx256f: div_four_independent_vectors_avx256f.c
	gcc $(CFLAGS) $< -o bin/$@
	objdump -S bin/$@ > ./objdump/$@.lst

div_four_independent_vectors_avx512f: div_four_independent_vectors_avx512f.c
	gcc $(CFLAGS) $< -o bin/$@
	objdump -S bin/$@ > ./objdump/$@.lst

div_avx256f_approx:	div_avx256f_approx.c
	gcc $(CFLAGS) $< -o bin/$@
	objdump -S bin/$@ > ./objdump/$@.lst

div_avx512f_approx: div_avx512f_approx.c
	gcc $(CFLAGS) $< -o bin/$@
	objdump -S bin/$@ > ./objdump/$@.lst

div_avx256f_approx_reversed:	div_avx256f_approx_reversed.c
	gcc $(CFLAGS) $< -o bin/$@
	objdump -S bin/$@ > ./objdump/$@.lst

div_avx512f_approx_reversed: div_avx512f_approx_reversed.c
	gcc $(CFLAGS) $< -o bin/$@
	objdump -S bin/$@ > ./objdump/$@.lst

div_two_independent_vectors_avx256f_approx:	div_two_independent_vectors_avx256f_approx.c
	gcc $(CFLAGS) $< -o bin/$@
	objdump -S bin/$@ > ./objdump/$@.lst

div_four_independent_vectors_avx256f_approx:	div_four_independent_vectors_avx256f_approx.c
	gcc $(CFLAGS) $< -o bin/$@
	objdump -S bin/$@ > ./objdump/$@.lst

div_two_independent_vectors_avx512f_approx:	div_two_independent_vectors_avx512f_approx.c
	gcc $(CFLAGS) $< -o bin/$@
	objdump -S bin/$@ > ./objdump/$@.lst

div_four_independent_vectors_avx512f_approx:	div_four_independent_vectors_avx512f_approx.c
	gcc $(CFLAGS) $< -o bin/$@
	objdump -S bin/$@ > ./objdump/$@.lst

fma_avx256f:	fma_avx256f.c
	gcc $(CFLAGS) -mfma $< -o bin/$@
	objdump -S bin/$@ > ./objdump/$@.lst

fma_avx512f:	fma_avx512f.c
	gcc $(CFLAGS) $< -o bin/$@
	objdump -S bin/$@ > ./objdump/$@.lst

harmonic_series:	harmonic_series.c
	gcc $(CFLAGS) $< -o bin/$@ -lrt -lm
	gdb -batch -ex "disassemble/rs HarmonicSeriesAVX512" bin/$@ > ./objdump/$@_HarmonicSeriesAVX512.lst
	gdb -batch -ex "disassemble/rs HarmonicSeriesAVX256" bin/$@ > ./objdump/$@_HarmonicSeriesAVX256.lst
	gdb -batch -ex "disassemble/rs HarmonicSeriesPlain"  bin/$@ > ./objdump/$@_HarmonicSeriesPlain.lst

mul_avx256f:	mul_avx256f.c
	gcc $(CFLAGS) $< -o bin/$@
	objdump -S bin/$@ > ./objdump/$@.lst

mul_avx512f:	mul_avx512f.c
	gcc $(CFLAGS) $< -o bin/$@
	objdump -S bin/$@ > ./objdump/$@.lst

sub_avx256f:	sub_avx256f.c
	gcc $(CFLAGS) $< -o bin/$@
	objdump -S bin/$@ > ./objdump/$@.lst

sub_avx512f:	sub_avx512f.c
	gcc $(CFLAGS) $< -o bin/$@
	objdump -S bin/$@ > ./objdump/$@.lst


