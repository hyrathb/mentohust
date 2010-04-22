ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src po

dist_doc_DATA = README

dist_man_MANS = mentohust.1

EXTRA_DIST = @PACKAGE_NAME@.spec.in m4 debian/changelog debian/compat debian/control debian/copyright debian/rules debian/watch

rpm: dist @PACKAGE_NAME@.spec
	rpmbuild -bb \
			--define "_sourcedir `pwd`" \
			--define "_builddir `pwd`" \
			--define "_specdir `pwd`" \
			--define "_rpmdir `pwd`" \
			--define "_srcrpmdir `pwd`" \
			@PACKAGE_NAME@.spec

deb: debian/rules
	dpkg-buildpackage -rfakeroot -uc -b -d
