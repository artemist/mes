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

#include <assert.h>

SCM
make_struct (SCM type, SCM fields, SCM printer)
{
  long size = 2 + length__ (fields);
  SCM v = alloc (size);
  SCM x = make_cell__ (TSTRUCT, size, v);
  SCM vt = vector_entry (type);
  TYPE (v) = TYPE (vt);
  CAR (v) = CAR (vt);
  CDR (v) = CDR (vt);
  SCM vp = vector_entry (printer);
  TYPE (v + 1) = TYPE (vp);
  CAR (v + 1) = CAR (vp);
  CDR (v + 1) = CDR (vp);
  long i;
  for (i = 2; i < size; i = i + 1)
    {
      SCM e = cell_unspecified;
      if (fields != cell_nil)
        {
          e = CAR (fields);
          fields = CDR (fields);
        }
      SCM ve = vector_entry (e);
      TYPE (v + i) = TYPE (ve);
      CAR (v + i) = CAR (ve);
      CDR (v + i) = CDR (ve);
    }
  return x;
}

SCM
struct_length (SCM x)
{
  assert (TYPE (x) == TSTRUCT);
  return MAKE_NUMBER (LENGTH (x));
}

SCM
struct_ref_ (SCM x, long i)
{
  assert (TYPE (x) == TSTRUCT);
  assert (i < LENGTH (x));
  SCM e = STRUCT (x) + i;
  if (TYPE (e) == TREF)
    e = REF (e);
  if (TYPE (e) == TCHAR)
    e = MAKE_CHAR (VALUE (e));
  if (TYPE (e) == TNUMBER)
    e = MAKE_NUMBER (VALUE (e));
  return e;
}

SCM
struct_set_x_ (SCM x, long i, SCM e)
{
  assert (TYPE (x) == TSTRUCT);
  assert (i < LENGTH (x));
  g_cells[STRUCT (x) + i] = g_cells[vector_entry (e)];
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
