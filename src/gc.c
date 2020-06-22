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

#include <errno.h>
#include <string.h>
#include <stdlib.h>

char *
cell_bytes (SCM x)
{
  return &CDR (x);
}

char *
news_bytes (SCM x)
{
  return &NCDR (x);
}

void
gc_init ()
{
#if SYSTEM_LIBC
  ARENA_SIZE = 100000000;       /* 2.3GiB */
#else
  ARENA_SIZE = 300000;          /* 32b: 3MiB, 64b: 6 MiB */
#endif
  MAX_ARENA_SIZE = 100000000;
  STACK_SIZE = 20000;

  JAM_SIZE = 20000;
  GC_SAFETY = 2000;
  MAX_STRING = 524288;

  char *p;
  if (p = getenv ("MES_MAX_ARENA"))
    MAX_ARENA_SIZE = atoi (p);
  if (p = getenv ("MES_ARENA"))
    ARENA_SIZE = atoi (p);
  JAM_SIZE = ARENA_SIZE / 10;
  if (p = getenv ("MES_JAM"))
    JAM_SIZE = atoi (p);
  GC_SAFETY = ARENA_SIZE / 100;
  if (p = getenv ("MES_SAFETY"))
    GC_SAFETY = atoi (p);
  if (p = getenv ("MES_STACK"))
    STACK_SIZE = atoi (p);
  if (p = getenv ("MES_MAX_STRING"))
    MAX_STRING = atoi (p);

  long arena_bytes = (ARENA_SIZE + JAM_SIZE) * sizeof (struct scm);
  void *a = malloc (arena_bytes + STACK_SIZE * sizeof (SCM));
  g_cells = a;
  g_stack_array = a + arena_bytes;

  TYPE (0) = TVECTOR;
  LENGTH (0) = 1000;
  VECTOR (0) = 0;
  g_cells = g_cells + 1;
  TYPE (0) = TCHAR;
  VALUE (0) = 'c';

  /* FIXME: remove MES_MAX_STRING, grow dynamically */
  g_buf = malloc (MAX_STRING);
}

SCM
gc_init_news ()                 /*:((internal)) */
{
  g_news = g_cells + g_free;
  NTYPE (0) = TVECTOR;
  NLENGTH (0) = 1000;
  NVECTOR (0) = 0;
  g_news = g_news + 1;
  NTYPE (0) = TCHAR;
  NVALUE (0) = 'n';
  return 0;
}

SCM
alloc (long n)
{
  SCM x = g_free;
  g_free = g_free + n;
  if (g_free > ARENA_SIZE)
    assert_msg (0, "alloc: out of memory");
  return x;
}

SCM
make_cell (long type, SCM car, SCM cdr)
{
  SCM x = g_free;
  g_free = g_free + 1;
  if (g_free > ARENA_SIZE)
    assert_msg (0, "alloc: out of memory");
  TYPE (x) = type;
  CAR (x) = car;
  CDR (x) = cdr;
  return x;
}

SCM
cons (SCM x, SCM y)
{
  return make_cell (TPAIR, x, y);
}

size_t
bytes_cells (size_t length)
{
  return (sizeof (long) + sizeof (long) + length - 1 + sizeof (SCM)) / sizeof (SCM);
}

SCM
make_bytes (char const *s, size_t length)
{
  size_t size = bytes_cells (length);
  SCM x = alloc (size);
  TYPE (x) = TBYTES;
  LENGTH (x) = length;
#if __M2_PLANET__
  char *p = &g_cells[x];
  p = p + 2 * sizeof (SCM);
#else
  char *p = &CDR (x);
#endif
  if (length == 0)
    p[0] = 0;
  else
    memcpy (p, s, length);

  return x;
}

SCM
make_char (int n)
{
  return make_cell (TCHAR, 0, n);
}

SCM
make_continuation (long n)
{
  return make_cell (TCONTINUATION, n, g_stack);
}

SCM
make_macro (SCM name, SCM x)    /*:((internal)) */
{
  return make_cell (TMACRO, x, STRING (name));
}

SCM
make_number (long n)
{
  return make_cell (TNUMBER, 0, n);
}

