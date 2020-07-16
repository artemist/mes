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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int
peekchar ()
{
  if (__stdin >= 0)
    {
      int c = readchar ();
      unreadchar (c);
      return c;
    }
  SCM port = current_input_port ();
  SCM string = STRING (port);
  size_t length = LENGTH (string);
  if (length == 0)
    return -1;
  char const *p = cell_bytes (STRING (string));
  return p[0];
}

int
readchar ()
{
  if (__stdin >= 0)
    return fdgetc (__stdin);
  SCM port = current_input_port ();
  SCM string = STRING (port);
  size_t length = LENGTH (string);
  if (length == 0)
    return -1;
  char const *p = cell_bytes (STRING (string));
  int c = p[0];
  p = p + 1;
  STRING (port) = make_string (p, length - 1);
  return c;
}

int
unreadchar (int c)
{
  if (__stdin >= 0)
    return fdungetc (c, __stdin);
  if (c == EOF) /* can't unread EOF */
    return c;
  SCM port = current_input_port ();
  SCM string = STRING (port);
  size_t length = LENGTH (string);
  char *p = cell_bytes (STRING (string));
  p = p - 1;
  string = make_string (p, length + 1);
  p = cell_bytes (STRING (string));
  p[0] = c;
  STRING (port) = string;
  return c;
}

SCM
peek_byte ()
{
  return make_number (peekchar ());
}

SCM
read_byte ()
{
  return make_number (readchar ());
}

SCM
unread_byte (SCM i)
{
  unreadchar (VALUE (i));
  return i;
}

SCM
peek_char ()
{
  return make_char (peekchar ());
}

SCM
read_char (SCM port)            /*:((arity . n)) */
{
  int fd = __stdin;
  if (TYPE (port) == TPAIR)
    if (TYPE (CAR (port)) == TNUMBER)
      __stdin = VALUE (CAR (port));
  SCM c = make_char (readchar ());
  __stdin = fd;
  return c;
}

SCM
unread_char (SCM i)
{
  unreadchar (VALUE (i));
  return i;
}

SCM
write_char (SCM i)              /*:((arity . n)) */
{
  write_byte (i);
  return i;
}

SCM
write_byte (SCM x)              /*:((arity . n)) */
{
  SCM c = car (x);
  SCM p = cdr (x);
  int fd = __stdout;
  if (TYPE (p) == TPAIR)
    {
      SCM f = CAR (p);
      if (TYPE (f) == TNUMBER)
        {
          long v = VALUE (f);
          if (v != 1)
            fd = v;
          if (v == 2)
            fd = __stderr;
        }
    }
  char cc = VALUE (c);
  write (fd, &cc, 1);
#if !__MESC__
  assert_msg (TYPE (c) == TNUMBER || TYPE (c) == TCHAR, "TYPE (c) == TNUMBER || TYPE (c) == TCHAR");
#endif
  return c;
}

SCM
getenv_ (SCM s)                 /*:((name . "getenv")) */
{
  char *p;
  p = getenv (cell_bytes (STRING (s)));
  if (p != 0)
    return make_string0 (p);
  return cell_f;
}

SCM
setenv_ (SCM s, SCM v)          /*:((name . "setenv")) */
{
  char *buf = __setenv_buf;
  strcpy (buf, cell_bytes (STRING (s)));
  setenv (buf, cell_bytes (STRING (v)), 1);
  return cell_unspecified;
}

SCM
access_p (SCM file_name, SCM mode)
{
  if (access (cell_bytes (STRING (file_name)), VALUE (mode)) == 0)
    return cell_t;
  return cell_f;
}

SCM
current_input_port ()
{
  if (__stdin >= 0)
    return make_number (__stdin);
  SCM x = g_ports;
  while (x && PORT (CAR (x)) != __stdin)
    x = CDR (x);
  return CAR (x);
}

SCM
open_input_file (SCM file_name)
{
  int filedes = mes_open (cell_bytes (STRING (file_name)), O_RDONLY, 0);
  if (filedes == -1)
    error (cell_symbol_system_error, cons (make_string0 ("No such file or directory"), file_name));
  return make_number (filedes);
}

SCM
open_input_string (SCM string)
{
  SCM port = make_string_port (string);
  g_ports = cons (port, g_ports);
  return port;
}

SCM
set_current_input_port (SCM port)
{
  SCM prev = current_input_port ();
  if (TYPE (port) == TNUMBER)
    {
      int p = VALUE (port);
      if (p != 0)
        __stdin = p;
      else
        __stdin = STDIN;
    }
  else if (TYPE (port) == TPORT)
    __stdin = PORT (port);
  return prev;
}

SCM
current_output_port ()
{
  return make_number (__stdout);
}

