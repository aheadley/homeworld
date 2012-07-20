# Some extra features if using Win32
if PLATFORM_WIN32
libhw_SDL_a_WIN32SOURCES = debugwnd.c wave.c wave.h
else
libhw_SDL_a_WIN32SOURCES = win32glue.c
endif

INCLUDES = -I$(top_srcdir)/src/Game -I$(top_srcdir)/src/JPG -I$(top_srcdir)/src/Ships -I$(top_srcdir)/include/glide2 -I$(top_srcdir)/include/Titan

noinst_LIBRARIES = libhw_SDL.a
libhw_SDL_a_SOURCES = avi.c avi.h color.c color.h debugwnd.h devstats.h font.c font.h fqcodec.h fqeffect.h fquant.h glcaps.c glcaps.h glcompat.c glcompat.h gldefines.h gldll.c gldll.h glext.h glinc.h leakyfaucet.h light.c light.h LinkEnd.c LinkLimits.h LinkStart.c main.c main.h mainrgn.c mainrgn.h mainswitches.h mixfft.c mouse.c mouse.h prim2d.c prim2d.h prim3d.c prim3d.h Queue.c Queue.h regkey.h render.c render.h resource.h rglu.c rglu.h rinit.c rinit.h screenshot.c screenshot.h smixer.c soundcmn.h soundlow.c soundlow.h sstglide.c sstglide.h sstream.c Subtitle.c Subtitle.h texreg.c texreg.h TimeoutTimer.c TimeoutTimer.h Titan.c Titan.h TitanInterfaceC.h TitanInterfaceC.stub.c trails.c trails.h utility.c utility.h $(libhw_SDL_a_WIN32SOURCES)

# render.c contains a task function, which requires ebp to be used as the
# frame pointer.
render.$(OBJEXT): render.c
	if $(COMPILE) -fno-omit-frame-pointer -MT $@ -MD -MP -MF "$(DEPDIR)/$*.Tpo" \
	  -c -o $@ `test -f '$<' || echo '$(srcdir)/'`$<; \
	then mv "$(DEPDIR)/$*.Tpo" "$(DEPDIR)/$*.Po"; \
	else rm -f "$(DEPDIR)/$*.Tpo"; exit 1; \
	fi
