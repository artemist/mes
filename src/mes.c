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

#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

// char const *MES_PKGDATADIR = "mes";

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
type_ (SCM x)
{
  return make_number (TYPE (x));
}

SCM
car_ (SCM x)
{
  if (TYPE (x) != TCONTINUATION && (TYPE (CAR (x)) == TPAIR     // FIXME: this is weird
                                    || TYPE (CAR (x)) == TREF
                                    || TYPE (CAR (x)) == TSPECIAL
                                    || TYPE (CAR (x)) == TSYMBOL || TYPE (CAR (x)) == TSTRING))
    return CAR (x);
  return make_number (CAR (x));
}

SCM
cdr_ (SCM x)
{
  if (TYPE (x) != TCHAR
      && TYPE (x) != TNUMBER
      && TYPE (x) != TPORT
      && (TYPE (CDR (x)) == TPAIR
          || TYPE (CDR (x)) == TREF
          || TYPE (CDR (x)) == TSPECIAL || TYPE (CDR (x)) == TSYMBOL || TYPE (CDR (x)) == TSTRING))
    return CDR (x);
  return make_number (CDR (x));
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
  if (x == y
      || ((TYPE (x) == TKEYWORD && TYPE (y) == TKEYWORD
           && string_equal_p (x, y) == cell_t))
      || (TYPE (x) == TCHAR && TYPE (y) == TCHAR
          && VALUE (x) == VALUE (y))
      || (TYPE (x) == TNUMBER && TYPE (y) == TNUMBER && VALUE (x) == VALUE (y)))
    return cell_t;
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
#if !__MESC_MES__
  SCM throw;
  if ((throw = module_ref (R0, cell_symbol_throw)) != cell_undefined)
    return apply (throw, cons (key, cons (x, cell_nil)), R0);
#endif
  display_error_ (key);
  eputs (": ");
  write_error_ (x);
  eputs ("\n");
  assert_msg (0, "0");
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
    while (a != cell_nil && x != CAAR (a))
      a = CDR (a);
  else if (t == TCHAR || t == TNUMBER)
    {
      long v = VALUE (x);
      while (a != cell_nil && v != VALUE (CAAR (a)))
        a = CDR (a);
    }
  else if (t == TKEYWORD)
    {
      while (a != cell_nil && string_equal_p (x, CAAR (a)) == cell_f)
        a = CDR (a);
    }
  else
    /* pointer equality, e.g. on strings. */
    while (a != cell_nil && x != CAAR (a))
      a = CDR (a);
  if (a != cell_nil)
    return CAR (a);
  return cell_f;
}

SCM
assoc (SCM x, SCM a)
{
  if (TYPE (x) == TSTRING)
    return assoc_string (x, a);
  while (a != cell_nil && equal2_p (x, CAAR (a)) == cell_f)
    a = CDR (a);
  if (a != cell_nil)
    return CAR (a);
  return cell_f;
}

SCM
mes_g_stack (SCM a)             /*:((internal)) */
{
  g_stack = STACK_SIZE;
  R0 = a;
  R1 = make_char (0);
  R2 = make_char (0);
  R3 = make_char (0);
  return R0;
}

SCM
mes_environment (int argc, char **argv)
{
  SCM a = init_symbols ();

  char *compiler = "gnuc";
#if __MESC__
  compiler = "mesc";
#elif __TINYC__
  compiler = "tcc";
#endif
  a = acons (cell_symbol_compiler, make_string0 (compiler), a);

  char *arch;
#if __i386__
  arch = "x86";
#elif __arm__
  arch = "arm";
#elif __x86_64__
  arch = "x86_64";
#else
#error arch not supported
#endif
  a = acons (cell_symbol_arch, make_string0 (arch), a);

#if !MES_MINI
  SCM lst = cell_nil;
  int i;
  for (i = argc - 1; i >= 0; i = i - 1)
    lst = cons (make_string0 (argv[i]), lst);
  a = acons (cell_symbol_argv, lst, a);
#endif

  return mes_g_stack (a);
}

