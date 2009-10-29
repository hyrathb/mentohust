

SUBDIRS = src


dist-rpm: dist-bzip2
	mkdir -p ~/rpmbuild/SOURCES/
	mv mentohust-*.tar.bz2 ~/rpmbuild/SOURCES/
	if [ -z "$(TARGET)" ] ;then \
	rpmbuild -bb $(top_srcdir)/mentohust.spec ; \
	else rpmbuild -bb $(top_srcdir)/mentohust.spec --target=$(TARGET) ;  fi
		
	mv ~/rpmbuild/RPMS/*$(TARGET)/mentohust-*$(TARGET).rpm ./
	rm ~/rpmbuild/SOURCES/mentohust*
	