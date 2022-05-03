# GNU Mes --- Maxwell Equations of Software
# Copyright © 2019 Jeremiah Orians <jeremiah@pdp10.guru>
# Copyright © 2018,2019,2020,2021,2022 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
#
# This file is part of GNU Mes.
#
# GNU Mes is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or (at
# your option) any later version.
#
# GNU Mes is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Mes.  If not, see <http://www.gnu.org/licenses/>.

CC = gcc
MES_CPU=x86_64
MES = bin/mes-gcc

MESCC_CPU=x86
MES = bin/mes-gcc

#CC = i686-unknown-linux-gnu-gcc
#MES_CPU = x86

M2_PLANET = M2-Planet
M2_PLANET_ARCH = x86
M2_PLANET_FUBAR = i386

#M2_PLANET_ARCH = amd64
#M2_PLANET_FUBAR = amd64
M2_PLANET_FLAGS = --bootstrap-mode --debug --architecture $(M2_PLANET_ARCH)

CFLAGS:=					\
  $(CFLAGS)					\
 -D _GNU_SOURCE					\
 -D const=					\
 -ggdb						\
 -D SYSTEM_LIBC=1				\
 -D 'MES_VERSION="git"'				\
 -D 'MES_PKGDATADIR="/usr/local/share/mes"'	\
 -I include					\
 -fno-builtin

LIBMES_SOURCES =				\
 src/builtins.c					\
 src/core.c					\
 src/display.c					\
 src/eval-apply.c				\
 src/gc.c					\
 src/hash.c					\
 src/lib.c					\
 src/math.c					\
 src/module.c					\
 src/posix.c					\
 src/reader.c					\
 src/string.c					\
 src/stack.c					\
 src/struct.c					\
 src/symbol.c					\
 src/vector.c

MES_SOURCES =					\
 $(LIBMES_SOURCES)				\
 src/mes.c

TEST_GC_SOURCES =				\
 $(LIBMES_SOURCES)				\
 src/test/gc.c

M2_SOURCES =					\
 lib/linux/x86-mes-m2/crt1.c			\
 lib/linux/x86-mes-m2/_exit.c			\
 lib/linux/x86-mes-m2/_write.c			\
 lib/mes/globals.c	                        \
 lib/m2/cast.c					\
 lib/m2/exit.c					\
 lib/mes/mini-write.c				\
 lib/linux/x86-mes-m2/syscall.c			\
 lib/stub/__raise.c				\
 lib/linux/brk.c				\
 lib/m2/malloc.c				\
 lib/string/memset.c				\
						\
 lib/m2/read.c					\
 lib/mes/fdgetc.c				\
 lib/stdio/getchar.c				\
 lib/stdio/putchar.c				\
 lib/m2/open.c					\
 lib/m2/mes_open.c				\
 lib/string/strlen.c				\
 lib/mes/eputs.c				\
 lib/mes/fdputc.c				\
 lib/mes/eputc.c				\
						\
 lib/mes/__assert_fail.c			\
 lib/mes/assert_msg.c				\
						\
 lib/mes/fdputc.c				\
 lib/string/strncmp.c				\
 lib/posix/getenv.c				\
 lib/mes/fdputs.c				\
 lib/mes/ntoab.c				\
 lib/ctype/isdigit.c				\
 lib/ctype/isxdigit.c				\
 lib/ctype/isspace.c				\
 lib/ctype/isnumber.c				\
 lib/mes/abtol.c				\
 lib/stdlib/atoi.c				\
 lib/string/memcpy.c				\
 lib/stdlib/free.c				\
 lib/stdlib/realloc.c				\
 lib/string/strcpy.c				\
 lib/mes/itoa.c					\
 lib/mes/ltoa.c					\
 lib/mes/fdungetc.c				\
 lib/posix/setenv.c				\
 lib/linux/access.c				\
 lib/m2/chmod.c					\
 lib/linux/ioctl3.c  				\
 lib/m2/isatty.c				\
 lib/linux/fork.c				\
 lib/m2/execve.c				\
 lib/m2/execv.c					\
 lib/m2/waitpid.c				\
 lib/linux/gettimeofday.c			\
 lib/m2/clock_gettime.c				\
 lib/m2/time.c					\
 lib/linux/_getcwd.c				\
 lib/m2/getcwd.c				\
 lib/linux/dup.c				\
 lib/linux/dup2.c				\
 lib/string/strcmp.c				\
 lib/string/memcmp.c				\
 lib/linux/unlink.c				\
 src/m2.c

M2_TODO =					\
 lib/m2/file_print.c				\
 lib/mes/ntoab.c				\
 lib/mes/fdgetc.c				\
 lib/mes/fdungetc.c

INCLUDES =					\
 include/mes/builtins.h				\
 include/mes/constants.h			\
 include/mes/mes.h				\
 include/mes/lib.h				\
 include/mes/lib-mini.h

MES_LIBC =					\
 lib/linux/$(MES_CPU)-mes-gcc/syscall.c		\
 lib/linux/_read.c				\
 lib/linux/_read.c				\
 lib/mes/__buffered_read.c

