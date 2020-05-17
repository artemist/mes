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

#include "mes/lib.h"
#include "mes/mes.h"

#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

// char const *MES_PKGDATADIR = "mes";

SCM
alloc (long n)
{
  SCM x = g_free;
  g_free = g_free + n;
  if (g_free > ARENA_SIZE)
    assert_msg (0, "alloc: out of memory");
  return x;
}

SCM
make_cell__ (long type, SCM car, SCM cdr)
{
  SCM x = alloc (1);
  TYPE (x) = type;
  CAR (x) = car;
  CDR (x) = cdr;
  return x;
}

SCM
make_cell_ (SCM type, SCM car, SCM cdr)
{
  assert_msg (TYPE (type) == TNUMBER, "TYPE (type) == TNUMBER");
  long t = VALUE (type);
  if (t == TCHAR || t == TNUMBER)
    {
      if (car != 0)
        car = CAR (car);
      else
        car = 0;
      if (cdr != 0)
        cdr = CDR (cdr);
      else
        cdr = 0;
      return make_cell__ (t, car, cdr);
    }
  return make_cell__ (t, car, cdr);
}

SCM
assoc_string (SCM x, SCM a)     /*:((internal)) */
{
  while (a != cell_nil && (TYPE (CAAR (a)) != TSTRING || string_equal_p (x, CAAR (a)) == cell_f))
    a = CDR (a);
  if (a != cell_nil)
    return CAR (a);
  return cell_f;
}

SCM
type_ (SCM x)
{
  return MAKE_NUMBER (TYPE (x));
}

SCM
car_ (SCM x)
{
  if (TYPE (x) != TCONTINUATION && (TYPE (CAR (x)) == TPAIR     // FIXME: this is weird
                                    || TYPE (CAR (x)) == TREF
                                    || TYPE (CAR (x)) == TSPECIAL
                                    || TYPE (CAR (x)) == TSYMBOL || TYPE (CAR (x)) == TSTRING))
    return CAR (x);
  return MAKE_NUMBER (CAR (x));
}

SCM
cdr_ (SCM x)
{
  if (TYPE (x) != TCHAR
      && TYPE (x) != TNUMBER
      && TYPE (x) != TPORT
      && (TYPE (CDR (x)) == TPAIR
          || TYPE (CDR (x)) == TREF
          || TYPE (CDR (x)) == TSPECIAL || TYPE (CDR (x)) == TSYMBOL || TYPE (CDR (x)) == TSTRING))
    return CDR (x);
  return MAKE_NUMBER (CDR (x));
}

SCM
cons (SCM x, SCM y)
{
  return make_cell__ (TPAIR, x, y);
}

SCM
car (SCM x)
{
#if !__MESC_MES__
  if (TYPE (x) != TPAIR)
    error (cell_symbol_not_a_pair, cons (x, cell_symbol_car));
#endif
  return CAR (x);
}

SCM
cdr (SCM x)
{
#if !__MESC_MES__
  if (TYPE (x) != TPAIR)
    error (cell_symbol_not_a_pair, cons (x, cell_symbol_cdr));
#endif
  return CDR (x);
}

SCM
list (SCM x)                    /*:((arity . n)) */
{
  return x;
}

SCM
null_p (SCM x)
{
  if (x == cell_nil)
    return cell_t;
  return cell_f;
}

SCM
eq_p (SCM x, SCM y)
{
  if (x == y
      || ((TYPE (x) == TKEYWORD && TYPE (y) == TKEYWORD
           && string_equal_p (x, y) == cell_t))
      || (TYPE (x) == TCHAR && TYPE (y) == TCHAR
          && VALUE (x) == VALUE (y))
      || (TYPE (x) == TNUMBER && TYPE (y) == TNUMBER && VALUE (x) == VALUE (y)))
    return cell_t;
  return cell_f;
}

SCM
values (SCM x)                  /*:((arity . n)) */
{
  SCM v = cons (0, x);
  TYPE (v) = TVALUES;
  return v;
}

SCM
acons (SCM key, SCM value, SCM alist)
{
  return cons (cons (key, value), alist);
}

long
length__ (SCM x)                /*:((internal)) */
{
  long n = 0;
  while (x != cell_nil)
    {
      n = n + 1;
      if (TYPE (x) != TPAIR)
        return -1;
      x = CDR (x);
    }
  return n;
}

SCM
length (SCM x)
{
  return MAKE_NUMBER (length__ (x));
}

