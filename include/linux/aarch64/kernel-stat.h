/* -*-comment-start: "//";
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2017,2022 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
 *
 * This file is part of GNU Mes.
 *
 * GNU Mes is free software;
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation;
 * your option) any later version.
 *
 * GNU Mes is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY;
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Mes.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __MES_LINUX_AARCH64_KERNEL_STAT_H
#define __MES_LINUX_AARCH64_KERNEL_STAT_H 1

// https://github.com/torvalds/linux/blob/master/arch/arm/include/uapi/asm/stat.h

#include <arch/syscall.h>

// *INDENT-OFF*
struct stat
{
  unsigned long	st_dev;
  unsigned long	st_ino;
  unsigned int	st_mode;
  unsigned int	st_nlink;
  unsigned int	st_uid;
  unsigned int	st_gid;
  unsigned long	st_rdev;
  unsigned long	__pad1;
  long		st_size;
  int		st_blksize;
  int		__pad2;
  long		st_blocks;
  long		st_atime;
  unsigned long	st_atime_nsec;
  long		st_mtime;
  unsigned long	st_mtime_nsec;
  long		st_ctime;
  unsigned long	st_ctime_nsec;
  unsigned int	__unused4;
  unsigned int	__unused5;
};

#endif // __MES_LINUX_AARCH64_KERNEL_STAT_H
