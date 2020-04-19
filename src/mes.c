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

#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

SCM
alloc (long n)
{
  SCM x = g_free;
  g_free = g_free + n;
  if (g_free > ARENA_SIZE)
    assert (!"alloc: out of memory");
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
  assert (TYPE (type) == TNUMBER);
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

// SCM
// car_to_cell_ (SCM x)
// {
//   return CAR (x);
// }

// SCM
// cdr_to_cell_ (SCM x)
// {
//   return CDR (x);
// }

// SCM
// car_to_number_ (SCM x)
// {
//   return MAKE_NUMBER (CAR (x));
// }

// SCM
// cdr_to_number_ (SCM x)
// {
//   return MAKE_NUMBER (CDR (x));
// }

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
  assert (0);
  exit (1);
}

/*  extra lib */
SCM
assert_defined (SCM x, SCM e)   /*:((internal)) */
{
  if (e == cell_undefined)
    return error (cell_symbol_unbound_variable, x);
  return e;
}

SCM
check_formals (SCM f, SCM formals, SCM args)    /*:((internal)) */
{
  long flen;
  if (TYPE (formals) == TNUMBER)
    flen = VALUE (formals);
  else
    flen = length__ (formals);
  long alen = length__ (args);
  if (alen != flen && alen != -1 && flen != -1)
    {
      char *s = "apply: wrong number of arguments; expected: ";
      eputs (s);
      eputs (itoa (flen));
      eputs (", got: ");
      eputs (itoa (alen));
      eputs ("\n");
      write_error_ (f);
      SCM e = MAKE_STRING0 (s);
      return error (cell_symbol_wrong_number_of_args, cons (e, f));
    }
  return cell_unspecified;
}

