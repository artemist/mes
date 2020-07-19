#! /bin/sh

set -x

ptr_once='s,([^N])(MACRO|NAME|VALUE) \(([^()]*)\),\1\3->\L\2,'
ptr_once='s,([^N])(BYTES|CAR|CDR|CLOSURE|CONTINUATION|LENGTH|MACRO|NAME|PORT|REF|STRING|STRUCT|TYPE|VALUE|VARIABLE|VECTOR) \(([^()]*)\),\1\3->\L\2,'
n_once='s,N(TYPE|CAR|CDR|LENGTH|VALUE|VECTOR) \(([^()]*)\),\2->\L\1,'
ncbytes='s,NC(BYTES) \(([^()]*)\),news_\L\1 (\2),'
cbytes='s,([^N])C(BYTES) \(([^()]*)\),\1cell_\L\2 (\3),'
cstring='s,C(STRING) \(([^()]*)\),cell_bytes (STRING (\2)),'
struct='s,SCM,struct scm *,g'
struct_scm_p='s,struct scm\* ?,struct scm *,g'

sed -ri                                                         \
    -e 's,POINTER_CELLS=0,POINTER_CELLS=1',                     \
    -e 's,#define POINTER_CELLS 0,#define POINTER_CELLS 1',     \
    -e "$ncbytes"                                               \
    -e "$cbytes"                                                \
    -e "$cstring"                                               \
                                                                \
    -e "$ncbytes"                                               \
    -e "$cbytes"                                                \
    -e "$cstring"                                               \
                                                                \
    -e "$ptr_once"                                              \
    -e "$ptr_once"                                              \
    -e "$ptr_once"                                              \
    -e 's,->\<struct\>,->structure,g'                           \
    -e "$struct"                                                \
    -e "$struct_scm_p"                                          \
                                                                \
    -e "$n_once"                                                \
    -e "$n_once"                                                \
                                                                \
    -e 's,CAR \(([^()]*)\),\1->cdr,'                            \
    -e 's,CAAR \(([^()]*)\),\1->car->car,'                      \
    -e 's,CADR \(([^()]*)\),\1->cdr->car,'                      \
    -e 's,CDAR \(([^()]*)\),\1->car->cdr,'                      \
    -e 's,CDDR \(([^()]*)\),\1->cdr->cdr,'                      \
    -e 's,CADAR \(([^()]*)\),\1->car->cdr->car,'                \
    -e 's,CADDR \(([^()]*)\),\1->cdr->cdr->car,'                \
    -e 's,CDDDR \(([^()]*)\),\1->cdr->cdr->cdr,'                \
    -e 's,CDADAR \(([^()]*)\),\1->cdr->car->cdr->car,'          \
                                                                \
    include/mes/builtins.h                                      \
    include/mes/mes.h                                           \
    include/mes/symbols.h                                       \
    include/mes/builtins.h                                      \
    src/builtins.c                                              \
    src/core.c                                                  \
    src/display.c                                               \
    src/eval-apply.c                                            \
    src/gc.c                                                    \
    src/hash.c                                                  \
    src/lib.c                                                   \
    src/math.c                                                  \
    src/mes.c                                                   \
    src/module.c                                                \
    src/posix.c                                                 \
    src/reader.c                                                \
    src/stack.c                                                 \
    src/string.c                                                \
    src/struct.c                                                \
    src/symbol.c                                                \
    src/vector.c                                                \
    simple.make
