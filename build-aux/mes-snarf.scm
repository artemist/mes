#! /bin/sh
# -*-scheme-*-
exec ${GUILE-guile} -L $(dirname 0) -e '(mes-snarf)' -s "$0" "$@"
!#

;;; Mes --- Maxwell Equations of Software
;;; Copyright © 2016,2017,2018 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
;;;
;;; mes-snarf.scm: This file is part of Mes.
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

(define-module (mes-snarf)
  #:use-module (srfi srfi-1)
  #:use-module (srfi srfi-8)
  #:use-module (srfi srfi-9)
  #:use-module (srfi srfi-26)
  #:use-module (ice-9 rdelim)
  #:export (main))

(cond-expand
 (mes
  (define %scheme "mes"))
 (guile-2
  (define %scheme "guile")
  (define-macro (mes-use-module . rest) #t))
 (guile
  (use-modules (ice-9 syncase))
  (define %scheme "guile")
  (define-macro (mes-use-module . rest) #t)))

(mes-use-module (mes guile))
(mes-use-module (srfi srfi-1))
(mes-use-module (srfi srfi-8))
(mes-use-module (srfi srfi-9))
(mes-use-module (srfi srfi-26))

(format (current-error-port) "mes-snarf[~a]...\n" %scheme)

(define (char->char from to char)
  (if (eq? char from) to char))

(define (string-replace-char string from to)
  (string-map (cut char->char from to <>) string))

(define (string-replace-suffix string from to)
  (if (string-suffix? from string)
      (string-replace string to (- (string-length string) (string-length from)))
      string))

(define (string-replace-string string from to)
  (cond ((string-contains string from) => (lambda (i) (string-replace string to i (+ i (string-length from)))))
        (else string)))

(define %gcc? #t)

(define-record-type file (make-file name content)
  file?
  (name file.name)
  (content file.content))

(define-record-type function (make-function name formals annotation)
  function?
  (name function.name)
  (formals function.formals)
  (annotation function.annotation))

(define (function-scm-name f)
  (or (assoc-ref (function.annotation f) 'name)
      (let ((name ((compose
                    identity
                    (cut string-replace-char <> #\_ #\-)
                    (cut string-replace-string <> "_to_" "->")
                    (cut string-replace-suffix <> "_x" "!")
                    (cut string-replace-suffix <> "_x_" "!-")
                    (cut string-replace-suffix <> "_p" "?")
                    )
                   (function.name f))))
        (if (not (string-suffix? "-" name)) name
            (string-append "core:" (string-drop-right name 1))))))

(define %builtin-prefix% "scm_")
(define (function-builtin-name f)
  (string-append %builtin-prefix% (function.name f)))

(define %cell-prefix% "cell_")
(define (function-cell-name f)
  (string-append %cell-prefix% (function.name f)))

(define %start 1)
(define (symbol->header s i)
  (format #f "#define cell_~a ~a\n" s i))

(define (symbol->source s i)
  (string-append
   (format #f "g_free++;\n")
   (format #f "g_cells[cell_~a] = scm_~a;\n\n" s s)))

(define (symbol->names s i)
  (if %gcc?
      (format #f "g_cells[cell_~a].car = cstring_to_list (scm_~a.name);\n" s s)
      (format #f "g_cells[cell_~a].car = cstring_to_list (scm_~a.car);\n" s s)))

(define (function->header f i)
  (let* ((arity (or (assoc-ref (function.annotation f) 'arity)
                    (if (string-null? (function.formals f)) 0
                        (length (string-split (function.formals f) #\,)))))
         (n (if (eq? arity 'n) -1 arity)))
    (string-append
     (format #f "SCM ~a (~a);\n" (function.name f) (function.formals f))
     (if %gcc?
         (format #f "struct function fun_~a = {.function~a=&~a, .arity=~a, .name=~s};\n" (function.name f) arity (function.name f) n (function-scm-name f))
         (format #f "struct function fun_~a = {&~a, ~a, ~s};\n" (function.name f) (function.name f) n (function-scm-name f)))
     (if %gcc?
         (format #f "struct scm ~a = {TFUNCTION, .name=0, .function=0};\n" (function-builtin-name f))
         (format #f "struct scm ~a = {TFUNCTION, 0, 0};\n" (function-builtin-name f)))
     (format #f "SCM cell_~a;\n\n" (function.name f)))))

(define (function->source f i)
  (string-append
   (if %gcc?
       (format #f "~a.function = g_function;\n" (function-builtin-name f))
       (format #f "~a.cdr = g_function;\n" (function-builtin-name f)))
   (format #f "g_functions[g_function++] = fun_~a;\n" (function.name f))
   (format #f "cell_~a = g_free++;\n" (function.name f))
   (format #f "g_cells[cell_~a] = ~a;\n\n" (function.name f) (function-builtin-name f))))

(define (function->environment f i)
  (string-append
   (if %gcc?
       (format #f "scm_~a.string = cstring_to_list (fun_~a.name);\n" (function.name f) (function.name f))
       (format #f "scm_~a.car = cstring_to_list (fun_~a.name);\n" (function.name f) (function.name f)))
   (if %gcc?
       (format #f "g_cells[cell_~a].string = MAKE_STRING (scm_~a.string);\n" (function.name f) (function.name f))
       (format #f "g_cells[cell_~a].car = MAKE_STRING (scm_~a.car);\n" (function.name f) (function.name f)))
   (if %gcc?
       (format #f "a = acons (lookup_symbol_ (scm_~a.string), ~a, a);\n\n" (function.name f) (function-cell-name f))
       (format #f "a = acons (lookup_symbol_ (scm_~a.car), ~a, a);\n\n" (function.name f) (function-cell-name f)))))

(define (disjoin . predicates)
  (lambda (. arguments)
    (any (cut apply <> arguments) predicates)))

(define (snarf-symbols string)
  (let* ((lines (string-split string #\newline))
         (scm (filter (cut string-prefix? "struct scm scm_" <>) lines))
         (symbols (filter (disjoin (cut string-contains <> "TSPECIAL") (cut string-contains <> "TSYMBOL")) scm)))
    (define (line->symbol line)
      ((compose
        (lambda (s) (string-take s (string-index s #\space)))
        (cut string-drop <> (string-length "struct scm scm_")))
       line))
    (map line->symbol symbols)))

(define (string-split-string string sep)
  (cond ((string-contains string sep) => (lambda (i) (list (string-take string i) (string-drop string (+ i (string-length sep))))))
        (else (list string #f))))

(define (snarf-functions string)
  (let ((lines (string-split string #\newline)))
    (filter-map
     (lambda (line previous)
       (receive (function rest)
           (apply values (string-split-string line " "))
         (and function
              (equal? (string-trim previous) "SCM")
              (not (string-null? function))
              (not (string-prefix? "#" function))
              (not (string-prefix? "/" function))
              rest
              (receive (parameter-list annotation)
                  (apply values (string-split-string rest " ///"))
                (let* ((parameters (string-drop parameter-list 1))
                       (parameters (string-drop-right parameters 1))
                       (formals (if (string-null? parameters) '()
                                    (string-split parameters #\,)))
                       (formals (map string-trim formals)))
                  (and parameters
                       (let* ((non-SCM (filter (negate (cut string-prefix? "SCM" <>)) formals)))
                         (and (null? non-SCM)
                              (let ((annotation (and annotation (with-input-from-string annotation read))))
                                (make-function function parameters annotation))))))))))
     lines (cons "\n" lines))))

(define (content? f)
  ((compose not string-null? file.content) f))

(define (internal? f)
  ((compose (cut assoc-ref <> 'internal) function.annotation) f))

(define (no-environment? f)
  ((compose (cut assoc-ref <> 'no-environment) function.annotation) f))

(define (generate-includes file-name)
  (let* ((string (with-input-from-file file-name read-string))
         (functions (snarf-functions string))
         (functions (delete-duplicates functions (lambda (a b) (equal? (function.name a) (function.name b)))))
         (functions (filter (negate internal?) functions))
         (symbols (snarf-symbols string))
         (base-name (basename file-name ".c"))
         (dir (or (getenv "OUT") (dirname file-name)))
         (base-name (string-append dir "/" base-name))
         (base-name (if %gcc? base-name
                        (string-append base-name ".mes")))
         (header (make-file
                  (string-append base-name ".h")
                  (string-join (map function->header functions (iota (length functions) (+ %start (length symbols)))) "")))
         (source (make-file
                  (string-append base-name ".i")
                  (string-join (map function->source (filter (negate no-environment?) functions) (iota (length functions) (+ (length symbols) %start))) ""))) 
         (environment (make-file
                       (string-append base-name ".environment.i")
                       (string-join (map function->environment (filter (negate no-environment?) functions) (iota (length functions) (+ (length symbols) %start))) "")))
         (symbols.h (make-file
                     (string-append base-name ".symbols.h")
                     (string-join (map symbol->header symbols (iota (length symbols) %start)) "")))
         (symbols.i (make-file
                     (string-append base-name ".symbols.i")
                     (string-join (map symbol->source symbols (iota (length symbols))) "")))
         (symbol-names.i (make-file
                          (string-append base-name ".symbol-names.i")
                          (string-join (map symbol->names symbols (iota (length symbols))) ""))))
    (list header source environment symbols.h symbols.i symbol-names.i)))

(define (file-write file)
  (with-output-to-file (file.name file) (lambda () (display (file.content file)))))

(define (main args)
  (let* ((files (if (not (and (pair? (cdr args)) (equal? (cadr args) "--mes"))) (cdr args)
                    (begin (set! %gcc? #f)
                           (cddr args))))
         (files (append-map generate-includes files)))
    (map file-write (filter content? files))))
