/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2017,2022 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
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
#ifndef __MES_ASSERT_H
#define __MES_ASSERT_H 1

#if SYSTEM_LIBC
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#undef __MES_ASSERT_H
#include_next <assert.h>
#else // ! SYSTEM_LIBC
#define assert(x) ((x) ? (void)0 : __assert_fail (#x, 0, 0, 0))
void __assert_fail (char const *s, char const *file, unsigned line,
                    char const *function);
#endif // ! SYSTEM_LIBC

#endif // __MES_ASSERT_H
