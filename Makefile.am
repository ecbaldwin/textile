ACLOCAL_AMFLAGS = -I m4

SUBDIRS = lib src test bin

EXTRA_DIST = version-gen
dist-hook:
	echo $(VERSION) > $(distdir)/version
