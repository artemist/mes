/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2016,2017,2020,2022 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
 * Copyright © 2020 Danny Milosavljevic <dannym@scratchpost.org>
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

void
_write ()
{
  asm ("SET_X0_FROM_BP");
  asm ("SUB_X0_24");
  asm ("DEREF_X0");
  asm ("SET_X2_FROM_X0");

  asm ("SET_X0_FROM_BP");
  asm ("SUB_X0_16");
  asm ("DEREF_X0");
  asm ("SET_X1_FROM_X0");

  asm ("SET_X0_FROM_BP");
  asm ("SUB_X0_8");
  asm ("DEREF_X0");

  asm ("SET_X8_TO_SYS_WRITE");
  asm ("SYSCALL");
}
