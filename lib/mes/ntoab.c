/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2016,2017,2018,2019,2020 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
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

#include <mes/lib.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

char *__itoa_buf;

char *
ntoab (long x, unsigned base, int signed_p)
{
#if 0
  if (! __itoa_buf)
    __itoa_buf = malloc (20);
  p = __itoa_buf + 11;
#else
  static char buf[20];
  char *p = buf + 19;
#endif

  p[0] = 0;
  p = p - 1;
  assert_msg (base > 0, "base > 0");

  int sign_p = 0;
  unsigned long u;
  if (signed_p != 0 && x < 0)
    {
      sign_p = 1;
      /* Avoid LONG_MIN */
      u = (unsigned long) (-(x + 1));
      ++u;
    }
  else
    u = x;

  do
    {
      unsigned long i;
#if __MESC__ && __arm__
      u = __mesabi_uldiv (u, (unsigned long) base, &i);
#else
      i = u % base;
      u = u / base;
#endif
      if (i > 9)
        p[0] = 'a' + i - 10;
      else
        p[0] = '0' + i;
      p = p - 1;
    }
  while (u != 0);

  if (sign_p && p[1] != '0')
    {
      p[0] = '-';
      p = p - 1;
    }

  return p + 1;
}
