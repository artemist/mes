/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2018,2022 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
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
#ifndef __MES_LINUX_AARCH64_SYSCALL_H
#define __MES_LINUX_AARCH64_SYSCALL_H 1

/* See https://github.com/torvalds/linux/blob/v6.5/include/uapi/asm-generic/unistd.h */

/* libc */
#define SYS_exit    93
#define SYS_write   64
#define SYS_clone   220
#define SYS_read    63
#define SYS_openat  56
#define SYS_wait4   260
#define SYS_execve  221
#define SYS_fchmod  52
#define SYS_faccessat 48
#define SYS_brk     234
#define SYS_ioctl   29
#define SYS_fsync   82
#define SYS_getcwd  17
#define SYS_dup     23
#define SYS_dup3    24
#define SYS_unlinkat 35
#define SYS_gettimeofday 169
#define SYS_clock_gettime 113

/* libc+tcc */
#define SYS_close  0x06
#define SYS_lseek  0x13
#define SYS_rmdir  0x28
#define SYS_stat   0x6a

/* libc+gnu */
#define SYS_chdir     0x0c
#define SYS_link      0x09
#define SYS_getpid    0x14
#define SYS_getuid    0x18
#define SYS_kill      0x25
#define SYS_rename    0x26
#define SYS_mkdir     0x27
#define SYS_pipe      0x2a
#define SYS_getgid    0x2f
#define SYS_rt_sigaction 0xae
#define SYS_rt_sigreturn 0xad
#define SYS_fcntl     0x37
#define SYS_getrusage 0x4d
#define SYS_lstat     0x6b
#define SYS_setitimer 0x68
#define SYS_fstat     0x6c
#define SYS_nanosleep 0xa2
#define SYS_getdents  0x8d

/* bash */
#define SYS_setuid    0x17
#define SYS_setgid    0x2e
#define SYS_geteuid   0x31
#define SYS_getegid   0x32
#define SYS_getppid   0x40

/* make+WITH_GLIBC */
#define SYS_rt_sigprocmask 0xaf

/* tar */
#define SYS_symlink   0x53
#define SYS_readlink  0x55
#define SYS_mknod     0x0e

#endif /* __MES_LINUX_AARCH64_SYSCALL_H */
