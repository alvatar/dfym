if DEBUG
  AM_CFLAGS =-I$(top_srcdir)/src/lib -Wall -g -O3
else
  AM_CFLAGS =-I$(top_srcdir)/src/lib -Wall
endif


# The libraries to build
noinst_LIBRARIES = libdfym-base.a

# The files to add to the library and to the source distribution
libdfym_base_a_SOURCES = \
										     $(libdfym_base_a_HEADERS) \
										     dfym_base.c
