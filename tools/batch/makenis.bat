pushd
cd \homeworld

if "%1" == "" (goto makeAll)

if exist datasrc\nis\%1\%1.sc (cl /E /u /FI%HW_Root%\Src\game\speechevent.h /FI%HW_Root%\Src\game\SoundEventDefs.h /FI%HW_Root%\Src\game\soundmusic.h  -nologo datasrc\nis\%1\%1.sc >! data\nis\%1.script)
if exist datasrc\nis\%1\%1r1.sc (cl /E /u /FI%HW_Root%\Src\game\speechevent.h /FI%HW_Root%\Src\game\SoundEventDefs.h /FI%HW_Root%\Src\game\soundmusic.h  -nologo datasrc\nis\%1\%1r1.sc >! data\nis\%1r1.script)
if exist datasrc\nis\%1\%1r2.sc (cl /E /u /FI%HW_Root%\Src\game\speechevent.h /FI%HW_Root%\Src\game\SoundEventDefs.h /FI%HW_Root%\Src\game\soundmusic.h  -nologo datasrc\nis\%1\%1r2.sc >! data\nis\%1r2.script)
if exist datasrc\nis\%1\%1r1a.sc (cl /E /u /FI%HW_Root%\Src\game\speechevent.h /FI%HW_Root%\Src\game\SoundEventDefs.h /FI%HW_Root%\Src\game\soundmusic.h  -nologo datasrc\nis\%1\%1r1a.sc >! data\nis\%1r1a.script)
if exist datasrc\nis\%1\%1r2a.sc (cl /E /u /FI%HW_Root%\Src\game\speechevent.h /FI%HW_Root%\Src\game\SoundEventDefs.h /FI%HW_Root%\Src\game\soundmusic.h  -nologo datasrc\nis\%1\%1r2a.sc >! data\nis\%1r2a.script)
if exist datasrc\nis\%1\%1r1b.sc (cl /E /u /FI%HW_Root%\Src\game\speechevent.h /FI%HW_Root%\Src\game\SoundEventDefs.h /FI%HW_Root%\Src\game\soundmusic.h  -nologo datasrc\nis\%1\%1r1b.sc >! data\nis\%1r1b.script)
if exist datasrc\nis\%1\%1r2b.sc (cl /E /u /FI%HW_Root%\Src\game\speechevent.h /FI%HW_Root%\Src\game\SoundEventDefs.h /FI%HW_Root%\Src\game\soundmusic.h  -nologo datasrc\nis\%1\%1r2b.sc >! data\nis\%1r2b.script)
if exist datasrc\nis\%1\%1r1c.sc (cl /E /u /FI%HW_Root%\Src\game\speechevent.h /FI%HW_Root%\Src\game\SoundEventDefs.h /FI%HW_Root%\Src\game\soundmusic.h  -nologo datasrc\nis\%1\%1r1c.sc >! data\nis\%1r1c.script)
if exist datasrc\nis\%1\%1r2c.sc (cl /E /u /FI%HW_Root%\Src\game\speechevent.h /FI%HW_Root%\Src\game\SoundEventDefs.h /FI%HW_Root%\Src\game\soundmusic.h  -nologo datasrc\nis\%1\%1r2c.sc >! data\nis\%1r2c.script)
if exist datasrc\nis\%1\%1r1d.sc (cl /E /u /FI%HW_Root%\Src\game\speechevent.h /FI%HW_Root%\Src\game\SoundEventDefs.h /FI%HW_Root%\Src\game\soundmusic.h  -nologo datasrc\nis\%1\%1r1d.sc >! data\nis\%1r1d.script)
if exist datasrc\nis\%1\%1r2d.sc (cl /E /u /FI%HW_Root%\Src\game\speechevent.h /FI%HW_Root%\Src\game\SoundEventDefs.h /FI%HW_Root%\Src\game\soundmusic.h  -nologo datasrc\nis\%1\%1r2d.sc >! data\nis\%1r2d.script)
if exist datasrc\nis\%1\%1r1e.sc (cl /E /u /FI%HW_Root%\Src\game\speechevent.h /FI%HW_Root%\Src\game\SoundEventDefs.h /FI%HW_Root%\Src\game\soundmusic.h  -nologo datasrc\nis\%1\%1r1e.sc >! data\nis\%1r1e.script)
if exist datasrc\nis\%1\%1r2e.sc (cl /E /u /FI%HW_Root%\Src\game\speechevent.h /FI%HW_Root%\Src\game\SoundEventDefs.h /FI%HW_Root%\Src\game\soundmusic.h  -nologo datasrc\nis\%1\%1r2e.sc >! data\nis\%1r2e.script)
if exist datasrc\nis\%1\%1r1f.sc (cl /E /u /FI%HW_Root%\Src\game\speechevent.h /FI%HW_Root%\Src\game\SoundEventDefs.h /FI%HW_Root%\Src\game\soundmusic.h  -nologo datasrc\nis\%1\%1r1f.sc >! data\nis\%1r1f.script)
if exist datasrc\nis\%1\%1r2f.sc (cl /E /u /FI%HW_Root%\Src\game\speechevent.h /FI%HW_Root%\Src\game\SoundEventDefs.h /FI%HW_Root%\Src\game\soundmusic.h  -nologo datasrc\nis\%1\%1r2f.sc >! data\nis\%1r2f.script)
if exist datasrc\nis\%1\%1r1g.sc (cl /E /u /FI%HW_Root%\Src\game\speechevent.h /FI%HW_Root%\Src\game\SoundEventDefs.h /FI%HW_Root%\Src\game\soundmusic.h  -nologo datasrc\nis\%1\%1r1g.sc >! data\nis\%1r1g.script)
if exist datasrc\nis\%1\%1r2g.sc (cl /E /u /FI%HW_Root%\Src\game\speechevent.h /FI%HW_Root%\Src\game\SoundEventDefs.h /FI%HW_Root%\Src\game\soundmusic.h  -nologo datasrc\nis\%1\%1r2g.sc >! data\nis\%1r2g.script)
if exist datasrc\nis\%1\%1a.sc (cl /E /u /FI%HW_Root%\Src\game\speechevent.h /FI%HW_Root%\Src\game\SoundEventDefs.h /FI%HW_Root%\Src\game\soundmusic.h  -nologo datasrc\nis\%1\%1a.sc >! data\nis\%1a.script)
if exist datasrc\nis\%1\%1b.sc (cl /E /u /FI%HW_Root%\Src\game\speechevent.h /FI%HW_Root%\Src\game\SoundEventDefs.h /FI%HW_Root%\Src\game\soundmusic.h  -nologo datasrc\nis\%1\%1b.sc >! data\nis\%1b.script)
if exist datasrc\nis\%1\%1c.sc (cl /E /u /FI%HW_Root%\Src\game\speechevent.h /FI%HW_Root%\Src\game\SoundEventDefs.h /FI%HW_Root%\Src\game\soundmusic.h  -nologo datasrc\nis\%1\%1c.sc >! data\nis\%1c.script)

