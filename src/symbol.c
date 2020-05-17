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

#include <string.h>

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
init_symbols ()                  /*:((internal)) */
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
