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

#ifndef __MES_MACROS_H
#define __MES_MACROS_H

#define TYPE(x) g_cells[x - g_cells].type
#define CAR(x) g_cells[x - g_cells].car
#define CDR(x) g_cells[x - g_cells].cdr

#define NTYPE(x) g_news[x - g_news].type
#define NCAR(x) g_news[x - g_news].car
#define NCDR(x) g_news[x - g_news].cdr

#define STYPE(x) TYPE (g_stack_array[x])
#define SCAR(x) CAR (g_stack_array[x])
#define SCDR(x) CDR (g_stack_arraynews[x])

#define BYTES(x) g_cells[x - g_cells].bytes
#define LENGTH(x) g_cells[x - g_cells].length
#define MACRO(x) g_cells[x - g_cells].macro
#define PORT(x) g_cells[x - g_cells].port
#define REF(x) g_cells[x - g_cells].ref
#define VARIABLE(x) g_cells[x - g_cells].variable

#define CLOSURE(x) g_cells[x - g_cells].closure
#define CONTINUATION(x) g_cells[x - g_cells].continuation

#define NAME(x) g_cells[x - g_cells].name
#define STRING(x) g_cells[x - g_cells].string
#define STRUCT(x) g_cells[x - g_cells].structure
#define VALUE(x) g_cells[x - g_cells].value
#define VECTOR(x) g_cells[x - g_cells].vector

#define NLENGTH(x) g_news[x - g_news].length
#define NVALUE(x) g_news[x - g_news].value
#define NSTRING(x) g_news[x - g_news].string
#define NVECTOR(x) g_news[x - g_news].vector

#define CAAR(x) CAR (CAR (x))
#define CADR(x) CAR (CDR (x))
#define CDAR(x) CDR (CAR (x))
#define CDDR(x) CDR (CDR (x))
#define CADAR(x) CAR (CDR (CAR (x)))
#define CADDR(x) CAR (CDR (CDR (x)))
#define CDADR(x) CDR (CAR (CDR (x)))
#define CDDAR(x) CDR (CDR (CAR (x)))

#endif //__MES_MACROS_H