SCM
error (SCM key, SCM x)
{
#if !__MESC_MES__
  SCM throw;
  if ((throw = module_ref (r0, cell_symbol_throw)) != cell_undefined)
    return apply (throw, cons (key, cons (x, cell_nil)), r0);
#endif
  display_error_ (key);
  eputs (": ");
  write_error_ (x);
  eputs ("\n");
  assert_msg (0, "0");
  exit (1);
}

SCM
append2 (SCM x, SCM y)
{
  if (x == cell_nil)
    return y;
  if (TYPE (x) != TPAIR)
    error (cell_symbol_not_a_pair, cons (x, cstring_to_symbol ("append2")));
  SCM r = cell_nil;
  while (x != cell_nil)
    {
      r = cons (CAR (x), r);
      x = CDR (x);
    }
  return reverse_x_ (r, y);
}

SCM
append_reverse (SCM x, SCM y)
{
  if (x == cell_nil)
    return y;
  if (TYPE (x) != TPAIR)
    error (cell_symbol_not_a_pair, cons (x, cstring_to_symbol ("append-reverse")));
  while (x != cell_nil)
    {
      y = cons (CAR (x), y);
      x = CDR (x);
    }
  return y;
}

SCM
reverse_x_ (SCM x, SCM t)
{
  if (x != cell_nil && TYPE (x) != TPAIR)
    error (cell_symbol_not_a_pair, cons (x, cstring_to_symbol ("core:reverse!")));
  SCM r = t;
  while (x != cell_nil)
    {
      t = CDR (x);
      CDR (x) = r;
      r = x;
      x = t;
    }
  return r;
}

SCM
assq (SCM x, SCM a)
{
  if (TYPE (a) != TPAIR)
    return cell_f;
  int t = TYPE (x);
  if (t == TSYMBOL || t == TSPECIAL)
    while (a != cell_nil && x != CAAR (a))
      a = CDR (a);
  else if (t == TCHAR || t == TNUMBER)
    {
      SCM v = VALUE (x);
      while (a != cell_nil && v != VALUE (CAAR (a)))
        a = CDR (a);
    }
  else if (t == TKEYWORD)
    {
      while (a != cell_nil && string_equal_p (x, CAAR (a)) == cell_f)
        a = CDR (a);
    }
  else
    /* pointer equality, e.g. on strings. */
    while (a != cell_nil && x != CAAR (a))
      a = CDR (a);
  if (a != cell_nil)
    return CAR (a);
  return cell_f;
}

SCM
assoc (SCM x, SCM a)
{
  if (TYPE (x) == TSTRING)
    return assoc_string (x, a);
  while (a != cell_nil && equal2_p (x, CAAR (a)) == cell_f)
    a = CDR (a);
  if (a != cell_nil)
    return CAR (a);
  return cell_f;
}

SCM
mes_g_stack (SCM a)             /*:((internal)) */
{
  g_stack = STACK_SIZE;
  r0 = a;
  r1 = MAKE_CHAR (0);
  r2 = MAKE_CHAR (0);
  r3 = MAKE_CHAR (0);
  return r0;
}

void
init_symbol (long x, long type, char const *name)
{
  TYPE (x) = type;
  int length = strlen (name);
  SCM string = make_string (name, length);
  CAR (x) = length;
  CDR (x) = STRING (string);
  hash_set_x (g_symbols, string, x);
}

