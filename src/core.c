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

/** Commentary:
    Essential functions, used by the eval/apply core.
 */

/** Code: */

#include "mes/lib.h"
#include "mes/mes.h"

#include <stdlib.h>

SCM
assoc_string (SCM x, SCM a)     /*:((internal)) */
{
  SCM b;
  while (a != cell_nil)
    {
      b = CAR (a);
      if (TYPE (CAR (b)) == TSTRING)
        if (string_equal_p (x, CAR (b)) == cell_t)
          return b;
      a = CDR (a);
    }
  if (a != cell_nil)
    return CAR (a);
  return cell_f;
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
  if (x == y)
    return cell_t;
  int t = TYPE (x);
  if (t == TKEYWORD)
    {
      if (TYPE (y) == TKEYWORD)
        return string_equal_p (x, y);
      return cell_f;
    }
  if (t == TCHAR)
    {
      if (TYPE (y) != TCHAR)
        return cell_f;
      if (VALUE (x) == VALUE (y))
        return cell_t;
      return cell_f;
    }
  if (t == TNUMBER)
    {
      if (TYPE (y) != TNUMBER)
        return cell_f;
      if (VALUE (x) == VALUE (y))
        return cell_t;
      return cell_f;
    }
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
  return make_number (length__ (x));
}

SCM
error (SCM key, SCM x)
{
#if !__MESC_MES__ && !__M2_PLANET__
  SCM throw = module_ref (R0, cell_symbol_throw);
  if (throw != cell_undefined)
    return apply (throw, cons (key, cons (x, cell_nil)), R0);
#endif
  display_error_ (key);
  eputs (": ");
  write_error_ (x);
  eputs ("\n");
  assert_msg (0, "ERROR");
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
    while (a != cell_nil)
      {
        if (x == CAAR (a))
          return CAR (a);
        a = CDR (a);
      }
  else if (t == TCHAR || t == TNUMBER)
    {
      long v = VALUE (x);
      while (a != cell_nil)
        {
          if (v == VALUE (CAAR (a)))
            return CAR (a);
          a = CDR (a);
        }
    }
  else if (t == TKEYWORD)
    {
      while (a != cell_nil)
        {
          if (string_equal_p (x, CAAR (a)) == cell_t)
            return CAR (a);
          a = CDR (a);
        }
    }
  else
    /* pointer equality, e.g. on strings. */
    while (a != cell_nil)
      {
        if (x == CAAR (a))
          return CAR (a);
        a = CDR (a);
      }
  return cell_f;
}

SCM
assoc (SCM x, SCM a)
{
  if (TYPE (x) == TSTRING)
    return assoc_string (x, a);
  while (a != cell_nil)
    {
      if (equal2_p (x, CAAR (a)) == cell_t)
        return CAR (a);
      a = CDR (a);
    }
  return cell_f;
}
