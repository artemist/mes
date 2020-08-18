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

#ifndef __MES_SYMBOLS_H
#define __MES_SYMBOLS_H

SCM cell_nil;
SCM cell_f;
SCM cell_t;
SCM cell_dot;
SCM cell_arrow;
SCM cell_undefined;
SCM cell_unspecified;
SCM cell_closure;
SCM cell_circular;
SCM cell_begin;
SCM cell_call_with_current_continuation;
SCM cell_vm_apply;
SCM cell_vm_apply2;
SCM cell_vm_begin;
SCM cell_vm_begin_eval;
SCM cell_vm_begin_expand;
SCM cell_vm_begin_expand_eval;
SCM cell_vm_begin_expand_macro;
SCM cell_vm_begin_expand_primitive_load;
SCM cell_vm_begin_primitive_load;
SCM cell_vm_begin_read_input_file;
SCM cell_vm_call_with_current_continuation2;
SCM cell_vm_call_with_values2;
SCM cell_vm_eval;
SCM cell_vm_eval2;
SCM cell_vm_eval_check_func;
SCM cell_vm_eval_define;
SCM cell_vm_eval_macro_expand_eval;
SCM cell_vm_eval_macro_expand_expand;
SCM cell_vm_eval_pmatch_car;
SCM cell_vm_eval_pmatch_cdr;
SCM cell_vm_eval_set_x;
SCM cell_vm_evlis;
SCM cell_vm_evlis2;
SCM cell_vm_evlis3;
SCM cell_vm_if;
SCM cell_vm_if_expr;
SCM cell_vm_macro_expand;
SCM cell_vm_macro_expand_car;
SCM cell_vm_macro_expand_cdr;
SCM cell_vm_macro_expand_define;
SCM cell_vm_macro_expand_define_macro;
SCM cell_vm_macro_expand_lambda;
SCM cell_vm_macro_expand_set_x;
SCM cell_vm_return;

SCM cell_symbol_lambda;
SCM cell_symbol_begin;
SCM cell_symbol_if;
SCM cell_symbol_quote;
SCM cell_symbol_define;
SCM cell_symbol_define_macro;
SCM cell_symbol_quasiquote;
SCM cell_symbol_unquote;
SCM cell_symbol_unquote_splicing;
SCM cell_symbol_syntax;
SCM cell_symbol_quasisyntax;
SCM cell_symbol_unsyntax;
SCM cell_symbol_unsyntax_splicing;
SCM cell_symbol_set_x;
SCM cell_symbol_sc_expand;
SCM cell_symbol_macro_expand;
SCM cell_symbol_portable_macro_expand;
SCM cell_symbol_sc_expander_alist;
SCM cell_symbol_call_with_values;
SCM cell_symbol_call_with_current_continuation;
SCM cell_symbol_boot_module;
SCM cell_symbol_current_module;
SCM cell_symbol_primitive_load;
SCM cell_symbol_car;
SCM cell_symbol_cdr;
SCM cell_symbol_not_a_number;
SCM cell_symbol_not_a_pair;
SCM cell_symbol_system_error;
SCM cell_symbol_throw;
SCM cell_symbol_unbound_variable;
SCM cell_symbol_wrong_number_of_args;
SCM cell_symbol_wrong_type_arg;
SCM cell_symbol_buckets;
SCM cell_symbol_builtin;
SCM cell_symbol_frame;
SCM cell_symbol_hashq_table;
SCM cell_symbol_module;
SCM cell_symbol_procedure;
SCM cell_symbol_record_type;
SCM cell_symbol_size;
SCM cell_symbol_stack;
SCM cell_symbol_argv;
SCM cell_symbol_mes_datadir;
SCM cell_symbol_mes_version;
SCM cell_symbol_internal_time_units_per_second;
SCM cell_symbol_compiler;
SCM cell_symbol_arch;
SCM cell_symbol_pmatch_car;
SCM cell_symbol_pmatch_cdr;
SCM cell_type_bytes;
SCM cell_type_char;
SCM cell_type_closure;
SCM cell_type_continuation;
SCM cell_type_function;
SCM cell_type_keyword;
SCM cell_type_macro;
SCM cell_type_number;
SCM cell_type_pair;
SCM cell_type_port;
SCM cell_type_ref;
SCM cell_type_special;
SCM cell_type_string;
SCM cell_type_struct;
SCM cell_type_symbol;
SCM cell_type_values;
SCM cell_type_variable;
SCM cell_type_vector;
SCM cell_type_broken_heart;
SCM cell_symbol_program;
SCM cell_symbol_test;

// CONSTANT SYMBOL_MAX 116
#define SYMBOL_MAX 116

// CONSTANT CELL_UNSPECIFIED 7
#define CELL_UNSPECIFIED 7

// CONSTANT CELL_SYMBOL_RECORD_TYPE 84
#define CELL_SYMBOL_RECORD_TYPE 84


#endif /* __MES_SYMBOLS_H */
