MAINTAINERCLEANFILES = Makefile.in

lordsawarshielddir = $(datadir)/lordsawar/shield

default.lws: default.lws.in
	LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/default.lws.in $(builddir)/default.lws
	$(top_srcdir)/create-lw-file $(builddir)/default.lws $(IMAGES)

DISTCLEANFILES = default.lws 

lordsawarshield_DATA = default.lws

IMAGES = \
	$(srcdir)/large-black-shield.png \
	$(srcdir)/large-darkblue-shield.png \
	$(srcdir)/large-green-shield.png \
	$(srcdir)/large-lightblue-shield.png \
	$(srcdir)/large-neutral-shield.png \
	$(srcdir)/large-orange-shield.png \
	$(srcdir)/large-red-shield.png \
	$(srcdir)/large-white-shield.png \
	$(srcdir)/large-yellow-shield.png \
	$(srcdir)/medium-black-shield.png \
	$(srcdir)/medium-darkblue-shield.png \
	$(srcdir)/medium-green-shield.png \
	$(srcdir)/medium-lightblue-shield.png \
	$(srcdir)/medium-neutral-shield.png \
	$(srcdir)/medium-orange-shield.png \
	$(srcdir)/medium-red-shield.png \
	$(srcdir)/medium-white-shield.png \
	$(srcdir)/medium-yellow-shield.png \
	$(srcdir)/small-black-shield.png \
	$(srcdir)/small-darkblue-shield.png \
	$(srcdir)/small-green-shield.png \
	$(srcdir)/small-lightblue-shield.png \
	$(srcdir)/small-neutral-shield.png \
	$(srcdir)/small-orange-shield.png \
	$(srcdir)/small-red-shield.png \
	$(srcdir)/small-white-shield.png \
	$(srcdir)/small-yellow-shield.png

EXTRA_DIST = \
	default.lws.in \
	README \
	$(IMAGES)
