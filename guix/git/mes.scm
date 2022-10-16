;;; GNU Mes --- Maxwell Equations of Software
;;; Copyright © 2016,2017,2018,2019,2020,2021,2022 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
;;;
;;; This file is part of GNU Mes.
;;;
;;; Also borrowing code from:
;;; guile-sdl2 --- FFI bindings for SDL2
;;; Copyright © 2015 David Thompson <davet@gnu.org>
;;;
;;; GNU Mes is free software; you can redistribute it and/or modify it
;;; under the terms of the GNU General Public License as published by
;;; the Free Software Foundation; either version 3 of the License, or (at
;;; your option) any later version.
;;;
;;; GNU Mes is distributed in the hope that it will be useful, but
;;; WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;
;;; You should have received a copy of the GNU General Public License
;;; along with GNU Mes.  If not, see <http://www.gnu.org/licenses/>.

(define-module (git mes)
  #:use-module (srfi srfi-1)
  #:use-module (srfi srfi-26)
  #:use-module (ice-9 match)
  #:use-module (ice-9 popen)
  #:use-module (ice-9 rdelim)
  #:use-module (gnu packages)
  #:use-module (gnu packages base)
  #:use-module (gnu packages commencement)
  #:use-module (gnu packages cross-base)
  #:use-module (gnu packages gcc)
  #:use-module (gnu packages graphviz)
  #:use-module (gnu packages guile)
  #:use-module (gnu packages man)
  #:use-module (gnu packages mes)
  #:use-module (gnu packages package-management)
  #:use-module (gnu packages version-control)
  #:use-module (gnu packages perl)
  #:use-module (gnu packages pkg-config)
  #:use-module (gnu packages texinfo)
  #:use-module (guix build-system gnu)
  #:use-module (guix build-system trivial)
  #:use-module (guix gexp)
  #:use-module (guix download)
  #:use-module (guix git-download)
  #:use-module (guix licenses)
  #:use-module (guix packages)
  #:use-module (guix utils))

(define %source-dir (getcwd))

(define-public mescc-tools
  (package
    (name "mescc-tools")
    (version "1.4.0")
    (source (origin
              (method git-fetch)
              (uri (git-reference
                    (url "https://git.savannah.nongnu.org/r/mescc-tools.git")
                    (commit (string-append "Release_" version))
                    (recursive? #t)))             ;for M2libc
              (file-name (git-file-name name version))
              (sha256
               (base32
                "0z2ni2qn2np1walcaqlxz8sinzb78d4hiq9glddzf26wxc226hs4"))))
    (build-system gnu-build-system)
    (supported-systems
     '("aarch64-linux" "armhf-linux" "i686-linux" "x86_64-linux"))
    (native-inputs (list which))
    (arguments
     `(#:make-flags (list (string-append "PREFIX=" (assoc-ref %outputs "out"))
                          (string-append "CC=" ,(cc-for-target)))
       #:test-target "test"
       #:phases (modify-phases %standard-phases
                  (delete 'configure))))
    (synopsis "Tools for the full source bootstrapping process")
    (description
     "Mescc-tools is a collection of tools for use in a full source
bootstrapping process.  It consists of the M1 macro assembler, the hex2
linker, the blood-elf symbol table generator, the kaem shell, exec_enable and
get_machine.")
    (home-page "https://savannah.nongnu.org/projects/mescc-tools")
    (license gpl3+)))

(define-public m2-planet
  (package
    (name "m2-planet")
    (version "1.9.0")
    (source (origin
              (method git-fetch)
              (uri (git-reference
                    (url "https://github.com/oriansj/m2-planet")
                    (commit (string-append "Release_" version))
                    (recursive? #t)))             ;for M2libc
              (file-name (git-file-name name version))
              (sha256
               (base32
                "0cgvvq91cbxxm93k8ayyvhpaf3c2lv10qw4wyqwn3hc1qb1cfyvr"))))
    (native-inputs (list mescc-tools))
    (build-system gnu-build-system)
    (arguments
     `(#:make-flags (list (string-append "PREFIX=" (assoc-ref %outputs "out"))
                          (string-append "CC=" ,(cc-for-target)))
       #:tests? #f
       #:phases (modify-phases %standard-phases
                  (delete 'bootstrap)
                  (delete 'configure))))
    (synopsis "The PLAtform NEutral Transpiler")
    (description
     "M2-Planet, The PLAtform NEutral Transpiler, when combined with
mescc-tools compiles a subset of the C language into working binaries
with introspective steps inbetween.")
    (home-page "https://github.com/oriansj/m2-planet")
    (license gpl3+)))

(define-public nyacc-0.99
  (package
    (name "nyacc")
    (version "0.99.0")
    (source (origin
              (method url-fetch)
              (uri (string-append "mirror://savannah/nyacc/nyacc-"
                                  version ".tar.gz"))
              (sha256
               (base32
                "0hl5qxx19i4x1r0839sxm19ziqq65g4hy97yik81cc2yb9yvgyv3"))
              (modules '((guix build utils)))
              (snippet
               '(begin
                  (substitute* (find-files "." "^Makefile\\.in$")
                    (("^SITE_SCM_DIR =.*")
                     "SITE_SCM_DIR = \
@prefix@/share/guile/site/@GUILE_EFFECTIVE_VERSION@\n")
                    (("^SITE_SCM_GO_DIR =.*")
                     "SITE_SCM_GO_DIR = \
@prefix@/lib/guile/@GUILE_EFFECTIVE_VERSION@/site-ccache\n")
                    (("^INFODIR =.*")
                     "INFODIR = @prefix@/share/info\n")
                    (("^DOCDIR =.*")
                     "DOCDIR = @prefix@/share/doc/$(PACKAGE_TARNAME)\n"))
                  #t))))
    (build-system gnu-build-system)
    (native-inputs (list pkg-config))
    (inputs (list guile-2.2))
    (synopsis "LALR(1) Parser Generator in Guile")
    (description
     "NYACC is an LALR(1) parser generator implemented in Guile.
The syntax and nomenclature should be considered not stable.  It comes with
extensive examples, including parsers for the Javascript and C99 languages.")
    (home-page "https://savannah.nongnu.org/projects/nyacc")
    (license (list gpl3+ lgpl3+))))

(define-public nyacc
  (package
    (inherit nyacc-0.99)
    (version "1.00.2")
    (source (origin
              (method url-fetch)
              (uri (string-append "mirror://savannah/nyacc/nyacc-"
                                  version ".tar.gz"))
              (modules '((guix build utils)))
              (snippet
               '(begin
                  (substitute* (find-files "." "^Makefile\\.in$")
                    (("^SITE_SCM_DIR =.*")
                     "SITE_SCM_DIR = \
@prefix@/share/guile/site/@GUILE_EFFECTIVE_VERSION@\n")
                    (("^SITE_SCM_GO_DIR =.*")
                     "SITE_SCM_GO_DIR = \
@prefix@/lib/guile/@GUILE_EFFECTIVE_VERSION@/site-ccache\n")
                    (("^INFODIR =.*")
                     "INFODIR = @prefix@/share/info\n")
                    (("^DOCDIR =.*")
                     "DOCDIR = @prefix@/share/doc/$(PACKAGE_TARNAME)\n"))
                  #t))
              (sha256
               (base32
                "065ksalfllbdrzl12dz9d9dcxrv97wqxblslngsc6kajvnvlyvpk"))))
    (inputs (list guile-3.0))))

(define-public mes
  (package
    (name "mes")
    (version #!mes!# "0.24.1")
    (source (origin
              (method url-fetch)
              (uri (string-append
                    "https://ftp.gnu.org/pub/gnu/mes/mes-" version ".tar.gz"))
              (sha256
               (base32 #!mes!# "0246sy5zm49zw4zs87hrq1j1x74mchijh4rlj2mzlwjccha5cybs"))))
    (build-system gnu-build-system)
    (supported-systems '("aarch64-linux" "armhf-linux" "i686-linux" "x86_64-linux"))
    (propagated-inputs (list mescc-tools nyacc))
    (native-inputs
     (append
      (list guile-3.0)
      (let ((target-system (or (%current-target-system)
                               (%current-system))))
        (cond
         ((string-prefix? "x86_64-linux" target-system)
          ;; Use cross-compiler rather than #:system "i686-linux" to get
          ;; MesCC 64 bit .go files installed ready for use with Guile.
          (list (cross-binutils "i686-unknown-linux-gnu")
                (cross-gcc "i686-unknown-linux-gnu")))
         ((string-prefix? "aarch64-linux" target-system)
          ;; Use cross-compiler rather than #:system "armhf-linux" to get
          ;; MesCC 64 bit .go files installed ready for use with Guile.
          (let ((triplet "arm-linux-gnueabihf"))
            (list (cross-binutils triplet) (cross-gcc triplet))))
         (else
          '())))
      (list graphviz help2man m2-planet
            perl                        ;build-aux/gitlog-to-changelog
            texinfo)))
    (arguments
     `(#:strip-binaries? #f)) ; binutil's strip b0rkes MesCC/M1/hex2 binaries
    (native-search-paths
     (list (search-path-specification
            (variable "C_INCLUDE_PATH")
            (files '("include")))
           (search-path-specification
            (variable "LIBRARY_PATH")
            (files '("lib")))
           (search-path-specification
            (variable "MES_PREFIX")
            (separator #f)
            (files '("")))))
    (synopsis "Scheme interpreter and C compiler for full source bootstrapping")
    (description
     "GNU Mes--Maxwell Equations of Software--brings the Reduced Binary Seed
bootstrap to Guix and aims to help create full source bootstrapping for
GNU/Linux distributions.  It consists of a mutual self-hosting Scheme
interpreter in C and a Nyacc-based C compiler in Scheme and is compatible with
Guile.")
    (home-page "https://www.gnu.org/software/mes")
    (license gpl3+)))

(define-public mes.git
 (let ((version #!mes!# "0.24.1")
        (revision "0")
        (commit (read-string (open-pipe "git show HEAD | head -1 | cut -d ' ' -f 2" OPEN_READ))))
    (package
      (inherit mes)
      (name "mes.git")
      (version (string-append version "-" revision "." (string-take commit 7)))
      (source (local-file %source-dir
                          #:recursive? #t
                          #:select? (git-predicate %source-dir))))))
