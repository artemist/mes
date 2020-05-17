/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2016,2017,2018,2019 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
 *
 * This file is part of GNU Mes.
 *
 * GNU Mes is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * GNU Mes is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Mes.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __MES_MES_H
#define __MES_MES_H

#include <sys/types.h>
#include "mes/cc.h"

struct scm
{
  long type;
  SCM car;
  SCM cdr;
};

/* mes */
char *g_datadir;
int g_debug;
char *g_buf;
SCM g_continuations;
SCM g_symbols;
SCM g_symbol_max;

/* a/env */
SCM R0;
/* param 1 */
SCM R1;
/* save 2 */
SCM R2;
/* continuation */
SCM R3;
/* current-module */
SCM M0;
/* macro */
SCM g_macros;
SCM g_ports;

/* gc */
long ARENA_SIZE;
long MAX_ARENA_SIZE;
long STACK_SIZE;
long JAM_SIZE;
long GC_SAFETY;
long MAX_STRING;
char *g_arena;
long g_free;
SCM g_stack;
SCM *g_stack_array;
struct scm *g_cells;
struct scm *g_news;

char **__execl_c_argv;
char *__getcwd_buf;
char *__open_boot_buf;
char *__open_boot_file_name;
char *__setenv_buf;
char *__reader_read_char_buf;

SCM alloc (long n);
SCM apply (SCM f, SCM x, SCM a);
SCM apply_builtin (SCM fn, SCM x);
SCM builtin_name (SCM builtin);
FUNCTION builtin_function (SCM builtin);
SCM cstring_to_list (char const *s);
SCM cstring_to_symbol (char const *s);
SCM fdisplay_ (SCM, int, int);
SCM gc_init ();
SCM gc_peek_frame ();
SCM gc_pop_frame ();
SCM gc_push_frame ();
SCM init_symbols ();
SCM init_time (SCM a);
SCM make_builtin_type ();
SCM make_bytes (char const *s, size_t length);
SCM make_cell__ (long type, SCM car, SCM cdr);
SCM make_hash_table_ (long size);
SCM make_hashq_type ();
SCM make_initial_module (SCM a);
SCM make_string (char const *s, size_t length);
SCM make_vector__ (long k);
SCM mes_builtins (SCM a);
SCM push_cc (SCM p1, SCM p2, SCM a, SCM c);
SCM struct_ref_ (SCM x, long i);
SCM struct_set_x_ (SCM x, long i, SCM e);
SCM vector_ref_ (SCM x, long i);
SCM vector_set_x_ (SCM x, long i, SCM e);
int peekchar ();
int readchar ();
int unreadchar ();
long length__ (SCM x);
size_t bytes_cells (size_t length);
void assert_max_string (size_t i, char const *msg, char *string);
void assert_msg (int check, char *msg);

#include "mes/builtins.h"
#include "mes/constants.h"
#include "mes/macros.h"

#endif /* __MES_MES_H */
