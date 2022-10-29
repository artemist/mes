/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2016,2017,2018,2019,2020,2022 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
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
#include <linux/syscall.h>
#include <arch/syscall.h>
#include <sys/types.h>

int
waitpid (int pid, int *status_ptr, int options)
{
  long long_pid = pid;
  long long_status_ptr = cast_voidp_to_long (status_ptr);
  long long_options = options;
//##if __i386__
//#  return _sys_call3 (SYS_waitpid, long_pid, long_status_ptr, long_options);
//##elif __x86_64__ || __arm__
  return _sys_call4 (SYS_wait4, long_pid, long_status_ptr, long_options, 0);
//##else
//##error arch not supported
//##endif
}
