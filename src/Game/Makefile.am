INCLUDES = -I$(top_srcdir)/src/JPG -I$(top_srcdir)/src/rgl -I$(top_srcdir)/src/SDL -I$(top_srcdir)/src/Ships -I$(top_srcdir)/include/Titan

# Task functions rely on the frame pointer, but it doesn't seem that we can
# enable it on a per-function basis with GCC.
# Another option would be to split off the tasks into separate sources and
# compile them without using the frame pointer.
CFLAGS += -fno-omit-frame-pointer

noinst_LIBRARIES = libhw_Game.a
libhw_Game_a_SOURCES = AIAttackMan.c AIAttackMan.c.bak AIAttackMan.h AIDefenseMan.c AIDefenseMan.h AIEvents.c AIEvents.h AIFeatures.h AIFleetMan.c AIFleetMan.h AIHandler.c AIHandler.h AIMoves1.c.h AIMoves2.c.h AIMoves.c AIMoves.h AIOrders2.c.h AIOrders.c AIOrders.h AIPlayer.c AIPlayer.h AIResourceMan.c AIResourceMan.h AIShip.c AIShip.h AITeam.c AITeam.h AITrack.c AITrack.h AIUtilities.c AIUtilities.h AIVar.c AIVar.h Alliance.c Alliance.h Animatic.c Animatic.h Attack.c Attack.h Attributes.h AutoDownloadMap.c AutoDownloadMap.h AutoLOD.c AutoLOD.h Battle.c Battle.h BigFile.c BigFile.h BitIO.c BitIO.h Blobs.c Blobs.h BMP.c BMP.h Bounties.c Bounties.h B-Spline.c B-Spline.h BTG.c BTG.h Camera.c CameraCommand.c CameraCommand.h Camera.h Captaincy.c Captaincy.h ChannelFSM.c ChannelFSM.h Chatting.c Chatting.h Clamp.c Clamp.h ClassDefs.h Clipper.c Clipper.h Clouds.c Clouds.h Collision.c Collision.h ColPick.c ColPick.h CommandDefs.h CommandLayer.c CommandLayer.h CommandNetwork.c CommandNetwork.h CommandWrap.c CommandWrap.h ConsMgr.c ConsMgr.h Crates.c Crates.h CRC32.c CRC32.h Damage.c Damage.h Debug.c Debug.h Demo.c Demo.h Dock.c Dock.h ETG.c ETG.h Eval.c Eval.h FastMath.c FastMath.h FEColour.h FEFlow.c FEFlow.h FEReg.c FEReg.h File.c File.h FILE_LIST_DELETE_THIS.txt FlightMan.c FlightManDefs.h FlightMan.h FontReg.c FontReg.h Formation.c FormationDefs.h Formation.h GameChat.c GameChat.h GamePick.c GamePick.h GameStats.h Globals.c Globals.h Gun.c Gun.h Hash.c Hash.h HorseRace.c HorseRace.h HS.c HS.h InfoOverlay.c InfoOverlay.h KAS.c KASFunc.c KASFunc.h KAS.h KeyBindings.c KeyBindings.h Key.c Key.h KNITransform.c LagPrint.c LagPrint.h LaunchMgr.c LaunchMgr.h LetterDefs.h LevelLoad.c LevelLoad.h LilOptions.h LinkedList.c LinkedList.h LOD.c LOD.h LZSS.c LZSS.h MadLinkIn.c MadLinkInDefs.h MadLinkIn.h Matrix.c Matrix.h MaxMultiplayer.h Memory.c Memory.h MeshAnim.c MeshAnim.h Mesh.c Mesh.h MEX.c MEX.h MultiplayerGame.c MultiplayerGame.h MultiplayerLANGame.c MultiplayerLANGame.h NavLights.c NavLights.h Nebulae.c Nebulae.h NetCheck.c NetCheck.h NIS.c NIS.h NumberDefs.h Objectives.c Objectives.h ObjTypes.c ObjTypes.h Options.c Options.h Particle.c Particle.h Physics.c Physics.h PiePlate.c PiePlate.h Ping.c Ping.h PlugScreen.c PlugScreen.h ProfileTimers.c ProfileTimers.h RaceDefs.h Randy.c Randy.h Region.c Region.h ResCollect.c ResCollect.h ResearchAPI.c ResearchAPI.h ResearchGUI.c ResearchGUI.h SaveGame.c SaveGame.h ScenPick.c ScenPick.h Scroller.c Scroller.h Select.c Select.h Sensors.c Sensors.h Shader.c Shader.h ShipDefs.h ShipSelect.c ShipSelect.h Ships.h ShipView.c ShipView.h SinglePlayer.c SinglePlayer.h SoundEvent.c SoundEventDefs.h SoundEvent.h SoundEventPlay.c SoundEventPrivate.h SoundEventStop.c SoundMusic.h SoundStructs.h SpaceObj.h SpeechEvent.c SpeechEvent.h Star3d.c Star3d.h Stats.c StatScript.c StatScript.h Stats.h Strings.c Strings.h StringsOnly.h Switches.h Tactical.c Tactical.h Tactics.c Tactics.h TaskBar.c TaskBar.h Task.c Task.h Teams.c Teams.h Timer.c Timer.h TitanNet.c TitanNet.h Tracking.c Tracking.h TradeMgr.c TradeMgr.h Transformer.c Transformer.h Tutor.c Tutor.h Tweak.c Tweak.h Twiddle.c Twiddle.h Types.h UIControls.c UIControls.h Undo.c Undo.h Universe.c Universe.h UnivUpdate.c UnivUpdate.h Vector.c Vector.h VolTweakDefs.h Volume.c Volume.h

# KNITransform.c requires SSE instructions, but we don't want to force SSE
# instructions throughout the project.
KNITransform.o: KNITransform.c
	if $(COMPILE) -msse -MT $@ -MD -MP -MF "$(DEPDIR)/$*.Tpo" \
	  -c -o $@ `test -f '$<' || echo '$(srcdir)/'`$<; \
	then mv "$(DEPDIR)/$*.Tpo" "$(DEPDIR)/$*.Po"; \
	else rm -f "$(DEPDIR)/$*.Tpo"; exit 1; \
	fi

# Optimization thrashes Task.c, ETG.c, and FastMath.c (although we should
# definitely fix FastMath.c at some point, being that it is straight C).
Task.o: Task.c
	if $(COMPILE) -O0 -MT $@ -MD -MP -MF "$(DEPDIR)/$*.Tpo" \
	  -c -o $@ `test -f '$<' || echo '$(srcdir)/'`$<; \
	then mv "$(DEPDIR)/$*.Tpo" "$(DEPDIR)/$*.Po"; \
	else rm -f "$(DEPDIR)/$*.Tpo"; exit 1; \
	fi

ETG.o: ETG.c
	if $(COMPILE) -O0 -MT $@ -MD -MP -MF "$(DEPDIR)/$*.Tpo" \
	  -c -o $@ `test -f '$<' || echo '$(srcdir)/'`$<; \
	then mv "$(DEPDIR)/$*.Tpo" "$(DEPDIR)/$*.Po"; \
	else rm -f "$(DEPDIR)/$*.Tpo"; exit 1; \
	fi

FastMath.o: FastMath.c
	if $(COMPILE) -O0 -MT $@ -MD -MP -MF "$(DEPDIR)/$*.Tpo" \
	  -c -o $@ `test -f '$<' || echo '$(srcdir)/'`$<; \
	then mv "$(DEPDIR)/$*.Tpo" "$(DEPDIR)/$*.Po"; \
	else rm -f "$(DEPDIR)/$*.Tpo"; exit 1; \
	fi
