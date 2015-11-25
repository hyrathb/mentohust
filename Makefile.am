SUBDIRS = src

dist_doc_DATA = README

dist_man_MANS = mentohust.1

SPEC = mentohust.spec

EXTRA_DIST = $(SPEC) debian/changelog debian/compat debian/control debian/copyright debian/rules debian/watch

rpm: dist-gzip
	mkdir -p ~/rpmbuild/SOURCES/
	mv mentohust-*.tar.gz ~/rpmbuild/SOURCES/
	if [ -z "$(TARGET)" ] ;then \
	rpmbuild -bb $(top_srcdir)/$(SPEC) ; \
	else rpmbuild -bb $(top_srcdir)/$(SPEC) --target=$(TARGET) ;  fi
	mv ~/rpmbuild/RPMS/*$(TARGET)/mentohust-*$(TARGET).rpm ./
	rm ~/rpmbuild/SOURCES/mentohust*

deb: debian/rules
	dpkg-buildpackage -rfakeroot -uc -b -d
