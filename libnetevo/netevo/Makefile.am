# ===========================================================================
# NetEvo Foundation Library
# Copyright (C) 2011 Thomas E. Gorochowski <tgorochowski@me.com>
# Bristol Centre for Complexity Sciences, University of Bristol, Bristol, UK
# ===========================================================================

## Source directory

h_sources =  netevo.h \
             system.h\
             simulate.h \
             evolve.h \
             evolve_sa.h \
             visual.h \
             gml.h
			
cc_sources = system.cc \
             simulate.cc \
             evolve_sa.cc \
             visual.cc \
             gml.cc

library_includedir=$(includedir)/$(GENERIC_LIBRARY_NAME)
library_include_HEADERS = $(h_sources)

INCLUDES = -I$(top_srcdir)

lib_LTLIBRARIES= libnetevo.la
libnetevo_la_SOURCES= $(h_sources) $(cc_sources)
libnetevo_la_LDFLAGS= -version-info $(GENERIC_LIBRARY_VERSION) -release $(GENERIC_RELEASE)

AUTOMAKE_OPTIONS = foreign
LDADD = -lemon
