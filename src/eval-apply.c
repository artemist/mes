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
      SCM e = make_string0 (s);
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
      SCM e = make_string0 (s);
      return error (cell_symbol_wrong_type_arg, cons (e, f));
    }
  return cell_unspecified;
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
  R1 = e;
  R0 = cl;
  return cell_unspecified;
}

SCM
make_closure_ (SCM args, SCM body, SCM a)       /*:((internal)) */
{
  return make_cell (TCLOSURE, cell_f, cons (cons (cell_circular, a), cons (args, body)));
}

SCM
make_variable_ (SCM var)        /*:((internal)) */
{
  return make_cell (TVARIABLE, var, 0);
}

SCM
macro_get_handle (SCM name)     /*:((internal)) */
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
    {
      SCM d = CDR (m);
      return MACRO (d);
    }
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
  SCM x = R3;
  R3 = c;
  R2 = p2;
  gc_push_frame ();
  R1 = p1;
  R0 = a;
  R3 = x;
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
        return 1;
      else
        return 0;
    }
  while (TYPE (formals) == TPAIR)
    {
      if (CAR (formals) == x)
        break;
      formals = CDR (formals);
    }
  if (TYPE (formals) == TSYMBOL)
    return formals == x;
  return TYPE (formals) == TPAIR;
}

