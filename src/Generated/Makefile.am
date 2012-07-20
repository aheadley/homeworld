INCLUDES = -I$(top_srcdir)/src/Game -I$(top_srcdir)/src/JPG -I$(top_srcdir)/src/SDL -I$(top_srcdir)/src/Ships -I$(top_srcdir)/src/SinglePlayer -I$(top_srcdir)/src/Tutorials

# We need to write to the strings defined in the mission code files (only
# temporary writes, for things such as checking the pixel width of a section
# of a string).
AM_CFLAGS = -fwritable-strings

noinst_LIBRARIES = libhw_Generated.a
libhw_Generated_a_SOURCES = Mission01.c Mission01.func.c Mission01.h Mission02.c Mission02.func.c Mission02.h Mission03.c Mission03.func.c Mission03.h Mission04.c Mission04.func.c Mission04.h Mission05.c Mission05.func.c Mission05.h Mission06.c Mission06.func.c Mission06.h Mission07.c Mission07.func.c Mission07.h Mission08.c Mission08.func.c Mission08.h Mission09.c Mission09.func.c Mission09.h Mission10.c Mission10.func.c Mission10.h Mission11.c Mission11.func.c Mission11.h Mission12.c Mission12.func.c Mission12.h Mission13.c Mission13.func.c Mission13.h Mission14.c Mission14.func.c Mission14.h Mission15.c Mission15.func.c Mission15.h Mission16.c Mission16.func.c Mission16.h Tutorial1.c Tutorial1.func.c Tutorial1.h

# Kinda weak dependency on kas2c (doesn't build it if it doesn't exist), but
# we should be fine as long as it's built first.
%.c %.h %.func.c: %.kp $(top_builddir)/src/kas2c/kas2c$(EXEEXT)
	$(top_builddir)/src/kas2c/kas2c$(EXEEXT) $< $(basename $@).c $(basename $@).h $(basename $@).func.c

# Single-player missions (note the "-x c" option to force C
# preprocessing...don't know if this is very portable)
Mission%.kp: ../SinglePlayer/Mission%.kas
	$(CPP) -x c $(INCLUDES) $< > $@

# Tutorials (again, uses "-x c" to force C preprocessing)
Tutorial%.kp: ../Tutorials/Tutorial%.kas
	$(CPP) -x c $(INCLUDES) $< > $@
