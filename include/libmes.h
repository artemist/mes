/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2016,2017,2018 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
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

#ifndef __MES_LIBMES_H
#define __MES_LIBMES_H

#ifndef _SIZE_T
#define _SIZE_T
#ifndef __SIZE_T
#define __SIZE_T
#ifndef __MES_SIZE_T
#define __MES_SIZE_T
#undef size_t
typedef unsigned long size_t;
#endif
#endif
#endif

#ifndef _SSIZE_T
#define _SSIZE_T
#ifndef __SSIZE_T
#define __SSIZE_T
#ifndef __MES_SSIZE_T
#define __MES_SSIZE_T
#undef ssize_t
typedef long ssize_t;
#endif
#endif
#endif

int __mes_debug ();
char const* number_to_ascii (long number, int base, int signed_p);
char const* itoa (long number);
char const* utoa (unsigned long number);
char const* itoab (long x, int base);
int _atoi (char const**, int base);
int atoi (char const *s);
int eputc (int c);
int eputs (char const* s);
int fdgetc (int fd);
int fdputc (int c, int fd);
int fdputs (char const* s, int fd);
int fdungetc (int c, int fd);
int isdigit (int c);
int isspace (int c);
int isxdigit (int c);
int oputs (char const* s);
ssize_t write (int filedes, void const *buffer, size_t size);

#endif //__MES_LIBMES_H
