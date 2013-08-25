ACLOCAL_AMFLAGS = -I m4

SUBDIRS = lib src test

EXTRA_DIST = version-gen
dist-hook:
	echo $(VERSION) > $(distdir)/version
