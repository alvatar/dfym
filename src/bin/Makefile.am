AM_CFLAGS =-I$(top_srcdir)/src/bin -I$(top_srcdir)/src/lib -Wall

bin_PROGRAMS = dfym
dfym_SOURCES = main.c

dfym_LDADD =  $(top_builddir)/src/lib/libdfym-base.a $(AM_LDFLAGS)