SCM
expand_variable_ (SCM x, SCM formals, int top_p)        /*:((internal)) */
{
  while (TYPE (x) == TPAIR)
    {
      SCM a = CAR (x);
      if (TYPE (a) == TPAIR)
        {
          if (CAR (a) == cell_symbol_lambda)
            {
              SCM f = CADR (a);
              formals = add_formals (formals, f);
            }
          else if (CAR (a) == cell_symbol_define || CAR (a) == cell_symbol_define_macro)
            {
              SCM f = CADR (a);
              formals = add_formals (formals, f);
            }
          if (CAR (a) != cell_symbol_quote)
            expand_variable_ (a, formals, 0);
        }
      else
        {
          if (a == cell_symbol_lambda)
            {
              SCM f = CADR (x);
              formals = add_formals (formals, f);
              x = CDR (x);
            }
          else if (a == cell_symbol_define || a == cell_symbol_define_macro)
            {
              SCM f = CADR (x);
              if (top_p != 0 && TYPE (f) == TPAIR)
                f = CDR (f);
              formals = add_formals (formals, f);
              x = CDR (x);
            }
          else if (a == cell_symbol_quote)
            return cell_unspecified;
          else if (TYPE (a) == TSYMBOL
                   && a != cell_symbol_boot_module
                   && a != cell_symbol_current_module
                   && a != cell_symbol_primitive_load
                   && formal_p (CAR (x), formals) == 0)
            {
              SCM v = module_variable (R0, a);
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
apply_builtin (SCM fn, SCM x)   /*:((internal)) */
{
  SCM a = builtin_arity (fn);
  int arity = VALUE (a);
  if ((arity > 0 || arity == -1) && x != cell_nil)
    {
      SCM a = CAR (x);
      if (TYPE (a) == TVALUES)
        x = cons (CADR (a), CDR (x));
    }
  if ((arity > 1 || arity == -1) && x != cell_nil)
    {
      SCM a = CAR (x);
      SCM d = CDR (x);
      if (TYPE (d) == TPAIR)
        if (TYPE (CAR (d)) == TVALUES)
          x = cons (a, cons (CADAR (d), d));
    }

#if __M2_PLANET__
  FUNCTION fp = builtin_function (fn);
  if (arity == 0)
    return fp ();
  else if (arity == 1)
    return fp (CAR (x));
  else if (arity == 2)
    return fp (CAR (x), CADR (x));
  else if (arity == 3)
    return fp (CAR (x), CADR (x), CADDR (x));
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
  SCM a;
  SCM c;
  SCM d;
  int t;
  long i;

eval_apply:
  if (R3 == cell_vm_evlis2)
    goto evlis2;
  else if (R3 == cell_vm_evlis3)
    goto evlis3;
  else if (R3 == cell_vm_eval_check_func)
    goto eval_check_func;
  else if (R3 == cell_vm_eval2)
    goto eval2;
  else if (R3 == cell_vm_apply2)
    goto apply2;
  else if (R3 == cell_vm_if_expr)
    goto if_expr;
  else if (R3 == cell_vm_begin_eval)
    goto begin_eval;
  else if (R3 == cell_vm_eval_set_x)
    goto eval_set_x;
  else if (R3 == cell_vm_macro_expand_car)
    goto macro_expand_car;
  else if (R3 == cell_vm_return)
    goto vm_return;
  else if (R3 == cell_vm_macro_expand_cdr)
    goto macro_expand_cdr;
  else if (R3 == cell_vm_eval_define)
    goto eval_define;
  else if (R3 == cell_vm_macro_expand)
    goto macro_expand;
  else if (R3 == cell_vm_macro_expand_lambda)
    goto macro_expand_lambda;
  else if (R3 == cell_vm_eval_pmatch_car)
    goto eval_pmatch_car;
  else if (R3 == cell_vm_begin_expand_macro)
    goto begin_expand_macro;
  else if (R3 == cell_vm_macro_expand_define)
    goto macro_expand_define;
  else if (R3 == cell_vm_begin_expand_eval)
    goto begin_expand_eval;
  else if (R3 == cell_vm_call_with_current_continuation2)
    goto call_with_current_continuation2;
  else if (R3 == cell_vm_macro_expand_set_x)
    goto macro_expand_set_x;
  else if (R3 == cell_vm_eval_pmatch_cdr)
    goto eval_pmatch_cdr;
  else if (R3 == cell_vm_macro_expand_define_macro)
    goto macro_expand_define_macro;
  else if (R3 == cell_vm_begin_primitive_load)
    goto begin_primitive_load;

  else if (R3 == cell_vm_evlis)
    goto evlis;
  else if (R3 == cell_vm_apply)
    goto apply;
  else if (R3 == cell_vm_eval)
    goto eval;
  else if (R3 == cell_vm_eval_macro_expand_eval)
    goto eval_macro_expand_eval;
  else if (R3 == cell_vm_eval_macro_expand_expand)
    goto eval_macro_expand_expand;
  else if (R3 == cell_vm_begin)
    goto begin;
  else if (R3 == cell_vm_begin_expand)
    goto begin_expand;
  else if (R3 == cell_vm_begin_expand_primitive_load)
    goto begin_expand_primitive_load;
  else if (R3 == cell_vm_if)
    goto vm_if;
  else if (R3 == cell_vm_call_with_values2)
    goto call_with_values2;
  else if (R3 == cell_unspecified)
    return R1;
  else
    assert_msg (0, "eval/apply unknown continuation");

evlis:
  if (R1 == cell_nil)
    goto vm_return;
  if (TYPE (R1) != TPAIR)
    goto eval;
  push_cc (CAR (R1), R1, R0, cell_vm_evlis2);
  goto eval;
evlis2:
  push_cc (CDR (R2), R1, R0, cell_vm_evlis3);
  goto evlis;
evlis3:
  R1 = cons (R2, R1);
  goto vm_return;

apply:
  g_stack_array[g_stack + FRAME_PROCEDURE] = CAR (R1);
  a = CAR (R1);
  t = TYPE (a);
  if (t == TSTRUCT && builtin_p (CAR (R1)) == cell_t)
    {
      check_formals (CAR (R1), builtin_arity (CAR (R1)), CDR (R1));
      R1 = apply_builtin (CAR (R1), CDR (R1));
      goto vm_return;
    }
  else if (t == TCLOSURE)
    {
      cl = CLOSURE (CAR (R1));
      body = CDDR (cl);
      formals = CADR (cl);
      args = CDR (R1);
      aa = CDAR (cl);
      aa = CDR (aa);
      check_formals (CAR (R1), formals, CDR (R1));
      p = pairlis (formals, args, aa);
      call_lambda (body, p, aa, R0);
      goto begin;
    }
  else if (t == TCONTINUATION)
    {
      a = CAR (R1);
      v = CONTINUATION (a);
      if (LENGTH (v) != 0)
        {
          for (i = 0; i < LENGTH (v); i = i + 1)
            g_stack_array[STACK_SIZE - LENGTH (v) + i] = vector_ref_ (v, i);
          g_stack = STACK_SIZE - LENGTH (v);
        }
      x = R1;
      gc_pop_frame ();
      R1 = CADR (x);
      goto eval_apply;
    }
  else if (t == TSPECIAL)
    {
      c = CAR (R1);
      if (c == cell_vm_apply)
        {
          push_cc (cons (CADR (R1), CADDR (R1)), R1, R0, cell_vm_return);
          goto apply;
        }
      else if (c == cell_vm_eval)
        {
          push_cc (CADR (R1), R1, CADDR (R1), cell_vm_return);
          goto eval;
        }
      else if (c == cell_vm_begin_expand)
        {
          push_cc (cons (CADR (R1), cell_nil), R1, CADDR (R1), cell_vm_return);
          goto begin_expand;
        }
      else
        check_apply (cell_f, CAR (R1));
    }
  else if (t == TSYMBOL)
    {
      c = CAR (R1);
      if (c == cell_symbol_call_with_current_continuation)
        {
          R1 = CDR (R1);
          goto call_with_current_continuation;
        }
      if (c == cell_symbol_call_with_values)
        {
          R1 = CDR (R1);
          goto call_with_values;
        }
      if (c == cell_symbol_current_module)
        {
          R1 = R0;
          goto vm_return;
        }
      if (c == cell_symbol_boot_module)
        {
          R1 = M0;
          goto vm_return;
        }
    }
  else if (t == TPAIR)
    {
      if (CAAR (R1) == cell_symbol_lambda)
        {
          formals = CADAR (R1);
          args = CDR (R1);
          body = CDDAR (R1);
          p = pairlis (formals, CDR (R1), R0);
          check_formals (R1, formals, args);
          call_lambda (body, p, p, R0);
          goto begin;
        }
    }
  push_cc (CAR (R1), R1, R0, cell_vm_apply2);
  goto eval;
apply2:
  check_apply (R1, CAR (R2));
  R1 = cons (R1, CDR (R2));
  goto apply;

eval:
  t = TYPE (R1);
  if (t == TPAIR)
    {
      c = CAR (R1);
      if (c == cell_symbol_pmatch_car)
        {
          push_cc (CADR (R1), R1, R0, cell_vm_eval_pmatch_car);
          goto eval;
        eval_pmatch_car:
          x = R1;
          gc_pop_frame ();
          R1 = CAR (x);
          goto eval_apply;
        }
      else if (c == cell_symbol_pmatch_cdr)
        {
          push_cc (CADR (R1), R1, R0, cell_vm_eval_pmatch_cdr);
          goto eval;
        eval_pmatch_cdr:
          x = R1;
          gc_pop_frame ();
          R1 = CDR (x);
          goto eval_apply;
        }
      else if (c == cell_symbol_quote)
        {
          x = R1;
          gc_pop_frame ();
          R1 = CADR (x);
          goto eval_apply;
        }
      else if (c == cell_symbol_begin)
        goto begin;
      else if (c == cell_symbol_lambda)
        {
          R1 = make_closure_ (CADR (R1), CDDR (R1), R0);
          goto vm_return;
        }
      else if (c == cell_symbol_if)
        {
          R1 = CDR (R1);
          goto vm_if;
        }
      else if (c == cell_symbol_set_x)
        {
          push_cc (CADDR (R1), R1, R0, cell_vm_eval_set_x);
          goto eval;
        eval_set_x:
          R1 = set_env_x (CADR (R2), R1, R0);
          goto vm_return;
        }
      else if (c == cell_vm_macro_expand)
        {
          push_cc (CADR (R1), R1, R0, cell_vm_eval_macro_expand_eval);
          goto eval;
        eval_macro_expand_eval:
          push_cc (R1, R2, R0, cell_vm_eval_macro_expand_expand);
          goto macro_expand;
        eval_macro_expand_expand:
          goto vm_return;
        }
      else
        {
          if (TYPE (R1) == TPAIR)
            if (CAR (R1) == cell_symbol_define || CAR (R1) == cell_symbol_define_macro)
              {
                global_p = 0;
                if (CAAR (R0) != cell_closure)
                  global_p = 1;
                macro_p = 0;
                if (CAR (R1) == cell_symbol_define_macro)
                  macro_p = 1;
                if (global_p != 0)
                  {
                    name = CADR (R1);
                    aa = CADR (R1);
                    if (TYPE (aa) == TPAIR)
                      name = CAR (name);
                    if (macro_p != 0)
                      {
                        entry = assq (name, g_macros);
                        if (entry == cell_f)
                          macro_set_x (name, cell_f);
                      }
                    else
                      {
                        entry = module_variable (R0, name);
                        if (entry == cell_f)
                          module_define_x (M0, name, cell_f);
                      }
                  }
                R2 = R1;
                aa = CADR (R1);
                if (TYPE (aa) != TPAIR)
                  {
                    push_cc (CADDR (R1), R2, cons (cons (CADR (R1), CADR (R1)), R0), cell_vm_eval_define);
                    goto eval;
                  }
                else
                  {
                    p = pairlis (CADR (R1), CADR (R1), R0);
                    formals = CDADR (R1);
                    body = CDDR (R1);

                    if (macro_p != 0 || global_p != 0)
                      expand_variable (body, formals);
                    R1 = cons (cell_symbol_lambda, cons (formals, body));
                    push_cc (R1, R2, p, cell_vm_eval_define);
                    goto eval;
                  }
              eval_define:
                name = CADR (R2);
                aa = CADR (R2);
                if (TYPE (aa) == TPAIR)
                  name = CAR (name);
                if (macro_p != 0)
                  {
                    entry = macro_get_handle (name);
                    R1 = make_macro (name, R1);
                    set_cdr_x (entry, R1);
                  }
                else if (global_p != 0)
                  {
                    entry = module_variable (R0, name);
                    set_cdr_x (entry, R1);
                  }
                else
                  {
                    entry = cons (name, R1);
                    aa = cons (entry, cell_nil);
                    set_cdr_x (aa, cdr (R0));
                    set_cdr_x (R0, aa);
                    cl = module_variable (R0, cell_closure);
                    set_cdr_x (cl, aa);
                  }
                R1 = cell_unspecified;
                goto vm_return;
              }
          push_cc (CAR (R1), R1, R0, cell_vm_eval_check_func);
          gc_check ();
          goto eval;
        eval_check_func:
          push_cc (CDR (R2), R2, R0, cell_vm_eval2);
          goto evlis;
        eval2:
          R1 = cons (CAR (R2), R1);
          goto apply;
        }
    }
  else if (t == TSYMBOL)
    {
      if (R1 == cell_symbol_boot_module)
        goto vm_return;
      if (R1 == cell_symbol_current_module)
        goto vm_return;
      if (R1 == cell_symbol_begin)
        goto vm_return;
      if (R1 == cell_symbol_call_with_current_continuation)
        goto vm_return;
      R1 = assert_defined (R1, module_ref (R0, R1));
      goto vm_return;
    }
  else if (t == TVARIABLE)
    {
      x = VARIABLE (R1);
      R1 = CDR (x);
      goto vm_return;
    }
  else if (t == TBROKEN_HEART)
    error (cell_symbol_system_error, R1);
  else
    goto vm_return;

macro_expand:
  if (TYPE (R1) != TPAIR || CAR (R1) == cell_symbol_quote)
    goto vm_return;

  if (CAR (R1) == cell_symbol_lambda)
    {
      push_cc (CDDR (R1), R1, R0, cell_vm_macro_expand_lambda);
      goto macro_expand;
    macro_expand_lambda:
      CDDR (R2) = R1;
      R1 = R2;
      goto vm_return;
    }

  if (TYPE (R1) == TPAIR)
    {
      macro = get_macro (CAR (R1));
      if (macro != cell_f)
        {
          R1 = cons (macro, CDR (R1));
          push_cc (R1, cell_nil, R0, cell_vm_macro_expand);
          goto apply;
        }
    }

  if (CAR (R1) == cell_symbol_define || CAR (R1) == cell_symbol_define_macro)
    {
      push_cc (CDDR (R1), R1, R0, cell_vm_macro_expand_define);
      goto macro_expand;
    macro_expand_define:
      CDDR (R2) = R1;
      R1 = R2;
      if (CAR (R1) == cell_symbol_define_macro)
        {
          push_cc (R1, R1, R0, cell_vm_macro_expand_define_macro);
          goto eval;
        macro_expand_define_macro:
          R1 = R2;
        }
      goto vm_return;
    }

  if (CAR (R1) == cell_symbol_set_x)
    {
      push_cc (CDDR (R1), R1, R0, cell_vm_macro_expand_set_x);
      goto macro_expand;
    macro_expand_set_x:
      CDDR (R2) = R1;
      R1 = R2;
      goto vm_return;
    }

  if (TYPE (R1) == TPAIR)
    {
      a = CAR (R1);
      if (TYPE (a) == TSYMBOL && a != cell_symbol_begin)
        {
          macro = macro_get_handle (cell_symbol_portable_macro_expand);
          if (macro != cell_f)
            {
              expanders = module_ref (R0, cell_symbol_sc_expander_alist);
              if (expanders != cell_undefined)
                {
                  macro = assq (CAR (R1), expanders);
                  if (macro != cell_f)
                    {
                      sc_expand = module_ref (R0, cell_symbol_macro_expand);
                      R2 = R1;
                      if (sc_expand != cell_undefined && sc_expand != cell_f)
                        {
                          R1 = cons (sc_expand, cons (R1, cell_nil));
                          goto apply;
                        }
                    }
                }
            }
        }
    }

  push_cc (CAR (R1), R1, R0, cell_vm_macro_expand_car);
  goto macro_expand;

macro_expand_car:
  CAR (R2) = R1;
  R1 = R2;
  if (CDR (R1) == cell_nil)
    goto vm_return;

  push_cc (CDR (R1), R1, R0, cell_vm_macro_expand_cdr);
  goto macro_expand;

macro_expand_cdr:
  CDR (R2) = R1;
  R1 = R2;

  goto vm_return;

begin:
  x = cell_unspecified;
  while (R1 != cell_nil)
    {
      gc_check ();
      if (TYPE (R1) == TPAIR)
        {
          if (CAAR (R1) == cell_symbol_primitive_load)
            {
              program = cons (CAR (R1), cell_nil);
              push_cc (program, R1, R0, cell_vm_begin_primitive_load);
              goto begin_expand;
            begin_primitive_load:
              CAR (R2) = R1;
              R1 = R2;
            }
        }

      if (TYPE (R1) == TPAIR)
        {
          a = CAR (R1);
          if (TYPE (a) == TPAIR)
            {
              if (CAR (a) == cell_symbol_begin)
                R1 = append2 (CDR (a), CDR (R1));
            }
        }
      if (CDR (R1) == cell_nil)
        {
          R1 = CAR (R1);
          goto eval;
        }
      push_cc (CAR (R1), R1, R0, cell_vm_begin_eval);
      goto eval;
    begin_eval:
      x = R1;
      R1 = CDR (R2);
    }
  R1 = x;
  goto vm_return;


begin_expand:
  x = cell_unspecified;
  while (R1 != cell_nil)
    {
    begin_expand_while:
      gc_check ();

      if (TYPE (R1) == TPAIR)
        {
          a = CAR (R1);
          if (TYPE (a) == TPAIR)
            if (CAAR (R1) == cell_symbol_begin)
              R1 = append2 (CDAR (R1), CDR (R1));
          if (CAAR (R1) == cell_symbol_primitive_load)
            {
              push_cc (CADAR (R1), R1, R0, cell_vm_begin_expand_primitive_load);
              goto eval;
            begin_expand_primitive_load:
              if ((TYPE (R1) == TNUMBER) && VALUE (R1) == 0)
                0;
              else if (TYPE (R1) == TSTRING)
                input = set_current_input_port (open_input_file (R1));
              else if (TYPE (R1) == TPORT)
                input = set_current_input_port (R1);
              else
                {
                  eputs ("begin_expand failed, R1=");
                  display_error_ (R1);
                  assert_msg (0, "begin-expand-boom 0");
                }

              push_cc (input, R2, R0, cell_vm_return);
              x = read_input_file_env (R0);
              if (g_debug > 5)
                module_printer (M0);
              gc_pop_frame ();
              input = R1;
              R1 = x;
              set_current_input_port (input);
              R1 = cons (cell_symbol_begin, R1);
              CAR (R2) = R1;
              R1 = R2;
              goto begin_expand_while;
              continue; /* FIXME: M2-PLanet */
            }
        }

      push_cc (CAR (R1), R1, R0, cell_vm_begin_expand_macro);
      goto macro_expand;
    begin_expand_macro:
      if (R1 != CAR (R2))
        {
          CAR (R2) = R1;
          R1 = R2;
          goto begin_expand_while;
          continue; /* FIXME: M2-PLanet */
        }
      R1 = R2;
      expand_variable (CAR (R1), cell_nil);
      push_cc (CAR (R1), R1, R0, cell_vm_begin_expand_eval);
      goto eval;
    begin_expand_eval:
      x = R1;
      R1 = CDR (R2);
    }
  R1 = x;
  goto vm_return;

vm_if:
  push_cc (CAR (R1), R1, R0, cell_vm_if_expr);
  goto eval;
if_expr:
  x = R1;
  R1 = R2;
  if (x != cell_f)
    {
      R1 = CADR (R1);
      goto eval;
    }
  if (CDDR (R1) != cell_nil)
    {
      R1 = CAR (CDDR (R1));
      goto eval;
    }
  R1 = cell_unspecified;
  goto vm_return;

call_with_current_continuation:
  gc_push_frame ();
  x = make_continuation (g_continuations);
  g_continuations = g_continuations + 1;
  v = make_vector_ (STACK_SIZE - g_stack, cell_unspecified);
  for (i = g_stack; i < STACK_SIZE; i = i + 1)
    vector_set_x_ (v, i - g_stack, g_stack_array[i]);
  CONTINUATION (x) = v;
  gc_pop_frame ();
  push_cc (cons (CAR (R1), cons (x, cell_nil)), x, R0, cell_vm_call_with_current_continuation2);
  goto apply;
call_with_current_continuation2:
  v = make_vector_ (STACK_SIZE - g_stack, cell_unspecified);
  for (i = g_stack; i < STACK_SIZE; i = i + 1)
    vector_set_x_ (v, i - g_stack, g_stack_array[i]);
  CONTINUATION (R2) = v;
  goto vm_return;

call_with_values:
  push_cc (cons (CAR (R1), cell_nil), R1, R0, cell_vm_call_with_values2);
  goto apply;
call_with_values2:
  if (TYPE (R1) == TVALUES)
    R1 = CDR (R1);
  R1 = cons (CADR (R2), R1);
  goto apply;

vm_return:
  x = R1;
  gc_pop_frame ();
  R1 = x;
  goto eval_apply;
}

SCM
apply (SCM f, SCM x, SCM a)     /*:((internal)) */
{
  push_cc (cons (f, x), cell_unspecified, R0, cell_unspecified);
  R3 = cell_vm_apply;
  return eval_apply ();
}