SCM
mes_symbols ()                  /*:((internal)) */
{
  g_free = cell_symbol_test + 1;
  g_symbol_max = g_free;
  g_symbols = make_hash_table_ (500);

  int size = VALUE (struct_ref_ (g_symbols, 3));
  // Weird: m2-planet exits 67 here...[printing size = 100]
  // if (size == 0) exit (66);
  // if (!size) exit (67);

  init_symbol (cell_nil, TSPECIAL, "()");
  init_symbol (cell_f, TSPECIAL, "#f");
  init_symbol (cell_t, TSPECIAL, "#t");
  init_symbol (cell_dot, TSPECIAL, ".");
  init_symbol (cell_arrow, TSPECIAL, "=>");
  init_symbol (cell_undefined, TSPECIAL, "*undefined*");
  init_symbol (cell_unspecified, TSPECIAL, "*unspecified*");
  init_symbol (cell_closure, TSPECIAL, "*closure*");
  init_symbol (cell_circular, TSPECIAL, "*circular*");
  init_symbol (cell_begin, TSPECIAL, "*begin*");
  init_symbol (cell_call_with_current_continuation, TSPECIAL, "*call/cc*");

  init_symbol (cell_vm_apply, TSPECIAL, "core:apply");
  init_symbol (cell_vm_apply2, TSPECIAL, "*vm-apply2*");
  init_symbol (cell_vm_begin, TSPECIAL, "*vm-begin*");
  init_symbol (cell_vm_begin_eval, TSPECIAL, "*vm:begin-eval*");
  init_symbol (cell_vm_begin_expand, TSPECIAL, "core:eval");
  init_symbol (cell_vm_begin_expand_eval, TSPECIAL, "*vm:begin-expand-eval*");
  init_symbol (cell_vm_begin_expand_macro, TSPECIAL, "*vm:begin-expand-macro*");
  init_symbol (cell_vm_begin_expand_primitive_load, TSPECIAL, "*vm:core:begin-expand-primitive-load*");
  init_symbol (cell_vm_begin_primitive_load, TSPECIAL, "*vm:core:begin-primitive-load*");
  init_symbol (cell_vm_begin_read_input_file, TSPECIAL, "*vm-begin-read-input-file*");
  init_symbol (cell_vm_call_with_current_continuation2, TSPECIAL, "*vm-call-with-current-continuation2*");
  init_symbol (cell_vm_call_with_values2, TSPECIAL, "*vm-call-with-values2*");
  init_symbol (cell_vm_eval, TSPECIAL, "core:eval-expanded");
  init_symbol (cell_vm_eval2, TSPECIAL, "*vm-eval2*");
  init_symbol (cell_vm_eval_check_func, TSPECIAL, "*vm-eval-check-func*");
  init_symbol (cell_vm_eval_define, TSPECIAL, "*vm-eval-define*");
  init_symbol (cell_vm_eval_macro_expand_eval, TSPECIAL, "*vm:eval-macro-expand-eval*");
  init_symbol (cell_vm_eval_macro_expand_expand, TSPECIAL, "*vm:eval-macro-expand-expand*");
  init_symbol (cell_vm_eval_pmatch_car, TSPECIAL, "*vm-eval-pmatch-car*");
  init_symbol (cell_vm_eval_pmatch_cdr, TSPECIAL, "*vm-eval-pmatch-cdr*");
  init_symbol (cell_vm_eval_set_x, TSPECIAL, "*vm-eval-set!*");
  init_symbol (cell_vm_evlis, TSPECIAL, "*vm-evlis*");
  init_symbol (cell_vm_evlis2, TSPECIAL, "*vm-evlis2*");
  init_symbol (cell_vm_evlis3, TSPECIAL, "*vm-evlis3*");
  init_symbol (cell_vm_if, TSPECIAL, "*vm-if*");
  init_symbol (cell_vm_if_expr, TSPECIAL, "*vm-if-expr*");
  init_symbol (cell_vm_macro_expand, TSPECIAL, "core:macro-expand");
  init_symbol (cell_vm_macro_expand_car, TSPECIAL, "*vm:core:macro-expand-car*");
  init_symbol (cell_vm_macro_expand_cdr, TSPECIAL, "*vm:macro-expand-cdr*");
  init_symbol (cell_vm_macro_expand_define, TSPECIAL, "*vm:core:macro-expand-define*");
  init_symbol (cell_vm_macro_expand_define_macro, TSPECIAL, "*vm:core:macro-expand-define-macro*");
  init_symbol (cell_vm_macro_expand_lambda, TSPECIAL, "*vm:core:macro-expand-lambda*");
  init_symbol (cell_vm_macro_expand_set_x, TSPECIAL, "*vm:core:macro-expand-set!*");
  init_symbol (cell_vm_return, TSPECIAL, "*vm-return*");

  init_symbol (cell_symbol_dot, TSYMBOL, "*dot*");
  init_symbol (cell_symbol_lambda, TSYMBOL, "lambda");
  init_symbol (cell_symbol_begin, TSYMBOL, "begin");
  init_symbol (cell_symbol_if, TSYMBOL, "if");
  init_symbol (cell_symbol_quote, TSYMBOL, "quote");
  init_symbol (cell_symbol_define, TSYMBOL, "define");
  init_symbol (cell_symbol_define_macro, TSYMBOL, "define-macro");

  init_symbol (cell_symbol_quasiquote, TSYMBOL, "quasiquote");
  init_symbol (cell_symbol_unquote, TSYMBOL, "unquote");
  init_symbol (cell_symbol_unquote_splicing, TSYMBOL, "unquote-splicing");
  init_symbol (cell_symbol_syntax, TSYMBOL, "syntax");
  init_symbol (cell_symbol_quasisyntax, TSYMBOL, "quasisyntax");
  init_symbol (cell_symbol_unsyntax, TSYMBOL, "unsyntax");
  init_symbol (cell_symbol_unsyntax_splicing, TSYMBOL, "unsyntax-splicing");

  init_symbol (cell_symbol_set_x, TSYMBOL, "set!");

  init_symbol (cell_symbol_sc_expand, TSYMBOL, "sc-expand");
  init_symbol (cell_symbol_macro_expand, TSYMBOL, "macro-expand");
  init_symbol (cell_symbol_portable_macro_expand, TSYMBOL, "portable-macro-expand");
  init_symbol (cell_symbol_sc_expander_alist, TSYMBOL, "*sc-expander-alist*");

  init_symbol (cell_symbol_call_with_values, TSYMBOL, "call-with-values");
  init_symbol (cell_symbol_call_with_current_continuation, TSYMBOL, "call-with-current-continuation");
  init_symbol (cell_symbol_boot_module, TSYMBOL, "boot-module");
  init_symbol (cell_symbol_current_module, TSYMBOL, "current-module");
  init_symbol (cell_symbol_primitive_load, TSYMBOL, "primitive-load");
  init_symbol (cell_symbol_read_input_file, TSYMBOL, "read-input-file");
  init_symbol (cell_symbol_write, TSYMBOL, "write");
  init_symbol (cell_symbol_display, TSYMBOL, "display");

  init_symbol (cell_symbol_car, TSYMBOL, "car");
  init_symbol (cell_symbol_cdr, TSYMBOL, "cdr");
  init_symbol (cell_symbol_not_a_number, TSYMBOL, "not-a-number");
  init_symbol (cell_symbol_not_a_pair, TSYMBOL, "not-a-pair");
  init_symbol (cell_symbol_system_error, TSYMBOL, "system-error");
  init_symbol (cell_symbol_throw, TSYMBOL, "throw");
  init_symbol (cell_symbol_unbound_variable, TSYMBOL, "unbound-variable");
  init_symbol (cell_symbol_wrong_number_of_args, TSYMBOL, "wrong-number-of-args");
  init_symbol (cell_symbol_wrong_type_arg, TSYMBOL, "wrong-type-arg");

  init_symbol (cell_symbol_buckets, TSYMBOL, "buckets");
  init_symbol (cell_symbol_builtin, TSYMBOL, "<builtin>");
  init_symbol (cell_symbol_frame, TSYMBOL, "<frame>");
  init_symbol (cell_symbol_hashq_table, TSYMBOL, "<hashq-table>");
  init_symbol (cell_symbol_module, TSYMBOL, "<module>");
  init_symbol (cell_symbol_procedure, TSYMBOL, "procedure");
  init_symbol (cell_symbol_record_type, TSYMBOL, "<record-type>");
  init_symbol (cell_symbol_size, TSYMBOL, "size");
  init_symbol (cell_symbol_stack, TSYMBOL, "<stack>");

  init_symbol (cell_symbol_argv, TSYMBOL, "%argv");
  init_symbol (cell_symbol_mes_datadir, TSYMBOL, "%datadir");
  init_symbol (cell_symbol_mes_version, TSYMBOL, "%version");

  init_symbol (cell_symbol_internal_time_units_per_second, TSYMBOL, "internal-time-units-per-second");
  init_symbol (cell_symbol_compiler, TSYMBOL, "%compiler");
  init_symbol (cell_symbol_arch, TSYMBOL, "%arch");

  init_symbol (cell_symbol_pmatch_car, TSYMBOL, "pmatch-car");
  init_symbol (cell_symbol_pmatch_cdr, TSYMBOL, "pmatch-cdr");

  init_symbol (cell_type_bytes, TSYMBOL, "<cell:bytes>");
  init_symbol (cell_type_char, TSYMBOL, "<cell:char>");
  init_symbol (cell_type_closure, TSYMBOL, "<cell:closure>");
  init_symbol (cell_type_continuation, TSYMBOL, "<cell:continuation>");
  init_symbol (cell_type_function, TSYMBOL, "<cell:function>");
  init_symbol (cell_type_keyword, TSYMBOL, "<cell:keyword>");
  init_symbol (cell_type_macro, TSYMBOL, "<cell:macro>");
  init_symbol (cell_type_number, TSYMBOL, "<cell:number>");
  init_symbol (cell_type_pair, TSYMBOL, "<cell:pair>");
  init_symbol (cell_type_port, TSYMBOL, "<cell:port>");
  init_symbol (cell_type_ref, TSYMBOL, "<cell:ref>");
  init_symbol (cell_type_special, TSYMBOL, "<cell:special>");
  init_symbol (cell_type_string, TSYMBOL, "<cell:string>");
  init_symbol (cell_type_struct, TSYMBOL, "<cell:struct>");
  init_symbol (cell_type_symbol, TSYMBOL, "<cell:symbol>");
  init_symbol (cell_type_values, TSYMBOL, "<cell:values>");
  init_symbol (cell_type_variable, TSYMBOL, "<cell:variable>");
  init_symbol (cell_type_vector, TSYMBOL, "<cell:vector>");
  init_symbol (cell_type_broken_heart, TSYMBOL, "<cell:broken-heart>");

  init_symbol (cell_symbol_test, TSYMBOL, "%%test");

  SCM a = cell_nil;
  a = acons (cell_symbol_call_with_values, cell_symbol_call_with_values, a);
  a = acons (cell_symbol_boot_module, cell_symbol_boot_module, a);
  a = acons (cell_symbol_current_module, cell_symbol_current_module, a);
  a = acons (cell_symbol_call_with_current_continuation, cell_call_with_current_continuation, a);

  a = acons (cell_symbol_mes_version, MAKE_STRING0 (MES_VERSION), a);
  a = acons (cell_symbol_mes_datadir, MAKE_STRING0 (g_datadir), a);

  a = acons (cell_type_bytes, MAKE_NUMBER (TBYTES), a);
  a = acons (cell_type_char, MAKE_NUMBER (TCHAR), a);
  a = acons (cell_type_closure, MAKE_NUMBER (TCLOSURE), a);
  a = acons (cell_type_continuation, MAKE_NUMBER (TCONTINUATION), a);
  a = acons (cell_type_keyword, MAKE_NUMBER (TKEYWORD), a);
  a = acons (cell_type_macro, MAKE_NUMBER (TMACRO), a);
  a = acons (cell_type_number, MAKE_NUMBER (TNUMBER), a);
  a = acons (cell_type_pair, MAKE_NUMBER (TPAIR), a);
  a = acons (cell_type_port, MAKE_NUMBER (TPORT), a);
  a = acons (cell_type_ref, MAKE_NUMBER (TREF), a);
  a = acons (cell_type_special, MAKE_NUMBER (TSPECIAL), a);
  a = acons (cell_type_string, MAKE_NUMBER (TSTRING), a);
  a = acons (cell_type_struct, MAKE_NUMBER (TSTRUCT), a);
  a = acons (cell_type_symbol, MAKE_NUMBER (TSYMBOL), a);
  a = acons (cell_type_values, MAKE_NUMBER (TVALUES), a);
  a = acons (cell_type_variable, MAKE_NUMBER (TVARIABLE), a);
  a = acons (cell_type_vector, MAKE_NUMBER (TVECTOR), a);
  a = acons (cell_type_broken_heart, MAKE_NUMBER (TBROKEN_HEART), a);

  a = acons (cell_closure, a, a);

  return a;
}

