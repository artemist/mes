;;; Mes --- Maxwell Equations of Software
;;; Copyright © 2016,2017,2018 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
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

(define-module (mescc compile)
  #:use-module (srfi srfi-1)
  #:use-module (srfi srfi-9 gnu)
  #:use-module (srfi srfi-26)
  #:use-module (system base pmatch)
  #:use-module (ice-9 optargs)
  #:use-module (ice-9 pretty-print)
  #:use-module (nyacc lang c99 pprint)

  #:use-module (mes guile)
  #:use-module (mes misc)

  #:use-module (mescc preprocess)
  #:use-module (mescc info)
  #:use-module (mescc as)
  #:use-module (mescc i386 as)
  #:use-module (mescc M1)
  #:export (c99-ast->info
            c99-input->info
            c99-input->object))

(define mes? (pair? (current-module)))

(define* (c99-input->info #:key (prefix "") (defines '()) (includes '()))
  (let ((ast (c99-input->ast #:prefix prefix #:defines defines #:includes includes)))
    (c99-ast->info ast)))

(define* (c99-ast->info o)
  (stderr "compiling: input\n")
  (let ((info (ast->info o (make <info> #:types i386:type-alist))))
    (clean-info info)))

(define (clean-info o)
  (make <info>
    #:functions (filter (compose pair? function:text cdr) (.functions o))
    #:globals (.globals o)))

(define %int-size 4)
(define %pointer-size %int-size)

(define (ident->constant name value)
  (cons name value))

(define (enum->type-entry name fields)
  (cons `(tag ,name) (make-type 'enum 4 fields)))

(define (struct->type-entry name fields)
  (let ((size (apply + (map (compose ->size cdr) fields))))
    (cons `(tag ,name) (make-type 'struct size fields))))

(define (union->type-entry name fields)
  (let ((size (apply max (map (compose ->size cdr) fields))))
    (cons `(tag ,name) (make-type 'union size fields))))

(define i386:type-alist
  `(("char" . ,(make-type 'signed 1 #f))
    ("short" . ,(make-type 'signed 2 #f))
    ("int" . ,(make-type 'signed 4 #f))
    ("long" . ,(make-type 'signed 4 #f))
    ("default" . ,(make-type 'signed 4 #f))
    ;;("long long" . ,(make-type 'signed 8 #f))
    ;;("long long int" . ,(make-type 'signed 8 #f))

    ("long long" . ,(make-type 'signed 4 #f))  ;; FIXME
    ("long long int" . ,(make-type 'signed 4 #f))

    ("void" . ,(make-type 'void 1 #f))
    ;; FIXME sign
    ("unsigned char" . ,(make-type 'unsigned 1 #f))
    ("unsigned short" . ,(make-type 'unsigned 2 #f))
    ("unsigned" . ,(make-type 'unsigned 4 #f))
    ("unsigned int" . ,(make-type 'unsigned 4 #f))
    ("unsigned long" . ,(make-type 'unsigned 4 #f))

    ;; ("unsigned long long" . ,(make-type 'builtin 8 #f))
    ;; ("unsigned long long int" . ,(make-type 'builtin 8 #f))
    ("unsigned long long" . ,(make-type 'unsigned 4 #f)) ;; FIXME
    ("unsigned long long int" . ,(make-type 'unsigned 4 #f))

    ("float" . ,(make-type 'float 4 #f))
    ("double" . ,(make-type 'float 8 #f))
    ("long double" . ,(make-type 'float 16 #f))

    ;;
    ("short int" . ,(make-type 'signed 2 #f))
    ("unsigned short int" . ,(make-type 'unsigned 2 #f))
    ("long int" . ,(make-type 'signed 4 #f))
    ("unsigned long int" . ,(make-type 'unsigned 4 #f))))

(define (signed? o)
  (eq? ((compose type:type ->type) o) 'signed))

(define (unsigned? o)
  (eq? ((compose type:type ->type) o) 'unsigned))

(define (->size o)
  (cond ((and (type? o) (eq? (type:type o) 'union))
         (apply max (map (compose ->size cdr) (struct->fields o))))
        ((type? o) (type:size o))
        ((pointer? o) %pointer-size)
        ((c-array? o) (* (c-array:count o) ((compose ->size c-array:type) o)))
        ((local? o) ((compose ->size local:type) o))
        ((global? o) ((compose ->size global:type) o))
        ((bit-field? o) ((compose ->size bit-field:type) o))
        ((and (pair? o) (pair? (car o)) (bit-field? (cdar o))) ((compose ->size cdar) o))
        ;; FIXME
        ;; (#t
        ;;  (stderr "o=~s\n" o)
        ;;  (format (current-error-port) "->size: not a <type>: ~s\n" o)
        ;;  4)
        (else (error "->size>: not a <type>:" o))))

(define (ast->type o info)
  (define (type-helper o info)
    (if (getenv "MESC_DEBUG")
        (stderr "type-helper: ~s\n" o))
    (pmatch o
      (,t (guard (type? t)) t)
      (,p (guard (pointer? p)) p)
      (,a (guard (c-array? a)) a)
      (,b (guard (bit-field? b)) b)

      ((char ,value) (get-type "char" info))
      ((enum-ref . _) (get-type "default" info))
      ((fixed ,value) (get-type "default" info))
      ((float ,float) (get-type "float" info))
      ((void) (get-type "void" info))

      ((ident ,name) (ident->type info name))
      ((tag ,name) (or (get-type o info)
                       o))

      (,name (guard (string? name))
             (let ((type (get-type name info)))
               (ast->type type info)))

      ((type-name (decl-spec-list ,type) (abs-declr (pointer . ,pointer)))
       (let ((rank (pointer->rank `(pointer ,@pointer)))
             (type (ast->type type info)))
         (rank+= type rank)))

      ((type-name ,type) (ast->type type info))
      ((type-spec ,type) (ast->type type info))

      ((sizeof-expr ,expr) (ast->type expr info))
      ((sizeof-type ,type) (ast->type type info))

      ((string ,string) (make-c-array (get-type "char" info) (1+ (string-length string))))

      ((decl-spec-list (type-spec ,type)) (ast->type type info))

      ((fctn-call (p-expr (ident ,name)) . _)
       (or (and=> (assoc-ref (.functions info) name) function:type)
           (get-type "default" info)))

      ((fctn-call (de-ref (p-expr (ident ,name))) . _)
       (or (and=> (assoc-ref (.functions info) name) function:type)
           (get-type "default" info)))

      ((fixed-type ,type) (ast->type type info))
      ((float-type ,type) (ast->type type info))
      ((type-spec ,type) (ast->type type info))
      ((typename ,type) (ast->type type info))

      ((array-ref ,index ,array) (rank-- (ast->type array info)))

      ((de-ref ,expr) (rank-- (ast->type expr info)))
      ((ref-to ,expr) (rank++ (ast->type expr info)))

      ((p-expr ,expr) (ast->type expr info))
      ((pre-inc ,expr) (ast->type expr info))
      ((post-inc ,expr) (ast->type expr info))

      ((struct-ref (ident ,type))
       (or (get-type type info)
           (let ((struct (if (pair? type) type `(tag ,type))))
             (ast->type struct info))))
      ((union-ref (ident ,type))
       (or (get-type type info)
           (let ((struct (if (pair? type) type `(tag ,type))))
             (ast->type struct info))))

      ((struct-def (ident ,name) . _)
       (ast->type `(tag ,name) info))
      ((union-def (ident ,name) . _)
       (ast->type `(tag ,name) info))
      ((struct-def (field-list . ,fields))
       (let ((fields (append-map (struct-field info) fields)))
         (make-type 'struct (apply + (map field:size fields)) fields)))
      ((union-def (field-list . ,fields))
       (let ((fields (append-map (struct-field info) fields)))
         (make-type 'union (apply + (map field:size fields)) fields)))
      ((enum-def (enum-def-list . ,fields))
       (get-type "default" info))

      ((d-sel (ident ,field) ,struct)
       (let ((type0 (ast->type struct info)))
         (ast->type (field-type info type0 field) info)))

      ((i-sel (ident ,field) ,struct)
       (let ((type0 (ast->type (rank-- (ast->type struct info)) info)))
         (ast->type (field-type info type0 field) info)))

      ;; arithmetic
      ((pre-inc ,a) (ast->type a info))
      ((pre-dec ,a) (ast->type a info))
      ((post-inc ,a) (ast->type a info))
      ((post-dec ,a) (ast->type a info))
      ((add ,a ,b) (ast->type a info))
      ((sub ,a ,b) (ast->type a info))
      ((bitwise-and ,a ,b) (ast->type a info))
      ((bitwise-not ,a) (ast->type a info))
      ((bitwise-or ,a ,b) (ast->type a info))
      ((bitwise-xor ,a ,b) (ast->type a info))
      ((lshift ,a ,b) (ast->type a info))
      ((rshift ,a ,b) (ast->type a info))
      ((div ,a ,b) (ast->type a info))
      ((mod ,a ,b) (ast->type a info))
      ((mul ,a ,b) (ast->type a info))
      ((not ,a) (ast->type a info))
      ((neg ,a) (ast->type a info))
      ((eq ,a ,b) (ast->type a info))
      ((ge ,a ,b) (ast->type a info))
      ((gt ,a ,b) (ast->type a info))
      ((ne ,a ,b) (ast->type a info))
      ((le ,a ,b) (ast->type a info))
      ((lt ,a ,b) (ast->type a info))

      ;; logical
      ((or ,a ,b) (ast->type a info))
      ((and ,a ,b) (ast->type a info))

      ((cast (type-name ,type) ,expr) (ast->type type info))

      ((cast (type-name ,type (abs-declr ,pointer)) ,expr)
       (let ((rank (pointer->rank pointer)))
         (rank+= (ast->type type info) rank)))

      ((decl-spec-list (type-spec ,type)) (ast->type type info))

      ;;  ;; `typedef int size; void foo (unsigned size u)
      ((decl-spec-list (type-spec ,type) (type-spec ,type2))
       (ast->type type info))

      ((assn-expr ,a ,op ,b) (ast->type a info))

      ((cond-expr _ ,a ,b) (ast->type a info))

      (_ (get-type o info))))

  (let ((type (type-helper o info)))
    (cond ((or (type? type)
               (pointer? type) type
               (c-array? type)) type)
          ((and (equal? type o) (pair? type) (eq? (car type) 'tag)) o)
          ((equal? type o)
           (error "ast->type: not supported: " o))
          (else (ast->type type info)))))

(define (ast->basic-type o info)
  (let ((type (->type (ast->type o info))))
    (cond ((type? type) type)
          ((equal? type o) o)
          (else (ast->type type info)))))

(define (get-type o info)
  (let ((t (assoc-ref (.types info) o)))
    (pmatch t
      ((typedef ,next) (or (get-type next info) o))
      (_ t))))


(define (ast-type->size info o)
  (let ((type (->type (ast->type o info))))
    (cond ((type? type) (type:size type))
          (else (stderr "ast-type->size barf: ~s => ~s\n" o type)
                4))))

(define (field:name o)
  (pmatch o
    ((struct (,name ,type ,size ,pointer) . ,rest) name)
    ((union (,name ,type ,size ,pointer) . ,rest) name)
    ((,name . ,type) name)
    (_ (error "field:name not supported:" o))))

(define (field:pointer o)
  (pmatch o
    ((struct (,name ,type ,size ,pointer) . ,rest) pointer)
    ((union (,name ,type ,size ,pointer) . ,rest) pointer)
    ((,name . ,type) (->rank type))
    (_ (error "field:pointer not supported:" o))))

(define (field:size o)
  (pmatch o
    ((struct . ,type) (apply + (map field:size (struct->fields type))))
    ((union . ,type) (apply max (map field:size (struct->fields type))))
    ((,name . ,type) (->size type))
    (_ (error (format #f "field:size: ~s\n" o)))))

(define (field-field info struct field)
  (let ((fields (type:description struct)))
    (let loop ((fields fields))
      (if (null? fields) (error (format #f "no such field: ~a in ~s" field struct))
          (let ((f (car fields)))
            (cond ((equal? (car f) field) f)
                  ((and (memq (car f) '(struct union)) (type? (cdr f))
                        (find (lambda (x) (equal? (car x) field)) (struct->fields (cdr f)))))
                  ((eq? (car f) 'bits) (assoc field (cdr f)))
                  (else (loop (cdr fields)))))))))

(define (field-offset info struct field)
  (if (eq? (type:type struct) 'union) 0
      (let ((fields (type:description struct)))
        (let loop ((fields fields) (offset 0))
          (if (null? fields) (error (format #f "no such field: ~a in ~s" field struct))
              (let ((f (car fields)))
                (cond ((equal? (car f) field) offset)
                      ((and (eq? (car f) 'struct) (type? (cdr f)))
                       (let ((fields (type:description (cdr f))))
                         (find (lambda (x) (equal? (car x) field)) fields)
                         (apply + (cons offset
                                        (map field:size
                                             (member field (reverse fields)
                                                     (lambda (a b)
                                                       (equal? a (car b) field))))))))
                      ((and (eq? (car f) 'union) (type? (cdr f))
                            (let ((fields (struct->fields (cdr f))))
                              (and (find (lambda (x) (equal? (car x) field)) fields)
                                   offset))))
                      ((and (eq? (car f) 'bits) (assoc-ref (cdr f) field)) offset)
                      (else (loop (cdr fields) (+ offset (field:size f)))))))))))

(define (field-pointer info struct field)
  (let ((field (field-field info struct field)))
    (field:pointer field)))

(define (field-size info struct field)
  (if (eq? (type:type struct) 'union) 0
      (let ((field (field-field info struct field)))
        (field:size field))))

(define (field-size info struct field)
  (let ((field (field-field info struct field)))
    (field:size field)))

(define (field-type info struct field)
  (let ((field (field-field info struct field)))
    (ast->type (cdr field) info)))

(define (struct->fields o)
  (pmatch o
    (_ (guard (and (type? o) (eq? (type:type o) 'struct)))
       (append-map struct->fields (type:description o)))
    (_ (guard (and (type? o) (eq? (type:type o) 'union)))
       (append-map struct->fields (type:description o)))
    ((struct . ,type) (list (car (type:description type))))
    ((union . ,type) (list (car (type:description type))))
    ((bits . ,bits) bits)
    (_ (list o))))

(define (struct->init-fields o)
  (pmatch o
    (_ (guard (and (type? o) (eq? (type:type o) 'struct)))
       (append-map struct->init-fields (type:description o)))
    (_ (guard (and (type? o) (eq? (type:type o) 'union)))
       (append-map struct->init-fields (type:description o)))
    ((struct . ,type) (struct->init-fields type))
    ((union . ,type) (list (car (type:description type))))
    (_ (list o))))

(define (byte->hex.m1 o)
  (string-drop o 2))

(define (asm->m1 o)
  (let ((prefix ".byte "))
    (if (not (string-prefix? prefix o)) (map (cut string-split <> #\space) (string-split o #\newline))
        (let ((s (string-drop o (string-length prefix))))
          (list (format #f "'~a'" (string-join (map byte->hex.m1 (cdr (string-split o #\space))) " ")))))))

(define (ident->variable info o)
  (or (assoc-ref (.locals info) o)
      (assoc-ref (.statics info) o)
      (assoc-ref (filter (negate static-global?) (.globals info)) o)
      (assoc-ref (.constants info) o)
      (assoc-ref (.functions info) o)
      (begin
        (error "ident->variable: undefined variable:" o))))

(define (static-global? o)
  ((compose global:function cdr) o))

(define (string-global? o)
  (and (pair? (car o))
       (eq? (caar o) #:string)))

(define (ident->type info o)
  (let ((var (ident->variable info o)))
    (cond ((global? var) (global:type var))
          ((local? var) (local:type var))
          ((function? var) (function:type var))
          ((assoc-ref (.constants info) o) (assoc-ref (.types info) "default"))
          ((pair? var) (car var))
          (else (stderr "ident->type ~s => ~s\n" o var)
                #f))))

(define (local:pointer o)
  (->rank o))

(define (ident->rank info o)
  (->rank (ident->variable info o)))

(define (ident->size info o)
  ((compose type:size (cut ident->type info <>)) o))

(define (pointer->rank o)
  (pmatch o
    ((pointer) 1)
    ((pointer ,pointer) (1+ (pointer->rank pointer)))))

(define (expr->rank info o)
  (->rank (ast->type o info)))

(define (ast->size o info)
  (->size (ast->type o info)))

(define (append-text info text)
  (clone info #:text (append (.text info) text)))

(define (push-global info)
  (lambda (o)
    (let ((rank (ident->rank info o)))
      (cond ((< rank 0) (list (i386:push-label `(#:address ,o)))) ;; FIXME
            (else (list (i386:push-label-mem `(#:address ,o))))))))

(define (push-local locals)
  (lambda (o)
    (wrap-as (i386:push-local (local:id o)))))

(define (push-global-address info)
  (lambda (o)
    (list (i386:push-label o))))

(define (push-local-address locals)
  (lambda (o)
    (wrap-as (i386:push-local-address (local:id o)))))

(define (push-local-de-ref info)
  (lambda (o)
    (let ((size (->size o)))
      (case size
        ((1) (wrap-as (i386:push-byte-local-de-ref (local:id o))))
        ((2) (wrap-as (i386:push-word-local-de-ref (local:id o))))
        ((4) (wrap-as (i386:push-local-de-ref (local:id o))))
        (else (error (format #f "TODO: push size >4: ~a\n" size)))))))

 ;; (if (= ptr 2) (ast-type->size info (local:type o)) ;; URG
 ;;                       4)
(define (push-local-de-de-ref info)
  (lambda (o)
    (let ((size (->size (rank-- (rank-- o)))))
      (if (= size 1)
          (wrap-as (i386:push-byte-local-de-de-ref (local:id o)))
          (error "TODO int-de-de-ref")))))

(define (make-global-entry name type value)
  (cons name (make-global name type value #f)))

(define (string->global-entry string)
  (let ((value (append (string->list string) (list #\nul))))
   (make-global-entry `(#:string ,string) "char" value))) ;; FIXME char-array

(define (make-local-entry name type id)
  (cons name (make-local name type id)))

(define* (mescc:trace name #:optional (type ""))
  (format (current-error-port) "    :~a~a\n" name type))

(define (push-ident info)
  (lambda (o)
    (cond ((assoc-ref (.locals info) o)
           =>
           (push-local (.locals info)))
          ((assoc-ref (.statics info) o)
           =>
           (push-global info))
          ((assoc-ref (filter (negate static-global?) (.globals info)) o)
           =>
           (push-global info))
          ((assoc-ref (.constants info) o)
           =>
           (lambda (constant)
             (wrap-as (append (i386:value->accu constant)
                              (i386:push-accu)))))
          (else
           ((push-global-address #f) `(#:address ,o))))))

(define (push-ident-address info)
  (lambda (o)
    (cond ((assoc-ref (.locals info) o)
           =>
           (push-local-address (.locals info)))
          ((assoc-ref (.statics info) o)
           =>
           (push-global-address info))
          ((assoc-ref (filter (negate static-global?) (.globals info)) o)
           =>
           (push-global-address info))
          (else
           ((push-global-address #f) `(#:address ,o))))))

(define (push-ident-de-ref info)
  (lambda (o)
    (cond ((assoc-ref (.locals info) o)
           =>
           (push-local-de-ref info))
          (else ((push-global info) o)))))

(define (push-ident-de-de-ref info)
  (lambda (o)
    (cond ((assoc-ref (.locals info) o)
           =>
           (push-local-de-de-ref info))
          (else
           (error "not supported: global push-ident-de-de-ref:" o)))))

(define (expr->arg info)
  (lambda (o)
    (pmatch o
      ((p-expr (string ,string))
       (let* ((globals ((globals:add-string (.globals info)) string))
              (info (clone info #:globals globals)))
         (append-text info ((push-global-address info) `(#:string ,string)))))
      (_ (let ((info (expr->accu o info)))
           (append-text info (wrap-as (i386:push-accu))))))))

(define (globals:add-string globals)
  (lambda (o)
    (let ((string `(#:string ,o)))
      (if (assoc-ref globals string) globals
          (append globals (list (string->global-entry o)))))))

(define (ident->accu info)
  (lambda (o)
    (cond ((assoc-ref (.locals info) o) => local->accu)
          ((assoc-ref (.statics info) o) => global->accu)
          ((assoc-ref (filter (negate static-global?) (.globals info)) o) => global->accu)
          ((assoc-ref (.constants info) o) => number->accu)
          (else (list (i386:label->accu `(#:address ,o)))))))

(define (local->accu o)
  (let* ((type (local:type o)))
    (cond ((or (c-array? type)
               (structured-type? type)) (wrap-as (i386:local-ptr->accu (local:id o))))
          (else (append (wrap-as (i386:local->accu (local:id o)))
                        (convert-accu type))))))

(define (global->accu o)
  (let ((type (global:type o)))
    (cond ((or (c-array? type)
               (structured-type? type)) (wrap-as (i386:label->accu `(#:address ,o))))
          (else (append (wrap-as (i386:label-mem->accu `(#:address ,o)))
                        (convert-accu type))))))

(define (number->accu o)
  (wrap-as (i386:value->accu o)))

(define (ident-address->accu info)
  (lambda (o)
    (cond ((assoc-ref (.locals info) o)
           =>
           (lambda (local) (wrap-as (i386:local-ptr->accu (local:id local)))))
          ((assoc-ref (.statics info) o)
           =>
           (lambda (global) (list (i386:label->accu `(#:address ,global)))))
          ((assoc-ref (filter (negate static-global?) (.globals info)) o)
           =>
           (lambda (global) (list (i386:label->accu `(#:address ,global)))))
          (else (list (i386:label->accu `(#:address ,o)))))))

(define (ident-address->base info)
  (lambda (o)
    (cond
     ((assoc-ref (.locals info) o)
      =>
      (lambda (local) (wrap-as (i386:local-ptr->base (local:id local)))))
     ((assoc-ref (.statics info) o)
      =>
      (lambda (global) (list (i386:label->base `(#:address ,global)))))
     ((assoc-ref (filter (negate static-global?) (.globals info)) o)
      =>
      (lambda (global) (list (i386:label->base `(#:address ,global)))))
     (else (list (i386:label->base `(#:address ,o)))))))

(define (value->accu v)
  (wrap-as (i386:value->accu v)))

(define (accu->local+n-text local n)
  (let ((id (local:id local))) (wrap-as (i386:accu->local+n id n))))

(define (accu->ident info)
  (lambda (o)
    (cond ((assoc-ref (.locals info) o)
           =>
           (lambda (local) (let ((size (->size local)))
                             (if (<= size 4) (wrap-as (i386:accu->local (local:id local)))
                                 (wrap-as (i386:accu*n->local (local:id local) size))))))
          ((assoc-ref (.statics info) o)
           =>
           (lambda (global) (let ((size (->size global)))
                              (if (<= size 4) (wrap-as (i386:accu->label global))
                                  (wrap-as (i386:accu*n->label global size))))))
          ((assoc-ref (filter (negate static-global?) (.globals info)) o)
           =>
           (lambda (global) (let ((size (->size global)))
                              (if (<= size 4) (wrap-as (i386:accu->label global))
                                  (wrap-as (i386:accu*n->label global size)))))))))

(define (value->ident info)
  (lambda (o value)
    (cond ((assoc-ref (.locals info) o)
           =>
           (lambda (local) (wrap-as (i386:value->local (local:id local) value))))
          ((assoc-ref (.statics info) o)
           =>
           (lambda (global) (list (i386:value->label `(#:address ,global) value))))
          ((assoc-ref (filter (negate static-global?) (.globals info)) o)
           =>
           (lambda (global) (list (i386:value->label `(#:address ,global) value)))))))

(define (ident-add info)
  (lambda (o n)
    (cond ((assoc-ref (.locals info) o)
           =>
           (lambda (local) (wrap-as (i386:local-add (local:id local) n))))
          ((assoc-ref (.statics info) o)
           =>
           (lambda (global) (list (i386:label-mem-add `(#:address ,o) n))))
          ((assoc-ref (filter (negate static-global?) (.globals info)) o)
           =>
           (lambda (global) (list (i386:label-mem-add `(#:address ,global) n)))))))

(define (ident-address-add info)
  (lambda (o n)
    (cond ((assoc-ref (.locals info) o)
           =>
           (lambda (local) (wrap-as (append (i386:push-accu)
                                            (i386:local->accu (local:id local))
                                            (i386:accu-mem-add n)
                                            (i386:pop-accu)))))
          ((assoc-ref (.statics info) o)
           =>
           (lambda (global) (list (wrap-as (append (i386:push-accu)
                                                   (i386:label->accu `(#:address ,global))
                                                   (i386:accu-mem-add n)
                                                   (i386:pop-accu))))))
          ((assoc-ref (filter (negate static-global?) (.globals info)) o)
           =>
           (lambda (global) (list (wrap-as (append (i386:push-accu)
                                                   (i386:label->accu `(#:address ,global))
                                                   (i386:accu-mem-add n)
                                                   (i386:pop-accu)))))))))

(define (make-comment o)
  (wrap-as `((#:comment ,o))))

(define (ast->comment o)
  (if mes? '()
      (let* ((source (with-output-to-string (lambda () (pretty-print-c99 o))))
             ;; Nyacc 0.80.42 fixups
             (source (string-substitute source "'\\'" "'\\\\'"))
             (source (string-substitute source "'\"'" "'\\\"'"))
             (source (string-substitute source "'''" "'\\''")))
        (make-comment (string-join (string-split source #\newline) " ")))))

(define (accu*n info n)
  (append-text info (wrap-as (case n
                               ((1) (i386:accu->base))
                               ((2) (i386:accu+accu))
                               ((3) (append (i386:accu->base)
                                            (i386:accu+accu)
                                            (i386:accu+base)))
                               ((4) (i386:accu-shl 2))
                               ((8) (append (i386:accu+accu)
                                            (i386:accu-shl 2)))
                               ((12) (append (i386:accu->base)
                                             (i386:accu+accu)
                                             (i386:accu+base)
                                             (i386:accu-shl 2)))
                               ((16) (i386:accu-shl 4))
                               (else (append (i386:value->base n)
                                             (i386:accu*base)))))))

(define (accu->base-mem*n- info n)
  (wrap-as
   (case n
     ((1) (i386:byte-accu->base-mem))
     ((2) (i386:word-accu->base-mem))
     ((4) (i386:accu->base-mem))
     (else (append (let loop ((i 0))
                     (if (>= i n) '()
                         (append (if (= i 0) '()
                                     (append (i386:accu+value 4)
                                             (i386:base+value 4)))
                                 (case (- n i)
                                   ((1) (append (i386:accu+value -3)
                                                (i386:base+value -3)
                                                (i386:accu-mem->base-mem)))
                                   ((2) (append (i386:accu+value -2)
                                                (i386:base+value -2)
                                                (i386:accu-mem->base-mem)))
                                   ((3) (append (i386:accu+value -1)
                                                (i386:base+value -1)
                                                (i386:accu-mem->base-mem)))
                                   (else (i386:accu-mem->base-mem)))
                                 (loop (+ i 4))))))))))

(define (accu->base-mem*n info n)
  (append-text info (accu->base-mem*n- info n)))

(define (expr->accu* o info)
  (pmatch o

    ((p-expr (ident ,name))
     (append-text info ((ident-address->accu info) name)))

    ((de-ref ,expr)
     (expr->accu expr info))

    ((d-sel (ident ,field) ,struct)
     (let* ((type (ast->basic-type struct info))
            (offset (field-offset info type field))
            (info (expr->accu* struct info)))
       (append-text info (wrap-as (i386:accu+value offset)))))

    ((i-sel (ident ,field) (fctn-call (p-expr (ident ,function)) . ,rest))
     (let* ((type (ast->basic-type `(fctn-call (p-expr (ident ,function)) ,@rest) info))
            (offset (field-offset info type field))
            (info (expr->accu `(fctn-call (p-expr (ident ,function)) ,@rest) info)))
       (append-text info (wrap-as (i386:accu+value offset)))))

    ((i-sel (ident ,field) ,struct)
     (let* ((type (ast->basic-type struct info))
            (offset (field-offset info type field))
            (info (expr->accu* struct info)))
       (append-text info (append (wrap-as (i386:mem->accu))
                                 (wrap-as (i386:accu+value offset))))))

    ((array-ref ,index ,array)
     (let* ((info (expr->accu index info))
            (size (ast->size o info))
            (info (accu*n info size))
            (info (expr->base array info)))
       (append-text info (wrap-as (i386:accu+base)))))

    ((cast ,type ,expr)
     (expr->accu `(ref-to ,expr) info))

    ((add ,a ,b)
     (let* ((rank (expr->rank info a))
            (rank-b (expr->rank info b))
            (type (ast->basic-type a info))
            (struct? (structured-type? type))
            (size (cond ((= rank 1) (ast-type->size info a))
                        ((> rank 1) 4)
                        ((and struct? (= rank 2)) 4)
                        (else 1))))
       (if (or (= size 1)) ((binop->accu* info) a b (i386:accu+base))
           (let* ((info (expr->accu b info))
                  (info (append-text info (wrap-as (append (i386:value->base size)
                                                           (i386:accu*base)
                                                           (i386:accu->base)))))
                  (info (expr->accu* a info)))
             (append-text info (wrap-as (i386:accu+base)))))))

    ((sub ,a ,b)
     (let* ((rank (expr->rank info a))
            (rank-b (expr->rank info b))
            (type (ast->basic-type a info))
            (struct? (structured-type? type))
            (size (->size type))
            (size  (cond ((= rank 1) size)
                         ((> rank 1) 4)
                         ((and struct? (= rank 2)) 4)
                         (else 1))))
       (if (or (= size 1) (or (= rank-b 2) (= rank-b 1)))
           (let ((info ((binop->accu* info) a b (i386:accu-base))))
             (if (and (not (= rank-b 2)) (not (= rank-b 1))) info
                 (append-text info (wrap-as (append (i386:value->base size)
                                                    (i386:accu/base))))))
           (let* ((info (expr->accu* b info))
                  (info (append-text info (wrap-as (append (i386:value->base size)
                                                           (i386:accu*base)
                                                           (i386:accu->base)))))
                  (info (expr->accu* a info)))
             (append-text info (wrap-as (i386:accu-base)))))))

    ((pre-dec ,expr)
     (let* ((rank (expr->rank info expr))
            (size (cond ((= rank 1) (ast-type->size info expr))
                        ((> rank 1) 4)
                        (else 1)))
            (info ((expr-add info) expr (- size)))
            (info (append (expr->accu* expr info))))
       info))

    ((pre-inc ,expr)
     (let* ((rank (expr->rank info expr))
            (size (cond ((= rank 1) (ast-type->size info expr))
                        ((> rank 1) 4)
                        (else 1)))
            (info ((expr-add info) expr size))
            (info (append (expr->accu* expr info))))
       info))

    ((post-dec ,expr)
     (let* ((info (expr->accu* expr info))
            (info (append-text info (wrap-as (i386:push-accu))))
            (post (clone info #:text '()))
            (post (append-text post (ast->comment o)))
            (post (append-text post (wrap-as (i386:pop-base))))
            (post (append-text post (wrap-as (i386:push-accu))))
            (post (append-text post (wrap-as (i386:base->accu))))
            (rank (expr->rank post expr))
            (size (cond ((= rank 1) (ast-type->size post expr))
                        ((> rank 1) 4)
                        (else 1)))
            (post ((expr-add post) expr (- size)))
            (post (append-text post (wrap-as (i386:pop-accu)))))
       (clone info #:post (.text post))))

    ((post-inc ,expr)
     (let* ((info (expr->accu* expr info))
            (info (append-text info (wrap-as (i386:push-accu))))
            (post (clone info #:text '()))
            (post (append-text post (ast->comment o)))
            (post (append-text post (wrap-as (i386:pop-base))))
            (post (append-text post (wrap-as (i386:push-accu))))
            (post (append-text post (wrap-as (i386:base->accu))))
            (rank (expr->rank post expr))
            (size (cond ((= rank 1) (ast-type->size post expr))
                        ((> rank 1) 4)
                        (else 1)))
            (post ((expr-add post) expr size))
            (post (append-text post (wrap-as (i386:pop-accu)))))
       (clone info #:post (.text post))))

    (_ (error "expr->accu*: not supported: " o))))

(define (expr-add info)
  (lambda (o n)
    (let* ((info (expr->accu* o info))
           (info (append-text info (wrap-as (i386:accu-mem-add n)))))
      info)))

(define (expr->accu o info)
  (let ((locals (.locals info))
        (text (.text info))
        (globals (.globals info)))
    (define (helper)
      (pmatch o
        ((expr) info)

        ((comma-expr) info)

        ((comma-expr ,a . ,rest)
         (let ((info (expr->accu a info)))
           (expr->accu `(comma-expr ,@rest) info)))

        ((p-expr (string ,string))
         (let* ((globals ((globals:add-string globals) string))
                (info (clone info #:globals globals)))
           (append-text info (list (i386:label->accu `(#:string ,string))))))

        ((p-expr (string . ,strings))
         (let* ((string (apply string-append strings))
                (globals ((globals:add-string globals) string))
                (info (clone info #:globals globals)))
           (append-text info (list (i386:label->accu `(#:string ,string))))))

        ((p-expr (fixed ,value))
         (let ((value (cstring->int value)))
           (append-text info (wrap-as (i386:value->accu value)))))

        ((p-expr (float ,value))
         (let ((value (cstring->float value)))
           (append-text info (wrap-as (i386:value->accu value)))))

        ((neg (p-expr (fixed ,value)))
         (let ((value (- (cstring->int value))))
           (append-text info (wrap-as (i386:value->accu value)))))

        ((p-expr (char ,char))
         (let ((char (char->integer (car (string->list char)))))
           (append-text info (wrap-as (i386:value->accu char)))))

        (,char (guard (char? char)) (append-text info (wrap-as (i386:value->accu char))))

        ((p-expr (ident ,name))
         (append-text info ((ident->accu info) name)))

        ((initzer ,initzer)
         (expr->accu initzer info))

        (((initzer ,initzer))
         (expr->accu initzer info))

        ;; offsetoff
        ((ref-to (i-sel (ident ,field) (cast (type-name (decl-spec-list ,struct) (abs-declr (pointer))) (p-expr (fixed ,base)))))
         (let* ((type (ast->basic-type struct info))
                (offset (field-offset info type field))
                (base (cstring->int base)))
           (append-text info (wrap-as (i386:value->accu (+ base offset))))))

        ;; &foo
        ((ref-to (p-expr (ident ,name)))
         (append-text info ((ident-address->accu info) name)))

        ;; &*foo
        ((ref-to (de-ref ,expr))
         (expr->accu expr info))

        ((ref-to ,expr)
         (expr->accu* expr info))

        ((sizeof-expr ,expr)
         (append-text info (wrap-as (i386:value->accu (ast->size expr info)))))

        ((sizeof-type ,type)
         (append-text info (wrap-as (i386:value->accu (ast->size type info)))))

        ((array-ref ,index ,array)
         (let* ((info (expr->accu* o info))
                (type (ast->type o info)))
           (append-text info (mem->accu type))))

        ((d-sel ,field ,struct)
         (let* ((info (expr->accu* o info))
                (info (append-text info (ast->comment o)))
                (type (ast->type o info))
                (size (->size type))
                (array? (c-array? type)))
           (if array? info
               (append-text info (mem->accu type)))))

        ((i-sel ,field ,struct)
         (let* ((info (expr->accu* o info))
                (info (append-text info (ast->comment o)))
                (type (ast->type o info))
                (size (->size type))
                (array? (c-array? type)))
           (if array? info
               (append-text info (mem->accu type)))))

        ((de-ref ,expr)
         (let* ((info (expr->accu expr info))
                (type (ast->type o info)))
           (append-text info (mem->accu type))))

        ((fctn-call (p-expr (ident ,name)) (expr-list . ,expr-list))
         (if (equal? name "asm") (let ((arg0 (cadr (cadar expr-list)))) ;; FIXME
                                   (append-text info (wrap-as (asm->m1 arg0))))
             (let* ((text-length (length text))
                    (args-info (let loop ((expressions (reverse expr-list)) (info info))
                                 (if (null? expressions) info
                                     (loop (cdr expressions) ((expr->arg info) (car expressions))))))
                    (n (length expr-list)))
               (if (not (assoc-ref locals name))
                   (begin
                     (if (and (not (assoc name (.functions info)))
                              (not (assoc name globals))
                              (not (equal? name (.function info))))
                         (stderr "warning: undeclared function: ~a\n" name))
                     (append-text args-info (list (i386:call-label name n))))
                   (let* ((empty (clone info #:text '()))
                          (accu (expr->accu `(p-expr (ident ,name)) empty)))
                     (append-text args-info (append (.text accu)
                                                    (list (i386:call-accu n)))))))))

        ((fctn-call ,function (expr-list . ,expr-list))
         (let* ((text-length (length text))
                (args-info (let loop ((expressions (reverse expr-list)) (info info))
                             (if (null? expressions) info
                                 (loop (cdr expressions) ((expr->arg info) (car expressions))))))
                (n (length expr-list))
                (empty (clone info #:text '()))
                (accu (expr->accu function empty)))
           (append-text args-info (append (.text accu)
                                          (list (i386:call-accu n))))))

        ((cond-expr . ,cond-expr)
         (ast->info `(expr-stmt ,o) info))

        ((post-inc ,expr)
         (let* ((info (append (expr->accu expr info)))
                (info (append-text info (wrap-as (i386:push-accu))))
                (rank (expr->rank info expr))
                (size (cond ((= rank 1) (ast-type->size info expr))
                            ((> rank 1) 4)
                            (else 1)))
                (info ((expr-add info) expr size))
                (info (append-text info (wrap-as (i386:pop-accu)))))
           info))

        ((post-dec ,expr)
         (let* ((info (append (expr->accu expr info)))
                (info (append-text info (wrap-as (i386:push-accu))))
                (rank (expr->rank info expr))
                (size (cond ((= rank 1) (ast-type->size info expr))
                            ((> rank 1) 4)
                            (else 1)))
                (info ((expr-add info) expr (- size)))
                (info (append-text info (wrap-as (i386:pop-accu)))))
           info))

        ((pre-inc ,expr)
         (let* ((rank (expr->rank info expr))
                (size (cond ((= rank 1) (ast-type->size info expr))
                            ((> rank 1) 4)
                            (else 1)))
                (info ((expr-add info) expr size))
                (info (append (expr->accu expr info))))
           info))

        ((pre-dec ,expr)
         (let* ((rank (expr->rank info expr))
                (size (cond ((= rank 1) (ast-type->size info expr))
                            ((> rank 1) 4)
                            (else 1)))
                (info ((expr-add info) expr (- size)))
                (info (append (expr->accu expr info))))
           info))



        ((add ,a (p-expr (fixed ,value)))
         (let* ((rank (expr->rank info a))
                (type (ast->basic-type a info))
                (struct? (structured-type? type))
                (size (cond ((= rank 1) (ast-type->size info a))
                            ((> rank 1) 4)
                            ((and struct? (= rank 2)) 4)
                            (else 1)))
                (info (expr->accu a info))
                (value (cstring->int value))
                (value (* size value)))
           (append-text info (wrap-as (i386:accu+value value)))))

        ((add ,a ,b)
         (let* ((rank (expr->rank info a))
                (rank-b (expr->rank info b))
                (type (ast->basic-type a info))
                (struct? (structured-type? type))
                (size (cond ((= rank 1) (ast-type->size info a))
                            ((> rank 1) 4)
                            ((and struct? (= rank 2)) 4)
                            (else 1))))
           (if (or (= size 1)) ((binop->accu info) a b (i386:accu+base))
               (let* ((info (expr->accu b info))
                      (info (append-text info (wrap-as (append (i386:value->base size)
                                                               (i386:accu*base)
                                                               (i386:accu->base)))))
                      (info (expr->accu a info)))
                 (append-text info (wrap-as (i386:accu+base)))))))

        ((sub ,a (p-expr (fixed ,value)))
         (let* ((rank (expr->rank info a))
                (type (ast->basic-type a info))
                (struct? (structured-type? type))
                (size (->size type))
                (size (cond ((= rank 1) size)
                            ((> rank 1) 4)
                            ((and struct? (= rank 2)) 4)
                            (else 1)))
                (info (expr->accu a info))
                (value (cstring->int value))
                (value (* size value)))
           (append-text info (wrap-as (i386:accu+value (- value))))))

        ((sub ,a ,b)
         (let* ((rank (expr->rank info a))
                (rank-b (expr->rank info b))
                (type (ast->basic-type a info))
                (struct? (structured-type? type))
                (size (->size type))
                (size  (cond ((= rank 1) size)
                             ((> rank 1) 4)
                             ((and struct? (= rank 2)) 4)
                             (else 1))))
           (if (or (= size 1) (or (= rank-b 2) (= rank-b 1)))
               (let ((info ((binop->accu info) a b (i386:accu-base))))
                 (if (and (not (= rank-b 2)) (not (= rank-b 1))) info
                     (append-text info (wrap-as (append (i386:value->base size)
                                                        (i386:accu/base))))))
               (let* ((info (expr->accu b info))
                      (info (append-text info (wrap-as (append (i386:value->base size)
                                                               (i386:accu*base)
                                                               (i386:accu->base)))))
                      (info (expr->accu a info)))
                 (append-text info (wrap-as (i386:accu-base)))))))

        ((bitwise-and ,a ,b) ((binop->accu info) a b (i386:accu-and-base)))
        ((bitwise-not ,expr)
         (let ((info (ast->info expr info)))
           (append-text info (wrap-as (i386:accu-not)))))
        ((bitwise-or ,a ,b) ((binop->accu info) a b (i386:accu-or-base)))
        ((bitwise-xor ,a ,b) ((binop->accu info) a b (i386:accu-xor-base)))
        ((lshift ,a ,b) ((binop->accu info) a b (i386:accu<<base)))
        ((rshift ,a ,b) ((binop->accu info) a b (i386:accu>>base)))
        ((div ,a ,b) ((binop->accu info) a b (i386:accu/base)))
        ((mod ,a ,b) ((binop->accu info) a b (i386:accu%base)))
        ((mul ,a ,b) ((binop->accu info) a b (i386:accu*base)))

        ((not ,expr)
         (let* ((test-info (ast->info expr info)))
           (clone info #:text
                  (append (.text test-info)
                          (wrap-as (i386:accu-negate)))
                  #:globals (.globals test-info))))

        ((neg ,expr)
         (let ((info (expr->base expr info)))
           (append-text info (append (wrap-as (i386:value->accu 0))
                                     (wrap-as (i386:sub-base))))))

        ((eq ,a ,b) ((binop->accu info) a b (append (i386:sub-base) (i386:z->accu))))

        ((ge ,a ,b)
         (let* ((type-a (ast->type a info))
                (type-b (ast->type b info))
                (test->accu (if (or (unsigned? type-a) (unsigned? type-b)) i386:ae?->accu i386:ge?->accu)))
           ((binop->accu info) a b (append (i386:sub-base) (test->accu) (i386:accu-test)))))

        ((gt ,a ,b)
         (let* ((type-a (ast->type a info))
                (type-b (ast->type b info))
                (test->accu (if (or (unsigned? type-a) (unsigned? type-b)) i386:a?->accu i386:g?->accu)))
           ((binop->accu info) a b (append (i386:sub-base) (test->accu) (i386:accu-test)))))

        ;; FIXME: set accu *and* flags
        ((ne ,a ,b) ((binop->accu info) a b (append (i386:push-accu)
                                                    (i386:sub-base)
                                                    (i386:nz->accu)
                                                    (i386:accu<->stack)
                                                    (i386:sub-base)
                                                    (i386:xor-zf)
                                                    (i386:pop-accu))))

        ((ne ,a ,b) ((binop->accu info) a b (append (i386:sub-base) (i386:xor-zf))))

        ((le ,a ,b)
         (let* ((type-a (ast->type a info))
                (type-b (ast->type b info))
                (test->accu (if (or (unsigned? type-a) (unsigned? type-b)) i386:be?->accu i386:le?->accu)))
           ((binop->accu info) a b (append (i386:sub-base) (test->accu) (i386:accu-test)))))

        ((lt ,a ,b)
         (let* ((type-a (ast->type a info))
                (type-b (ast->type b info))
                (test->accu (if (or (unsigned? type-a) (unsigned? type-b)) i386:b?->accu i386:l?->accu)))
           ((binop->accu info) a b (append (i386:sub-base) (test->accu) (i386:accu-test)))))

        ((or ,a ,b)
         (let* ((info (expr->accu a info))
                (here (number->string (length (.text info))))
                (skip-b-label (string-append "_" (.function info) "_" here "_or_skip_b"))
                (info (append-text info (wrap-as (i386:accu-test))))
                (info (append-text info (wrap-as (i386:jump-nz skip-b-label))))
                (info (append-text info (wrap-as (i386:accu-test))))
                (info (expr->accu b info))
                (info (append-text info (wrap-as (i386:accu-test))))
                (info (append-text info (wrap-as `((#:label ,skip-b-label))))))
           info))

        ((and ,a ,b)
         (let* ((info (expr->accu a info))
                (here (number->string (length (.text info))))
                (skip-b-label (string-append "_" (.function info) "_" here "_and_skip_b"))
                (info (append-text info (wrap-as (i386:accu-test))))
                (info (append-text info (wrap-as (i386:jump-z skip-b-label))))
                (info (append-text info (wrap-as (i386:accu-test))))
                (info (expr->accu b info))
                (info (append-text info (wrap-as (i386:accu-test))))
                (info (append-text info (wrap-as `((#:label ,skip-b-label))))))
           info))

        ((cast ,type ,expr)
         (let ((info (expr->accu expr info))
               (type (ast->type o info)))
           (append-text info (convert-accu type))))

        ((assn-expr (de-ref (post-inc (p-expr (ident ,name)))) (op ,op) ,b)
         (let* ((info (expr->accu `(assn-expr (de-ref (p-expr (ident ,name))) (op ,op) ,b) info))
                (type (ident->type info name))
                (rank (ident->rank info name))
                (size (if (> rank 1) 4 1)))
           (append-text info ((ident-add info) name size))))

        ((assn-expr (de-ref (post-dec (p-expr (ident ,name)))) (op ,op) ,b)
         (let* ((info (expr->accu `(assn-expr (de-ref (p-expr (ident ,name))) (op ,op) ,b) info))
                (type (ident->type info name))
                (rank (ident->rank info name))
                (size (if (> rank 1) 4 1)))
           (append-text info ((ident-add info) name (- size)))))

        ((assn-expr ,a (op ,op) ,b)
         (let* ((info (append-text info (ast->comment o)))
                (type (ast->type a info))
                (rank (->rank type))
                (type-b (ast->type b info))
                (rank-b (->rank type-b))
                (size (if (zero? rank) (->size type) 4))
                (size-b (if (zero? rank-b) (->size type-b) 4))
                (info (expr->accu b info))
                (info (if (equal? op "=") info
                          (let* ((struct? (structured-type? type))
                                 (size (cond ((= rank 1) (ast-type->size info a))
                                             ((> rank 1) 4)
                                             ((and struct? (= rank 2)) 4)
                                             (else 1)))
                                 (info (if (or (= size 1) (= rank-b 1)) info
                                           (let ((info (append-text info (wrap-as (i386:value->base size)))))
                                             (append-text info (wrap-as (i386:accu*base))))))
                                 (info (append-text info (wrap-as (i386:push-accu))))
                                 (info (expr->accu a info))
                                 (info (append-text info (wrap-as (i386:pop-base))))
                                 (info (append-text info (cond ((equal? op "+=") (wrap-as (i386:accu+base)))
                                                               ((equal? op "-=") (wrap-as (i386:accu-base)))
                                                               ((equal? op "*=") (wrap-as (i386:accu*base)))
                                                               ((equal? op "/=") (wrap-as (i386:accu/base)))
                                                               ((equal? op "%=") (wrap-as (i386:accu%base)))
                                                               ((equal? op "&=") (wrap-as (i386:accu-and-base)))
                                                               ((equal? op "|=") (wrap-as (i386:accu-or-base)))
                                                               ((equal? op "^=") (wrap-as (i386:accu-xor-base)))
                                                               ((equal? op ">>=") (wrap-as (i386:accu>>base)))
                                                               ((equal? op "<<=") (wrap-as (i386:accu<<base)))
                                                               (else (error (format #f "mescc: op ~a not supported: ~a\n" op o)))))))
                            (cond ((not (and (= rank 1) (= rank-b 1))) info)
                                  ((equal? op "-=") (append-text info (wrap-as (append (i386:value->base size)
                                                                                       (i386:accu/base)))))
                                  (else (error (format #f "invalid operands to binary ~s (have ~s* and ~s*)" op type (ast->basic-type b info)))))))))
           (when (and (equal? op "=")
                      (not (= size size-b))
                      (not (and (or (= size 1) (= size 2))
                                (or (= size-b 2) (= size-b 4))))
                      (not (and (= size 2)
                                (= size-b 4)))
                      (not (and (= size 4)
                                (or (= size-b 1) (= size-b 2)))))
             (stderr "ERROR assign: ~a" (with-output-to-string (lambda () (pretty-print-c99 o))))
             (stderr "   size[~a]:~a != size[~a]:~a\n"  rank size rank-b size-b))
           (pmatch a
             ((p-expr (ident ,name))
              (if (or (<= size 4) ;; FIXME: long long = int
                      (<= size-b 4)) (append-text info ((accu->ident info) name))
                      (let ((info (expr->base* a info)))
                        (accu->base-mem*n info size))))
             (_ (let* ((info (expr->base* a info))
                       (info (if (not (bit-field? type)) info
                                 (let* ((bit (bit-field:bit type))
                                        (bits (bit-field:bits type))
                                        (set-mask (- (ash bits 1) 1))
                                        (shifted-set-mask (ash set-mask bit))
                                        (clear-mask (logxor shifted-set-mask #b11111111111111111111111111111111))
                                        (info (append-text info (wrap-as (i386:push-base))))
                                        (info (append-text info (wrap-as (i386:push-accu))))

                                        (info (append-text info (wrap-as (i386:base-mem->accu))))
                                        (info (append-text info (wrap-as (i386:accu-and clear-mask))))
                                        (info (append-text info (wrap-as (i386:accu->base))))

                                        (info (append-text info (wrap-as (i386:pop-accu))))
                                        (info (append-text info (wrap-as (i386:accu-and set-mask))))
                                        (info (append-text info (wrap-as (i386:accu-shl bit))))
                                        (info (append-text info (wrap-as (i386:accu-or-base))))

                                        (info (append-text info (wrap-as (i386:pop-base)))))
                                   info))))
                  (accu->base-mem*n info (min size (max 4 size-b)))))))) ;; FIXME: long long = int

        (_ (error "expr->accu: not supported: " o))))

    (let ((info (helper)))
      (if (null? (.post info)) info
          (append-text (clone info #:post '()) (.post info))))))

(define (mem->accu type)
  (let ((size (->size type)))
    (case size
      ((1) (append (wrap-as (i386:byte-mem->accu)) (convert-accu type)))
      ((2) (append (wrap-as (i386:word-mem->accu)) (convert-accu type)))
      ((4) (wrap-as (i386:mem->accu)))
      (else '()))))

(define (convert-accu type)
  (if (not (type? type)) '()
      (let ((sign (signed? type))
            (size (->size type)))
        (cond ((and (= size 1) sign)
               (wrap-as (i386:signed-byte-accu)))
              ((= size 1)
               (wrap-as (i386:byte-accu)))
              ((and (= size 2) sign)
               (wrap-as (i386:signed-word-accu)))
              ((= size 1)
               (wrap-as (i386:word-accu)))
              (else '())))))

(define (expr->base o info)
  (let* ((info (append-text info (wrap-as (i386:push-accu))))
         (info (expr->accu o info))
         (info (append-text info (wrap-as (append (i386:accu->base) (i386:pop-accu))))))
    info))

(define (binop->accu info)
  (lambda (a b c)
    (let* ((info (expr->accu a info))
           (info (expr->base b info)))
      (append-text info (wrap-as c)))))

(define (binop->accu* info)
  (lambda (a b c)
    (let* ((info (expr->accu* a info))
           (info (expr->base b info)))
      (append-text info (wrap-as c)))))

(define (wrap-as o . annotation)
  `(,@annotation ,o))

(define (expr->base* o info)
  (let* ((info (append-text info (wrap-as (i386:push-accu))))
         (info (expr->accu* o info))
         (info (append-text info (wrap-as (i386:accu->base))))
         (info (append-text info (wrap-as (i386:pop-accu)))))
    info))

(define (comment? o)
  (and (pair? o) (pair? (car o)) (eq? (caar o) #:comment)))

(define (test-jump-label->info info label)
  (define (jump type . test)
    (lambda (o)
      (let* ((info (ast->info o info))
             (info (append-text info (make-comment "jmp test LABEL")))
             (jump-text (wrap-as (type label))))
        (append-text info (append (if (null? test) '() (car test))
                                  jump-text)))))
  (lambda (o)
    (pmatch o
      ((expr) info)
      ((le ,a ,b) ((jump i386:jump-z) o))
      ((lt ,a ,b) ((jump i386:jump-z) o))
      ((ge ,a ,b) ((jump i386:jump-z) o))
      ((gt ,a ,b) ((jump i386:jump-z) o))
      ((ne ,a ,b) ((jump i386:jump-nz) o))
      ((eq ,a ,b) ((jump i386:jump-nz) o))
      ((not _) ((jump i386:jump-z) o))

      ((and ,a ,b)
       (let* ((info ((test-jump-label->info info label) a))
              (info ((test-jump-label->info info label) b)))
         info))

      ((or ,a ,b)
       (let* ((here (number->string (length (if mes? (.text info)
                                                (filter (negate comment?) (.text info))))))
              (skip-b-label (string-append label "_skip_b_" here))
              (b-label (string-append label "_b_" here))
              (info ((test-jump-label->info info b-label) a))
              (info (append-text info (wrap-as (i386:jump skip-b-label))))
              (info (append-text info (wrap-as `((#:label ,b-label)))))
              (info ((test-jump-label->info info label) b))
              (info (append-text info (wrap-as `((#:label ,skip-b-label))))))
         info))

      ((array-ref ,index ,expr) (let* ((rank (expr->rank info expr))
                                       (size (if (= rank 1) (ast-type->size info expr)
                                                 4)))
                                  ((jump (if (= size 1) i386:jump-byte-z
                                             i386:jump-z)
                                         (wrap-as (i386:accu-zero?))) o)))

      ((de-ref ,expr) (let* ((rank (expr->rank info expr))
                             (size (if (= rank 1) (ast-type->size info expr)
                                       4)))
                        ((jump (if (= size 1) i386:jump-byte-z
                                   i386:jump-z)
                               (wrap-as (i386:accu-zero?))) o)))

      ((assn-expr (p-expr (ident ,name)) ,op ,expr)
       ((jump i386:jump-z
              (append ((ident->accu info) name)
                      (wrap-as (i386:accu-zero?)))) o))

      (_ ((jump i386:jump-z (wrap-as (i386:accu-zero?))) o)))))

(define (cstring->int o)
  (let ((o (cond ((string-suffix? "ULL" o) (string-drop-right o 3))
                 ((string-suffix? "UL" o) (string-drop-right o 2))
                 ((string-suffix? "LL" o) (string-drop-right o 2))
                 ((string-suffix? "L" o) (string-drop-right o 1))
                 (else o))))
    (or (cond ((string-prefix? "0x" o) (string->number (string-drop o 2) 16))
              ((string-prefix? "0b" o) (string->number (string-drop o 2) 2))
              ((string-prefix? "0" o) (string->number o 8))
              (else (string->number o)))
        (error "cstring->int: not supported:" o))))

(define (cstring->float o)
  (or (string->number o)
      (error "cstring->float: not supported:" o)))

(define (try-expr->number info o)
  (pmatch o
    ((fixed ,a) (cstring->int a))
    ((p-expr ,expr) (expr->number info expr))
    ((neg ,a)
     (- (expr->number info a)))
    ((add ,a ,b)
     (+ (expr->number info a) (expr->number info b)))
    ((bitwise-and ,a ,b)
     (logand (expr->number info a) (expr->number info b)))
    ((bitwise-not ,a)
     (lognot (expr->number info a)))
    ((bitwise-or ,a ,b)
     (logior (expr->number info a) (expr->number info b)))
    ((div ,a ,b)
     (quotient (expr->number info a) (expr->number info b)))
    ((mul ,a ,b)
     (* (expr->number info a) (expr->number info b)))
    ((sub ,a ,b)
     (- (expr->number info a) (expr->number info b)))
    ((sizeof-type ,type)
     (->size (ast->type type info)))
    ((sizeof-expr ,expr)
     (->size (ast->type expr info)))
    ((lshift ,x ,y)
     (ash (expr->number info x) (expr->number info y)))
    ((rshift ,x ,y)
     (ash (expr->number info x) (- (expr->number info y))))
    ((p-expr (ident ,name))
     (let ((value (assoc-ref (.constants info) name)))
       (or value
           (error (format #f "expr->number: undeclared identifier: ~s\n" o)))))
    ((cast ,type ,expr) (expr->number info expr))
    ((cond-expr ,test ,then ,else)
     (if (p-expr->bool info test) (expr->number info then) (expr->number info else)))
    (,string (guard (string? string)) (cstring->int string))
    ((ident ,name) (assoc-ref (.constants info) name))
    (_  #f)))

(define (expr->number info o)
  (or (try-expr->number info  o)
      (error (format #f "expr->number: not supported: ~s\n" o))))

(define (p-expr->bool info o)
  (pmatch o
    ((eq ,a ,b) (eq? (expr->number info a) (expr->number info b)))))

(define (struct-field info)
  (lambda (o)
    (pmatch o
      ((comp-decl (decl-spec-list (type-spec (enum-def (ident ,name) (enum-def-list . ,fields)))) (comp-declr-list . ,decls))
       (let (
             ;;(constants (enum-def-list->constants (.constants info) fields))
             ;;(type-entry (enum->type-entry name fields))
             )
         (append-map (lambda (o)
                       ((struct-field info) `(comp-decl (decl-spec-list (type-spec "int")) (comp-declr-list ,o))))
                     decls)))
    ((comp-decl (decl-spec-list (type-spec ,type)) (comp-declr-list (comp-declr (ident ,name))))
       (list (cons name (ast->type type info))))
      ((comp-decl (decl-spec-list (type-spec ,type)) (comp-declr-list (comp-declr (ptr-declr ,pointer (ident ,name)))))
       (let ((rank (pointer->rank pointer)))
         (list (cons name (rank+= (ast->type type info) rank)))))
      ((comp-decl (decl-spec-list (type-spec ,type)) (comp-declr-list (comp-declr (ftn-declr (scope (ptr-declr ,pointer (ident ,name))) _))))
       (let ((rank (pointer->rank pointer)))
         (list (cons name (rank+= (ast->type type info) rank)))))
      ((comp-decl (decl-spec-list (type-spec ,type)) (comp-declr-list (comp-declr (ptr-declr ,pointer (array-of (ident ,name) ,count)))))
       (let ((rank (pointer->rank pointer))
             (count (expr->number info count)))
         (list (cons name (make-c-array (rank+= type rank) count)))))
      ((comp-decl (decl-spec-list (type-spec ,type)) (comp-declr-list (comp-declr (array-of (ident ,name) ,count))))
       (let ((count (expr->number info count)))
         (list (cons name (make-c-array (ast->type type info) count)))))
      ((comp-decl (decl-spec-list (type-spec (struct-def (field-list . ,fields)))))
       (let ((fields (append-map (struct-field info) fields)))
         (list (cons 'struct (make-type 'struct (apply + (map field:size fields)) fields)))))
      ((comp-decl (decl-spec-list (type-spec (union-def (field-list . ,fields)))))
       (let ((fields (append-map (struct-field info) fields)))
         (list (cons 'union (make-type 'union (apply + (map field:size fields)) fields)))))
      ((comp-decl (decl-spec-list (type-spec ,type)) (comp-declr-list (comp-declr (bit-field (ident ,name) (p-expr (fixed ,bits)))) . ,fields))
       (let ((type (ast->type type info)))
         (list (cons 'bits (let loop ((o `((comp-declr (bit-field (ident ,name) (p-expr (fixed ,bits)))) . ,fields)) (bit 0))
                             (if (null? o) '()
                                 (let ((field (car o)))
                                   (pmatch field
                                     ((comp-declr (bit-field (ident ,name) (p-expr (fixed ,bits))))
                                      (let ((bits (cstring->int bits)))
                                        (cons (cons name (make-bit-field type bit bits))
                                              (loop (cdr o) (+ bit bits)))))
                                     (_ (error "struct-field: not supported:" field o))))))))))
      ((comp-decl (decl-spec-list ,type) (comp-declr-list . ,decls))
       (append-map (lambda (o)
                     ((struct-field info) `(comp-decl (decl-spec-list ,type) (comp-declr-list ,o))))
                   decls))
      (_ (error "struct-field: not supported: " o)))))

(define (local-var? o) ;; formals < 0, locals > 0
  (positive? (local:id o)))

(define (ptr-declr->rank o)
  (pmatch o
    ((pointer) 1)
    ((pointer (pointer)) 2)
    ((pointer (pointer (pointer))) 3)
    (_ (error "ptr-declr->rank not supported: " o))))

(define (ast->info o info)
  (let ((functions (.functions info))
        (globals (.globals info))
        (locals (.locals info))
        (constants (.constants info))
        (types (.types info))
        (text (.text info)))
    (pmatch o
      (((trans-unit . _) . _) (ast-list->info o info))
      ((trans-unit . ,_) (ast-list->info _ info))
      ((fctn-defn . ,_) (fctn-defn->info _ info))

      ((cpp-stmt (define (name ,name) (repl ,value)))
       info)

      ((cast (type-name (decl-spec-list (type-spec (void)))) _)
       info)

      ((break)
       (let ((label (car (.break info))))
         (append-text info (wrap-as (i386:jump label)))))

      ((continue)
       (let ((label (car (.continue info))))
         (append-text info (wrap-as (i386:jump label)))))

      ;; FIXME: expr-stmt wrapper?
      (trans-unit info)
      ((expr-stmt) info)

      ((compd-stmt (block-item-list . ,_)) (ast-list->info _ info))

      ((asm-expr ,gnuc (,null ,arg0 . string))
       (append-text info (wrap-as (asm->m1 arg0))))

      ((expr-stmt (fctn-call (p-expr (ident ,name)) (expr-list . ,expr-list)))
       (if (equal? name "asm") (let ((arg0 (cadr (cadar expr-list))))
                                 (append-text info (wrap-as (asm->m1 arg0))))
           (let* ((info (append-text info (ast->comment o)))
                  (info (expr->accu `(fctn-call (p-expr (ident ,name)) (expr-list . ,expr-list)) info)))
             (append-text info (wrap-as (i386:accu-zero?))))))

      ((if ,test ,then)
       (let* ((info (append-text info (ast->comment `(if ,test (ellipsis)))))
              (here (number->string (length text)))
              (label (string-append "_" (.function info) "_" here "_"))
              (break-label (string-append label "break"))
              (else-label (string-append label "else"))
              (info ((test-jump-label->info info break-label) test))
              (info (ast->info then info))
              (info (append-text info (wrap-as (i386:jump break-label))))
              (info (append-text info (wrap-as `((#:label ,break-label))))))
         (clone info
                #:locals locals)))

      ((if ,test ,then ,else)
       (let* ((info (append-text info (ast->comment `(if ,test (ellipsis) (ellipsis)))))
              (here (number->string (length text)))
              (label (string-append "_" (.function info) "_" here "_"))
              (break-label (string-append label "break"))
              (else-label (string-append label "else"))
              (info ((test-jump-label->info info else-label) test))
              (info (ast->info then info))
              (info (append-text info (wrap-as (i386:jump break-label))))
              (info (append-text info (wrap-as `((#:label ,else-label)))))
              (info (ast->info else info))
              (info (append-text info (wrap-as `((#:label ,break-label))))))
         (clone info
                #:locals locals)))

      ;; Hmm?
      ((expr-stmt (cond-expr ,test ,then ,else))
       (let* ((info (append-text info (ast->comment `(cond-expr ,test (ellipsis) (ellipsis)))))
              (here (number->string (length text)))
              (label (string-append "_" (.function info) "_" here "_"))
              (else-label (string-append label "else"))
              (break-label (string-append label "break"))
              (info ((test-jump-label->info info else-label) test))
              (info (ast->info then info))
              (info (append-text info (wrap-as (i386:jump break-label))))
              (info (append-text info (wrap-as `((#:label ,else-label)))))
              (info (ast->info else info))
              (info (append-text info (wrap-as `((#:label ,break-label))))))
         info))

      ((switch ,expr (compd-stmt (block-item-list . ,statements)))
       (define (clause? o)
         (pmatch o
           ((case . _) 'case)
           ((default . _) 'default)
           ((labeled-stmt _ ,statement) (clause? statement))
           (_ #f)))
       (define clause-number
         (let ((i 0))
           (lambda (o)
             (let ((n i))
               (when (clause? (car o))
                 (set! i (1+ i)))
               n))))
       (let* ((info (append-text info (ast->comment `(switch ,expr (compd-stmt (block-item-list (ellipsis)))))))
              (here (number->string (length text)))
              (label (string-append "_" (.function info) "_" here "_"))
              (break-label (string-append label "break"))
              (info (expr->accu expr info))
              (info (clone info #:break (cons break-label (.break info))))
              (count (length (filter clause? statements)))
              (default? (find (cut eq? <> 'default) (map clause? statements)))
              (info (fold (cut switch->info #t label (1- count) <> <> <>) info statements
                          (unfold null? clause-number cdr statements)))
              (last-clause-label (string-append label "clause" (number->string count)))
              (default-label (string-append label "default"))
              (info (if (not default?) info
                        (append-text info (wrap-as (i386:jump break-label)))))
              (info (append-text info (wrap-as `((#:label ,last-clause-label)))))
              (info (if (not default?) info
                        (append-text info (wrap-as (i386:jump default-label)))))
              (info (append-text info (wrap-as `((#:label ,break-label))))))
         (clone info
                #:locals locals
                #:break (cdr (.break info)))))

      ((for ,init ,test ,step ,body)
       (let* ((info (append-text info (ast->comment `(for ,init ,test ,step (ellipsis)))))
              (here (number->string (length text)))
              (label (string-append "_" (.function info) "_" here "_"))
              (break-label (string-append label "break"))
              (loop-label (string-append label "loop"))
              (continue-label (string-append label "continue"))
              (initial-skip-label (string-append label "initial_skip"))
              (info (ast->info init info))
              (info (clone info #:break (cons break-label (.break info))))
              (info (clone info #:continue (cons continue-label (.continue info))))
              (info (append-text info (wrap-as (i386:jump initial-skip-label))))
              (info (append-text info (wrap-as `((#:label ,loop-label)))))
              (info (ast->info body info))
              (info (append-text info (wrap-as `((#:label ,continue-label)))))
              (info (expr->accu step info))
              (info (append-text info (wrap-as `((#:label ,initial-skip-label)))))
              (info ((test-jump-label->info info break-label) test))
              (info (append-text info (wrap-as (i386:jump loop-label))))
              (info (append-text info (wrap-as `((#:label ,break-label))))))
         (clone info
                #:locals locals
                #:break (cdr (.break info))
                #:continue (cdr (.continue info)))))

      ((while ,test ,body)
       (let* ((info (append-text info (ast->comment `(while ,test (ellipsis)))))
              (here (number->string (length text)))
              (label (string-append "_" (.function info) "_" here "_"))
              (break-label (string-append label "break"))
              (loop-label (string-append label "loop"))
              (continue-label (string-append label "continue"))
              (info (append-text info (wrap-as (i386:jump continue-label))))
              (info (clone info #:break (cons break-label (.break info))))
              (info (clone info #:continue (cons continue-label (.continue info))))
              (info (append-text info (wrap-as `((#:label ,loop-label)))))
              (info (ast->info body info))
              (info (append-text info (wrap-as `((#:label ,continue-label)))))
              (info ((test-jump-label->info info break-label) test))
              (info (append-text info (wrap-as (i386:jump loop-label))))
              (info (append-text info (wrap-as `((#:label ,break-label))))))
         (clone info
                #:locals locals
                #:break (cdr (.break info))
                #:continue (cdr (.continue info)))))

      ((do-while ,body ,test)
       (let* ((info (append-text info (ast->comment `(do-while ,test (ellipsis)))))
              (here (number->string (length text)))
              (label (string-append "_" (.function info) "_" here "_"))
              (break-label (string-append label "break"))
              (loop-label (string-append label "loop"))
              (continue-label (string-append label "continue"))
              (info (clone info #:break (cons break-label (.break info))))
              (info (clone info #:continue (cons continue-label (.continue info))))
              (info (append-text info (wrap-as `((#:label ,loop-label)))))
              (info (ast->info body info))
              (info (append-text info (wrap-as `((#:label ,continue-label)))))
              (info ((test-jump-label->info info break-label) test))
              (info (append-text info (wrap-as (i386:jump loop-label))))
              (info (append-text info (wrap-as `((#:label ,break-label))))))
         (clone info
                #:locals locals
                #:break (cdr (.break info))
                #:continue (cdr (.continue info)))))

      ((labeled-stmt (ident ,label) ,statement)
       (let ((info (append-text info `(((#:label ,(string-append "_" (.function info) "_label_" label)))))))
         (ast->info statement info)))

      ((goto (ident ,label))
       (append-text info (wrap-as (i386:jump (string-append "_" (.function info) "_label_" label)))))

      ((return ,expr)
       (let ((info (expr->accu expr info)))
         (append-text info (append (wrap-as (i386:ret))))))

      ((decl . ,decl)
       ;;FIXME: ridiculous performance hit with mes
       ;; Nyacc 0.80.42: missing  (enum-ref (ident "fred"))
       (let ( ;;(info (append-text info (ast->comment o)))
             )
         (decl->info info decl)))
      ;; ...
      ((gt . _) (expr->accu o info))
      ((ge . _) (expr->accu o info))
      ((ne . _) (expr->accu o info))
      ((eq . _) (expr->accu o info))
      ((le . _) (expr->accu o info))
      ((lt . _) (expr->accu o info))
      ((lshift . _) (expr->accu o info))
      ((rshift . _) (expr->accu o info))

      ;; EXPR
      ((expr-stmt ,expression)
       (let ((info (expr->accu expression info)))
         (append-text info (wrap-as (i386:accu-zero?)))))

      ;; FIXME: why do we get (post-inc ...) here
      ;; (array-ref
      (_ (let ((info (expr->accu o info)))
           (append-text info (wrap-as (i386:accu-zero?))))))))

(define (ast-list->info o info)
  (fold ast->info info o))

(define (switch->info clause? label count o i info)
  (let* ((i-string (number->string i))
         (i+1-string (number->string (1+ i)))
         (body-label (string-append label "body" i-string))
         (clause-label (string-append label "clause" i-string))
         (last? (= i count))
         (break-label (string-append label "break"))
         (next-clause-label (string-append label "clause" i+1-string))
         (default-label (string-append label "default")))
    (define (jump label)
      (wrap-as (i386:jump label)))
    (pmatch o
      ((case ,test)
       (define (jump-nz label)
         (wrap-as (i386:jump-nz label)))
       (define (jump-z label)
         (wrap-as (i386:jump-z label)))
       (define (test->text test)
         (let ((value (pmatch test
                        (0 0)
                        ((p-expr (char ,value)) (char->integer (car (string->list value))))
                        ((p-expr (ident ,constant)) (assoc-ref (.constants info) constant))
                        ((p-expr (fixed ,value)) (cstring->int value))
                        ((neg (p-expr (fixed ,value))) (- (cstring->int value)))
                        (_ (error "case test: not supported: " test)))))
           (append (wrap-as (i386:accu-cmp-value value))
                   (jump-z body-label))))
       (let ((info (if clause? (append-text info (wrap-as `((#:label ,clause-label))))
                       info)))
         (append-text info (test->text test))))
      ((case ,test (case . ,case1))
       (let ((info (if clause? (append-text info (wrap-as `((#:label ,clause-label))))
                       info)))
         (fold (cut switch->info #f label count <> i <>) info (cons `(case ,test) `((case ,@case1))))))
      ((case ,test (default . ,rest))
       (let ((info (if clause? (append-text info (wrap-as `((#:label ,clause-label))))
                       info)))
         (fold (cut switch->info #f label count <> i <>) info (cons `(case ,test) `(default ,@rest)))))
      ((case ,test ,statement)
       (let* ((info (if clause? (append-text info (wrap-as `((#:label ,clause-label))))
                        info))
              (info (switch->info #f label count `(case ,test) i info))
              (info (append-text info (jump next-clause-label)))
              (info (append-text info (wrap-as `((#:label ,body-label))))))
         (ast->info statement info)))
      ((case ,test (case . ,case1) . ,rest)
       (let ((info (if clause? (append-text info (wrap-as `((#:label ,clause-label))))
                       info)))
         (fold (cut switch->info #f label count <> i <>) info (cons `(case ,test) `((case ,@case1) ,@rest)))))
      ((default (case . ,case1) . ,rest)
       (let* ((info (if clause? (append-text info (wrap-as `((#:label ,clause-label))))
                        info))
               (info (if last? info
                         (append-text info (jump next-clause-label))))
              (info (append-text info (wrap-as `((#:label ,default-label)))))
              (info (append-text info (jump body-label))))
         (fold (cut switch->info #f label count <> i <>) info `((case ,@case1) ,@rest))))
      (default
        (let* ((info (if clause? (append-text info (wrap-as `((#:label ,clause-label))))
                         info))
               (info (if last? info
                         (append-text info (jump next-clause-label))))
               (info (append-text info (wrap-as `((#:label ,default-label))))))
          (append-text info (jump body-label))))
      ((default ,statement)
       (let* ((info (if clause? (append-text info (wrap-as `((#:label ,clause-label))))
                        info))
              (info (if last? info
                        (append-text info (jump next-clause-label))))
              (info (append-text info (wrap-as `((#:label ,body-label)))))
              (info (append-text info (wrap-as `((#:label ,default-label))))))
         (ast->info statement info)))
      ((default ,statement ,rest)
       (let* ((info (if clause? (append-text info (wrap-as `((#:label ,clause-label))))
                        info))
              (info (if last? info
                        (append-text info (jump next-clause-label))))
              (info (append-text info (wrap-as `((#:label ,body-label)))))
              (info (append-text info (wrap-as `((#:label ,default-label))))))
         (fold ast->info (ast->info statement info) rest)))
      ((labeled-stmt (ident ,goto-label) ,statement)
       (let ((info (append-text info `(((#:label ,(string-append "_" (.function info) "_label_" goto-label)))))))
         (switch->info clause? label count statement i info)))
      (_ (ast->info o info)))))

(define (global->static function)
  (lambda (o)
    (cons (car o) (set-field (cdr o) (global:function) function))))

(define (decl->info info o)
  (pmatch o
    (((decl-spec-list (type-spec ,type)) (init-declr-list . ,inits))
     (let* ((info (type->info type #f info))
            (type (ast->type type info)))
       (fold (cut init-declr->info type <> <>) info (map cdr inits))))
    (((decl-spec-list (type-spec ,type)))
     (type->info type #f info))
    (((decl-spec-list (stor-spec (typedef)) (type-spec ,type)) (init-declr-list (init-declr (ident ,name))))
     (let* ((info (type->info type name info))
            (type (ast->type type info)))
       (clone info #:types (acons name type (.types info)))))
    ;; FIXME: recursive types, pointer, array
    (((decl-spec-list (stor-spec (typedef)) (type-spec ,type)) (init-declr-list (init-declr (array-of (ident ,name) ,count))))
     (let* ((info (type->info type name info))
            (type (ast->type type info))
            (count (expr->number info count))
            (type (make-c-array type count)))
       (clone info #:types (acons name type (.types info)))))
    (((decl-spec-list (stor-spec (typedef)) (type-spec ,type)) (init-declr-list (init-declr (ptr-declr ,pointer (ident ,name)))))
     (let* ((info (type->info type name info))
            (type (ast->type type info))
            (rank (pointer->rank pointer))
            (type (rank+= type rank)))
       (clone info #:types (acons name type (.types info)))))
    (((decl-spec-list (stor-spec (,store)) (type-spec ,type)) (init-declr-list . ,inits))
     (let* ((info (type->info type #f info))
            (type (ast->type type info))
            (function (.function info)))
       (if (not function) (fold (cut init-declr->info type <> <>) info (map cdr inits))
           (let* ((tmp (clone info #:function #f #:globals '()))
                  (tmp (fold (cut init-declr->info type <> <>) tmp (map cdr inits)))
                  (statics (map (global->static function) (.globals tmp)))
                  (strings (filter string-global? (.globals tmp))))
             (clone info #:globals (append (.globals info) strings)
                    #:statics (append statics (.statics info)))))))
    (((decl-spec-list (stor-spec (,store)) (type-spec ,type)))
     (type->info type #f info))
    (((@ . _))
     (stderr "decl->info: skip: ~s\n" o)
     info)
    (_ (error "decl->info: not supported:" o))))

(define (ast->name o)
  (pmatch o
    ((ident ,name) name)
    ((array-of ,array . ,_) (ast->name array))
    ((ftn-declr (scope (ptr-declr ,pointer (ident ,name))) . _) name)
    ((ptr-declr ,pointer ,decl . ,_) (ast->name decl))
    ((ptr-declr ,pointer (ident ,name)) name)
    (_ (error "ast->name not supported: " o))))

(define (init-declr->count info o)
  (pmatch o
    ((array-of (ident ,name) ,count) (expr->number info count))
    (_ #f)))

(define (init->accu o info)
  (pmatch o
    ((initzer-list (initzer ,expr)) (expr->accu expr info))
    (((#:string ,string))
     (append-text info (list (i386:label->accu `(#:string ,string)))))
    ((,number . _) (guard (number? number))
     (append-text info (wrap-as (i386:value->accu 0))))
    ((,c . ,_) (guard (char? c)) info)
    (_ (expr->accu o info))))

(define (init-struct-field local field init info)
  (let* ((offset (field-offset info (local:type local) (car field)))
         (size (field:size field))
         (empty (clone info #:text '())))
    (clone info #:text
           (append
            (.text info)
            (local->accu local)
            (wrap-as (append (i386:accu->base)))
            (wrap-as (append (i386:push-base)))
            (.text (expr->accu init empty))
            (wrap-as (append (i386:pop-base)))
            (wrap-as (case size
                       ((1) (i386:byte-accu->base-mem+n offset))
                       ((2) (i386:word-accu->base-mem+n offset))
                       (else (i386:accu->base-mem+n offset))))))))

(define (init-array-entry local index init info)
  (let* ((type (local:type local))
         (size (cond ((pointer? type) %pointer-size)
                     ((and (c-array? type) ((compose pointer? c-array:type) type)) %pointer-size)
                     ((c-array? type) ((compose type:size c-array:type) type))
                     (else (type:size type))))
         (offset (* index size))
         (empty (clone info #:text '())))
    (clone info #:text
           (append
            (.text info)
            (local->accu local)
            (wrap-as (append (i386:accu->base)))
            (wrap-as (append (i386:push-base)))
            (.text (expr->accu init empty))
            (wrap-as (append (i386:pop-base)))
            (wrap-as (case size
                       ((1) (i386:byte-accu->base-mem+n offset))
                       ((2) (i386:word-accu->base-mem+n offset))
                       (else (i386:accu->base-mem+n offset))))))))


(define (init-local local o n info)
  (pmatch o
    (#f info)
    ((initzer ,init)
     (init-local local init n info))
    ((initzer-list ,init)
     (init-local local init n info))
    ((initzer-list . ,inits)
     (let ((struct? (structured-type? local)))
       (cond (struct?
              (let ((fields ((compose struct->init-fields local:type) local)))
                (fold (cut init-struct-field local <> <> <>) info fields (append inits (map (const '(p-expr (fixed "22"))) (iota (max 0 (- (length fields) (length inits)))))))))
             (else (fold (cut init-local local <> <> <>) info inits (iota (length inits)))))))
    (,string (guard (string? string))
             (let ((inits (string->list string)))
               (fold (cut init-array-entry local <> <> <>) info (iota (length inits)) inits)))
    (((initzer (initzer-list . ,inits)))
     (fold (cut init-array-entry local <> <> <>) info (iota (length inits)) inits))
    (() info)
    (_ (let ((info (init->accu o info)))
         (append-text info (accu->local+n-text local n))))))

(define (local->info type name o init info)
  (let* ((locals (.locals info))
         (id (if (or (null? locals) (not (local-var? (cdar locals)))) 1
                 (1+ (local:id (cdar locals)))))
         (local (make-local-entry name type id))
         (pointer (->rank (cdr local)))
         (array? (or (and (c-array? type) type)
                     (and (pointer? type)
                          (c-array? (pointer:type type))
                          (pointer:type type))
                     (and (pointer? type)
                          (pointer? (pointer:type type))
                          (c-array? (pointer:type (pointer:type type)))
                          (pointer:type (pointer:type type)))))
         (struct? (structured-type? type))
         (size (->size type))
         (string (and array? (array-init->string init)))
         (init (or string init))
         (local (if (not array?) local
                    (let ((size (or (and string (max size (1+ (string-length string))))
                                    size)))
                      (make-local-entry name type (+ (local:id (cdr local)) -1 (quotient (+ size 3) 4))))))
         (local (if struct? (make-local-entry name type (+ (local:id (cdr local)) (quotient (+ size 3) 4)))
                    local))
         (locals (cons local locals))
         (info (clone info #:locals locals))
         (local (cdr local)))
    (init-local local init 0 info)))

(define (global->info type name o init info)
  (let* ((rank (->rank type))
         (size (->size type))
         (data (cond ((not init) (string->list (make-string size #\nul)))
                     ((c-array? type)
                      (let* ((string (array-init->string init))
                             (size (or (and string (max size (1+ (string-length string))))
                                       size))
                             (data  (or (and=> string string->list)
                                        (array-init->data type size init info))))
                        (append data (string->list (make-string (max 0 (- size (length data))) #\nul)))))
                     ((structured-type? type)
                      (let ((data (init->data type init info)))
                        (append data (string->list (make-string (max 0 (- size (length data))) #\nul)))))
                     (else
                      (let ((data (init->data type init info)))
                        (append data (string->list (make-string (max 0 (- size (length data))) #\nul)))))))
         (global (make-global-entry name type data)))
    (clone info #:globals (append (.globals info) (list global)))))

(define (array-init-element->data type o info)
  (pmatch o
    ((initzer (p-expr (string ,string)))
     `((#:string ,string)))
    ((initzer (p-expr (fixed ,fixed)))
     (int->bv type (expr->number info fixed)))
    ((initzer (initzer-list . ,inits))
      (if (structured-type? type)
          (let* ((fields (map cdr (struct->init-fields type)))
                 (missing (max 0 (- (length fields) (length inits))))
                 (inits (append inits
                                (map (const '(fixed "0")) (iota missing)))))
            (map (cut init->data <> <> info) fields inits))
          (begin
            (stderr "array-init-element->data: oops:~s\n" o)
            (stderr "type:~s\n" type)
            (error "array-init-element->data: not supported: " o))))
    (_ (init->data type o info))
    (_ (error "array-init-element->data: not supported: " o))))

(define (array-init->data type size o info)
  (pmatch o
    ((initzer (initzer-list . ,inits))
     (let ((type (c-array:type type)))
       (map (cut array-init-element->data type <> info) inits)))

    (((initzer (initzer-list . ,inits)))
     (array-init->data type size (car o) info))

    ((initzer (p-expr (string ,string)))
     (let ((data (string->list string)))
       (if (not size) data
           (append data (string->list (make-string (max 0 (- size (length data))) #\nul))))))

    (((initzer (p-expr (string ,string))))
     (array-init->data type size (car o) info))

    ((initzer (p-expr (string . ,strings)))
     (let ((data (string->list (apply string-append strings))))
       (if (not size) data
           (append data (string->list (make-string (max 0 (- size (length data))) #\nul))))))

    (((initzer (p-expr (string . ,strings))))
     (array-init->data type size (car o) info))

    ((initzer (p-expr (fixed ,fixed)))
     (int->bv type (expr->number info fixed)))

    (() (string->list (make-string size #\nul)))
    (_ (error "array-init->data: not supported: " o))))

(define (array-init->string o)
  (pmatch o
    ((p-expr (string ,string)) string)
    ((p-expr (string . ,strings)) (apply string-append strings))
    ((initzer ,init) (array-init->string init))
    (((initzer ,init)) (array-init->string init))
    ((initzer-list (initzer (p-expr (char ,c))) . ,inits)
     (list->string (map (lambda (i) (pmatch i
                                      ((initzer (p-expr (char ,c))) ((compose car string->list) c))
                                      ((initzer (p-expr (fixed ,fixed)))
                                       (let ((value (cstring->int fixed)))
                                         (if (and (>= value 0) (<= value 255))
                                             (integer->char value)
                                             (error "array-init->string: not supported:" i o))))
                                      (_ (error "array-init->string: not supported:" i o))))
                        (cdr o))))
    (_ #f)))

(define (init-declr->info type o info)
  (pmatch o
    (((ident ,name))
     (if (.function info) (local->info type name o #f info)
         (global->info type name o #f info)))
    (((ident ,name) (initzer ,init))
     (let* ((strings (init->strings init info))
            (info (if (null? strings) info
                      (clone info #:globals (append (.globals info) strings)))))
       (if (.function info) (local->info type name o init info)
           (global->info type name o init info))))
    (((ftn-declr (ident ,name) . ,_))
     (let ((functions (.functions info)))
       (if (member name functions) info
           (let* ((type (ftn-declr:get-type info `(ftn-declr (ident ,name) ,@_)))
                  (function (make-function name type  #f)))
             (clone info #:functions (cons (cons name function) functions))))))
    (((ftn-declr (scope (ptr-declr ,pointer (ident ,name))) ,param-list) ,init)
     (let* ((rank (pointer->rank pointer))
            (type (rank+= type rank)))
       (if (.function info) (local->info type name o init info)
           (global->info type name o init info))))
    (((ftn-declr (scope (ptr-declr ,pointer (ident ,name))) ,param-list))
     (let* ((rank (pointer->rank pointer))
            (type (rank+= type rank)))
       (if (.function info) (local->info type name o '() info)
           (global->info type name o '() info))))
    (((ptr-declr ,pointer . ,_) . ,init)
     (let* ((rank (pointer->rank pointer))
            (type (rank+= type rank)))
       (init-declr->info type (append _ init) info)))
    (((array-of (ident ,name) ,count) . ,init)
     (let* ((strings (init->strings init info))
            (info (if (null? strings) info
                      (clone info #:globals (append (.globals info) strings))))
            (count (expr->number info count))
            (type (make-c-array type count)))
       (if (.function info) (local->info type name o init info)
           (global->info type name o init info))))
    (((array-of (ident ,name)) . ,init)
     (let* ((strings (init->strings init info))
            (info (if (null? strings) info
                      (clone info #:globals (append (.globals info) strings))))
            (count (length (cadar init)))
            (type (make-c-array type count)))
       (if (.function info) (local->info type name o init info)
           (global->info type name o init info))))
    ;; FIXME: recursion
    (((array-of (array-of (ident ,name) ,count1) ,count) . ,init)
     (let* ((strings (init->strings init info))
            (info (if (null? strings) info
                      (clone info #:globals (append (.globals info) strings))))
            (count (expr->number info count))
            (count1 (expr->number info count1))
            (type (make-c-array (make-c-array type count1) count)))
       (if (.function info) (local->info type name o init info)
           (global->info type name o init info))))
    (_ (error "init-declr->info: not supported: " o))))

(define (enum-def-list->constants constants fields)
  (let loop ((fields fields) (i 0) (constants constants))
    (if (pair? fields)
        (let ((field (car fields)))
          (mescc:trace (cadr (cadr field)) " <e>")))
    (if (null? fields) constants
        (let* ((field (car fields))
               (name (pmatch field
                       ((enum-defn (ident ,name) . _) name)))
               (i (pmatch field
                    ((enum-defn ,name) i)
                    ((enum-defn ,name ,exp) (expr->number #f exp))
                    (_ (error "not supported enum field=~s\n" field)))))
          (loop (cdr fields)
                (1+ i)
                (append constants (list (ident->constant name i))))))))

(define (init->data type o info)
  (pmatch o
    ((p-expr ,expr) (init->data type expr info))
    ((fixed ,fixed) (int->bv type (expr->number info o)))
    ((char ,char) (int->bv type (char->integer (string-ref char 0))))
    ((string ,string) `((#:string ,string)))
    ((string . ,strings) `((#:string ,(string-join strings ""))))
    ((ident ,name) (let ((var (ident->variable info name)))
                     `((#:address ,var))))
    ((initzer-list . ,inits)
     (cond ((structured-type? type)
            (map (cut init->data <> <> info) (map cdr (struct->init-fields type)) inits))
           ((c-array? type)
            (let ((size (->size type)))
             (array-init->data type size `(initzer ,o) info)))
           (else
            (append-map (cut init->data type <> info) inits))))
    (((initzer (initzer-list . ,inits)))
     (init->data type `(initzer-list . ,inits) info))
    ((ref-to (p-expr (ident ,name)))
     (let ((var (ident->variable info name)))
       `((#:address ,var))))
    ((ref-to (i-sel (ident ,field) (cast (type-name (decl-spec-list ,struct) (abs-declr (pointer))) (p-expr (fixed ,base)))))
     (let* ((type (ast->type struct info))
            (offset (field-offset info type field))
            (base (cstring->int base)))
       (int->bv type (+ base offset))))
    ((,char . _) (guard (char? char)) o)
    ((,number . _) (guard (number? number))
     (append (map int->bv type o)))
    ((initzer ,init) (init->data type init info))
    (((initzer ,init)) (init->data type init info))
    ((cast _ ,expr) (init->data type expr info))
    (() '())
    (_ (let ((number (try-expr->number info o)))
         (cond (number (int->bv type number))
               (else (error "init->data: not supported: " o)))))))

(define (int->bv type o)
  (let ((size (->size type)))
    (case size
      ;;((1) (int->bv8 o))
      ;;((2) (int->bv16 o))
      (else (int->bv32 o)))))

(define (init->strings o info)
  (let ((globals (.globals info)))
    (pmatch o
      ((p-expr (string ,string))
       (let ((g `(#:string ,string)))
         (if (assoc g globals) '()
             (list (string->global-entry string)))))
      ((p-expr (string . ,strings))
       (let* ((string (string-join strings ""))
              (g `(#:string ,string)))
         (if (assoc g globals) '()
             (list (string->global-entry string)))))
      (((initzer (initzer-list . ,init)))
       (append-map (cut init->strings <> info) init))
      ((initzer ,init)
       (init->strings init info))
      (((initzer ,init))
       (init->strings init info))
      ((initzer-list . ,init)
       (append-map (cut init->strings <> info) init))
      (_ '()))))

(define (type->info o name info)
  (pmatch o

    ((enum-def (ident ,name) (enum-def-list . ,fields))
     (mescc:trace name " <t>")
     (let* ((type-entry (enum->type-entry name fields))
            (constants (enum-def-list->constants (.constants info) fields)))
       (clone info
              #:types (cons type-entry (.types info))
              #:constants (append constants (.constants info)))))

    ((enum-def (enum-def-list . ,fields))
     (mescc:trace name " <t>")
     (let* ((type-entry (enum->type-entry name fields))
            (constants (enum-def-list->constants (.constants info) fields)))
       (clone info
              #:types (cons type-entry (.types info))
              #:constants (append constants (.constants info)))))

    ((struct-def (field-list . ,fields))
     (mescc:trace name " <t>")
     (let* ((info (fold field->info info fields))
            (type-entry (struct->type-entry name (append-map (struct-field info) fields))))
       (clone info #:types (cons type-entry (.types info)))))

    ((struct-def (ident ,name) (field-list . ,fields))
     (mescc:trace name " <t>")
     (let* ((info (fold field->info info fields))
            (type-entry (struct->type-entry name (append-map (struct-field info) fields))))
       (clone info #:types (cons type-entry (.types info)))))

    ((union-def (ident ,name) (field-list . ,fields))
     (mescc:trace name " <t>")
     (let ((type-entry (union->type-entry name (append-map (struct-field info) fields))))
       (clone info #:types (cons type-entry (.types info)))))

    ((union-def (field-list . ,fields))
     (mescc:trace name " <t>")
     (let ((type-entry (union->type-entry name (append-map (struct-field info) fields))))
       (clone info #:types (cons type-entry (.types info)))))

    ((enum-ref . _) info)
    ((struct-ref . _) info)
    ((typename ,name) info)
    ((union-ref . _) info)
    ((fixed-type . _) info)
    ((float-type . _) info)
    ((void) info)

    (_ ;;(error "type->info: not supported:" o)
     (stderr "type->info: not supported: ~s\n" o)
     info
     )))

(define (field->info o info)
  (pmatch o
    ((comp-decl (decl-spec-list (type-spec (struct-def (ident ,name) (field-list . ,fields)))) . _)
     (let* ((fields (append-map (struct-field info) fields))
            (struct (make-type 'struct (apply + (map field:size fields)) fields)))
       (clone info #:types (acons `(tag ,name) struct (.types info)))))
    ((comp-decl (decl-spec-list (type-spec (union-def (ident ,name) (field-list . ,fields)))) . _)
     (let* ((fields (append-map (struct-field info) fields))
            (union (make-type 'union (apply + (map field:size fields)) fields)))
       (clone info #:types (acons `(tag ,name) union (.types info))) ))
    ((comp-decl (decl-spec-list (type-spec (enum-def (enum-def-list . ,fields)))) . _)
     (let ((constants (enum-def-list->constants (.constants info) fields)))
       (clone info
              #:constants (append constants (.constants info)))))
    ((comp-decl (decl-spec-list (type-spec (enum-def (ident ,name) (enum-def-list . ,fields)))) . _)
     (let ((constants (enum-def-list->constants (.constants info) fields))
           (type-entry (enum->type-entry name fields)))
       (clone info
              #:types (cons type-entry (.types info))
              #:constants (append constants (.constants info)))))
    (_ info)))

;;; fctn-defn
(define (param-decl:get-name o)
  (pmatch o
    ((ellipsis) #f)
    ((param-decl (decl-spec-list (type-spec (void)))) #f)
    ((param-decl _ (param-declr ,ast)) (ast->name ast))
    (_ (error "param-decl:get-name not supported:" o))))

(define (fctn-defn:get-name o)
  (pmatch o
    ((_ (ftn-declr (ident ,name) _) _) name)
    ((_ (ptr-declr (pointer . _) (ftn-declr (ident ,name) _)) _) name)
    (_ (error "fctn-defn:get-name not supported:" o))))

(define (param-decl:get-type o info)
  (pmatch o
    ((ellipsis) #f)
    ((param-decl (decl-spec-list ,type)) (ast->type type info))
    ((param-decl (decl-spec-list (type-spec ,type)) (param-declr (ptr-declr ,pointer (ident ,name))))
     (let ((rank (pointer->rank pointer)))
       (rank+= (ast->type type info) rank)))
    ((param-decl (decl-spec-list ,type) (param-declr (ptr-declr ,pointer (array-of _))))
     (let ((rank (pointer->rank pointer)))
       (rank+= (ast->type type info) (1+ rank))))
    ((param-decl ,type _) (ast->type type info))
    (_ (error "param-decl:get-type not supported:" o))))

(define (fctn-defn:get-formals o)
  (pmatch o
    ((_ (ftn-declr _ ,formals) _) formals)
    ((_ (ptr-declr (pointer . _) (ftn-declr _ ,formals)) _) formals)
    (_ (error "fctn-defn->formals: not supported:" o))))

(define (formal->text n)
  (lambda (o i)
    ;;(i386:formal i n)
    '()
    ))

(define (param-list->text o)
  (pmatch o
    ((param-list . ,formals)
     (let ((n (length formals)))
       (wrap-as (append (i386:function-preamble)
                        (append-map (formal->text n) formals (iota n))
                        (i386:function-locals)))))
    (_ (error "param-list->text: not supported: " o))))

(define (param-list->locals o info)
  (pmatch o
    ((param-list . ,formals)
     (let ((n (length formals)))
       (map make-local-entry
            (map param-decl:get-name formals)
            (map (cut param-decl:get-type <> info) formals)
            (iota n -2 -1))))
    (_ (error "param-list->locals: not supported:" o))))

(define (fctn-defn:get-type info o)
  (pmatch o
    (((decl-spec-list (type-spec ,type)) (ptr-declr ,pointer . _) ,statement)
     (let* ((type (ast->type type info))
            (rank (ptr-declr->rank pointer)))
       (if (zero? rank) type
           (make-pointer type rank))))
    (((decl-spec-list (type-spec ,type)) . _)
     (ast->type type info))
    (((decl-spec-list (stor-spec ,store) (type-spec ,type)) . _)
     (ast->type type info))

    ;; (((decl-spec-list (stor-spec ,store) (type-spec ,type)) (ftn-declr (ident _) _) _)
    ;;  (ast->type type info))
    ;; (((decl-spec-list (stor-spec ,store) (type-spec ,type)) (ptr-declr ,pointer (ftn-declr (ident _) _)) _)
    ;;  (ast->type type info))

    (_ (error "fctn-defn:get-type: not supported:" o))))

(define (ftn-declr:get-type info o)
  (pmatch o
    ((ftn-declr (ident _) . _) #f)
    (_ (error "fctn-decrl:get-type: not supported:" o))))

(define (fctn-defn:get-statement o)
  (pmatch o
    ((_ (ftn-declr (ident _) _) ,statement) statement)
    ((_ (ptr-declr (pointer . _) (ftn-declr (ident _) . _)) ,statement) statement)
    (_ (error "fctn-defn:get-statement: not supported: " o))))

(define (fctn-defn->info o info)
  (define (assert-return text)
    (let ((return (wrap-as (i386:ret))))
      (if (equal? (list-tail text (- (length text) (length return))) return) text
          (append text return))))
  (let ((name (fctn-defn:get-name o)))
    (mescc:trace name)
    (let* ((type (fctn-defn:get-type info o))
           (formals (fctn-defn:get-formals o))
           (text (param-list->text formals))
           (locals (param-list->locals formals info))
           (statement (fctn-defn:get-statement o))
           (function (cons name (make-function name type '())))
           (functions (cons function (.functions info)))
           (info (clone info #:locals locals #:function name #:text text #:functions functions #:statics '()))
           (info (ast->info statement info))
           (locals (.locals info))
           (local (and (pair? locals) (car locals)))
           (count (and=> local (compose local:id cdr)))
           (stack (and count (* count 4))))
      (if (and stack (getenv "MESC_DEBUG")) (stderr "        stack: ~a\n" stack))
      (clone info
             #:function #f
             #:globals (append (.statics info) (.globals info))
             #:statics '()
             #:functions (append (.functions info) (list (cons name (make-function name type (assert-return (.text info))))))))))
