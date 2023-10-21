/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2016,2017,2018,2022 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
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

#include <errno.h>
#include <linux/aarch64/syscall.h>

int errno;

int
__sys_call (int sys_call)
{
  asm ("SET_X0_FROM_BP");
  asm ("SUB_X0_8");
  asm ("DEREF_X0");
  asm ("SET_X8_FROM_X0");
  asm ("SYSCALL");
}

int
__sys_call1 (int sys_call, int one)
{
  asm ("SET_X0_FROM_BP");
  asm ("SUB_X0_8");
  asm ("DEREF_X0");
  asm ("SET_X8_FROM_X0");

  asm ("SET_X0_FROM_BP");
  asm ("SUB_X0_16");
  asm ("DEREF_X0");

  asm ("SYSCALL");
}

int
__sys_call2 (int sys_call, int one, int two)
{
  asm ("SET_X0_FROM_BP");
  asm ("SUB_X0_8");
  asm ("DEREF_X0");
  asm ("SET_X8_FROM_X0");

  asm ("SET_X0_FROM_BP");
  asm ("SUB_X0_24");
  asm ("DEREF_X0");
  asm ("SET_X1_FROM_X0");

  asm ("SET_X0_FROM_BP");
  asm ("SUB_X0_16");
  asm ("DEREF_X0");

  asm ("SYSCALL");
}

int
__sys_call3 (int sys_call, int one, int two, int three)
{
  asm ("SET_X0_FROM_BP");
  asm ("SUB_X0_8");
  asm ("DEREF_X0");
  asm ("SET_X8_FROM_X0");

  asm ("SET_X0_FROM_BP");
  asm ("SUB_X0_24");
  asm ("SUB_X0_8");
  asm ("DEREF_X0");
  asm ("SET_X2_FROM_X0");

  asm ("SET_X0_FROM_BP");
  asm ("SUB_X0_24");
  asm ("DEREF_X0");
  asm ("SET_X1_FROM_X0");

  asm ("SET_X0_FROM_BP");
  asm ("SUB_X0_16");
  asm ("DEREF_X0");

  asm ("SYSCALL");
}

int
__sys_call4 (int sys_call, int one, int two, int three, int four)
{
  asm ("SET_X0_FROM_BP");
  asm ("SUB_X0_8");
  asm ("DEREF_X0");
  asm ("SET_X8_FROM_X0");

  asm ("SET_X0_FROM_BP");
  asm ("SUB_X0_24");
  asm ("SUB_X0_16");
  asm ("DEREF_X0");
  asm ("SET_X3_FROM_X0");

  asm ("SET_X0_FROM_BP");
  asm ("SUB_X0_24");
  asm ("SUB_X0_8");
  asm ("DEREF_X0");
  asm ("SET_X2_FROM_X0");

  asm ("SET_X0_FROM_BP");
  asm ("SUB_X0_24");
  asm ("DEREF_X0");
  asm ("SET_X1_FROM_X0");

  asm ("SET_X0_FROM_BP");
  asm ("SUB_X0_16");
  asm ("DEREF_X0");

  asm ("SYSCALL");
}

int
__sys_call6 (int sys_call, int one, int two, int three, int four, int five, int six)
{
  asm ("SET_X0_FROM_BP");
  asm ("SUB_X0_8");
  asm ("DEREF_X0");
  asm ("SET_X8_FROM_X0");

  asm ("SET_X0_FROM_BP");
  asm ("SUB_X0_24");
  asm ("SUB_X0_24");
  asm ("SUB_X0_8");
  asm ("DEREF_X0");
  asm ("SET_X5_FROM_X0");

  asm ("SET_X0_FROM_BP");
  asm ("SUB_X0_24");
  asm ("SUB_X0_24");
  asm ("DEREF_X0");
  asm ("SET_X4_FROM_X0");

  asm ("SET_X0_FROM_BP");
  asm ("SUB_X0_24");
  asm ("SUB_X0_16");
  asm ("DEREF_X0");
  asm ("SET_X3_FROM_X0");

  asm ("SET_X0_FROM_BP");
  asm ("SUB_X0_24");
  asm ("SUB_X0_8");
  asm ("DEREF_X0");
  asm ("SET_X2_FROM_X0");

  asm ("SET_X0_FROM_BP");
  asm ("SUB_X0_24");
  asm ("DEREF_X0");
  asm ("SET_X1_FROM_X0");

  asm ("SET_X0_FROM_BP");
  asm ("SUB_X0_16");
  asm ("DEREF_X0");

  asm ("SYSCALL");
}

int
_sys_call (int sys_call)
{
  int r = __sys_call (sys_call);
  if (r < 0)
    {
      errno = -r;
      r = -1;
    }
  else
    errno = 0;
  return r;
}

int
_sys_call1 (int sys_call, int one)
{
  int r = __sys_call1 (sys_call, one);
  if (r < 0)
    {
      errno = -r;
      r = -1;
    }
  else
    errno = 0;
  return r;
}

int
_sys_call2 (int sys_call, int one, int two)
{
  int r = __sys_call2 (sys_call, one, two);
  if (r < 0)
    {
      errno = -r;
      r = -1;
    }
  else
    errno = 0;
  return r;
}

int
_sys_call3 (int sys_call, int one, int two, int three)
{
  int r = __sys_call3 (sys_call, one, two, three);
  if (r < 0)
    {
      errno = -r;
      r = -1;
    }
  else
    errno = 0;
  return r;
}

int
_sys_call4 (int sys_call, int one, int two, int three, int four)
{
  int r = __sys_call4 (sys_call, one, two, three, four);
  if (r < 0)
    {
      errno = -r;
      r = -1;
    }
  else
    errno = 0;
  return r;
}

int
_sys_call6 (int sys_call, int one, int two, int three, int four, int five, int six)
{
  int r = __sys_call6 (sys_call, one, two, three, four, five, six);
  if (r < 0)
    {
      errno = -r;
      r = -1;
    }
  else
    errno = 0;
  return r;
}