SCM
mes_environment (int argc, char **argv)
{
  SCM a = mes_symbols ();

  char *compiler = "gnuc";
#if __MESC__
  compiler = "mesc";
#elif __TINYC__
  compiler = "tcc";
#endif
  a = acons (cell_symbol_compiler, MAKE_STRING0 (compiler), a);

  char *arch;
#if __i386__
  arch = "x86";
#elif __arm__
  arch = "arm";
#elif __x86_64__
  arch = "x86_64";
#else
#error arch not supported
#endif
  a = acons (cell_symbol_arch, MAKE_STRING0 (arch), a);

#if !MES_MINI
  SCM lst = cell_nil;
  int i;
  for (i = argc - 1; i >= 0; i = i - 1)
    lst = cons (MAKE_STRING0 (argv[i]), lst);
  a = acons (cell_symbol_argv, lst, a);
#endif

  return mes_g_stack (a);
}

int
try_open_boot (char *file_name, char const *boot, char const *location)
{
  strcpy (file_name + strlen (file_name), boot);
  if (g_debug > 1)
    {
      eputs ("mes: reading boot-0 [");
      eputs (location);
      eputs ("]: ");
      eputs (file_name);
      eputs ("\n");
    }
  int fd = mes_open (file_name, O_RDONLY, 0);
  if (g_debug != 0 && fd > 0)
    {
      eputs ("mes: read boot-0: ");
      eputs (file_name);
      eputs ("\n");
    }
  return fd;
}