if exist datasrc\nis\%1\%1.lws (lwexport -+datasrc\nis\%1\%1.lws -=data\nis\%1.nis)
if exist datasrc\nis\%1\%1r1.lws (lwexport -+datasrc\nis\%1\%1r1.lws -=data\nis\%1r1.nis)
if exist datasrc\nis\%1\%1r2.lws (lwexport -+datasrc\nis\%1\%1r2.lws -=data\nis\%1r2.nis)
if exist datasrc\nis\%1\%1r1a.lws (lwexport -+datasrc\nis\%1\%1r1a.lws -=data\nis\%1r1a.nis)
if exist datasrc\nis\%1\%1r2a.lws (lwexport -+datasrc\nis\%1\%1r2a.lws -=data\nis\%1r2a.nis)
if exist datasrc\nis\%1\%1r1b.lws (lwexport -+datasrc\nis\%1\%1r1b.lws -=data\nis\%1r1b.nis)
if exist datasrc\nis\%1\%1r2b.lws (lwexport -+datasrc\nis\%1\%1r2b.lws -=data\nis\%1r2b.nis)
if exist datasrc\nis\%1\%1r1c.lws (lwexport -+datasrc\nis\%1\%1r1c.lws -=data\nis\%1r1c.nis)
if exist datasrc\nis\%1\%1r2c.lws (lwexport -+datasrc\nis\%1\%1r2c.lws -=data\nis\%1r2c.nis)
if exist datasrc\nis\%1\%1r1d.lws (lwexport -+datasrc\nis\%1\%1r1d.lws -=data\nis\%1r1d.nis)
if exist datasrc\nis\%1\%1r2d.lws (lwexport -+datasrc\nis\%1\%1r2d.lws -=data\nis\%1r2d.nis)
if exist datasrc\nis\%1\%1r1e.lws (lwexport -+datasrc\nis\%1\%1r1e.lws -=data\nis\%1r1e.nis)
if exist datasrc\nis\%1\%1r2e.lws (lwexport -+datasrc\nis\%1\%1r2e.lws -=data\nis\%1r2e.nis)
if exist datasrc\nis\%1\%1r1f.lws (lwexport -+datasrc\nis\%1\%1r1f.lws -=data\nis\%1r1f.nis)
if exist datasrc\nis\%1\%1r2f.lws (lwexport -+datasrc\nis\%1\%1r2f.lws -=data\nis\%1r2f.nis)
if exist datasrc\nis\%1\%1r1g.lws (lwexport -+datasrc\nis\%1\%1r1g.lws -=data\nis\%1r1g.nis)
if exist datasrc\nis\%1\%1r2g.lws (lwexport -+datasrc\nis\%1\%1r2g.lws -=data\nis\%1r2g.nis)
if exist datasrc\nis\%1\%1a.lws (lwexport -+datasrc\nis\%1\%1a.lws -=data\nis\%1a.nis)
if exist datasrc\nis\%1\%1b.lws (lwexport -+datasrc\nis\%1\%1b.lws -=data\nis\%1b.nis)
if exist datasrc\nis\%1\%1c.lws (lwexport -+datasrc\nis\%1\%1c.lws -=data\nis\%1c.nis)

goto end

:makeAll
wmake /h /z /f datasrc\nis\makefile %1 %2 %3 %4 %5

:end

popd