SCM
make_ref (SCM x)                /*:((internal)) */
{
  return make_cell (TREF, x, 0);
}

SCM
make_string (char const *s, size_t length)
{
  if (length > MAX_STRING)
    assert_max_string (length, "make_string", s);
  SCM x = make_cell (TSTRING, length, 0);
  SCM v = make_bytes (s, length + 1);
  CDR (x) = v;
  return x;
}

SCM
make_string0 (char const *s)
{
  return make_string (s, strlen (s));
}

SCM
make_string_port (SCM x)        /*:((internal)) */
{
  return make_cell (TPORT, -length__ (g_ports) - 2, x);
}

void
gc_up_arena ()
{
  long old_arena_bytes = (ARENA_SIZE + JAM_SIZE) * sizeof (struct scm);
  if (ARENA_SIZE >> 1 < MAX_ARENA_SIZE >> 2)
    {
      ARENA_SIZE = ARENA_SIZE << 1;
      JAM_SIZE = JAM_SIZE << 1;
      GC_SAFETY = GC_SAFETY << 1;
    }
  else
    ARENA_SIZE = MAX_ARENA_SIZE - JAM_SIZE;
  long arena_bytes = (ARENA_SIZE + JAM_SIZE) * sizeof (struct scm);
  void *p = realloc (g_cells - 1, arena_bytes + STACK_SIZE * sizeof (SCM));
  if (p == 0)
    {
      eputs ("realloc failed, g_free=");
      eputs (itoa (g_free));
      eputs (":");
      eputs (itoa (ARENA_SIZE - g_free));
      eputs ("\n");
      assert_msg (0, "0");
      exit (1);
    }
  g_cells = p;
  memcpy (p + arena_bytes, p + old_arena_bytes, STACK_SIZE * sizeof (SCM));
  g_cells = g_cells + 1;
}

void
gc_flip ()
{
  if (g_debug > 2)
    {
      eputs (";;;   => jam[");
      eputs (itoa (g_free));
      eputs ("]\n");
    }
  if (g_free > JAM_SIZE)
    JAM_SIZE = g_free + g_free / 2;
  memcpy (g_cells - 1, g_news - 1, (g_free + 2) * sizeof (struct scm));
}

SCM
gc_copy (SCM old)               /*:((internal)) */
{
  if (TYPE (old) == TBROKEN_HEART)
    return CAR (old);
  SCM new = g_free;
  g_free = g_free + 1;
  g_news[new] = g_cells[old];
  if (NTYPE (new) == TSTRUCT || NTYPE (new) == TVECTOR)
    {
      NVECTOR (new) = g_free;
      long i;
      for (i = 0; i < LENGTH (old); i = i + 1)
        {
          g_news[g_free] = g_cells[VECTOR (old) + i];
          g_free = g_free + 1;
        }
    }
  else if (NTYPE (new) == TBYTES)
    {
      char const *src = cell_bytes (old);
      char *dest = news_bytes (new);
      size_t length = NLENGTH (new);
      memcpy (dest, src, length);
      g_free = g_free + bytes_cells (length) - 1;

      if (g_debug > 4)
        {
          eputs ("gc copy bytes: ");
          eputs (src);
          eputs ("\n");
          eputs ("    length: ");
          eputs (itoa (LENGTH (old)));
          eputs ("\n");
          eputs ("    nlength: ");
          eputs (itoa (NLENGTH (new)));
          eputs ("\n");
          eputs ("        ==> ");
          eputs (dest);
          eputs ("\n");
        }
    }
  TYPE (old) = TBROKEN_HEART;
  CAR (old) = new;
  return new;
}

SCM
gc_relocate_car (SCM new, SCM car)      /*:((internal)) */
{
  NCAR (new) = car;
  return cell_unspecified;
}

SCM
gc_relocate_cdr (SCM new, SCM cdr)      /*:((internal)) */
{
  NCDR (new) = cdr;
  return cell_unspecified;
}

