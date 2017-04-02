.PHONY: tree-clean-p

READMES:=\
 AUTHORS\
 COPYING\
 HACKING\
 INSTALL\
 NEWS\
 README\
#

COMMIT:=$(shell test -d .git && (git show 2>/dev/null | head -1 | cut -d' ' -f 2) || cat .tarball-version)
GUIX-HASH:=out/guix-hash
TARBALL_DIR:=$(PACKAGE)-$(VERSION)
TARBALL:=$(OUT)/$(TARBALL_DIR).tar.gz

OPT_CLEAN:=$(OPT_CLEAN) $(TARBALL) .tarball-version

GIT_ARCHIVE_HEAD:=git archive HEAD --
GIT_LS_FILES:=git ls-files
ifeq ($(wildcard .git/HEAD),)
GIT_ARCHIVE_HEAD:=tar -cf-
GIT_LS_FILES:=find
endif

ifeq ($(GUIX),)
DATADIR:=$(PREFIX)/share/mes
DOCDIR:=$(DATADIR)/doc/mes
else
DATADIR:=$(PREFIX)/share
DOCDIR:=$(DATADIR)/doc
endif
MODULEDIR:=$(DATADIR)/module

.tarball-version: tree-clean-p
	echo $(COMMIT) > $@

dist: $(TARBALL)

tree-clean-p:
	test ! -d .git || git diff --exit-code > /dev/null
	test ! -d .git || git diff --cached --exit-code > /dev/null
	@echo commit:$(COMMIT)

$(TARBALL): tree-clean-p .tarball-version ChangeLog
	mkdir -p $(OUT)
	($(GIT_LS_FILES) --exclude=$(OUT);\
		echo $^ | tr ' ' '\n' | grep -Ev 'tree-clean-p')\
		| tar --transform=s,^,$(TARBALL_DIR)/,S -T- -czf $@

ChangeLog:
	build-aux/gitlog-to-changelog > $@

install: all ChangeLog
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	install mes $(DESTDIR)$(PREFIX)/bin/mes
	install mes-mini-mes $(DESTDIR)$(PREFIX)/bin/mes-mini-mes
	install scripts/mescc.mes $(DESTDIR)$(PREFIX)/bin/mescc.mes
	install scripts/repl.mes $(DESTDIR)$(PREFIX)/bin/repl.mes
	install guile/mescc.scm $(DESTDIR)$(PREFIX)/bin/mescc.scm
	mkdir -p $(DESTDIR)$(DATADIR)
	$(GIT_ARCHIVE_HEAD) module\
		| tar -C $(DESTDIR)$(DATADIR) -xf-
	$(GIT_ARCHIVE_HEAD) guile\
		| tar -C $(DESTDIR)$(DATADIR) -xf-
	sed -i \
	    -e 's,module/,$(DATADIR)/module/,' \
	    -e 's,@DATADIR@,$(DATADIR)/,g' \
	    -e 's,@DOCDIR@,$(DOCDIR)/,g' \
	    -e 's,@PREFIX@,$(PREFIX)/,g' \
	    -e 's,@VERSION@,$(VERSION),g' \
		$(DESTDIR)$(DATADIR)/module/mes/base-0.mes \
		$(DESTDIR)$(PREFIX)/bin/mescc.mes \
		$(DESTDIR)$(PREFIX)/bin/mescc.scm \
		$(DESTDIR)$(PREFIX)/bin/repl.mes
	cp module/mes/read-0.mo $(DESTDIR)$(DATADIR)/module/mes
	cp module/mes/read-0-32.mo $(DESTDIR)$(DATADIR)/module/mes
	mkdir -p $(DESTDIR)$(DOCDIR)
	$(GIT_ARCHIVE_HEAD) $(READMES) \
		| tar -C $(DESTDIR)$(DOCDIR) -xf-
	$(GIT_ARCHIVE_HEAD) doc \
		| tar -C $(DESTDIR)$(DOCDIR) --strip=1 -xf-
	cp ChangeLog $(DESTDIR)$(DOCDIR)

release: tree-clean-p check dist
	git tag v$(VERSION)
	git push --tags origin master
	git push origin master

$(GUIX-HASH): tree-clean-p
	rm -rf out/mes && mkdir -p out && git clone . out/mes && guix hash -rx out/mes > $@

update-hash: $(GUIX-HASH) .tarball-version
	@echo -n hash:
	cat $^
	sed -i \
		-e 's,(base32 "[^"]*"),(base32 "$(shell cat $<)"),'\
		-e 's,(commit "[^"]*"),(commit "$(shell cat .tarball-version)"),'\
		-e 's,(version "[^"]*"),(version "$(VERSION).$(shell cut -b1-8 .tarball-version)"),'\
		guix.scm
	! git diff --exit-code
	git commit -m 'guix hash: $(shell cat $<)' guix.scm

