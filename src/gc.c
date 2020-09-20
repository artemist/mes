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
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

int g_dump_filedes;

#if __M2_PLANET__
#define M2_CELL_SIZE 12
// CONSTANT M2_CELL_SIZE 12
#else
#define M2_CELL_SIZE 1
// CONSTANT M2_CELL_SIZE 12
#endif

char *
cell_bytes (SCM x)
{
#if POINTER_CELLS
  char *p = x;
  return p + (2 * sizeof (long));
#else
  return &CDR (x);
#endif
}

char *
news_bytes (SCM x)
{
#if POINTER_CELLS
  char *p = x;
  return p + (2 * sizeof (long));
#else
  return &NCDR (x);
#endif
}

void
gc_init ()
{
#if SYSTEM_LIBC
  ARENA_SIZE = 100000000;       /* 2.3GiB */
#elif ! __M2_PLANET__
  ARENA_SIZE = 300000;          /* 32b: 3MiB, 64b: 6 MiB */
  ARENA_SIZE = 600000;          /* 32b: 6MiB, 64b: 12 MiB */
#else
  ARENA_SIZE = 20000000;
#endif
  STACK_SIZE = 20000;

#if !POINTER_CELLS
  JAM_SIZE = 20000;
  MAX_ARENA_SIZE = 100000000;
#elif !__M2_PLANET__
  JAM_SIZE = 10;
  MAX_ARENA_SIZE = 10000000;
#else
  JAM_SIZE = 10;
  MAX_ARENA_SIZE = 20000000;
#endif
  GC_SAFETY = 2000;
  MAX_STRING = 524288;

  char *p;
  p = getenv ("MES_MAX_ARENA");
  if (p != 0)
    MAX_ARENA_SIZE = atoi (p);
  p = getenv ("MES_ARENA");
  if (p != 0)
    ARENA_SIZE = atoi (p);
  JAM_SIZE = ARENA_SIZE / 10;
  p = getenv ("MES_JAM");
  if (p != 0)
    JAM_SIZE = atoi (p);
  GC_SAFETY = ARENA_SIZE / 100;
  p = getenv ("MES_SAFETY");
  if (p != 0)
    GC_SAFETY = atoi (p);
  p = getenv ("MES_STACK");
  if (p != 0)
    STACK_SIZE = atoi (p);
  p = getenv ("MES_MAX_STRING");
  if (p != 0)
    MAX_STRING = atoi (p);

  long arena_bytes = (ARENA_SIZE + JAM_SIZE) * sizeof (struct scm);
#if !POINTER_CELLS || GC_NOFLIP
  long alloc_bytes = arena_bytes + (STACK_SIZE * sizeof (struct scm));
#else
  long alloc_bytes = (arena_bytes * 2) + (STACK_SIZE * sizeof (struct scm*));
#endif

  g_arena = malloc (alloc_bytes);
  g_cells = g_arena;

#if !POINTER_CELLS || GC_NOFLIP
  g_stack_array = g_arena + arena_bytes;
#else
  g_stack_array = g_arena + (arena_bytes * 2);
#endif

#if !POINTER_CELLS
  /* The vector that holds the arenea. */
  cell_arena = -1;
#else
  /* The vector that holds the arenea. */
  cell_arena = g_cells;
#endif

  cell_zero = cell_arena + M2_CELL_SIZE;

  g_cells = g_cells + M2_CELL_SIZE; /* Hmm? */

  TYPE (cell_arena) = TVECTOR;
  LENGTH (cell_arena) = 1000;
  VECTOR (cell_arena) = cell_zero;

  TYPE (cell_zero) = TCHAR;
  VALUE (cell_zero) = 'c';

#if !POINTER_CELLS
  g_free = 1;
#else
  g_free = g_cells + M2_CELL_SIZE;
#endif

  /* FIXME: remove MES_MAX_STRING, grow dynamically. */
  g_buf = malloc (MAX_STRING);
}

long
gc_free ()
{
#if POINTER_CELLS
  return (g_free - g_cells) / M2_CELL_SIZE;
#else
  return g_free;
#endif
}

