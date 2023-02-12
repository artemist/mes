/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2016,2017,2018,2019,2023 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
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

#include <linux/syscall.h>
#include <arch/syscall.h>
#include <mes/lib.h>
#include <fcntl.h>

ssize_t
read (int filedes, void *buffer, size_t size)
{
  long long_filedes = filedes;
  ssize_t bytes = _sys_call3 (SYS_read, long_filedes, (long) buffer, (long) size);
  if (__mes_debug () > 4)
    {
      if (bytes == 1)
        {
          eputs ("read fd=");
          eputs (itoa (filedes));
          eputs (" c=");
          char *s = buffer;
          eputc (s[0]);
          eputs ("\n");
        }
      else
        {
          eputs ("read fd=");
          eputs (itoa (filedes));
          eputs (" bytes=");
          eputs (itoa (bytes));
          eputs ("\n");
        }
    }
  return bytes;
}
