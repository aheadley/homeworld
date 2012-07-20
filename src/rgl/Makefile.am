# Software renderer hates optimization at this point.  Don't know what file is
# causing problems, and we still need it for the menu when the game is started
# up for the first time, so compile it without optimization.
# Since COMPILE isn't defined at this point in the Makefile, we'll just tag
# "-O0" at the end of CFLAGS, since it is placed after all other options in
# COMPILE
CFLAGS   += -O0
CXXFLAGS += -O0

lib_LTLIBRARIES = librgl.la librglsw.la

# Not quite ready for prime time...
#nodist_LTLIBRARIES = librglu.la librglfx.la

librgl_la_SOURCES = asm.c asm.h clip.c clip.h fixed.h gldefines.h hash.c hash.h invert.c kgl.c kgl.h kgl_macros.h kvb.c kvb.h maths.c maths.h rglext.c rglext.h wgl.c
librgl_la_LDFLAGS = -avoid-version -no-undefined

#librglu_la_SOURCES = fixed.h gldefines.h hash.h invert.c kgl.h kgl_macros.h kglu.c kvb.h maths.c maths.h rglext.h
#librglu_la_LIBADD = librgl.la
#librglu_la_LDFLAGS = -avoid-version -no-undefined

librglsw_la_SOURCES = asm.c asm.h sdldraw.c kgl.h lines.c lines.h linetemp.h newblendtextri.c newpersptri.c newrgbapersptri.c newrgbatextri.c newtextri.c newtriangle.c newtriangle.h newtritemp.h scan.c scan.h span.c span.h stipple.c.h stipple.h swdriver.c swdrv.h
librglsw_la_LIBADD = librgl.la
librglsw_la_LDFLAGS = -avoid-version -no-undefined

#librglfx_la_SOURCES = fxdriver.c fxdrv.h fxtmg.c kgl.h
#librglfx_la_LIBADD = librgl.la
#librglfx_la_LDFLAGS = -avoid-version -no-undefined -lglide2x

# asm.c requires SSE instructions, but we don't want to force SSE instructions
# throughout the project.
asm.o: asm.c
	if $(COMPILE) -msse -MT $@ -MD -MP -MF "$(DEPDIR)/$*.Tpo" \
	  -c -o $@ `test -f '$<' || echo '$(srcdir)/'`$<; \
	then mv "$(DEPDIR)/$*.Tpo" "$(DEPDIR)/$*.Po"; \
	else rm -f "$(DEPDIR)/$*.Tpo"; exit 1; \
	fi

asm.lo: asm.c
	if $(LTCOMPILE) -msse -MT $@ -MD -MP -MF "$(DEPDIR)/$*.Tpo" \
	  -c -o $@ `test -f '$<' || echo '$(srcdir)/'`$<; \
	then mv "$(DEPDIR)/$*.Tpo" "$(DEPDIR)/$*.Plo"; \
	else rm -f "$(DEPDIR)/$*.Tpo"; exit 1; \
	fi