void
gc_stats_ (char const* where)
{
#if !POINTER_CELLS
  long i = g_free;
#else
  long i = g_free - g_cells;
  i = i / M2_CELL_SIZE;
#endif
  eputs (where);
  eputs (": [");
  eputs (ltoa (i));
  eputs ("]\n");
}

SCM
alloc (long n)
{
  SCM x = g_free;
  g_free = g_free + (n * M2_CELL_SIZE);
#if !POINTER_CELLS
  long i = g_free;
#else
  long i = g_free - g_cells;
  i = i / M2_CELL_SIZE;
#endif

  if (i > ARENA_SIZE)
    assert_msg (0, "alloc: out of memory");
  return x;
}

SCM
make_cell (long type, SCM car, SCM cdr)
{
  SCM x = g_free;
  g_free = g_free + M2_CELL_SIZE;
#if !POINTER_CELLS
  long i = g_free;
#else
  long i = g_free - g_cells;
  i = i / M2_CELL_SIZE;
#endif
  if (i > ARENA_SIZE)
    assert_msg (0, "alloc: out of memory");
  TYPE (x) = type;
  CAR (x) = car;
  CDR (x) = cdr;
  return x;
}

void
copy_cell (SCM to, SCM from)
{
  TYPE (to) = TYPE (from);
  CAR (to) = CAR (from);
  CDR (to) = CDR (from);
}

void
copy_news (SCM to, SCM from)
{
  NTYPE (to) = TYPE (from);
  NCAR (to) = CAR (from);
  NCDR (to) = CDR (from);
}

void
copy_stack (long index, SCM from)
{
  g_stack_array[index] = from;
}

