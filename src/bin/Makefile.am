if DEBUG
  AM_CFLAGS =-I$(top_srcdir)/src/bin -I$(top_srcdir)/src/lib -Wall -g -O3
else
  AM_CFLAGS =-I$(top_srcdir)/src/bin -I$(top_srcdir)/src/lib -Wall -O3
endif

AM_CFLAGS += $(GLIB_CFLAGS)
AM_CFLAGS += $(SQLITE3_CFLAGS)

bin_PROGRAMS = dfym
dfym_SOURCES = main.c

dfym_LDADD = $(top_builddir)/src/lib/libdfym-base.a $(AM_LDFLAGS)
dfym_LDADD += $(GLIB_LIBS)
dfym_LDADD += $(SQLITE3_LIBS)