SCM
check_apply (SCM f, SCM e)      /*:((internal)) */
{
  char *type = 0;
  if (f == cell_f || f == cell_t)
    type = "bool";
  if (f == cell_nil)
    type = "nil";
  if (f == cell_unspecified)
    type = "*unspecified*";
  if (f == cell_undefined)
    type = "*undefined*";
  if (TYPE (f) == TCHAR)
    type = "char";
  if (TYPE (f) == TNUMBER)
    type = "number";
  if (TYPE (f) == TSTRING)
    type = "string";
  if (TYPE (f) == TSTRUCT && builtin_p (f) == cell_f)
    type = "#<...>";
  if (TYPE (f) == TBROKEN_HEART)
    type = "<3";

  if (type != 0)
    {
      char *s = "cannot apply: ";
      eputs (s);
      eputs (type);
      eputs ("[");
      write_error_ (e);
      eputs ("]\n");
      SCM e = MAKE_STRING0 (s);
      return error (cell_symbol_wrong_type_arg, cons (e, f));
    }
  return cell_unspecified;
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
pairlis (SCM x, SCM y, SCM a)
{
  if (x == cell_nil)
    return a;
  if (TYPE (x) != TPAIR)
    return cons (cons (x, y), a);
  return cons (cons (car (x), car (y)), pairlis (cdr (x), cdr (y), a));
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
set_car_x (SCM x, SCM e)
{
  if (TYPE (x) != TPAIR)
    error (cell_symbol_not_a_pair, cons (x, cstring_to_symbol ("set-car!")));
  CAR (x) = e;
  return cell_unspecified;
}

SCM
set_cdr_x (SCM x, SCM e)
{
  if (TYPE (x) != TPAIR)
    error (cell_symbol_not_a_pair, cons (x, cstring_to_symbol ("set-cdr!")));
  CDR (x) = e;
  return cell_unspecified;
}

SCM
set_env_x (SCM x, SCM e, SCM a)
{
  SCM p;
  if (TYPE (x) == TVARIABLE)
    p = VARIABLE (x);
  else
    p = assert_defined (x, module_variable (a, x));
  if (TYPE (p) != TPAIR)
    error (cell_symbol_not_a_pair, cons (p, x));
  return set_cdr_x (p, e);
}

SCM
call_lambda (SCM e, SCM x, SCM aa, SCM a)       /*:((internal)) */
{
  SCM cl = cons (cons (cell_closure, x), x);
  r1 = e;
  r0 = cl;
  return cell_unspecified;
}

SCM
make_closure_ (SCM args, SCM body, SCM a)       /*:((internal)) */
{
  return make_cell__ (TCLOSURE, cell_f, cons (cons (cell_circular, a), cons (args, body)));
}

SCM
make_variable_ (SCM var)        /*:((internal)) */
{
  return make_cell__ (TVARIABLE, var, 0);
}

SCM
macro_get_handle (SCM name)
{
  if (TYPE (name) == TSYMBOL)
    return hashq_get_handle (g_macros, name, cell_nil);
  return cell_f;
}

SCM
get_macro (SCM name)            /*:((internal)) */
{
  SCM m = macro_get_handle (name);
  if (m != cell_f)
    return MACRO (CDR (m));
  return cell_f;
}

SCM
macro_set_x (SCM name, SCM value)       /*:((internal)) */
{
  return hashq_set_x (g_macros, name, value);
}

SCM
push_cc (SCM p1, SCM p2, SCM a, SCM c)  /*:((internal)) */
{
  SCM x = r3;
  r3 = c;
  r2 = p2;
  gc_push_frame ();
  r1 = p1;
  r0 = a;
  r3 = x;
  return cell_unspecified;
}

SCM
add_formals (SCM formals, SCM x)
{
  while (TYPE (x) == TPAIR)
    {
      formals = cons (CAR (x), formals);
      x = CDR (x);
    }
  if (TYPE (x) == TSYMBOL)
    formals = cons (x, formals);
  return formals;
}

int
formal_p (SCM x, SCM formals)   /*:((internal)) */
{
  if (TYPE (formals) == TSYMBOL)
    {
      if (x == formals)
        return x;
      else
        return cell_f;
    }
  while (TYPE (formals) == TPAIR && CAR (formals) != x)
    formals = CDR (formals);
  if (TYPE (formals) == TSYMBOL)
    return formals == x;
  return TYPE (formals) == TPAIR;
}

SCM
expand_variable_ (SCM x, SCM formals, int top_p)        /*:((internal)) */
{
  while (TYPE (x) == TPAIR)
    {
      if (TYPE (CAR (x)) == TPAIR)
        {
          if (CAAR (x) == cell_symbol_lambda)
            {
              SCM f = CAR (CDAR (x));
              formals = add_formals (formals, f);
            }
          else if (CAAR (x) == cell_symbol_define || CAAR (x) == cell_symbol_define_macro)
            {
              SCM f = CAR (CDAR (x));
              formals = add_formals (formals, f);
            }
          if (CAAR (x) != cell_symbol_quote)
            expand_variable_ (CAR (x), formals, 0);
        }
      else
        {
          if (CAR (x) == cell_symbol_lambda)
            {
              SCM f = CADR (x);
              formals = add_formals (formals, f);
              x = CDR (x);
            }
          else if (CAR (x) == cell_symbol_define || CAR (x) == cell_symbol_define_macro)
            {
              SCM f = CADR (x);
              if (top_p && TYPE (f) == TPAIR)
                f = CDR (f);
              formals = add_formals (formals, f);
              x = CDR (x);
            }
          else if (CAR (x) == cell_symbol_quote)
            return cell_unspecified;
          else if (TYPE (CAR (x)) == TSYMBOL
                   && CAR (x) != cell_symbol_boot_module
                   && CAR (x) != cell_symbol_current_module
                   && CAR (x) != cell_symbol_primitive_load && !formal_p (CAR (x), formals))
            {
              SCM v = module_variable (r0, CAR (x));
              if (v != cell_f)
                CAR (x) = make_variable_ (v);
            }
        }
      x = CDR (x);
      top_p = 0;
    }
  return cell_unspecified;
}

SCM
expand_variable (SCM x, SCM formals)    /*:((internal)) */
{
  return expand_variable_ (x, formals, 1);
}

SCM
eval_apply ()
{
  SCM aa;
  SCM args;
  SCM body;
  SCM cl;
  SCM entry;
  SCM expanders;
  SCM formals;
  SCM input;
  SCM name;
  SCM macro;
  SCM p;
  SCM program;
  SCM sc_expand;
  SCM v;
  SCM x;
  int global_p;
  int macro_p;
  int t;
  long c;

eval_apply:
  if (r3 == cell_vm_evlis2)
    goto evlis2;
  else if (r3 == cell_vm_evlis3)
    goto evlis3;
  else if (r3 == cell_vm_eval_check_func)
    goto eval_check_func;
  else if (r3 == cell_vm_eval2)
    goto eval2;
  else if (r3 == cell_vm_apply2)
    goto apply2;
  else if (r3 == cell_vm_if_expr)
    goto if_expr;
  else if (r3 == cell_vm_begin_eval)
    goto begin_eval;
  else if (r3 == cell_vm_eval_set_x)
    goto eval_set_x;
  else if (r3 == cell_vm_macro_expand_car)
    goto macro_expand_car;
  else if (r3 == cell_vm_return)
    goto vm_return;
  else if (r3 == cell_vm_macro_expand_cdr)
    goto macro_expand_cdr;
  else if (r3 == cell_vm_eval_define)
    goto eval_define;
  else if (r3 == cell_vm_macro_expand)
    goto macro_expand;
  else if (r3 == cell_vm_macro_expand_lambda)
    goto macro_expand_lambda;
  else if (r3 == cell_vm_eval_pmatch_car)
    goto eval_pmatch_car;
  else if (r3 == cell_vm_begin_expand_macro)
    goto begin_expand_macro;
  else if (r3 == cell_vm_macro_expand_define)
    goto macro_expand_define;
  else if (r3 == cell_vm_begin_expand_eval)
    goto begin_expand_eval;
  else if (r3 == cell_vm_call_with_current_continuation2)
    goto call_with_current_continuation2;
  else if (r3 == cell_vm_macro_expand_set_x)
    goto macro_expand_set_x;
  else if (r3 == cell_vm_eval_pmatch_cdr)
    goto eval_pmatch_cdr;
  else if (r3 == cell_vm_macro_expand_define_macro)
    goto macro_expand_define_macro;
  else if (r3 == cell_vm_begin_primitive_load)
    goto begin_primitive_load;

  else if (r3 == cell_vm_evlis)
    goto evlis;
  else if (r3 == cell_vm_apply)
    goto apply;
  else if (r3 == cell_vm_eval)
    goto eval;
  else if (r3 == cell_vm_eval_macro_expand_eval)
    goto eval_macro_expand_eval;
  else if (r3 == cell_vm_eval_macro_expand_expand)
    goto eval_macro_expand_expand;
  else if (r3 == cell_vm_begin)
    goto begin;
  else if (r3 == cell_vm_begin_expand)
    goto begin_expand;
  else if (r3 == cell_vm_begin_expand_primitive_load)
    goto begin_expand_primitive_load;
  else if (r3 == cell_vm_if)
    goto vm_if;
  else if (r3 == cell_vm_call_with_values2)
    goto call_with_values2;
  else if (r3 == cell_unspecified)
    return r1;
  else
    error (cell_symbol_system_error, MAKE_STRING0 ("eval/apply unknown continuation"));

evlis:
  if (r1 == cell_nil)
    goto vm_return;
  if (TYPE (r1) != TPAIR)
    goto eval;
  push_cc (CAR (r1), r1, r0, cell_vm_evlis2);
  goto eval;
evlis2:
  push_cc (CDR (r2), r1, r0, cell_vm_evlis3);
  goto evlis;
evlis3:
  r1 = cons (r2, r1);
  goto vm_return;

apply:
  g_stack_array[g_stack + FRAME_PROCEDURE] = CAR (r1);
  t = TYPE (CAR (r1));
  if (t == TSTRUCT && builtin_p (CAR (r1)) == cell_t)
    {
      check_formals (CAR (r1), builtin_arity (CAR (r1)), CDR (r1));
      r1 = apply_builtin (CAR (r1), CDR (r1));
      goto vm_return;
    }
  else if (t == TCLOSURE)
    {
      cl = CLOSURE (CAR (r1));
      body = CDDR (cl);
      formals = CADR (cl);
      args = CDR (r1);
      aa = CDAR (cl);
      aa = CDR (aa);
      check_formals (CAR (r1), formals, CDR (r1));
      p = pairlis (formals, args, aa);
      call_lambda (body, p, aa, r0);
      goto begin;
    }
  else if (t == TCONTINUATION)
    {
      v = CONTINUATION (CAR (r1));
      if (LENGTH (v) != 0)
        {
          for (t = 0; t < LENGTH (v); t = t + 1)
            g_stack_array[STACK_SIZE - LENGTH (v) + t] = vector_ref_ (v, t);
          g_stack = STACK_SIZE - LENGTH (v);
        }
      x = r1;
      gc_pop_frame ();
      r1 = CADR (x);
      goto eval_apply;
    }
  else if (t == TSPECIAL)
    {
      c = CAR (r1);
      if (c == cell_vm_apply)
        {
          push_cc (cons (CADR (r1), CADDR (r1)), r1, r0, cell_vm_return);
          goto apply;
        }
      else if (c == cell_vm_eval)
        {
          push_cc (CADR (r1), r1, CADDR (r1), cell_vm_return);
          goto eval;
        }
      else if (c == cell_vm_begin_expand)
        {
          push_cc (cons (CADR (r1), cell_nil), r1, CADDR (r1), cell_vm_return);
          goto begin_expand;
        }
      else if (c == cell_call_with_current_continuation)
        {
          r1 = CDR (r1);
          goto call_with_current_continuation;
        }
      else
        check_apply (cell_f, CAR (r1));
    }
  else if (t == TSYMBOL)
    {
      if (CAR (r1) == cell_symbol_call_with_values)
        {
          r1 = CDR (r1);
          goto call_with_values;
        }
      if (CAR (r1) == cell_symbol_current_module)
        {
          r1 = r0;
          goto vm_return;
        }
      if (CAR (r1) == cell_symbol_boot_module)
        {
          r1 = m0;
          goto vm_return;
        }
    }
  else if (t == TPAIR)
    {
      if (CAAR (r1) == cell_symbol_lambda)
        {
          formals = CADR (CAR (r1));
          args = CDR (r1);
          body = CDDR (CAR (r1));
          p = pairlis (formals, CDR (r1), r0);
          check_formals (r1, formals, args);
          call_lambda (body, p, p, r0);
          goto begin;
        }
    }
  // write_error_ (CAR (r1));
  // eputs ("\n");
  push_cc (CAR (r1), r1, r0, cell_vm_apply2);
  goto eval;
apply2:
  check_apply (r1, CAR (r2));
  r1 = cons (r1, CDR (r2));
  goto apply;

eval:
  t = TYPE (r1);
  if (t == TPAIR)
    {
      c = CAR (r1);
      if (c == cell_symbol_pmatch_car)
        {
          push_cc (CADR (r1), r1, r0, cell_vm_eval_pmatch_car);
          goto eval;
        eval_pmatch_car:
          x = r1;
          gc_pop_frame ();
          r1 = CAR (x);
          goto eval_apply;
        }
      else if (c == cell_symbol_pmatch_cdr)
        {
          push_cc (CADR (r1), r1, r0, cell_vm_eval_pmatch_cdr);
          goto eval;
        eval_pmatch_cdr:
          x = r1;
          gc_pop_frame ();
          r1 = CDR (x);
          goto eval_apply;
        }
      else if (c == cell_symbol_quote)
        {
          x = r1;
          gc_pop_frame ();
          r1 = CADR (x);
          goto eval_apply;
        }
      else if (c == cell_symbol_begin)
        goto begin;
      else if (c == cell_symbol_lambda)
        {
          r1 = make_closure_ (CADR (r1), CDDR (r1), r0);
          goto vm_return;
        }
      else if (c == cell_symbol_if)
        {
          r1 = CDR (r1);
          goto vm_if;
        }
      else if (c == cell_symbol_set_x)
        {
          push_cc (CAR (CDDR (r1)), r1, r0, cell_vm_eval_set_x);
          goto eval;
        eval_set_x:
          r1 = set_env_x (CADR (r2), r1, r0);
          goto vm_return;
        }
      else if (c == cell_vm_macro_expand)
        {
          push_cc (CADR (r1), r1, r0, cell_vm_eval_macro_expand_eval);
          goto eval;
        eval_macro_expand_eval:
          push_cc (r1, r2, r0, cell_vm_eval_macro_expand_expand);
          goto macro_expand;
        eval_macro_expand_expand:
          goto vm_return;
        }
      else
        {
          if (TYPE (r1) == TPAIR && (CAR (r1) == cell_symbol_define || CAR (r1) == cell_symbol_define_macro))
            {
              global_p = CAAR (r0) != cell_closure;
              macro_p = CAR (r1) == cell_symbol_define_macro;
              if (global_p != 0)
                {
                  name = CADR (r1);
                  if (TYPE (CADR (r1)) == TPAIR)
                    name = CAR (name);
                  if (macro_p != 0)
                    {
                      entry = assq (name, g_macros);
                      if (entry == cell_f)
                        macro_set_x (name, cell_f);
                    }
                  else
                    {
                      entry = module_variable (r0, name);
                      if (entry == cell_f)
                        module_define_x (m0, name, cell_f);
                    }
                }
              r2 = r1;
              if (TYPE (CADR (r1)) != TPAIR)
                {
                  push_cc (CAR (CDDR (r1)), r2, cons (cons (CADR (r1), CADR (r1)), r0), cell_vm_eval_define);
                  goto eval;
                }
              else
                {
                  p = pairlis (CADR (r1), CADR (r1), r0);
                  formals = CDR (CADR (r1));
                  body = CDDR (r1);

                  if (macro_p || global_p)
                    expand_variable (body, formals);
                  r1 = cons (cell_symbol_lambda, cons (formals, body));
                  push_cc (r1, r2, p, cell_vm_eval_define);
                  goto eval;
                }
            eval_define:
              name = CADR (r2);
              if (TYPE (CADR (r2)) == TPAIR)
                name = CAR (name);
              if (macro_p != 0)
                {
                  entry = macro_get_handle (name);
                  r1 = MAKE_MACRO (name, r1);
                  set_cdr_x (entry, r1);
                }
              else if (global_p != 0)
                {
                  entry = module_variable (r0, name);
                  set_cdr_x (entry, r1);
                }
              else
                {
                  entry = cons (name, r1);
                  aa = cons (entry, cell_nil);
                  set_cdr_x (aa, cdr (r0));
                  set_cdr_x (r0, aa);
                  cl = module_variable (r0, cell_closure);
                  set_cdr_x (cl, aa);
                }
              r1 = cell_unspecified;
              goto vm_return;
            }
          push_cc (CAR (r1), r1, r0, cell_vm_eval_check_func);
          gc_check ();
          goto eval;
        eval_check_func:
          push_cc (CDR (r2), r2, r0, cell_vm_eval2);
          goto evlis;
        eval2:
          r1 = cons (CAR (r2), r1);
          goto apply;
        }
    }
  else if (t == TSYMBOL)
    {
      if (r1 == cell_symbol_boot_module)
        goto vm_return;
      if (r1 == cell_symbol_current_module)
        goto vm_return;
      if (r1 == cell_symbol_begin)      // FIXME
        {
          r1 = cell_begin;
          goto vm_return;
        }
      r1 = assert_defined (r1, module_ref (r0, r1));
      goto vm_return;
    }
  else if (t == TVARIABLE)
    {
      r1 = CDR (VARIABLE (r1));
      goto vm_return;
    }
  else if (t == TBROKEN_HEART)
    error (cell_symbol_system_error, r1);
  else
    goto vm_return;

macro_expand:
  {
    macro;
    expanders;

    if (TYPE (r1) != TPAIR || CAR (r1) == cell_symbol_quote)
      goto vm_return;

    if (CAR (r1) == cell_symbol_lambda)
      {
        push_cc (CDDR (r1), r1, r0, cell_vm_macro_expand_lambda);
        goto macro_expand;
      macro_expand_lambda:
        CDDR (r2) = r1;
        r1 = r2;
        goto vm_return;
      }

    if (TYPE (r1) == TPAIR && (macro = get_macro (CAR (r1))) != cell_f)
      {
        r1 = cons (macro, CDR (r1));
        push_cc (r1, cell_nil, r0, cell_vm_macro_expand);
        goto apply;
      }

    if (CAR (r1) == cell_symbol_define || CAR (r1) == cell_symbol_define_macro)
      {
        push_cc (CDDR (r1), r1, r0, cell_vm_macro_expand_define);
        goto macro_expand;
      macro_expand_define:
        CDDR (r2) = r1;
        r1 = r2;
        if (CAR (r1) == cell_symbol_define_macro)
          {
            push_cc (r1, r1, r0, cell_vm_macro_expand_define_macro);
            goto eval;
          macro_expand_define_macro:
            r1 = r2;
          }
        goto vm_return;
      }

    if (CAR (r1) == cell_symbol_set_x)
      {
        push_cc (CDDR (r1), r1, r0, cell_vm_macro_expand_set_x);
        goto macro_expand;
      macro_expand_set_x:
        CDDR (r2) = r1;
        r1 = r2;
        goto vm_return;
      }

    if (TYPE (r1) == TPAIR
        && TYPE (CAR (r1)) == TSYMBOL
        && CAR (r1) != cell_symbol_begin
        && ((macro = macro_get_handle (cell_symbol_portable_macro_expand)) != cell_f)
        && ((expanders = module_ref (r0, cell_symbol_sc_expander_alist)) != cell_undefined)
        && ((macro = assq (CAR (r1), expanders)) != cell_f))
      {
        sc_expand = module_ref (r0, cell_symbol_macro_expand);
        r2 = r1;
        if (sc_expand != cell_undefined && sc_expand != cell_f)
          {
            r1 = cons (sc_expand, cons (r1, cell_nil));
            goto apply;
          }
      }

    push_cc (CAR (r1), r1, r0, cell_vm_macro_expand_car);
    goto macro_expand;

  macro_expand_car:
    CAR (r2) = r1;
    r1 = r2;
    if (CDR (r1) == cell_nil)
      goto vm_return;

    push_cc (CDR (r1), r1, r0, cell_vm_macro_expand_cdr);
    goto macro_expand;

  macro_expand_cdr:
    CDR (r2) = r1;
    r1 = r2;

    goto vm_return;
  }

begin:
  x = cell_unspecified;
  while (r1 != cell_nil)
    {
      gc_check ();
      if (TYPE (r1) == TPAIR)
        {
          if (CAAR (r1) == cell_symbol_primitive_load)
            {
              program = cons (CAR (r1), cell_nil);
              push_cc (program, r1, r0, cell_vm_begin_primitive_load);
              goto begin_expand;
            begin_primitive_load:
              CAR (r2) = r1;
              r1 = r2;
            }
        }

      if (TYPE (r1) == TPAIR && TYPE (CAR (r1)) == TPAIR)
        {
          if (CAAR (r1) == cell_symbol_begin)
            r1 = append2 (CDAR (r1), CDR (r1));
        }
      if (CDR (r1) == cell_nil)
        {
          r1 = CAR (r1);
          goto eval;
        }
      push_cc (CAR (r1), r1, r0, cell_vm_begin_eval);
      goto eval;
    begin_eval:
      x = r1;
      r1 = CDR (r2);
    }
  r1 = x;
  goto vm_return;


begin_expand:
  x = cell_unspecified;
  while (r1 != cell_nil)
    {
      gc_check ();

      if (TYPE (r1) == TPAIR)
        {
          if (TYPE (CAR (r1)) == TPAIR && CAAR (r1) == cell_symbol_begin)
            r1 = append2 (CDAR (r1), CDR (r1));
          if (CAAR (r1) == cell_symbol_primitive_load)
            {
              push_cc (CADR (CAR (r1)), r1, r0, cell_vm_begin_expand_primitive_load);
              goto eval;        // FIXME: expand too?!
            begin_expand_primitive_load:
              if (TYPE (r1) == TNUMBER && VALUE (r1) == 0)
                0;
              else if (TYPE (r1) == TSTRING)
                input = set_current_input_port (open_input_file (r1));
              else if (TYPE (r1) == TPORT)
                input = set_current_input_port (r1);
              else
                assert (0);

              push_cc (input, r2, r0, cell_vm_return);
              x = read_input_file_env (r0);
              if (g_debug > 5)
                module_printer (m0);
              gc_pop_frame ();
              input = r1;
              r1 = x;
              set_current_input_port (input);
              r1 = cons (cell_symbol_begin, r1);
              CAR (r2) = r1;
              r1 = r2;
              continue;
            }
        }

      push_cc (CAR (r1), r1, r0, cell_vm_begin_expand_macro);
      goto macro_expand;
    begin_expand_macro:
      if (r1 != CAR (r2))
        {
          CAR (r2) = r1;
          r1 = r2;
          continue;
        }
      r1 = r2;
      expand_variable (CAR (r1), cell_nil);
      push_cc (CAR (r1), r1, r0, cell_vm_begin_expand_eval);
      goto eval;
    begin_expand_eval:
      x = r1;
      r1 = CDR (r2);
    }
  r1 = x;
  goto vm_return;

vm_if:
  push_cc (CAR (r1), r1, r0, cell_vm_if_expr);
  goto eval;
if_expr:
  x = r1;
  r1 = r2;
  if (x != cell_f)
    {
      r1 = CADR (r1);
      goto eval;
    }
  if (CDDR (r1) != cell_nil)
    {
      r1 = CAR (CDDR (r1));
      goto eval;
    }
  r1 = cell_unspecified;
  goto vm_return;

call_with_current_continuation:
  gc_push_frame ();
  x = MAKE_CONTINUATION (g_continuations);
  g_continuations = g_continuations + 1;
  v = make_vector__ (STACK_SIZE - g_stack);
  for (t = g_stack; t < STACK_SIZE; t = t + 1)
    vector_set_x_ (v, t - g_stack, g_stack_array[t]);
  CONTINUATION (x) = v;
  gc_pop_frame ();
  push_cc (cons (CAR (r1), cons (x, cell_nil)), x, r0, cell_vm_call_with_current_continuation2);
  goto apply;
call_with_current_continuation2:
  v = make_vector__ (STACK_SIZE - g_stack);
  for (t = g_stack; t < STACK_SIZE; t = t + 1)
    vector_set_x_ (v, t - g_stack, g_stack_array[t]);
  CONTINUATION (r2) = v;
  goto vm_return;

call_with_values:
  push_cc (cons (CAR (r1), cell_nil), r1, r0, cell_vm_call_with_values2);
  goto apply;
call_with_values2:
  if (TYPE (r1) == TVALUES)
    r1 = CDR (r1);
  r1 = cons (CADR (r2), r1);
  goto apply;

vm_return:
  x = r1;
  gc_pop_frame ();
  r1 = x;
  goto eval_apply;
}

SCM
apply (SCM f, SCM x, SCM a)     /*:((internal)) */
{
  push_cc (cons (f, x), cell_unspecified, r0, cell_unspecified);
  r3 = cell_vm_apply;
  return eval_apply ();
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

SCM
apply_builtin (SCM fn, SCM x)   /*:((internal)) */
{
  int arity = VALUE (builtin_arity (fn));
  if ((arity > 0 || arity == -1) && x != cell_nil && TYPE (CAR (x)) == TVALUES)
    x = cons (CADAR (x), CDR (x));
  if ((arity > 1 || arity == -1) && x != cell_nil && TYPE (CDR (x)) == TPAIR && TYPE (CADR (x)) == TVALUES)
    x = cons (CAR (x), cons (CDADAR (x), CDR (x)));

#if __M2_PLANET__
  FUNCTION fp = builtin_function (fn);
  if (arity == 0)
    return fp ();
  else if (arity == 1)
    return fp (CAR (x));
  else if (arity == 2)
    return fp (CAR (x), CADR (x));
  else if (arity == 3)
    return fp (CAR (x), CADR (x), CAR (CDDR (x)));
  else if (arity == -1)
    return fp (x);
#else // !__M2_PLANET__
  if (arity == 0)
    {
      SCM (*fp) (void) = (function0_t) builtin_function (fn);
      return fp ();
    }
  else if (arity == 1)
    {
      SCM (*fp) (SCM) = (function1_t) builtin_function (fn);
      return fp (CAR (x));
    }
  else if (arity == 2)
    {
      SCM (*fp) (SCM, SCM) = (function2_t) builtin_function (fn);
      return fp (CAR (x), CADR (x));
    }
  else if (arity == 3)
    {
      SCM (*fp) (SCM, SCM, SCM) = (function3_t) builtin_function (fn);
      return fp (CAR (x), CADR (x), CAR (CDDR (x)));
    }
  else if (arity == -1)
    {
      SCM (*fp) (SCM) = (function1_t) builtin_function (fn);
      return fp (x);
    }
#endif //! __M2_PLANET__
  return cell_unspecified;
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
