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
    Scheme library functions not used by the eval/apply core.
 */

/** Code: */

#include "mes/lib.h"
#include "mes/mes.h"

#include <stdlib.h>

SCM
type_ (SCM x)
{
  return make_number (TYPE (x));
}

SCM
car_ (SCM x)
{
  SCM a = CAR (x);
  if (TYPE (x) == TPAIR)
    return a;
  return make_number (a);
}

SCM
cdr_ (SCM x)
{
  SCM d = CDR (x);
  if (TYPE (x) == TPAIR || TYPE (x) == TCLOSURE)
    return d;
  return make_number (d);
}

SCM
xassq (SCM x, SCM a)            /* For speed in core. */
{
  while (a != cell_nil)
    {
      if (x == CDAR (a))
        return CAR (a);
      a = CDR (a);
    }
  return cell_f;
}

SCM
memq (SCM x, SCM a)
{
  int t = TYPE (x);
  if (t == TCHAR || t == TNUMBER)
    {
      long v = VALUE (x);
      while (a != cell_nil)
        {
          if (v == VALUE (CAR (a)))
            return a;
          a = CDR (a);
        }
      return cell_f;
    }
  if (t == TKEYWORD)
    {
      while (a != cell_nil)
        {
          if (TYPE (CAR (a)) == TKEYWORD)
            if (string_equal_p (x, CAR (a)) == cell_t)
              return a;
          a = CDR (a);
        }
      return cell_f;
    }
  while (a != cell_nil)
    {
      if (x == CAR (a))
        return a;
      a = CDR (a);
    }
  return cell_f;
}

SCM
equal2_p (SCM a, SCM b)
{
equal2:
  if (a == b)
    return cell_t;
  if (TYPE (a) == TPAIR && TYPE (b) == TPAIR)
    {
      if (equal2_p (CAR (a), CAR (b)) == cell_t)
        {
          a = CDR (a);
          b = CDR (b);
          goto equal2;
        }
      return cell_f;
    }
  if (TYPE (a) == TSTRING && TYPE (b) == TSTRING)
    return string_equal_p (a, b);
  if (TYPE (a) == TVECTOR && TYPE (b) == TVECTOR)
    {
      if (LENGTH (a) != LENGTH (b))
        return cell_f;
      long i;
      for (i = 0; i < LENGTH (a); i = i + 1)
        {
          SCM ai = cell_ref (VECTOR (a), i);
          SCM bi = cell_ref (VECTOR (b), i);
          if (TYPE (ai) == TREF)
            ai = REF (ai);
          if (TYPE (bi) == TREF)
            bi = REF (bi);
          if (equal2_p (ai, bi) == cell_f)
            return cell_f;
        }
      return cell_t;
    }
  return eq_p (a, b);
}

SCM
last_pair (SCM x)
{
  while (x != cell_nil)
    {
      if (CDR (x) == cell_nil)
        return x;
      x = CDR (x);
    }
  return x;
}

SCM
pair_p (SCM x)
{
  if (TYPE (x) == TPAIR)
    return cell_t;
  return cell_f;
}

SCM
char_to_integer (SCM x)
{
  return make_number (VALUE (x));
}

SCM
integer_to_char (SCM x)
{
  return make_char (VALUE (x));
}
