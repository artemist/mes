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

#ifndef __MES_M2_H
#define __MES_M2_H

char **environ;
int __stdin;
int __stdout;
int __stderr;

int __ungetc_p (int filedes);
int eputs (char *s);
int oputs (char *s);
int puts (char *s);
size_t strlen (char *s);
ssize_t _write ();
ssize_t write (int filedes, void *buffer, size_t size);
void __ungetc_clear (int filedes);
void __ungetc_init ();
void __ungetc_set (int filedes, int c);

/* Recieved SCM in program
struct timespec
{
  long tv_sec;
  long tv_nsec;
};

struct timeval
{
  long tv_sec;
  long tv_usec;
};

*/

#define struct_size 12
#define CELL(x) ((x*struct_size)+g_cells)
#define TYPE(x) ((x*struct_size)+g_cells)->type
#define CAR(x) ((x*struct_size)+g_cells)->car
#define CDR(x) ((x*struct_size)+g_cells)->cdr

#define NTYPE(x) ((x*struct_size)+g_news)->type
#define NCAR(x) ((x*struct_size)+g_news)->car
#define NCDR(x) ((x*struct_size)+g_news)->cdr


#define BYTES(x) ((x*struct_size)+g_cells)->car
#define LENGTH(x) ((x*struct_size)+g_cells)->car
#define MACRO(x) ((x*struct_size)+g_cells)->car
#define PORT(x) ((x*struct_size)+g_cells)->car
#define REF(x) ((x*struct_size)+g_cells)->car
#define VARIABLE(x) ((x*struct_size)+g_cells)->car

#define CLOSURE(x) ((x*struct_size)+g_cells)->cdr
#define CONTINUATION(x) ((x*struct_size)+g_cells)->cdr

#define CBYTES(x) (((x*struct_size)+g_cells) + 8)

#define NAME(x) ((x*struct_size)+g_cells)->cdr
#define STRING(x) ((x*struct_size)+g_cells)->cdr
#define STRUCT(x) ((x*struct_size)+g_cells)->cdr
#define VALUE(x) ((x*struct_size)+g_cells)->cdr
#define VECTOR(x) ((x*struct_size)+g_cells)->cdr

#define NLENGTH(x) ((x*struct_size)+g_news)->car

#define NVALUE(x) ((x*struct_size)+g_news)->cdr
#define NSTRING(x) ((x*struct_size)+g_news)->cdr
#define NVECTOR(x) ((x*struct_size)+g_news)->cdr

#define CAAR(x) CAR (CAR (x))
#define CADR(x) CAR (CDR (x))
#define CDAR(x) CDR (CAR (x))
#define CDDR(x) CDR (CDR (x))
#define CADAR(x) CAR (CDR (CAR (x)))
#define CADDR(x) CAR (CDR (CDR (x)))
#define CDADAR(x) CAR (CDR (CAR (CDR (x))))

#endif /* __MES_M2_H */