void
open_boot ()
{
  __stdin = -1;
  char *boot = __open_boot_buf;
  char *file_name = __open_boot_file_name;
  strcpy (g_datadir, ".");
  if (getenv ("MES_BOOT"))
    strcpy (boot, getenv ("MES_BOOT"));
  else
    strcpy (boot, "boot-0.scm");
  if (getenv ("MES_PREFIX"))
    {
      strcpy (g_datadir, getenv ("MES_PREFIX"));
      strcpy (g_datadir + strlen (g_datadir), "/mes");
      strcpy (file_name, g_datadir);
      strcpy (file_name + strlen (file_name), "/module/mes/");
      __stdin = try_open_boot (file_name, boot, "MES_PREFIX");
      if (__stdin < 0)
        {
          strcpy (g_datadir, getenv ("MES_PREFIX"));
          strcpy (g_datadir + strlen (g_datadir), "/share/mes");
          strcpy (file_name, g_datadir);
          strcpy (file_name + strlen (file_name), "/module/mes/");
          __stdin = try_open_boot (file_name, boot, "MES_PREFIX/share/mes");
        }
    }
  if (__stdin < 0)
    {
      g_datadir[0] = 0;
      if (getenv ("srcdest"))
        strcpy (g_datadir, getenv ("srcdest"));
      strcpy (g_datadir + strlen (g_datadir), "mes");
      strcpy (file_name, g_datadir);
      strcpy (file_name + strlen (file_name), "/module/mes/");
      __stdin = try_open_boot (file_name, boot, "${srcdest}mes");
    }
  if (__stdin < 0)
    {
      file_name[0] = 0;
      __stdin = try_open_boot (file_name, boot, "<boot>");
    }
  if (__stdin < 0)
    {
      eputs ("mes: boot failed: no such file: ");
      eputs (boot);
      eputs ("\n");
      exit (1);
    }
}

