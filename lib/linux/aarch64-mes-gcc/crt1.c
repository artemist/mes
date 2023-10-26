/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2017,2018,2019,2020 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
 * Copyright © 2019,2020 Danny Milosavljevic <dannym@scratchpost.org>
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

#include <mes/lib-mini.h>
//int main (int argc, char *argv[], char *envp[]);

/* Note: GCC may or may not omit the frame pointer,
 * depending on whether it thinks this is a leaf function.
 */
// *INDENT-OFF*
#if !__TINYC__
void
_start ()
{
  asm (
       "mov    w0,#0\n\t"
       "mov    %0,w0\n"
       : "=r" (__stdin)
       : //no inputs ""
       );

  asm (
       "mov    w0,#1\n\t"
       "mov    %0,w0\n"
       : "=r" (__stdout)
       : //no inputs ""
       );

  asm (
       "mov    w0,#2\n\t"
       "mov    %0,w0\n"
       : "=r" (__stderr)
       : //no inputs ""
       );

  /* environ = argv + argc + 1 */
  asm (
       "ldr     x0,[sp]\n\t" /* x0 = argc */
       "add     x1,sp,#8\n\t" /* x1 = &argv[0] */
       "add     x2,x0,#1\n\t" /* x2 = argc + 1 (skip zero word after argv) */
       "lsl     x2,#3\n\t" /* x2 = (argc + 1) << 3 */
       "add     x2,x2,x1\n\t" /* x2 = ((argc + 1) << 3) + &argv[0] */
       "stp     x1,x2,[sp,#0x10]!\n\t" /* argv, envp */
       "stp     xzr,x0,[sp,#0x10]!\n\t" /* 0, argc */
       "mov     %0,x2\n\t"
       : "=r" (environ)
       : //no inputs ""
       );
  asm (
       "ldp     xzr,x0,[sp],#0x10\n\t" /* 0, argc */
       "ldp     x1,x2,[sp],#0x10\n\t" /* argv, envp */
       "bl      main\n\t"
       "mov     x7, #93\n\t"
       "svc     #0\n\t"
       "wfi     \n\t"
       );
}
#else //__TINYC__
void
_start ()
{
#if 0
  __asm__ (".int 0xe320f000\n"); //nop   {0}
  __asm__ (".int 0xe320f000\n"); //nop   {0}
#endif

  int *in = &__stdin;
  int *out = &__stdout;
  int *err = &__stderr;
  char ***env = &environ;

#if 0
  __asm__ (".int 0xe320f000\n"); //nop   {0}
  __asm__ (".int 0xe320f000\n"); //nop   {0}

  __asm__ (".int 0xe320f000\n"); //nop   {0}
  *in = 0x22;
  __asm__ (".int 0xe320f000\n"); //nop   {0}
  *out = 0x33;
  __asm__ (".int 0xe320f000\n"); //nop   {0}
  *err = 0x44;
  __asm__ (".int 0xe320f000\n"); //nop   {0}
  *env = 0x55;
  __asm__ (".int 0xe320f000\n"); //nop   {0}
#endif

  /* environ = argv + argc + 1 */
  __asm__ (".int 0xe59b000c\n"); //ldr   r0, [fp, #12]
  __asm__ (".int 0xe28b1010\n"); //add   r1, fp, #16

  __asm__ (".int 0xe2802001\n"); //add   r2, r0, #1
  __asm__ (".int 0xe1a02102\n"); //lsl   r2, r2, #2
  __asm__ (".int 0xe0822001\n"); //add   r2, r2, r1

  // setup argc, argv, envp parameters on stack
  __asm__ (".int 0xe52d2004\n"); //push  {r2}      ; (str r2, [sp, #-4]!)
  __asm__ (".int 0xe52d1004\n"); //push  {r1}      ; (str r1, [sp, #-4]!)
  __asm__ (".int 0xe52d0004\n"); //push  {r0}      ; (str r0, [sp, #-4]!)
  __asm__ (".int 0xe1a02002\n"); //mov   r2, r2

  // *in = 0;
  __asm__ (".int 0xe3a01000\n"); //mov   r1, #0
  __asm__ (".int 0xe51b0004\n"); //ldr   r0, [fp, #-4]
  __asm__ (".int 0xe5801000\n"); //str   r1, [r0]
  __asm__ (".int 0xe320f000\n"); //nop   {0}

  // *out = 1;
  __asm__ (".int 0xe3a01001\n"); //mov   r1, #1
  __asm__ (".int 0xe51b0008\n"); //ldr   r0, [fp, #-8]
  __asm__ (".int 0xe5801000\n"); //str   r1, [r0]
  __asm__ (".int 0xe320f000\n"); //nop   {0}

  // *err = 2;
  __asm__ (".int 0xe3a01002\n"); //mov   r1, #2
  __asm__ (".int 0xe51b000c\n"); //ldr   r0, [fp, #-12]
  __asm__ (".int 0xe5801000\n"); //str   r1, [r0]
  __asm__ (".int 0xe320f000\n"); //nop   {0}

  // *env = [sp, #8]
  __asm__ (".int 0xe59d1008\n"); //ldr   r1, [sp, #8]
  __asm__ (".int 0xe51b0010\n"); //ldr   r0, [fp, #-16]
  __asm__ (".int 0xe5801000\n"); //str   r1, [r0]
  __asm__ (".int 0xe320f000\n"); //nop   {0}

  // setup argc, argv, envp parameters in registers
  __asm__ (".int 0xe59d0000\n"); //ldr   r0, [sp]
  __asm__ (".int 0xe59d1004\n"); //ldr   r1, [sp, #4]
  __asm__ (".int 0xe59d2008\n"); //ldr   r2, [sp, #8]
  main ();

  __asm__ (".int 0xe3a07001\n"); //mov   r7, #1
  __asm__ (".int 0xef000000\n"); //svc   0x00000000
  __asm__ (".int 0xe320f003\n"); //wfi
  __asm__ (".int 0xe320f000\n"); //nop   {0}
}
#endif //__TINYC__
