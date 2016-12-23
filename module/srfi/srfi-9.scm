;;; -*-scheme-*-

;;; Mes --- Maxwell Equations of Software
;;; Copyright (c) 1993-2004 by Richard Kelsey and Jonathan Rees.
;;; Copyright © 2016 Jan Nieuwenhuizen <janneke@gnu.org>
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

;;; srfi-9.mes - records.  Assumes record-0.mes and record.mes are
;;; available.  Modified from
;;; scheme48-1.1/scheme/alt/jar-defrecord.scm to implement SRFI-9.

;;; Code:

;;; Copyright (c) 1993-2004 by Richard Kelsey and Jonathan Rees. See file COPYING.

;;; scheme48-1.1/COPYING

;; Copyright (c) 1993-2004 Richard Kelsey and Jonathan Rees
;; All rights reserved.

;; Redistribution and use in source and binary forms, with or without
;; modification, are permitted provided that the following conditions
;; are met:
;; 1. Redistributions of source code must retain the above copyright
;;    notice, this list of conditions and the following disclaimer.
;; 2. Redistributions in binary form must reproduce the above copyright
;;    notice, this list of conditions and the following disclaimer in the
;;    documentation and/or other materials provided with the distribution.
;; 3. The name of the authors may not be used to endorse or promote products
;;    derived from this software without specific prior written permission.

;; THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
;; IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
;; OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
;; IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
;; INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
;; NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
;; DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
;; THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
;; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
;; THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

; This is JAR's define-record-type, which doesn't resemble Richard's.

; There's no implicit name concatenation, so it can be defined
; entirely using syntax-rules.  Example:
;  (define-record-type foo :foo
;    (make-foo x y)
;    foo?              - predicate name is optional
;    (x foo-x)
;    (y foo-y)
;    (z foo-z set-foo-z!))

(define-syntax define-record-type
  (syntax-rules ()
    ((define-record-type type
       (constructor arg ...)
       (field . field-stuff)
       ...)
     (begin (define type (make-record-type 'type '(field ...)))
            (define constructor (record-constructor type '(arg ...)))
            (define-accessors type (field . field-stuff) ...)))
    ((define-record-type type
       (constructor arg ...)
       pred
       more ...)
     (begin (define-record-type type
              (constructor arg ...)
              more ...)
            (define pred (record-predicate type))))))

;; Straightforward version
(define-syntax define-accessors
  (syntax-rules ()
    ((define-accessors type field-spec ...)
     (begin (define-accessor type . field-spec) ...))))

(define-syntax define-accessor
  (syntax-rules ()
    ((define-accessor type field accessor)
     (define accessor (record-accessor type 'field)))
    ((define-accessor type field accessor modifier)
     (begin (define accessor (record-accessor type 'field))
            (define modifier (record-modifier type 'field))))))