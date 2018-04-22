;;; Mes --- Maxwell Equations of Software
;;; Copyright © 2018 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
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

(cond-expand
 (guile
  )
 (mes
;;;;;;;;;;;;;;;
  (define (cons* . rest)
    (if (null? (cdr rest)) (car rest)
        (cons (car rest) (core:apply cons* (cdr rest) (current-module)))))

  (define (apply f h . t)
    (if (null? t) (core:apply f h (current-module))
        (apply f (apply cons* (cons h t)))))

  (define (append . rest)
    (if (null? rest) '()
        (if (null? (cdr rest)) (car rest)
            (append2 (car rest) (apply append (cdr rest))))))

  (define-macro (and . x)
    (if (null? x) #t
        (if (null? (cdr x)) (car x)
            (list (quote if) (car x) (cons (quote and) (cdr x))
                  #f))))

  (define (string->list s)
    (core:car s))

  (define <cell:string> 10)

  (define (string . lst)
    (core:make-cell <cell:string> lst 0))

  (define (map1 f lst)
    (if (null? lst) (list)
        (cons (f (car lst)) (map1 f (cdr lst)))))

  (define map map1)

  (define (string-append . rest)
    (apply string (apply append (map string->list rest))))
;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;
  (define (symbol->list s)
    (core:car s))

  (define (symbol->string s)
    (apply string (symbol->list s)))

  (define (string-join lst infix)
    (if (null? (cdr lst)) (car lst)
        (string-append (car lst) infix (string-join (cdr lst) infix))))
;;;;;;;;;;;;;;;;;;

  (define (string->symbol s)
    (core:lookup-symbol (core:car s)))

  (define-macro (load file)
    (list 'primitive-load file))

  (define (not x) (if x #f #t))

  (define (memq x lst)
    (if (null? lst) #f
        (if (eq? x (car lst)) lst
            (memq x (cdr lst)))))
  ))

(define %moduledir "./")
(primitive-load "module/mes/module.mes")
(mes-use-module (scaffold boot data bar))