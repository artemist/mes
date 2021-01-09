/*
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2021 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
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

#include <stdio.h>
#include <string.h>

/* for GCC --with-system-libc */
#define eputs(x) fputs (x, stderr)

int
test (char *string)
{
  static char local_static[32];
  if (string != 0)
    strcpy (local_static, string);
  eputs (local_static);
}

int
main ()
{
  test ("hello local static\n");
  test (0);
  return 0;
}
