/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2018,2019 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
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
make_struct (SCM type, SCM fields, SCM printer)
{
  long size = 2 + length__ (fields);
  SCM x = alloc (1);
  SCM v = alloc (size);
  TYPE (x) = TSTRUCT;
  LENGTH (x) = size;
  STRUCT (x) = v;
  copy_cell (v, vector_entry (type));
  copy_cell (cell_ref (v, 1), vector_entry (printer));
  long i;
  for (i = 2; i < size; i = i + 1)
    {
      SCM e = cell_unspecified;
      if (fields != cell_nil)
        {
          e = CAR (fields);
          fields = CDR (fields);
        }
      copy_cell (cell_ref (v, i), vector_entry (e));
    }
  return x;
}

SCM
struct_length (SCM x)
{
  assert_msg (TYPE (x) == TSTRUCT, "TYPE (x) == TSTRUCT");
  return make_number (LENGTH (x));
}

SCM
struct_ref_ (SCM x, long i)
{
  assert_msg (TYPE (x) == TSTRUCT, "TYPE (x) == TSTRUCT");
  assert_msg (i < LENGTH (x), "i < LENGTH (x)");
  SCM e = cell_ref (STRUCT (x), i);
  if (TYPE (e) == TREF)
    e = REF (e);
  if (TYPE (e) == TCHAR)
    e = make_char (VALUE (e));
  if (TYPE (e) == TNUMBER)
    e = make_number (VALUE (e));
  return e;
}

SCM
struct_set_x_ (SCM x, long i, SCM e)
{
  assert_msg (TYPE (x) == TSTRUCT, "TYPE (x) == TSTRUCT");
  assert_msg (i < LENGTH (x), "i < LENGTH (x)");
  copy_cell (cell_ref (STRUCT (x), i), vector_entry (e));
  return cell_unspecified;
}

SCM
struct_ref (SCM x, SCM i)
{
  return struct_ref_ (x, VALUE (i));
}

SCM
struct_set_x (SCM x, SCM i, SCM e)
{
  return struct_set_x_ (x, VALUE (i), e);
}
