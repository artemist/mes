#! /bin/sh
# -*-scheme-*-
GODIR=${GODIR-@GODIR@}
GUILEDIR=${GUILEDIR-@GUILEDIR@}
[ "$GODIR" = @"GODIR"@ ] && GODIR=$(dirname $0)
[ "$GUILEDIR" = @"GUILEDIR"@ ] && GUILEDIR=$(dirname $0)
export GUILE_AUTO_COMPILE=${GUILE_AUTO_COMPILE-0}
exec ${GUILE-guile} -L $GUILEDIR -C $GODIR -e '(mescc)' -s "$0" "$@"
!#

;;; Mes --- The Maxwell Equations of Software
;;; Copyright © 2016,2017 Jan Nieuwenhuizen <janneke@gnu.org>
;;;
;;; This file is part of GNU Guix.
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

;; The Maxwell Equations of Software -- John McCarthy page 13
;; http://www.softwarepreservation.org/projects/LISP/book/LISP%201.5%20Programmers%20Manual.pdf

#!
Run with Guile-1.8:
GUILE='~/src/guile-1.8/build/pre-inst-guile --debug -q' guile/mescc.scm
!#

(define-module (mescc)
  #:use-module (language c99 compiler)
  #:use-module (ice-9 getopt-long)
  #:use-module (ice-9 pretty-print)
  #:use-module (srfi srfi-1)
  #:use-module (srfi srfi-26)
  #:export (main))

(define %prefix (if (string-prefix? "@PREFIX" "@PREFIX@") "" "@PREFIX@"))
(define %datadir (if (string-prefix? "@DATADIR" "@DATADIR@") "" "@DATADIR@"))
(define %docdir (if (string-prefix? "@DOCDIR" "@DOCDIR@") "doc/" "@DOCDIR@"))
(define %moduledir "module/")
(define %version (if (string-prefix? "@VERSION" "@VERSION@") "git" "@VERSION@"))
(module-define! (resolve-module '(language c99 compiler)) '%datadir %datadir)
(module-define! (resolve-module '(language c99 compiler)) '%docdir %docdir)
(module-define! (resolve-module '(language c99 compiler)) '%moduledir %moduledir)
(module-define! (resolve-module '(language c99 compiler)) '%prefix %prefix)
(module-define! (resolve-module '(language c99 compiler)) '%version %version)

(define (parse-opts args)
  (let* ((option-spec
          '((c (single-char #\c))
            (D (single-char #\D) (value #t))
            (E (single-char #\E))
            (help (single-char #\h))
            (I (single-char #\I) (value #t))
            (o (single-char #\o) (value #t))
            (version (single-char #\V) (value #t))))
         (options (getopt-long args option-spec))
         (help? (option-ref options 'help #f))
         (files (option-ref options '() '()))
         (usage? (and (not help?) (null? files)))
         (version? (option-ref options 'version #f)))
    (or
     (and version?
          (format (current-output-port) "mescc.scm (mes) ~a\n" %version))
     (and (or help? usage?)
          (format (or (and usage? (current-error-port)) (current-output-port)) "\
Usage: mescc [OPTION]... FILE...
  -c                 compile and assemble, but do not link
  -D DEFINE          define DEFINE
  -E                 preprocess only; do not compile, assemble or link
  -h, --help         display this help and exit
  -I DIR             append DIR to include path
  -o FILE            write output to FILE
  -v, --version      display version and exit
")
          (exit (or (and usage? 2) 0)))
     options)))

(define (object->info file)
  (let* ((lst (with-input-from-file file read))
         (module (resolve-module '(language c99 compiler))))
    (eval lst module)))

(define (main:ast->info file)
  (let ((ast (with-input-from-file file read)))
    (with-input-from-file file
      (lambda ()
        (c99-ast->info ast)))))

(define (source->ast defines includes)
  (lambda (file)
    (with-input-from-file file
      (lambda ()
        (pretty-print (c99-input->ast #:defines defines #:includes includes))))))

(define (source->info defines includes)
  (lambda (file)
    (with-input-from-file file
      (lambda ()
        ((c99-input->info #:defines defines #:includes includes))))))

(define (ast? o)
  (or (string-suffix? ".E" o)
      (string-suffix? ".guile-E" o)))

(define (object? o)
  (or (string-suffix? ".o" o)
      (string-suffix? ".guile-o" o)))

(define (main args)
  (let* ((options (parse-opts args))
         (files (option-ref options '() '()))
         (file (if (null? files) (string-append %docdir "examples/main.c")
                   (car files)))
         (preprocess? (option-ref options 'E #f))
         (compile? (option-ref options 'c #f))
         (asts (filter ast? files))
         (objects (filter object? files))
         (sources (filter (cut string-suffix? ".c" <>) files))
         (base (substring file (1+ (or (string-rindex file #\/) -1)) (- (string-length file) 2)))
         (out (option-ref options 'o (cond (compile? (string-append base ".o"))
                                           (preprocess? (string-append base ".E"))
                                           (else "a.out"))))
         (multi-opt (lambda (option) (lambda (o) (and (eq? (car o) option) (cdr o)))))
         (defines (reverse (filter-map (multi-opt 'D) options)))
         (includes (reverse (filter-map (multi-opt 'I) options))))
    (when (getenv "MES_DEBUG") (format (current-error-port) "options=~s\n" options)
          (format (current-error-port) "output: ~a\n" out))
    (if (and (pair? sources) (pair? objects)) (error "cannot mix source and object files:" files))
    (with-output-to-file out
      (lambda ()
        (if (and (not compile?)
                 (not preprocess?)) (set-port-encoding! (current-output-port) "ISO-8859-1"))
        (cond ((pair? objects) (let ((infos (map object->info objects)))
                                 (if compile? (infos->object infos)
                                     (infos->elf infos))))
              ((pair? asts) (let ((infos (map main:ast->info asts)))
                              (if compile? (infos->object infos)
                                  (infos->elf infos))))
              ((pair? sources) (if preprocess? (map (source->ast defines includes) sources)
                                   (let ((infos (map (source->info defines includes) sources)))
                                     (if compile? (infos->object infos)
                                         (infos->elf infos))))))))
    (if (and (not compile?)
             (not preprocess?))
        (chmod out #o755))))
