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

SCM
make_vector__ (long k)
{
  SCM v = alloc (k);
  SCM x = make_cell (TVECTOR, k, v);
  long i;
  for (i = 0; i < k; i = i + 1)
    copy_cell (cell_ref (v, i), vector_entry (cell_unspecified));

  return x;
}

SCM
make_vector_ (SCM n)
{
  return make_vector__ (VALUE (n));
}

SCM
vector_length (SCM x)
{
  assert_msg (TYPE (x) == TVECTOR, "TYPE (x) == TVECTOR");
  return make_number (LENGTH (x));
}

SCM
vector_ref_ (SCM x, long i)
{
  assert_msg (TYPE (x) == TVECTOR, "TYPE (x) == TVECTOR");
  assert_msg (i < LENGTH (x), "i < LENGTH (x)");
  SCM e = cell_ref (VECTOR (x), i);
  if (TYPE (e) == TREF)
    e = REF (e);
  if (TYPE (e) == TCHAR)
    e = make_char (VALUE (e));
  if (TYPE (e) == TNUMBER)
    e = make_number (VALUE (e));
  return e;
}

SCM
vector_ref (SCM x, SCM i)
{
  return vector_ref_ (x, VALUE (i));
}

SCM
vector_entry (SCM x)
{
  if (TYPE (x) != TCHAR && TYPE (x) != TNUMBER)
    x = make_ref (x);
  return x;
}

SCM
vector_set_x_ (SCM x, long i, SCM e)
{
  assert_msg (TYPE (x) == TVECTOR, "TYPE (x) == TVECTOR");
  assert_msg (i < LENGTH (x), "i < LENGTH (x)");
  copy_cell (cell_ref (VECTOR (x), i), vector_entry (e));
  return cell_unspecified;
}

SCM
vector_set_x (SCM x, SCM i, SCM e)
{
  return vector_set_x_ (x, VALUE (i), e);
}

SCM
list_to_vector (SCM x)
{
  SCM v = make_vector__ (length__ (x));
  SCM p = VECTOR (v);
  while (x != cell_nil)
    {
      copy_cell (p, vector_entry (car (x)));
      p = p + 1;
      x = cdr (x);
    }
  return v;
}

SCM
vector_to_list (SCM v)
{
  SCM x = cell_nil;
  long i;
  for (i = LENGTH (v); i; i = i - 1)
    {
      SCM e = cell_ref (VECTOR (v), i - 1);
      if (TYPE (e) == TREF)
        e = REF (e);
      x = cons (e, x);
    }
  return x;
}
