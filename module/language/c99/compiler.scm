;;; -*-scheme-*-

;;; Mes --- Maxwell Equations of Software
;;; Copyright © 2016,2017 Jan Nieuwenhuizen <janneke@gnu.org>
;;;
;;; This file is part of Mes.
;;;
;;; Mes is free software; you can redistribute it and/or modify it
;;; under the terms of the GNU General Public License as published by
;;; the Free Software Foundation; either version 3 of the License, or (at
;;; your option) any later version.
;;;
;;; Mes is distributed in the hope that it will be useful, but
;;; WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;
;;; You should have received a copy of the GNU General Public License
;;; along with Mes.  If not, see <http://www.gnu.org/licenses/>.

;;; Commentary:

;;; Code:

(define-module (language c99 compiler)
  #:use-module (srfi srfi-1)
  #:use-module (srfi srfi-26)
  #:use-module (system base pmatch)
  #:use-module (ice-9 optargs)
  #:use-module (ice-9 pretty-print)
  #:use-module (mes elf-util)
  #:use-module (mes elf)
  #:use-module (mes as-i386)
  #:use-module (mes hex2)
  #:use-module (nyacc lang c99 parser)
  #:use-module (nyacc lang c99 pprint)
  #:export (c99-ast->info
            c99-input->ast
            c99-input->elf
            c99-input->info
            c99-input->object
            info->object))

(cond-expand
 (guile-2)
 (guile
  (use-modules (ice-9 syncase)))
 (mes))

(include-from-path "language/c99/compiler.mes")