SCM
read_boot ()                    /*:((internal)) */
{
  r2 = read_input_file_env (r0);
  __stdin = STDIN;
  return r2;
}

void
init (char **envp)
{
  environ = envp;
  __execl_c_argv = malloc (1024 * sizeof (char *));     /* POSIX minimum: 4096 */
  __getcwd_buf = malloc (PATH_MAX);
  __open_boot_buf = malloc (PATH_MAX);
  __open_boot_file_name = malloc (PATH_MAX);
  __reader_read_char_buf = malloc (10);
  __setenv_buf = malloc (1024);
  g_datadir = malloc (1024);

  char *p;
  if (p = getenv ("MES_DEBUG"))
    g_debug = atoi (p);
  open_boot ();
  gc_init ();
  g_ports = 1;
}

int
main (int argc, char **argv, char **envp)
{
  init (envp);

  SCM a = mes_environment (argc, argv);
  a = mes_builtins (a);
  a = init_time (a);
  m0 = make_initial_module (a);
  g_macros = make_hash_table_ (0);

  if (g_debug > 5)
    module_printer (m0);

  SCM program = read_boot ();
  push_cc (r2, cell_unspecified, r0, cell_unspecified);

  if (g_debug > 2)
    {
      eputs ("\ngc stats: [");
      eputs (itoa (g_free));
      eputs ("]\n");
    }
  if (g_debug > 3)
    {
      eputs ("program: ");
      write_error_ (r1);
      eputs ("\n");
    }
  r3 = cell_vm_begin_expand;
  r1 = eval_apply ();
  if (g_debug != 0)
    {
      write_error_ (r1);
      eputs ("\n");
    }
  if (g_debug != 0)
    {
      if (g_debug > 5)
        module_printer (m0);

      eputs ("\ngc stats: [");
      eputs (itoa (g_free));
      MAX_ARENA_SIZE = 0;

      gc (g_stack);
      eputs (" => ");
      eputs (itoa (g_free));
      eputs ("]\n");
      eputs ("\n");

      if (g_debug > 5)
        {
          eputs ("ports:");
          write_error_ (g_ports);
          eputs ("\n");
        }
      eputs ("\n");


    }
  return 0;
}