SCM
current_error_port ()
{
  return make_number (__stderr);
}

SCM
open_output_file (SCM x)        /*:((arity . n)) */
{
  SCM file_name = car (x);
  x = cdr (x);
  int mode = S_IRUSR | S_IWUSR;
  if (TYPE (x) == TPAIR && TYPE (car (x)) == TNUMBER)
    mode = VALUE (car (x));
  return make_number (mes_open (cell_bytes (STRING (file_name)), O_WRONLY | O_CREAT | O_TRUNC, mode));
}

SCM
set_current_output_port (SCM port)
{
  if (VALUE (port) != 0)
    __stdout = VALUE (port);
  else
    __stdout = STDOUT;
  return current_output_port ();
}

SCM
set_current_error_port (SCM port)
{
  if (VALUE (port) != 0)
    __stderr = VALUE (port);
  else
    __stderr = STDERR;
  return current_error_port ();
}

SCM
chmod_ (SCM file_name, SCM mode)        /*:((name . "chmod")) */
{
  chmod (cell_bytes (STRING (file_name)), VALUE (mode));
  return cell_unspecified;
}

SCM
isatty_p (SCM port)
{
  if (isatty (VALUE (port)) != 0)
    return cell_t;
  return cell_f;
}

SCM
primitive_fork ()
{
  return make_number (fork ());
}

SCM
execl_ (SCM file_name, SCM args)        /*:((name . "execl")) */
{
  char **c_argv = __execl_c_argv;
  int i = 0;

  if (length__ (args) > 1000)
    error (cell_symbol_system_error,
           cons (file_name, cons (make_string0 ("too many arguments"), cons (file_name, args))));
  c_argv[i] = cell_bytes (STRING (file_name));
  i = i + 1;
  while (args != cell_nil)
    {
      assert_msg (TYPE (CAR (args)) == TSTRING, "TYPE (CAR (args)) == TSTRING");
      SCM arg = CAR (args);
      c_argv[i] = cell_bytes (STRING (arg));
      i = i + 1;
      args = CDR (args);
      if (g_debug > 2)
        {
          eputs ("arg[");
          eputs (itoa (i));
          eputs ("]: ");
          eputs (c_argv[i - 1]);
          eputs ("\n");
        }
    }
  c_argv[i] = 0;
  return make_number (execv (c_argv[0], c_argv));
}

SCM
waitpid_ (SCM pid, SCM options)
{
  int status;
  int child = waitpid (VALUE (pid), &status, VALUE (options));
  return cons (make_number (child), make_number (status));
}

#if __x86_64__
/* Nanoseconds on 64-bit systems with POSIX timers.  */
#define TIME_UNITS_PER_SECOND 1000000000
#else
/* Milliseconds for everyone else.  */
#define TIME_UNITS_PER_SECOND 1000
#endif

#if __M2_PLANET
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
#endif

struct timespec g_start_time;
SCM
init_time (SCM a)               /*:((internal)) */
{
  clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &g_start_time);
  a = acons (cell_symbol_internal_time_units_per_second, make_number (TIME_UNITS_PER_SECOND), a);
}

SCM
current_time ()
{
  return make_number (time (0));
}

SCM
gettimeofday_ ()                /*:((name . "gettimeofday")) */
{
#if __M2_PLANET__
  return make_number (0);
#else
  struct timeval time;
  gettimeofday (&time, 0);
  return cons (make_number (time.tv_sec), make_number (time.tv_usec));
#endif
}

long
seconds_and_nanoseconds_to_long (long s, long ns)
{
  return s * TIME_UNITS_PER_SECOND + ns / (1000000000 / TIME_UNITS_PER_SECOND);
}

SCM
get_internal_run_time ()
{
#if __M2_PLANET__
  return make_number (0);
#else
  struct timespec ts;
  clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &ts);
  long time = seconds_and_nanoseconds_to_long (ts.tv_sec - g_start_time.tv_sec,
                                               ts.tv_nsec - g_start_time.tv_nsec);
  return make_number (time);
#endif
}

SCM
getcwd_ ()                      /*:((name . "getcwd")) */
{
  char *buf = __getcwd_buf;
  return make_string0 (getcwd (buf, PATH_MAX));
}

SCM
dup_ (SCM port)                 /*:((name . "dup")) */
{
  return make_number (dup (VALUE (port)));
}

SCM
dup2_ (SCM old, SCM new)        /*:((name . "dup2")) */
{
  dup2 (VALUE (old), VALUE (new));
  return cell_unspecified;
}

SCM
delete_file (SCM file_name)
{
  unlink (cell_bytes (STRING (file_name)));
  return cell_unspecified;
}
