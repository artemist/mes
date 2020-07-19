# GNU Mes --- Maxwell Equations of Software
# Copyright © 2019 Jeremiah Orians <jeremiah@pdp10.guru>
# Copyright © 2018,2019 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
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
M2_PLANET_FLAGS = --architecture amd64

CFLAGS:=					\
  $(CFLAGS)					\
 -D _GNU_SOURCE					\
 -D const=					\
 -ggdb						\
 -D SYSTEM_LIBC=1				\
 -D 'MES_VERSION="git"'				\
 -D 'MES_PKGDATADIR="/usr/local/share/mes"'	\
 -I include					\
 -fno-builtin					\
 -Wno-discarded-qualifiers			\
 -Wno-discarded-array-qualifiers		\
 -Wno-ignored-qualifiers			\
 -Wno-incompatible-pointer-types		\
 -Wno-int-conversion

MES_SOURCES =					\
 src/builtins.c					\
 src/core.c					\
 src/display.c					\
 src/eval-apply.c				\
 src/gc.c					\
 src/hash.c					\
 src/lib.c					\
 src/math.c					\
 src/mes.c					\
 src/module.c					\
 src/posix.c					\
 src/reader.c					\
 src/string.c					\
 src/struct.c					\
 src/symbol.c					\
 src/vector.c

LIB_SOURCES =					\
 lib/mes/eputs.c				\
 lib/mes/assert_msg.c				\
 lib/mes/itoa.c

M2_SOURCES =					\

M2_TODO =					\
 lib/m2/file_print.c				\
 lib/m2/ntoab.c					\
 lib/mes/fdgetc.c				\
 lib/mes/fdungetc.c

SOURCES = $(M2_SOURCES) $(LIB_SOURCES) $(MES_SOURCES)
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
 lib/mes/eputc.c				\
 lib/mes/fdgetc.c				\
 lib/mes/fdputc.c				\
 lib/mes/fdputs.c				\
 lib/mes/fdungetc.c				\
 lib/mes/mes_open.c				\
 lib/mes/ntoab.c				\
 $(SOURCES)

mes-gcc: bin/mes-gcc
mes-m2: bin/mes-m2

bin/mes-gcc: $(MAKEFILES) $(GCC_SOURCES) $(INCLUDES) | bin
	$(CC) $(CFLAGS) $(GCC_SOURCES) -o $@

M2_PLANET_INCLUDES =				\
 include/mes/mes.h				\
 include/mes/m2.h				\
 include/mes/builtins.h				\
 include/mes/constants.h			\
 include/mes/symbols.h

M2_PLANET_PREFIX = ../M2-Planet
M2_PLANET_SOURCES =						\
 $(M2_PLANET_PREFIX)/test/common_amd64/functions/exit.c		\
 $(M2_PLANET_PREFIX)/test/common_amd64/functions/malloc.c	\
 $(M2_PLANET_PREFIX)/functions/calloc.c				\
 $(M2_PLANET_INCLUDES:%.h=%.h.m2)				\
 $(SOURCES:%.c=%.c.m2)

%.h.m2: %.h $(MAKEFILES)
	@sed -r					\
	    -e 's,^//,@@,'			\
	    -e 's@^(#include.*)@/* \1 */@'	\
	    $<					\
	| $(CC) -E -I include			\
	    -D __M2_PLANET__=1			\
	    -D FUNCTION0=FUNCTION		\
	    -D FUNCTION1=FUNCTION		\
	    -D FUNCTION2=FUNCTION		\
	    -D FUNCTION3=FUNCTION		\
	    -D FUNCTIONN=FUNCTION		\
	    -D const=				\
	    -D long=SCM				\
	    -D size_t=SCM			\
	    -D ssize_t=SCM			\
	    -D unsigned=SCM			\
            -include mes/m2.h			\
	    -x c -				\
	| sed -r				\
	    -e 's,^@@,//,'			\
	    > $@				\

%.c.m2: %.c $(MAKEFILES)
	@sed -r					\
	    -e 's,^//,@@,'			\
	    -e 's@^(#include.*)@/* \1 */@'	\
	    $<					\
	| $(CC) -E -I include			\
	    -D __M2_PLANET__=1			\
	    -D FUNCTION0=FUNCTION		\
	    -D FUNCTION1=FUNCTION		\
	    -D FUNCTION2=FUNCTION		\
	    -D FUNCTION3=FUNCTION		\
	    -D FUNCTIONN=FUNCTION		\
	    -D const=				\
	    -D long=SCM				\
	    -D size_t=SCM			\
	    -D ssize_t=SCM			\
	    -D unsigned=SCM			\
            -include mes/m2.h			\
	    -x c -				\
	| sed -r				\
	    -e 's,^@@,//,'			\
	    > $@

bin/mes-m2: $(MAKEFILES) $(M2_PLANET_SOURCES) $(M2_PLANET_INCLUDES) | bin
	$(M2_PLANET) $(M2_PLANET_FLAGS) $(M2_PLANET_SOURCES:%=-f %) -o $@ || rm -f $@

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
#	MES_DEBUG=1 MES_PREFIX=mes MES=$(MES) sh -x scripts/mescc -- -I include -nostdlib lib/string/strlen.c lib/mes/eputs.c scaffold/hello.c
	MES_DEBUG=1 MES_PREFIX=mes MES=$(MES) sh -x scripts/mescc -- -m 32 -I include -nostdlib lib/linux/$(MESCC_CPU)-mes-mescc/crt1.c scaffold/main.c
	./a.out; r=$$?; if [ $$r != 42 ]; then exit 1; fi


# Directories
bin:
	mkdir -p bin

TAGS:
	etags $(shell find . -name '*.c' -o -name '*.h') --language=scheme $(shell find mes module -name '*.mes' -o -name '*.scm')