void
gc_loop (SCM scan)              /*:((internal)) */
{
  SCM car;
  SCM cdr;
  while (scan < g_free)
    {
      long t = NTYPE (scan);
      if (t == TBROKEN_HEART)
        assert_msg (0, "broken heart");
      /* *INDENT-OFF* */
      if (t == TMACRO
          || t == TPAIR
          || t == TREF
          || t == TVARIABLE)
        /* *INDENT-ON* */
        {
          car = gc_copy (NCAR (scan));
          gc_relocate_car (scan, car);
        }
      /* *INDENT-OFF* */
      if ((t == TCLOSURE
           || t == TCONTINUATION
           || t == TKEYWORD
           || t == TMACRO
           || t == TPAIR
           || t == TPORT
           || t == TSPECIAL
           || t == TSTRING
           /*|| t == TSTRUCT handled by gc_copy */
           || t == TSYMBOL
           || t == TVALUES
           /*|| t == TVECTOR handled by gc_copy */
           )
          && NCDR (scan))   /* Allow for 0 terminated list of symbols. */
        /* *INDENT-ON* */
        {
          cdr = gc_copy (NCDR (scan));
          gc_relocate_cdr (scan, cdr);
        }
      if (t == TBYTES)
        scan = scan + bytes_cells (NLENGTH (scan));
      else
        scan = scan + 1;
    }
  gc_flip ();
}

SCM
gc_check ()
{
  if (g_free + GC_SAFETY > ARENA_SIZE)
    gc ();
  return cell_unspecified;
}

void
gc_ ()
{
  gc_init_news ();
  if (g_debug == 2)
    eputs (".");
  if (g_debug > 2)
    {
      eputs (";;; gc[");
      eputs (itoa (g_free));
      eputs (":");
      eputs (itoa (ARENA_SIZE - g_free));
      eputs ("]...");
    }
  g_free = 1;

  if (ARENA_SIZE < MAX_ARENA_SIZE && g_news > 0)
    {
      if (g_debug == 2)
        eputs ("+");
      if (g_debug > 2)
        {
          eputs (" up[");
          eputs (itoa (g_cells));
          eputs (",");
          eputs (itoa (g_news));
          eputs (":");
          eputs (itoa (ARENA_SIZE));
          eputs (",");
          eputs (itoa (MAX_ARENA_SIZE));
          eputs ("]...");
        }
      gc_up_arena ();
    }

  long i;
  for (i = g_free; i < g_symbol_max; i = i + 1)
    gc_copy (i);
  g_symbols = gc_copy (g_symbols);
  g_macros = gc_copy (g_macros);
  g_ports = gc_copy (g_ports);
  M0 = gc_copy (M0);
  for (i = g_stack; i < STACK_SIZE; i = i + 1)
    g_stack_array[i] = gc_copy (g_stack_array[i]);
  gc_loop (1);
}

SCM
gc ()
{
  if (g_debug > 5)
    {
      eputs ("symbols: ");
      write_error_ (g_symbols);
      eputs ("\n");
      eputs ("R0: ");
      write_error_ (R0);
      eputs ("\n");
    }
  gc_push_frame ();
  gc_ ();
  gc_pop_frame ();
  if (g_debug > 5)
    {
      eputs ("symbols: ");
      write_error_ (g_symbols);
      eputs ("\n");
      eputs ("R0: ");
      write_error_ (R0);
      eputs ("\n");
    }
  return cell_unspecified;
}

void
gc_push_frame ()
{
  if (g_stack < FRAME_SIZE)
    assert_msg (0, "STACK FULL");
  g_stack_array[g_stack - 1] = cell_f;
  g_stack_array[g_stack - 2] = R0;
  g_stack_array[g_stack - 3] = R1;
  g_stack_array[g_stack - 4] = R2;
  g_stack_array[g_stack - 5] = R3;
  g_stack = g_stack - FRAME_SIZE;
}

void
gc_peek_frame ()
{
  R3 = g_stack_array[g_stack];
  R2 = g_stack_array[g_stack + 1];
  R1 = g_stack_array[g_stack + 2];
  R0 = g_stack_array[g_stack + 3];
  g_stack_array[g_stack + FRAME_PROCEDURE];
}

void
gc_pop_frame ()
{
  gc_peek_frame ();
  g_stack = g_stack + FRAME_SIZE;
}
