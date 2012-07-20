INCLUDES = -I$(top_srcdir)/src/Game

noinst_PROGRAMS = kas2c

# Needed for access to tokens.
AM_YFLAGS="-d"
AM_LFLAGS="-i"

kas2c_SOURCES = parser.y lexer.l kas2c.c kas2c.h
kas2c_LDADD = @LEXLIB@