int
try_open_boot (char *file_name, char const *boot, char const *location)
{
  strcpy (file_name + strlen (file_name), boot);
  if (g_debug > 1)
    {
      eputs ("mes: reading boot-0 [");
      eputs (location);
      eputs ("]: ");
      eputs (file_name);
      eputs ("\n");
    }
  int fd = mes_open (file_name, O_RDONLY, 0);
  if (g_debug != 0 && fd > 0)
    {
      eputs ("mes: read boot-0: ");
      eputs (file_name);
      eputs ("\n");
    }
  return fd;
}

void
open_boot ()
{
  __stdin = -1;
  char *boot = __open_boot_buf;
  char *file_name = __open_boot_file_name;
  strcpy (g_datadir, ".");
  if (getenv ("MES_BOOT"))
    strcpy (boot, getenv ("MES_BOOT"));
  else
    strcpy (boot, "boot-0.scm");
  if (getenv ("MES_PREFIX"))
    {
      strcpy (g_datadir, getenv ("MES_PREFIX"));
      strcpy (g_datadir + strlen (g_datadir), "/mes");
      strcpy (file_name, g_datadir);
      strcpy (file_name + strlen (file_name), "/module/mes/");
      __stdin = try_open_boot (file_name, boot, "MES_PREFIX");
      if (__stdin < 0)
        {
          strcpy (g_datadir, getenv ("MES_PREFIX"));
          strcpy (g_datadir + strlen (g_datadir), "/share/mes");
          strcpy (file_name, g_datadir);
          strcpy (file_name + strlen (file_name), "/module/mes/");
          __stdin = try_open_boot (file_name, boot, "MES_PREFIX/share/mes");
        }
    }
  if (__stdin < 0)
    {
      g_datadir[0] = 0;
      if (getenv ("srcdest"))
        strcpy (g_datadir, getenv ("srcdest"));
      strcpy (g_datadir + strlen (g_datadir), "mes");
      strcpy (file_name, g_datadir);
      strcpy (file_name + strlen (file_name), "/module/mes/");
      __stdin = try_open_boot (file_name, boot, "${srcdest}mes");
    }
  if (__stdin < 0)
    {
      file_name[0] = 0;
      __stdin = try_open_boot (file_name, boot, "<boot>");
    }
  if (__stdin < 0)
    {
      eputs ("mes: boot failed: no such file: ");
      eputs (boot);
      eputs ("\n");
      exit (1);
    }
}

SCM
read_boot ()                    /*:((internal)) */
{
  R2 = read_input_file_env (R0);
  __stdin = STDIN;
  return R2;
}

void
init (char **envp)
{
  environ = envp;
  __execl_c_argv = malloc (1024 * sizeof (char *));     /* POSIX minimum: 4096 */
  __getcwd_buf = malloc (PATH_MAX);
  __open_boot_buf = malloc (PATH_MAX);
  __open_boot_file_name = malloc (PATH_MAX);
  __reader_read_char_buf = malloc (10);
  __setenv_buf = malloc (1024);
  g_datadir = malloc (1024);

  char *p;
  if (p = getenv ("MES_DEBUG"))
    g_debug = atoi (p);
  open_boot ();
  gc_init ();
}

int
main (int argc, char **argv, char **envp)
{
  init (envp);

  SCM a = mes_environment (argc, argv);
  a = mes_builtins (a);
  a = init_time (a);
  M0 = make_initial_module (a);
  g_macros = make_hash_table_ (0);

  if (g_debug > 5)
    module_printer (M0);

  SCM program = read_boot ();
  push_cc (R2, cell_unspecified, R0, cell_unspecified);

  if (g_debug > 2)
    gc_stats_ ("\n gc boot");
  if (g_debug > 3)
    {
      eputs ("program: ");
      write_error_ (R1);
      eputs ("\n");
    }
  R3 = cell_vm_begin_expand;
  R1 = eval_apply ();
  if (g_debug != 0)
    {
      write_error_ (R1);
      eputs ("\n");
    }
  if (g_debug != 0)
    {
      if (g_debug > 5)
        module_printer (M0);

      if (g_debug < 3)
        gc_stats_ ("\ngc run");
      MAX_ARENA_SIZE = 0;

      gc (g_stack);
      if (g_debug < 3)
        gc_stats_ (" => ");

      if (g_debug > 5)
        {
          eputs ("\nports:");
          write_error_ (g_ports);
          eputs ("\n");
        }
      eputs ("\n");
    }

  return 0;
}
