/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2016,2017,2018,2019 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
 * Copyright © 2021 W. J. van der Laan <laanwj@protonmail.com>
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

#include <stdlib.h>

SCM
frame_printer (SCM frame)
{
  fdputs ("#<", __stdout);
  display_ (struct_ref_ (frame, 2));
  fdputc (' ', __stdout);
  fdputs ("procedure: ", __stdout);
  display_ (struct_ref_ (frame, 3));
  fdputc ('>', __stdout);
}

SCM
make_frame_type ()              /*:((internal)) */
{
  SCM fields = cell_nil;
  fields = cons (cell_symbol_procedure, fields);
  fields = cons (fields, cell_nil);
  fields = cons (cell_symbol_frame, fields);
  return make_struct (cell_symbol_record_type, fields, cell_unspecified);
}

SCM
make_frame (SCM stack, long index)
{
  SCM frame_type = make_frame_type ();
  long array_index = 0;
  SCM procedure = 0;
  if (index != 0)
    {
      array_index = (STACK_SIZE - (index * FRAME_SIZE));
      procedure = g_stack_array[array_index + FRAME_PROCEDURE];
    }
  if (procedure == 0)
    procedure = cell_f;
  SCM values = cell_nil;
  values = cons (procedure, values);
  values = cons (cell_symbol_frame, values);
  return make_struct (frame_type, values, cstring_to_symbol ("frame-printer"));
}

SCM
make_stack_type ()              /*:((internal)) */
{
  SCM fields = cell_nil;
  fields = cons (cstring_to_symbol ("frames"), fields);
  fields = cons (fields, cell_nil);
  fields = cons (cell_symbol_stack, fields);
  return make_struct (cell_symbol_record_type, fields, cell_unspecified);
}

SCM
make_stack (SCM stack)          /*:((arity . n)) */
{
  SCM stack_type = make_stack_type ();
  long size = (STACK_SIZE - g_stack) / FRAME_SIZE;
  SCM frames = make_vector_ (size, cell_unspecified);
  long i;
  for (i = 0; i < size; i = i + 1)
    {
      SCM frame = make_frame (stack, i);
      vector_set_x_ (frames, i, frame);
    }
  SCM values = cell_nil;
  values = cons (frames, values);
  values = cons (cell_symbol_stack, values);
  return make_struct (stack_type, values, cell_unspecified);
}

SCM
stack_length (SCM stack)
{
  SCM frames = struct_ref_ (stack, 3);
  return vector_length (frames);
}

SCM
stack_ref (SCM stack, SCM index)
{
  SCM frames = struct_ref_ (stack, 3);
  return vector_ref (frames, index);
}