GCC_SOURCES =					\
 lib/mes/__mes_debug.c				\
 lib/mes/cast.c					\
 lib/mes/eputc.c				\
 lib/mes/eputs.c				\
 lib/mes/fdgetc.c				\
 lib/mes/fdputc.c				\
 lib/mes/fdputs.c				\
 lib/mes/fdungetc.c				\
 lib/mes/mes_open.c				\
 lib/mes/ntoab.c				\
 lib/mes/itoa.c					\
 lib/mes/ltoa.c					\
 lib/mes/assert_msg.c				\
 src/cc.c					\
 src/globals.c

mes-gcc: bin/mes-gcc
mes-m2: bin/mes-m2

gc-gcc: bin/gc-gcc
gc-m2: bin/gc-m2

bin/mes-gcc: simple.make $(GCC_SOURCES) $(MES_SOURCES) $(INCLUDES) | bin
	$(CC) $(CFLAGS) -o $@ $(GCC_SOURCES) $(MES_SOURCES)
	cp -f $@ bin/mes

bin/gc-gcc: simple.make $(GCC_SOURCES) $(TEST_GC_SOURCES) $(INCLUDES) | bin
	$(CC) $(CFLAGS) -D GC_TEST=1 -o $@ $(GCC_SOURCES) $(TEST_GC_SOURCES)

M2_PLANET_INCLUDES =				\
 include/m2/lib.h				\
 include/linux/x86/syscall.h			\
 include/mes/mes.h				\
 include/mes/builtins.h				\
 include/mes/constants.h			\
 include/mes/symbols.h				\
 include/linux/$(M2_PLANET_ARCH)/syscall.h

M2_PLANET_SOURCES =				\
 $(M2_PLANET_INCLUDES:%.h=%.h)			\
 $(M2_SOURCES)

m2/mes-m2.M1: simple.make $(M2_PLANET_SOURCES) $(MES_SOURCES) $(M2_PLANET_INCLUDES) | m2
	$(M2_PLANET) $(M2_PLANET_FLAGS) $(M2_PLANET_SOURCES:%=-f %)  $(MES_SOURCES:%.c=-f %.c) -o $@ || rm -f $@

m2/mes-m2.blood-elf.M1: m2/mes-m2.M1 | m2
	blood-elf --little-endian -f $< -o $@

m2/mes-m2.hex2: m2/mes-m2.blood-elf.M1
	M1					\
	    --architecture $(M2_PLANET_ARCH)	\
	    --little-endian			\
	    -f lib/m2/x86/x86_defs.M1		\
	    -f lib/x86-mes/x86.M1		\
	    -f lib/linux/x86-mes-m2/crt1.M1	\
	    -f m2/mes-m2.M1			\
	    -f m2/mes-m2.blood-elf.M1		\
	    -o $@

bin/mes-m2: m2/mes-m2.hex2 | bin
	hex2						\
	    --architecture $(M2_PLANET_ARCH)		\
	    --little-endian				\
	    --base-address 0x1000000			\
	    -f lib/linux/x86-mes/elf32-header.hex2	\
	    -f m2/mes-m2.hex2				\
	    -o $@
	cp -f $@ bin/mes

# Clean up after ourselves
.PHONY: clean
clean:
	rm -rf bin/

.PHONY: check check-gcc check-m2 check-hello check-base check-gc check-mescc
check-gcc:
	$(MAKE) -f simple.make check MES=bin/mes-gcc
check-m2:
	$(MAKE) -f simple.make check MES=bin/mes-m2

check: check-hello check-base check-gc check-mescc

check-hello: $(MES)
	echo '(display "hello\n")' | MES_BOOT=boot-01.scm $(MES)

check-base: $(MES)
	MES_DEBUG=4 MES_PREFIX=mes MES=$(MES) tests/base.test

check-gc: $(MES)
	MES_DEBUG=3 MES_ARENA=10000 MES_MAX_ARENA=10000 MES_BOOT=tests/gc.test $(MES)

check-mescc: $(MES)
	rm -f a.out
# this already needs succesful GC
#	LIBRARY_PATH=lib MES_DEBUG=1 MES_PREFIX=mes MES=$(MES) sh -x scripts/mescc -- -I include -nostdlib lib/mes/globals.c lib/string/strlen.c lib/mes/eputs.c scaffold/hello.c
	LIBRARY_PATH=lib MES_DEBUG=1 MES_PREFIX=mes MES=$(MES) sh -x scripts/mescc -- -m 32 -I include -nostdlib lib/mes/globals.c lib/linux/$(MESCC_CPU)-mes-mescc/crt1.c scaffold/main.c
	./a.out; r=$$?; if [ $$r != 42 ]; then exit 1; fi


# Directories
bin:
	mkdir -p $@

m2:
	mkdir -p $@

TAGS:
	etags $(shell find . -name '*.c' -o -name '*.h') --language=scheme $(shell find mes module -name '*.mes' -o -name '*.scm')