SCM
cell_ref (SCM cell, long index)
{
  return cell + (index * M2_CELL_SIZE);
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
  char *p = cell_bytes (x);
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
gc_init_news ()
{
#if !POINTER_CELLS
  g_news = g_cells + g_free;
  SCM ncell_arena = cell_arena;
#else

#if GC_NOFLIP
  g_news = g_free;
#else
  char* p = g_cells - M2_CELL_SIZE;
  if (p == g_arena)
    g_news = g_free;
  else
    g_news = g_arena;
#endif

  SCM ncell_arena = g_news;
#endif

  SCM ncell_zero = ncell_arena + M2_CELL_SIZE;

  g_news = g_news + M2_CELL_SIZE;

  NTYPE (ncell_arena) = TVECTOR;
  NLENGTH (ncell_arena) = LENGTH (cell_arena);

#if !POINTER_CELLS
  NVECTOR (ncell_arena) = 0;
#else
  NVECTOR (ncell_arena) = g_news;
#endif

  NTYPE (ncell_zero) = TCHAR;
  NVALUE (ncell_zero) = 'n';
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
#if !POINTER_CELLS
  long stack_offset = arena_bytes;
  long realloc_bytes = arena_bytes + (STACK_SIZE * sizeof (struct scm));
#else
  long stack_offset = (arena_bytes * 2);
  long realloc_bytes = (arena_bytes * 2) + (STACK_SIZE * sizeof (struct scm));
#endif
  void *p = realloc (g_cells - M2_CELL_SIZE, realloc_bytes);
  if (p == 0)
    {
      eputs ("realloc failed, g_free=");
      eputs (ltoa (g_free));
      eputs (":");
#if !POINTER_CELLS
      long i = g_free;
#else
      long i = g_free - g_cells;
      i = i / M2_CELL_SIZE;
#endif
      eputs (ltoa (ARENA_SIZE - i));
      eputs ("\n");
      assert_msg (0, "0");
      exit (1);
    }
  g_cells = p;
  memcpy (p + stack_offset, p + old_arena_bytes, STACK_SIZE * sizeof (SCM));
  g_cells = g_cells + M2_CELL_SIZE;
}

/* A pointer relocating memcpy for pointer cells to avoid using only
   half of the allocated cells.

   For number based cells a simply memcpy could be used, as number
   references are relative.

   A simple stop and copy (SICP 5.3) garbage collector allocates twice
   the cell arena size only for the garbage collector.  The garbage
   collector switches back and forth between cells and news, thus
   utilizing only half the allocated memory. */
void
gc_cellcpy (struct scm *dest, struct scm *src, size_t n)
{
  void *p = src;
  void *q = dest;
  long dist = p - q;
  while (n != 0)
    {
      long t = src->type;
      long a = src->car;
      long d = src->cdr;
      dest->type = t;
      if (t == TBROKEN_HEART)
        {
          dest->type = 0;
          a = 0;
          d = 0;
#if 0
          assert_msg (0, "gc_cellcpy: broken heart");
#endif
        }
      if (t == TMACRO
          || t == TPAIR
          || t == TREF
          || t == TVARIABLE)
        dest->car = a - dist;
      else
        dest->car = a;
      if (t == TBYTES
          || t == TCLOSURE
          || t == TCONTINUATION
          || t == TKEYWORD
          || t == TMACRO
          || t == TPAIR
          || t == TPORT
          || t == TSPECIAL
          || t == TSTRING
          || t == TSTRUCT
          || t == TSYMBOL
          || t == TVALUES
          || t == TVECTOR)
        dest->cdr = d - dist;
      else
        dest->cdr = d;
      if (t == TBYTES)
        {
          if (g_debug > 5)
            {
              eputs ("copying bytes[");
              eputs (ntoab (cell_bytes (src), 16, 0));
              eputs (", ");
              eputs (ntoab (a, 10, 0));
              eputs ("]: ");
              eputs (cell_bytes (src));
              eputs ("\n to [");
              eputs (ntoab (cell_bytes (dest), 16, 0));
            }
          memcpy (cell_bytes (dest), cell_bytes (src), a);
          if (g_debug > 5)
            {
              eputs ("]: ");
              eputs (cell_bytes (dest));
              eputs ("\n");
            }
          int i = bytes_cells (a);
          n = n - i;
          int c = i * M2_CELL_SIZE;
          dest = dest + c;
          src = src + c;
        }
      else
        {
          n = n - 1;
          dest = dest + M2_CELL_SIZE;
          src = src + M2_CELL_SIZE;
        }
    }
}

/* We do not actually flip cells and news, instead we move news back to
   cells. */
void
gc_flip ()
{
#if POINTER_CELLS
  if (g_free - g_news > JAM_SIZE)
    JAM_SIZE = (g_free - g_news) + ((g_free - g_news) / 2);

#if GC_NOFLIP
  cell_arena = g_cells - M2_CELL_SIZE; /* FIXME? */
  gc_cellcpy (g_cells, g_news, (g_free - g_news) / M2_CELL_SIZE);

  long dist = g_news - g_cells;
  g_free = g_free - dist;
  g_symbols = g_symbols - dist;
  g_macros = g_macros - dist;
  g_ports = g_ports - dist;
  M0 = M0 - dist;

  long i;
  for (i = g_stack; i < STACK_SIZE; i = i + 1)
    g_stack_array[i] = g_stack_array[i] - dist;

#else

  g_cells = g_news;
  cell_arena = g_news - M2_CELL_SIZE;
  cell_zero = cell_arena + M2_CELL_SIZE;
  cell_nil = cell_zero + M2_CELL_SIZE;
#endif
#endif

  if (g_debug > 2)
    gc_stats_ (";;; => jam");

#if POINTER_CELLS
  /* Nothing. */
  return;
#else
  if (g_free > JAM_SIZE)
    JAM_SIZE = g_free + g_free / 2;
  memcpy (g_cells, g_news, g_free * sizeof (struct scm));
#endif
}

SCM
gc_copy (SCM old)               /*:((internal)) */
{
  if (TYPE (old) == TBROKEN_HEART)
    return CAR (old);
  SCM new = g_free;
  g_free = g_free + M2_CELL_SIZE;
  copy_news (new, old);
  if (NTYPE (new) == TSTRUCT || NTYPE (new) == TVECTOR)
    {
      NVECTOR (new) = g_free;
      long i;
      for (i = 0; i < LENGTH (old); i = i + 1)
        {
          copy_news (g_free, cell_ref (VECTOR (old), i));
          g_free = g_free + M2_CELL_SIZE;
        }
    }
  else if (NTYPE (new) == TBYTES)
    {
      char const *src = cell_bytes (old);
      char *dest = news_bytes (new);
      size_t length = NLENGTH (new);
      memcpy (dest, src, length);
      g_free = g_free + ((bytes_cells (length) - 1) * M2_CELL_SIZE);

      if (g_debug > 4)
        {
          eputs ("gc copy bytes: ");
          eputs (src);
          eputs ("\n");
          eputs ("    length: ");
          eputs (ltoa (LENGTH (old)));
          eputs ("\n");
          eputs ("    nlength: ");
          eputs (ltoa (NLENGTH (new)));
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
gc_loop (SCM scan)
{
  SCM car;
  SCM cdr;
  while (scan < g_free)
    {
      long t = NTYPE (scan);
      if (t == TBROKEN_HEART)
        {
          NTYPE (scan) = 0;
          NCAR (scan) = 0;
          NCDR (scan) = 0;
#if 0
          assert_msg (0, "gc_loop: broken heart");
#endif
        }
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
      if (t == TCLOSURE
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
        /* *INDENT-ON* */
        {
          cdr = gc_copy (NCDR (scan));
          gc_relocate_cdr (scan, cdr);
        }
      if (t == TBYTES)
        scan = scan + (bytes_cells (NLENGTH (scan)) * M2_CELL_SIZE);
      else
        scan = scan + M2_CELL_SIZE;
    }
  gc_flip ();
}

SCM
gc_check ()
{
#if !POINTER_CELLS
  long used = g_free + GC_SAFETY;
#else
  long used = ((g_free - g_cells) / M2_CELL_SIZE) + GC_SAFETY;
#endif
  if (used >= ARENA_SIZE)
    return gc ();
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
      gc_stats_ (";;; gc");
      eputs (";;; free: [");
      eputs (ltoa (ARENA_SIZE - gc_free ()));
      eputs ("]...");
    }
#if !POINTER_CELLS
  g_free = 1;
#else
  g_free = g_news + M2_CELL_SIZE;
#endif

  if (ARENA_SIZE < MAX_ARENA_SIZE && g_cells == g_arena + M2_CELL_SIZE)
    {
      if (g_debug == 2)
        eputs ("+");
      if (g_debug > 2)
        {
          eputs (" up[");
          eputs (ltoa (g_cells));
          eputs (",");
          eputs (ltoa (g_news));
          eputs (":");
          eputs (ltoa (ARENA_SIZE));
          eputs (",");
          eputs (ltoa (MAX_ARENA_SIZE));
          eputs ("]...");
        }
      gc_up_arena ();
    }

  SCM new_cell_nil = g_free;
  SCM s;
  for (s = cell_nil; s < g_symbol_max; s = s + M2_CELL_SIZE)
    gc_copy (s);

#if POINTER_CELLS && !GC_NOFLIP
  cell_nil = new_cell_nil;
  cell_arena = g_news - M2_CELL_SIZE; /* for debugging */

#if GC_TEST
  cell_zero = cell_nil - M2_CELL_SIZE;
  g_symbol_max = g_free;
#else
  long save_gsymbols = g_symbols;
  g_symbols = 0;
  g_free = new_cell_nil;
  init_symbols_ ();
  g_symbol_max = g_symbol;
  g_symbols = save_gsymbols;
#endif

#endif

#if !GC_TEST
  g_symbols = gc_copy (g_symbols);
  g_macros = gc_copy (g_macros);
  g_ports = gc_copy (g_ports);
  M0 = gc_copy (M0);

  long i;
  for (i = g_stack; i < STACK_SIZE; i = i + 1)
    copy_stack (i, gc_copy (g_stack_array[i]));
#endif

  gc_loop (new_cell_nil);
}

SCM
gc ()
{
  if (getenv ("MES_DUMP") != 0)
    gc_dump_arena (g_cells, gc_free ());
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
#if POINTER_CELLS && !GC_NOFLIP
  gc_ ();
#endif
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
  if (getenv ("MES_DUMP") != 0)
    gc_dump_arena (g_cells, gc_free ());
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

void
dumpc (char c)
{
  fdputc (c, g_dump_filedes);
}

void
dumps (char const *s)
{
  fdputs (s, g_dump_filedes);
}

void
gc_dump_register (char const* n, SCM r)
{
  dumps (n); dumps (": ");
#if !POINTER_CELLS
  long i = r;
#else
  long i = r;
  long a = g_arena;
  i = i - a;
  i = i / M2_CELL_SIZE;
#endif
  dumps (ltoa (i));
  dumps ("\n");
}

void
gc_dump_state ()
{
  gc_dump_register ("R0", R0);
  gc_dump_register ("R1", R1);
  gc_dump_register ("R2", R2);
  gc_dump_register ("R3", R3);
  gc_dump_register ("M0", M0);
  gc_dump_register ("g_symbols", g_symbols);
  gc_dump_register ("g_symbol_max", g_symbol_max);
  gc_dump_register ("g_macros", g_macros);
  gc_dump_register ("g_ports", g_ports);
  gc_dump_register ("cell_zero", cell_zero);
  gc_dump_register ("cell_nil", cell_nil);
}

void
gc_dump_stack ()
{
  long i = g_stack;
  while (i < STACK_SIZE)
    {
      gc_dump_register (itoa (i), g_stack_array[i]);
      i = i + 1;
    }
}

void
gc_dump_arena (struct scm *cells, long size)
{
#if !POINTER_CELLS
  SCM end = size;
  struct scm *dist = 0;
#else
  SCM end = g_cells + (size * M2_CELL_SIZE);
  struct scm *dist = cells;
#endif
  if (g_dump_filedes == 0)
    g_dump_filedes = mes_open ("dump.mo", O_CREAT|O_WRONLY, 0644);
  dumps ("stack="); dumps (ltoa (g_stack)); dumpc ('\n');
  dumps ("size="); dumps (ltoa (size)); dumpc ('\n');
  gc_dump_state ();
  gc_dump_stack ();
  while (TYPE (end) == 0 && CAR (end) == 0 && CDR (end) == 0)
    {
      end = end - M2_CELL_SIZE;
      size = size - 1;
    }
  while (size > 0)
    {
      int i;
      for (i=0; i < 16; i = i + 1)
        {
          long t = cells->type;
          long a = cells->car;
          long d = cells->cdr;
          if (size == 0)
            dumps ("0 0 0");
          else
            {
              dumps (ltoa (t));
              dumpc (' ');
#if POINTER_CELLS
              if (t == TMACRO
                  || t == TPAIR
                  || t == TREF
                  || t == TVARIABLE)
                {
                  dumps (ltoa ((cells->car - dist) / M2_CELL_SIZE));
                  /* dumps ("["); dumps (ltoa (a)); dumps ("]"); */
                }
              else
#endif
                dumps (ltoa (a));
              dumpc (' ');
              if (t != TBYTES)
                {
#if POINTER_CELLS
                  if (t == TCLOSURE
                      || t == TCONTINUATION
                      || t == TKEYWORD
                      || t == TMACRO
                      || t == TPAIR
                      || t == TPORT
                      || t == TSPECIAL
                      || t == TSTRING
                      || t == TSTRUCT
                      || t == TSYMBOL
                      || t == TVALUES
                      || t == TVECTOR)
                    {
                      dumps (ltoa ((cells->cdr - dist) / M2_CELL_SIZE));
                      /* dumps ("["); dumps (ltoa (d)); dumps ("]"); */
                    }
                  else
#endif
                    if (t == TNUMBER && d > 1000)
                      dumps (ltoa (1001));
                    else
                      dumps (ltoa (d));
                }
              if (t == TBYTES)
                {
                  int c = bytes_cells (a);
                  char *p = cell_bytes (cells);
                  size = size - c;
                  dumpc ('"');
                  while (a > 0)
                    {
                      if (p[0] != 0)
                        dumpc (p[0]);
                      p = p + 1;
                      a = a - 1;
                    }
                  dumpc ('"');
                  cells = cells + c * M2_CELL_SIZE;
                  size = size - c;
                }
#if 0
              else if (t == TSTRUCT)
                {
                  cells = cells + (a + 1) * M2_CELL_SIZE;
                  size = size - a - 1;
                }
#endif
              else
                {
                  cells = cells + M2_CELL_SIZE;
                  size = size - 1;
                }
            }
          if (i != 15)
            dumps ("  ");
          else
            dumpc ('\n');
        }
      dumpc ('\n');
    }
}
