

SPEC = mentohust.spec

SUBDIRS = src

dist_doc_DATA = README

EXTRA_DIST = $(SPEC)

install_conf: src/mentohust.conf
	mkdir -p $(DESTDIR)/etc
	$(INSTALL) $< $(DESTDIR)/etc

dist-rpm: dist-bzip2
	mkdir -p ~/rpmbuild/SOURCES/
	mv mentohust-*.tar.bz2 ~/rpmbuild/SOURCES/
	if [ -z "$(TARGET)" ] ;then \
	rpmbuild -bb $(top_srcdir)/$(SPEC) ; \
	else rpmbuild -bb $(top_srcdir)/$(SPEC) --target=$(TARGET) ;  fi
		
	mv ~/rpmbuild/RPMS/*$(TARGET)/mentohust-*$(TARGET).rpm ./
	rm ~/rpmbuild/SOURCES/mentohust*
	