/******************************************
****                                   ****
****             ALL SHIPS             ****
****                                   ****
******************************************/

#define FrickinForgotToStartAtZero				0

/* ALL SHIPS - Capital Ships */
#define STAT_Cap_Damaged_Specific				1	//	1. STATUS_CapitalShip-Damaged[Report] (ALL SHIPS)
#define STAT_Cap_Surroundings_Enemy				2	//	2. STATUS_CapitalShip-Surroundings[Report] (ALL SHIPS)
#define STAT_Cap_Surroundings_Resources			3	//	2. STATUS_CapitalShip-Surroundings[Report] (ALL SHIPS)
#define STAT_Cap_Surroundings_Nothing			4	//	2. STATUS_CapitalShip-Surroundings[Report] (ALL SHIPS)
#define STAT_Cap_LargeHit_Query					5	//	7. STATUS_CapitalShip-LargeHit[Query] (ALL SHIPS)
#define STAT_Cap_LargeHit_More_Query			6	//	7. STATUS_CapitalShip-LargeHit[Query] (ALL SHIPS)
#define STAT_Cap_LargeHit_Resp					7	//	8. STATUS_CapitalShip-LargeHit[Response] (ALL SHIPS)
#define STAT_Cap_LargeHit_More_Resp				8	//	8. STATUS_CapitalShip-LargeHit[Response] (ALL SHIPS)
#define STAT_Cap_Damaged						9	//	9. STATUS_CapitalShip-Damaged[Report] (ALL SHIPS)
#define STAT_Cap_Dies							10	//	12. STATUS_CapitalShip_Dies (ALL SHIPS)
#define STAT_Cap_Dies_Report					11	//	13. STATUS_CapitalShip-Dies['A'Report] (ALL SHIPS)
#define COMM_Cap_Retire							12	//	15. COMMAND_CapitalShip-Retire (ALL SHIPS)

/* ALL SHIPS - Ship-Specific Events */
#define STAT_Strike_Damaged						13	//	16. STATUS_StrikeCraft_Damaged (ALL SHIPS)
#define COMM_Strike_Retired						14	//	17. COMMAND_StrikeCraft-Retired (ALL SHIPS)
#define COMM_SCVette_Salvage					15	//	26. COMMAND_SalvageCorvette-Salvage
#define STAT_SCVette_TargetAcquired				16	//	27. STATUS_SalvageCorvette-TargetAcquired (ALL SHIPS)
#define STAT_RepVette_StartedRepairs			17	//	29. STATUS_RepairCorvette-StartedRepairs (ALL SHIPS)
#define STAT_RepVette_FinishedRepairs			18	//	30. STATUS_RepairCorvette-FinishedRepairs (ALL SHIPS)
#define COMM_DDF_LaunchDrones					19	//	33. COMMAND_DroneFrigate-LaunchDrones (ALL SHIPS)
#define COMM_DDF_RetractDrones					20	//	34. COMMAND_DroneFrigate-RetractDrones (ALL SHIPS)
#define STAT_Carrier_Damaged					21	//	36. STATUS_Carrier-Damaged[Report] (ALL SHIPS)
#define STAT_Cruiser_Damaged					22	//	38. STATUS_HeavyCruiser-Damaged[Report] (ALL SHIPS)
#define STAT_MoShip_LargeAttack					23	//	41. STATUS_MothershipUnderLargeAttack[Report] (ALL SHIPS)
#define STAT_MoShip_LargeAttack_Resp			24	//	42. STATUS_MothershipUnderLargeAttack[Response] (ALL SHIPS)
#define COMM_Grav_On							25	//	44. COMMAND_GravityWellGenerator-On (ALL SHIPS)
#define STAT_Grav_Collapse						26	//	45. STATUS_GravityWellGenerator-Collapse (ALL SHIPS)
#define COMM_Cloak_CloakingOn					27	//	47. COMMAND_CloakingShips-CloakingOn[Report] (ALL SHIPS)
#define STAT_Cloak_CloakingOn_Resp				28	//	48. STATUS_CloakingShips-CloakingOn[Response] (ALL SHIPS)
#define COMM_Cloak_InsufficientPower			29	//	49. COMMAND_CloakingShips-InsufficientPowerToCloak (ALL SHIPS)
#define STAT_Cloak_CloakPowerLow				30	//	50. STATUS_CloakingShips-CloakingPowerLow[Report] (ALL SHIPS)
#define COMM_Cloak_Decloak						31	//	51. COMMAND_CloakingShips-DeCloak (ALL SHIPS)

/* ALL SHIPS - Resourcing */
#define STAT_ResCol_Grp_Damaged					32	//	52. STATUS_GroupCollectors-Damaged[Report] (ALL SHIPS)
#define STAT_ResCol_Destroyed					33	//	53. STATUS_ResourceCollector-Destroyed[Report] (ALL SHIPS)
#define STAT_ResCol_ResourcesTransferred		34	//	54. STATUS_ResourceCollector-ResourcesTransferred (ALL SHIPS)
#define COMM_ResCol_Harvest						35	//	55. COMMAND_ResourceCollector-Harvest (ALL SHIPS)
#define STAT_ResCol_Full						36	//	57. STATUS_ResourceCollector-Full (ALL SHIPS)
#define STAT_ResCol_Damaged						37	//	58. STATUS_ResourceController-Damaged (ALL SHIPS)

/* ALL SHIPS - Dogfighting*/
#define STAT_Fighter_WingmanChased				38	//	60. STATUS_Fighter-WingmanChased[Report] (ALL SHIPS)
#define STAT_Fighter_WingmanChased_Rsp			39	//	61. STATUS_Fighter-WingmanChased[Response] (ALL SHIPS)
#define STAT_Fighter_WingmanHit					40	//	62. STATUS_Fighter-WingmanHit[Report] (ALL SHIPS)
#define STAT_Fighter_WingmanHit_Rsp				41	//	63. STATUS_Fighter-WingmanHit[Response] (ALL SHIPS)
#define STAT_Fighter_WingmanLethal_Rsp			42	//	65. STATUS_Fighter-WingmanLethallyDamaged[Response] (ALL SHIPS)
#define STAT_Fighter_WingmanDies				43	//	66. STATUS_Fighter-WingmanDies[Report] (ALL SHIPS)
#define STAT_Fighter_LeaderChased				44	//	68. STATUS_Fighter-LeaderBeingChased[Report] (ALL SHIPS)
#define CHAT_Fighter_Wingman					45	//	67. CHATTER_Fighter-Wingman[Report] (ALL SHIPS)
#define COMM_LInt_Kamikaze						46	//	69. COMMAND-LightInterceptor-Kamikaze (ALL SHIPS)

/* ALL SHIPS - Battle Events */
#define STAT_AssGrp_UnderAttack					47	//	70. STATUS_AssignedGroup-UnderAttack[Report] (ALL SHIPS)
#define STAT_Grp_UnderAttack					48	//	71. STATUS_GenericGroup-UnderAttack[Report] (ALL SHIPS)
#define STAT_Grp_UnderAttack_Rsp				49	//	72. STATUS_GenericGroup-UnderAttack[Response] (ALL SHIPS)
#define STAT_Grp_StartBattleDisadvant			50	//	73. STATUS_GenericGroup-StartBattleDisadvantaged[Report] (ALL SHIPS)
#define STAT_Grp_StartBattleDisadvant_Rsp		51	//	74. STATUS_GenericGroup-StartBattleDisadvantaged[Response] (ALL SHIPS)
#define STAT_Grp_StartBattleFair				52	//	75. STATUS_GenericGroup-StartBattleFair[Report] (ALL SHIPS)
#define STAT_Grp_StartBattleFair_Rsp			53	//	76. STATUS_GenericGroup-StartBattleFair[Response] (ALL SHIPS)
#define STAT_Grp_StartBattleAdvantaged			54	//	77. STATUS_GenericGroup-StartBattleAdvantaged[Report] (ALL SHIPS)
#define STAT_Grp_StartBattleAdvantaged_Rsp		55	//	78. STATUS_GenericGroup-StartBattleAdvantaged[Response] (ALL SHIPS)
#define COMM_Group_Retreat						56	//	79. COMMAND_GenericGroup-Retreat (ALL SHIPS)
#define COMM_Group_Retreat_Rsp					57	//	80. COMMAND_GenericGroup-Retreat[Response] (ALL SHIPS)
#define STAT_Grp_WinningBattle					58	//	81. STATUS_GenericGroup-WinningBattle[Report] (ALL SHIPS)
#define STAT_Grp_WinningBattle_Rsp				59	//	82. STATUS_GenericGroup-WinningBattle[Response] (ALL SHIPS)
#define STAT_Grp_KickingButt					60	//	83. STATUS_GenericGroup-KickingButt[Report] (ALL SHIPS)
#define STAT_Grp_KickingButt_Rsp				61	//	84. STATUS_GenericGroup-KickingButt[Response] (ALL SHIPS)
#define STAT_Grp_BattleWon						62	//	85. STATUS_GenericGroup-BattleWon[Report] (ALL SHIPS)
#define STAT_Grp_LosingBattle					63	//	86. STATUS_GenericGroup-LosingBattle[Report] (ALL SHIPS)
#define STAT_Grp_LosingBattle_More				64	//	86. STATUS_GenericGroup-LosingBattle[Report] (ALL SHIPS)
#define STAT_Grp_LosingBattle_Rsp				65	//	87. STATUS_GenericGroup-LosingBattle[Response] (ALL SHIPS)
#define STAT_Grp_LosingBadly					66	//	88. STATUS_GenericGroup-LosingBadly[Report] (ALL SHIPS)
#define STAT_Grp_LosingBadly_More				67	//	88. STATUS_GenericGroup-LosingBadly[Report] (ALL SHIPS)
#define STAT_Grp_LosingBadly_Rsp				68	//	89. STATUS_GenericGroup-LosingBadly[Response] (ALL SHIPS)
#define CHAT_Grp_PositiveBattle					69	//	90. CHATTER_GenericGroup-PositiveBattle[Report] (ALL SHIPS)
#define CHAT_Grp_PositiveBattle_More			70	//	90. CHATTER_GenericGroup-PositiveBattle[Report] (ALL SHIPS)
#define CHAT_Grp_NegativeBattle					71	//	91. CHATTER_GenericGroup-NegativeBattle[Report] (ALL SHIPS)
#define CHAT_Grp_NegativeBattle_More			72	//	91. CHATTER_GenericGroup-NegativeBattle[Report] (ALL SHIPS)
#define CHAT_Grp_NegativeBattle_More2			73	//	91. CHATTER_GenericGroup-NegativeBattle[Report] (ALL SHIPS)
#define CHAT_Grp_NeutralBattle					74	//	92. CHATTER_GenericGroup-NeutralBattle[Report] (ALL SHIPS)
#define CHAT_Grp_NeutralBattle_More				75	//	92. CHATTER_GenericGroup-NeutralBattle[Report] (ALL SHIPS)
#define CHAT_Grp_FriendlyFire					76	//	93. CHATTER_GenericGroup-FriendlyFire (ALL SHIPS)

/* ALL SHIPS - Selection, Movement and Other Commands */
#define COMM_AssGrp_Select						77	//	95. COMMAND_AnyShip-SelectHotkeyGroup (ALL SHIPS)
#define COMM_Selection							78	//	96. COMMAND_AnyShip-Selection (ALL SHIPS)
#define COMM_Move								79	//	97. COMMAND_AnyShip-SpecifyDestination (ALL SHIPS)
#define COMM_MoveCancelled						80	//	98. COMMAND_AnyShip-MoveCancelled (ALL SHIPS)
#define COMM_Attack								81	//	102. COMMAND_AnyShip-Attack (ALL SHIPS)
#define COMM_AttackCancelled					82	//	103. COMMAND_AnyShip-AttackCancelled (ALL SHIPS)
#define COMM_AttackPlayerUnits					83	//	105. COMMAND_AnyShip-ForceAttackPlayerUnits (ALL SHIPS)
#define COMM_AttackPlayerUnits_Cond				84	//	106. COMMAND_AnyShip-ForceAttackPlayerUnits[Conditional] (ALL SHIPS)
#define COMM_AttackResources					85	//	107. COMMAND_AnyShip-ForceAttackResources (ALL SHIPS)
#define COMM_AttackResourcesGeneric				86	//	108. COMMAND_AnyShip-ForceAttackResourcesGeneric (ALL SHIPS)
#define COMM_SetFormation						87	//	111. COMMAND_AnyShip-SetFormation (ALL SHIPS)
#define COMM_Guard								88	//	113. COMMAND_AnyShip-Guard (ALL SHIPS)
#define STAT_Strike_LowOnFuel					89	//	115. STATUS_StrikeCraft-LowOnFuel (ALL SHIPS)
#define STAT_Strike_OutOfFuel					90	//	116. STATUS_StrikeCraft-OutOfFuel (ALL SHIPS)
#define COMM_CannotComply						91	//	119. COMMAND_AnyShip-CannotComply (ALL SHIPS)

/* ALL SHIPS - Hyperspace */
#define COMM_Hyper_Autodock						92	//	120. COMMAND_AnyShip-Hyperspace&Autodock (ALL SHIPS)
#define STAT_Hyper_Abandoned					93	//	122. STATUS_AnyShip-AbandonedByHyperspace (ALL SHIPS)

/* ALL SHIPS - Construction and Docking */
#define CHAT_Const_DuringBattle					94	//	123. CHATTER_CarrierOrMShip-ConstructionDuringBattle (ALL SHIPS)
#define COMM_Const_BuildCapShip					95	//	124. COMMAND_CarrierOrMShip-BuildCapitalShip (ALL SHIPS)
#define CHAT_Const								96	//	125. CHATTER_CarrierOrMShip-Construction (ALL SHIPS)
#define CHAT_Const_More							97	//	125. CHATTER_CarrierOrMShip-Construction (ALL SHIPS)
#define STAT_Cap_Launched						98	//	126. STATUS_CarrierOrMShip-CapitalShipLaunched (ALL SHIPS)
#define STAT_Cap_Welcomed						99	//	127. STATUS_AnyShip-CapitalShipWelcomed (ALL SHIPS)
#define COMM_Strike_Dock						100	//	128. COMMAND_StrikeCraft-Dock (ALL SHIPS)
#define CHAT_Docking							101	//	130. CHATTER_AnyShip-Docking (ALL SHIPS)
#define CHAT_Docking_More						102	//	130. CHATTER_AnyShip-Docking (ALL SHIPS)
#define STAT_Cap_ShipDocked						103	//	131. STATUS_CapitalShips-ShipDocked (ALL SHIPS)
#define STAT_Research_Docking					104	//	133. STATUS_ResearchShip-Docking (ALL SHIPS)
#define STAT_Research_Docked					105	//	134. STATUS_ResearchShip-Docked (ALL SHIPS)

/* ALL SHIPS - New Events */
#define COMM_ResCol_NoMoreRUs					106	//	136. COMMAND_ResourceCollector-NoMoreRUs
#define STAT_Strike_AlsoOutOfFuel				107	//	138. STATUS_StrikeCraft-AlsoOutofFuel
#define STAT_Strike_GenericUsToo				108	//	139. STATUS_StrikeCraft-GenericUsToo
#define	STAT_Strike_StuckInGrav					109	//	140. STATUS_StrikeCraft-StuckbyGravWell
#define STAT_Strike_AttackComplete				110	//	141. STATUS_ StrikeCraft-AttackComplete
#define	COMM_TacticsEvasive						111	//	142. COMMAND_AnyShip-SetTacticsEvasive
#define COMM_TacticsNeutral						112	//	143. COMMAND_AnyShip-SetTacticsNeutral
#define COMM_TacticsAggressive					113	//	144. COMMAND_AnyShip-SetTacticsAggressive
#define COMM_Support							114	//	145. COMMAND_SupportShip-Support
#define COMM_Grav_Off							115	//	146. COMMAND_GravityWellGenerator-Off
#define COMM_HVette_BurstAttack					116	//	147. COMMAND_HeavyCorvette-BurstAttack
#define COMM_MLVette_ForceDrop					117	//	148. COMMAND_MinelayerCorvette-ForceDropMines
#define COMM_MissleDest_VolleyAttack			118	//	149. COMMAND_MissileDestroyer-VolleyAttack
#define COMM_Strike_OutOfFuelMove				119	//	150. COMMAND_StrikeCraft-OutOfFuelMove
#define COMM_Kamikaze_NoTargets					120	//	151. COMMAND_AnyShip-KamikazeNoTargets
#define STAT_Grp_RetreatSuccessful				121	//	152. STATUS_GenericGroup-RetreatSuccessful
#define STAT_AssGrp_RetreatSuccessful			122	//	153. STATUS_AssignedGroup-RetreatSuccessful
#define STAT_Grp_EnemyRetreated					123	//	154. STATUS_GenericGroup-EnemyRetreated
#define STAT_AssGrp_EnemyRetreated				124	//	155. STATUS_AssignedGroup-EnemyRetreated
#define STAT_Grp_AttackFromAlly					125	//	156. STATUS_Group-UnderAttackFromAlly
#define STAT_AssGrp_FriendlyFire				126	//	159. STATUS_AssignedGroup-FriendlyFire
#define STAT_SCVette_DroppingOff				127	//	160. STATUS_SalvageCorvette-DroppingOff
#define STAT_Grp_AttackRetaliate				128	//	161. STATUS_GenericGroup-UnderAttackRetaliate[Report]
#define STAT_Grp_AttackRetaliate_Repeat			129	//	162. STATUS_GenericGroup-UnderAttackRetaliate[Repeat]
#define STAT_AssGrp_AttackRetaliate				130	//	163. STATUS_AssignedGroup-UnderAttackRetaliate[Report]
#define STAT_AssGrp_AttackRetaliate_Repeat		131	//	164. STATUS_AssignedGroup-UnderAttackRetaliate[Report]
#define STAT_Grp_InMineField					132	//	166. STATUS_GenericGroup-InMineField
#define	STAT_AssGrp_InMineField					133	//	167. STATUS_AssignedGroup-InMineField
#define STAT_Research_StationOnline				134	//	170. STATUS_ResearchShip-StationOnline

/* ALL SHIPS - Appendix */
#define STAT_Grp_EnemyFightersDecloaking		135	//	STATUS_GenericGroup_EnemyCloakedFightersDecloaking
#define STAT_Grp_EnemyGeneratorDecloaking		136	//	STATUS_GenericGroup_EnemyCloakGeneratorDecloaking



/******************************************
****                                   ****
****           FLEET COMMAND           ****
****                                   ****
******************************************/

/* FLEET COMMAND - Dialogues and Warnings */
#define STAT_F_AssGrp_UnderAttack				137	//	1. STATUS_AssignedGroup-UnderAttack (FLEET)
#define COMM_F_Group_Assigning					138	//	2. COMMAND_HotkeyGroups-Assigning (FLEET)
#define STAT_F_AssGrp_Victory					139	//	3. STATUS_HotkeyGroup-Victory (FLEET)
#define STAT_F_AssGrp_Defeat					140	//	5. STATUS_HotkeyGroup-Defeat (FLEET)
#define COMM_F_Probe_Deployed					141	//	6. COMMAND_Probe-Deployed (FLEET)
#define STAT_F_Probe_Arrived					142	//	7. STATUS_Probe-Arrived (FLEET)
#define STAT_F_Probe_Expiring					143	//	8. STATUS_Probe-Expiring (FLEET)
#define STAT_F_EnemyProbe_Detected				144	//	9. STATUS_EnemyProbe-Detected (FLEET)
#define STAT_F_ProxSensor_Detection				145	//	10. STATUS_ProximitySensor-Detection (FLEET)
#define STAT_F_ResCol_Damaged					146	//	11. STATUS_ResourceCollector-Damaged[Singular] (FLEET)
#define STAT_F_ResCol_Dies						147	//	12. STATUS_ResourceCollector-Dies (FLEET)
#define STAT_F_ResLevels_Nil					148	//	14. STATUS_ResourceLevels-Nil (FLEET)
#define STAT_F_SuperCap_Damaged					149	//	15. STATUS_SuperCapitalShip-Damaged (FLEET)
#define STAT_F_Cap_Dies							150	//	16. STATUS_CapitalShip-Dies (FLEET)
#define STAT_F_MoShip_UnderAttack				151	//	17. STATUS_Mothership-UnderAttack (FLEET)
#define COMM_F_MoShip_Move						152	//	18. COMMAND_Mothership-Move (FLEET)
#define COMM_F_MoShip_Arrived					153	//	19. COMMAND_Mothership-ArrivesAtDestination (FLEET)
#define COMM_F_Hyper_Engage						154	//	20. COMMAND_Hyperspace_Engage (FLEET)
#define STAT_F_Hyper_Autodock					155	//	21. STATUS_Hyperspace-Autodock (FLEET)
#define STAT_F_Hyper_TakingOff					156	//	22. STATUS_Hyperspace-TakingOff (FLEET)
#define COMM_F_Hyper_Abort						157	//	23. COMMAND_Hyperspace-Abort (FLEET)

/* FLEET COMMAND - Construction */
#define COMM_F_Const_Start						158	//	27. COMMAND_StartConstruction (FLEET)
#define COMM_F_Const_Pause						159	//	28. COMMAND_PauseConstruction (FLEET)
#define STAT_F_ConstInProgs_ResInsufficient		160	//	29. STATUS_ConstructionInProgress-ResourceInsufficient (FLEET)
#define STAT_F_Const_UnitLimit					161	//	31. STATUS_Construction-ShipUnitLimitHit (FLEET)
#define STAT_F_Const_TotalLimit					162	//	32. STATUS_Construction-TotalUnitLimitHit (FLEET)
#define COMM_F_Const_CancelAll					163	//	33. COMMAND_CancelAllJobs (FLEET)
#define COMM_F_Const_CancelBatch				164	//	34. COMMAND_CancelBatch (FLEET)
#define COMM_F_Const_Resume						165	//	36. COMMAND_ResumeConstruction (FLEET)
#define STAT_F_Const_Complete					166	//	37. STATUS_ConstructionComplete (FLEET)
#define STAT_F_Const_CapShipComplete			167	//	38. STATUS_Construction-CapitalShipComplete (FLEET)

/* FLEET COMMAND - Launching */
#define COMM_F_Launch							168	//	39. COMMAND_Launch (FLEET)
#define COMM_F_AutolaunchOn						169	//	40. COMMAND_Autolaunching-On (FLEET)

/* FLEET COMMAND - Research */
#define COMM_F_Research_R1_Start				170	//	41. COMMAND_Research-Start (FLEET)
#define COMM_F_Research_R2_Start				171	//	41. COMMAND_Research-Start (FLEET)
#define COMM_F_Research_Stop					172	//	42. COMMAND_Research-Stop (FLEET)
#define STAT_F_Research_R1_NearCompletion		173	//	43. STATUS_Research-NearingCompletion (FLEET)
#define STAT_F_Research_R2_NearCompletion		174	//	43. STATUS_Research-NearingCompletion (FLEET)
#define STAT_F_Research_R1_Completed			175	//	44. STATUS_Research-Completed (FLEET)
#define STAT_F_Research_R2_Completed			176	//	44. STATUS_Research-Completed (FLEET)
#define STAT_F_Research_CompletedShip			177	//	45. STATUS_Research-CompletedShip (FLEET)

/* FLEET COMMAND - New Events */
#define COMM_F_ScuttleReaffirm					178	//	48. COMMAND_ScuttleReaffirm
#define COMM_F_AssGrp_AddingShips				179	//	49. COMMAND_AssignedGroup-AddingShips
#define STAT_F_ResourcesAllHarvested			180	//	50. STATUS_ResourcesAllHarvested
#define STAT_F_ReassignedToCarrier				181	//	51. STATUS_FleetCommandReassignedtoCarrier
#define STAT_F_GravWellDetected					182	//	52. STATUS_GravWellDetected
#define STAT_F_CloakedShipsDetected				183	//	53. STATUS_CloakedShipsDetected
#define STAT_F_EnemyShipCaptured				184	//	54. STATUS_EnemyShipCaptured
#define STAT_F_ShipStolen						185	//	55. STATUS_ShipStolen
#define STAT_F_ResourceInjectionRecieved		186	//	56. STATUS_ResourceInjectionReceived
#define STAT_F_UnderAttackFromAlly				187	//	58. STATUS_UnderAttackFromAlly
#define STAT_F_Cap_Retired						188	//	59. STATUS_CapitalShipRetired
#define STAT_F_Cap_Retired_PercentRecovered		189	//	59. STATUS_CapitalShipRetired
#define COMM_F_AllianceFormed					190	//	61. COMMAND_AllianceFormed
#define COMM_F_AllianceBroken					191	//	62. COMMAND_AllianceBroken
#define COMM_F_ResourcesTransferred				192	//	63. COMMAND_ResourcesTransferred
#define COMM_F_ResourcesReceived				193	//	64. COMMAND_ResourcesReceived

/* FLEET COMMAND - Single-Player Events:*/
#define STAT_F_HyperspaceSuccessful				194	//	70. STATUS_HyperspaceSuccessful NEW
#define STAT_F_HyperspaceInterrupted			195	//	70.5 STATUS_HyperspaceInterrupted NEW
#define STAT_F_HyperspaceDrivesAvailable		196	//	71. STATUS_HyperspaceDrivesAvailable NEW

/* FLEET COMMAND - Appendix */
#define COMM_F_BuildCenterSelected				197	//	COMMAND_BuildCentreSelected
#define COMM_F_MoShip_Selected					198	//	COMMAND_MothershipSelected
#define COMM_F_MoShip_CantMove					199	//	COMMAND_MothershipCantMove


/******************************************
****                                   ****
****             TUTORIAL              ****
****                                   ****
******************************************/

#define TUT_Intro								200	//	Welcome to the Homeworld Training Mission.  Here we will teach you all the basic controls needed to play Homeworld. Lets begin!  Leftclick on the Next button to proceed.

/* Lesson 1: Using the Camera */
#define TUT_SaveCamera							201	//	Using the camera.  Click the Next button to begin.
#define TUT_CameraMoveDescription				202	//	To rotate the camera, hold the right mouse button and drag the mouse.  Click Next to continue.
#define TUT_CameraMove							203	//	To advance to the next lesson, rotate the camera 180 degrees around your Mothership.
#define TUT_CameraMoveUp						204	//	Next rotate the camera upward to view the Mothership from above.
#define TUT_CameraMoveDown						205	//	Now view the Mothership from below.
#define TUT_CameraBackToMiddle					206	//	Great!  Move the camera back to the horizontal.  Then I'll show you how to zoom in and out.
#define TUT_CameraZoomDescription				207	//	To zoom in or out, hold both mouse buttons and drag the mouse up or down.  If your mouse has a wheel, you can use the wheel to zoom as well.  Click Next to continue.
#define TUT_CameraZoomOut						208	//	Zoom the camera out as far as you can.
#define TUT_CameraZoomIn						209	//	From this position, we get a good overview of the action.  Now zoom the camera in as far as you can.
#define TUT_CameraPractice						210	//	Excellent.  From this position we get a good close-up view of the action.  Practice rotating and zooming the camera.  When you're ready to move on, click the Next button.

/* Lesson 2: TitleBuild */
#define TUT_TaskbarSave							211	//	Building Ships.  Click Next to begin.
#define TUT_TaskbarIntro						212	//	Most of Homeworld's user controls are accessible from a popup taskbar.  Move your cursor to the bottom of the screen to see the taskbar.
#define TUT_TaskbarDescription					213	//	There it is.  Normally the taskbar would disappear when you move your cursor away from it, but let's keep it up to take a closer look.  Click Next to continue.
#define TUT_TaskbarBuildManagerButton			214	//	This button brings up the Build Manager.  Leftclick on the Build button now.

/* Lesson 3: Building Ships */
#define TUT_BuildManagerIntro					215	//	Welcome to the Build Manager.  Here you can build new ships for the fleet.  Click Next to continue.
#define TUT_BuildManagerRUCount					216	//	This is your RU count.  It displays the amount of Resource Units available for building.  Click Next to continue.
#define TUT_BuildManagerShipList				217	//	This is the ship list.  It displays the ships available for building, their cost, and the number of ships in each build queue.  Click Next to continue.
#define TUT_BuildManagerAddResCollector			218	//	Let's build some ships.  Leftclick on the Resource Collector to select it.
#define TUT_3_4_BuildManagerIncrease			219	//	Leftclick again on the Resource Collector to add one to the build queue.
#define TUT_BuildManagerBuildRC					220	//	Leftclick on the Build button to begin construction.
#define TUT_BuildManagerIDProgressBar			221	//	Notice the progress bar.  This indicates how much of the ship has been built.  Click Next to continue.
#define TUT_BuildManagerBuildResearchShip		222	//	Now add a Research Ship to the build queue.  Leftclick twice on the Research Ship...
#define TUT_BuildManagerHitBuildRS				223	//	...then leftclick on the Build button.
#define TUT_BuildManagerBuildScouts				224	//	Now add 10 scouts to the build queue.
#define TUT_BuildManagerHitBuildScouts			225	//	Good.  Now leftclick on the Build button to begin construction.
#define TUT_BuildManagerProgressBars			226	//	Take a close look at the progress bar.  The darker top bar shows the progress of the ship currently being built. Click Next to continue.
#define TUT_3_12_BuildManagerBottomProgressBar	227	//	And the lighter bottom bar shows the progress of the entire batch. Click Next to continue.
#define TUT_BuildManagerClose					228	//	To exit the Build Manager, leftclick on the Close button or press [ESCAPE].  Construction will continue.

/* Lesson 4: Selecting Ships */
#define TUT_SaveSelectingShips					229	//	Selecting and Focusing.   Click Next to begin.
#define TUT_4_1_SelectIntroZoomOut				230	//	Zoom out to bring the entire Mothership into view.
#define TUT_SelectIntro							231	//	Select the Mothership by leftclicking on it.
#define TUT_4_5_DeselectMothership				232	//	De-select it by leftclicking on any empty space.
#define TUT_BandboxMothership					233	//	Hold down the left mouse button and drag a selection box around the Mothership.
#define TUT_SelectInfoOverlay					234	//	At the top right of the screen is the Info Overlay.  It displays all the selected ships.  To proceed, zoom the camera all the way out.
#define TUT_SelectBandboxAll					235	//	Now drag a single selection box around as many ships as you can.
#define TUT_SelectOverlayClick					236	//	Notice how the Info Overlay lists all the selected ships.  Click on the Resource Collector in the list to select it.
#define TUT_SelectFocusResCollect				237	//	Now the Resource Collector is selected.  Click next to continue.
#define Shanes_A_Dummy!						238	//
#define Shanes_A_Big_Dummy!					239	//
#define TUT_4_12_FocusResCollect				240 	//	Homeworld's camera always rotates around a ship or group of ships.  This is called the camera's focus.  To focus the camera on the Resource Collector, click your middle mouse button or press the [F] key.
#define TUT_SelectMoveCameraFocus				241	//	Good.  Rotate the camera.  Zoom it in and out.  Notice the camera is centered around the Resource Collector.  Click next to continue.
#define TUT_4_14_CameraFocusAgain				242	//	Click the middle mouse button again, or press the [F] key again to zoom in as close as possible.
#define TUT_SelectFocusIn						243	//	Zoom the camera out again to continue.
#define TUT_4_16_SelectScouts					244	//	You have a line of Scouts in front of the Mothership.  Hold down the left mouse button and drag a selection box around all of them.
#define TUT_4_17_SelectFocusScouts				245	//	Click the middle mouse button or press the [F] key.
#define TUT_4_18_ScoutsCenteredFocusIn			246	//	Now the camera is focused on all your Scouts.  Click the middle mouse button or press the [F] key again.
#define TUT_4_19_ScoutsCloseup					247	//	Now we're zoomed in on the Scouts.  Move the camera around to take a look at them.
#define TUT_SelectFocusPractice					248	//	Take some time to practice selecting and focusing.  When the action heats up, it's important to be comfortable with these controls.  When you're ready to move on, click the Next button.
#define TUT_SelectResourceCollectorFirst		249	//	Select the Resource Collector, focus on it and zoom in until you're reasonably close.
#define TUT_SelectResCollHealthBar				250	//	This is the ship's health bar.  It indicates the ship is selected.  If the ship gets damaged, the green bar will shrink and change color.  Click Next to continue.
#define TUT_SelectResCollContextMenu			251	//	When selected, a ship's functions are accessible through its rightclick menu.  Right click on the Resource Collector to bring up this menu.
#define TUT_SelectResCollDescribeContextMenu	252	//	Begin harvesting by leftclicking on Harvest in the menu.
#define TUT_SelectResCollCollecting				253	//	The Resource Collector will now find the nearest resources to harvest.  Note that you can still rotate and zoom the camera while it moves.  Click next to continue.
#define TUT_SelectFocusResearchShip				254	//	To move on to the next lesson, select and focus on the Research Ship.  To help you find it, I have drawn a pointer to it.

/* Lesson 5: TitleResearch */
#define TUT_SaveResearch						255	//	Researching Technology.  Click Next to begin.
#define TUT_ResearchIntro						256	//	This is a Research Ship.  The more you have of these, the faster you can research.  To access the Research Manager, bring up the taskbar by moving your cursor to the bottom of the screen.
#define TUT_TaskbarResearchManagerButton		257	//	This button brings up the Research Manager.  Leftclick on it now.
#define TUT_ResearchManagerScreen				258	//	Welcome to the Research Manager.  Here you can research technology which will make new ship types available for construction.  Click Next to continue.
#define TUT_ResearchManagerTopicWindow			259	//	This window is the technology list window.  Here you select the technologies to research.  Click Next to continue.
#define TUT_5_4_ResearchManagerGreenDot			260	//	The green dot indicates that Capital Ship Drive technology has already been researched.
#define TUT_5_5_ResearchManagerSelectTech		261	//	Leftclick on the Capital Ship Chassis technology to select it.
#define TUT_ResearchManagerTopicDescription		262	//	Leftclick on the Research button.
#define TUT_ResearchManagerResearching			263	//	Notice that the technology has a progress bar.  This indicates how much research has been done.  Click Next to continue.
#define TUT_5_8_ResearchManagerClose			264	//	Leftclick on the Close button to exit the Research Manager.

/* Lesson 6: Moving Ships */
#define TUT_SaveMovingShips						265	//	Moving Ships.  Click Next to begin.
#define TUT_MoveSelectScouts					266	//	Now it's time to move your 10 Scouts.  Zoom out and select them all.
#define TUT_MoveFocusScouts						267	//	Focus on the scouts.
#define TUT_MoveCameraOverview					268	//	Now rotate the camera so we're looking straight down on the Scouts.
#define TUT_MoveCameraZoomOut					269	//	Lastly, zoom out as far as you can.
#define TUT_MoveContextMenu						270	//	This gives us a nice view for moving the ships.  Rightclick on one of the Scouts to bring up its menu.
#define TUT_MoveSelectMove						271	//	Select the Move option from the menu.
#define TUT_MovePizzaDishIntro					272	//	This is the Movement Disc.  Leftclick anywhere on the screen to move the Scouts there.
#define TUT_MoveScoutsMove						273	//	Moving ships is very important, so let's try it again.  Click Next to continue.
#define TUT_MoveHitM							274	//	As a shortcut, bring up the Movement Disc by pressing the [M] key.
#define TUT_MoveCameraPizzaDish					275	//	This time, rotate and zoom the camera before issuing the move command.
#define TUT_6_10_MoveHitM2						276	//	Now let's try some 3D movement.  Bring up the Movement Disc again.
#define TUT_MoveUp								277	//	Hold the [SHIFT] key and drag the mouse up or down to change the height of your destination point.  When you have found an acceptable height, release the [SHIFT] key.
#define TUT_6_14_MoveUpRelease					278	//	Now the height of the destination is fixed.  Drag the mouse to move the destination cursor at this level.
#define TUT_6_15_MovePractice					279	//	Nice work!  Take some time to practice horizontal and vertical movement.  When you feel comfortable with these controls, click Next.

/* Lesson 7: TitleSM */
#define TUT_SMSave								280	//	The Sensors Manager.  Click Next to begin.
#define TUT_SMIntro								281	//	The Sensors Manager is Homeworld's large-scale map.  To access it, first bring up the taskbar at the bottom of your screen.
#define TUT_SMPressSensors						282	//	Leftclick on this button to enter the Sensors Manager.
#define TUT_SMSensorsManager					283	//	Welcome to the Sensors Manager.  >From here you can get an overview of your surroundings and move your ships long distances.  Notice that you can still rotate and zoom the camera.  Click the Next button to continue.
#define TUT_SMDescription						284	//	This is your Mothership.
#define TUT_7_4_SMDescriptionGreenDot			285	//	The green points represent your ships.
#define TUT_7_5_SMDescriptionRedDot				286	//	The red points represent enemy ships.
#define TUT_7_6_SMDescriptionLgBrownDot			287	//	The large brown points represent harvestable resources.
#define TUT_7_7_SMDescriptionSmBrownDot			288	//	The small brown points represent space dust that cannot be collected.
#define TUT_SMBandBox							289	//	Leftclick on any of your ships to exit the Sensors Manager and move the camera to that position.
#define TUT_SMExit								290	//	In this way you can cover large distances quickly.  Use the taskbar or press [SPACE] to get back into the Sensors Manager.
#define TUT_SMSelected							291	//	Selected ships are displayed as flashing points in the Sensors Manager.  Click Next to continue.
#define TUT_7_12_SMMoveButton					292	//	The Sensors Manager allows you to move ships the same way you do normally.
#define TUT_SMFocus								293	//	Focusing also works normally in the Sensors Manager.  Press [F] or click the middle mouse button to focus on the selected ships.
#define TUT_SMPractice							294	//	You've learned a lot.  Practice moving your Scouts and using the Sensors Manager.  When you are ready, focus on the Scouts and click Next.

/* Lesson 8: TitleFormations */
#define TUT_SaveFormation						295	//	Formations. Click Next to begin.
#define TUT_FormationIntro						296	//	Right click on the scouts to bring up their menu.
#define TUT_FormationDelta						297	//	Move your cursor to Formation and then leftclick on Delta.
#define TUT_FormationDeltaBroad					298	//	Now the fighters are in the Delta formation.  Use the rightclick menu again to assign Broad formation.
#define TUT_FormationBroadX						299	//	Good.  Next try Formation X.
#define TUT_FormationXClaw						300	//	Formations can also be assigned using the [TAB] key.  Press [TAB] to set Claw formation.
#define TUT_FormationClawWall					301	//	Notice that [TAB] will cycle to the next formation.  Press [TAB] again to see how.
#define TUT_FormationWallSphere					302	//	This is the Wall.  Press [TAB] for Sphere formation.
#define TUT_FormationPractice					303	//	Take some time to familiarize yourself with the formations.  When you're ready to continue, put the Scouts into Claw formation and leftclick on the Next button.
#define TUT_8_9_LookForEnemy					304	//	To begin the combat lesson, use the Sensors Manager to move your Scouts near the enemy ships.
#define TUT_8_10_RedDots						305	//	These are the enemies.  Move your Scouts here.
#define TUT_8_11_LeaveSensors					306	//	Once you have issued the move, you can leave the Sensors Manager.

/* Lesson 9: TitleAttacking */
#define TUT_AttackingSave						307	//	Combat.  Click Next to begin.
#define TUT_AttackingMouseOver					308	//	Rotate the camera so you can see the enemy ships.  Leftclick on one of them to attack it.
#define TUT_9_2_AttackingBandbox				309	//	Good.  To attack them all at once, hold down [CTRL] and drag a selection box around the enemy ships.
#define TUT_9_3_AttackingTimeStop				310	//	I've paused the action to teach you how to focus the camera without selecting.

/* Lesson 10: TitleAltFocus */
#define TUT_10_ALT_Focusing						311	//  Lesson 10: [ALT]-Focusing.  Click Next to begin.
#define TUT_10_1_AltFocus						312	//	Hold down the [ALT] key and leftclick on any ship.  You can even use this to focus on enemy ships.
#define TUT_10_2_AltFocusBandbox				313	//	You can also focus on multiple ships at once by holding [ALT] and dragging a selection box around them.  Try it now.
#define TUT_10_3_PracticeAndUnPause				314	//	Good.  Now practice focusing until you're ready to move on.  Press [P] to unpause the action.
#define TUT_10_4_BuildFrigate					315	//	The Capital Ship Drive research you started earlier has been completed.  You can now build an Assault Frigate.  Go into the Build Manager.
#define TUT_10_5_BuildFrigateHintRetard			316	//	Try using the taskbar.
#define TUT_10_6_BuildFrigateStart				317	//	Build an Assault Frigate.
#define TUT_10_7_BuildFrigateUnderway			318	//	Let's return to your Scouts.  Leftclick on the Close button.

/* Lesson 11: TitleDock */
#define TUT_11_Docking							319	//  Lesson 11: Docking.  Click Next to begin.
#define TUT_11_0_Docking						320	//	Some of your scouts might be damaged or low on fuel.  To dock them with the Mothership, first bring up the rightclick menu.
#define TUT_11_1_SelectDock						321	//	Now select Dock.
#define TUT_11_2_ScoutsDocking					322	//	The Scouts are on their way to the Mothership to be repaired and refueled.

/* Lesson 12: Cancelling Commands */
#define TUT_12_CancellingOrders					323	//  Lesson 12: Cancelling Orders.  Click Next to begin.
#define TUT_12_0_CommandCancelIntro				324	//	You've learned how to give orders.  Now let's learn how to cancel one.
#define TUT_12_1_CommandCancelContextMenu		325	//	Rightclick on the Scouts and bring up their menu.
#define TUT_12_2_CancelCommand					326	//	Select cancel.
#define TUT_12_3_CancelCommandDescription		327	//	Notice that your Scouts have stopped.  We actually do want them to dock, so issue the dock order again.
#define TUT_12_4_TutorialPractice				328	//	I have a feeling a battle is imminent.  When your Frigate is built, I bet you will be attacked by a small group of Corvettes and Scouts.
#define TUT_12_7_TutorialFinish					329	//	Congratulations! You have now finished the Homeworld tutorial.


/******************************************
****                                   ****
****          Single Player            ****
****                                   ****
******************************************/

/******************************************
****            Mission 1              ****
******************************************/

/* 0.3 ANIMATIC - Opening */
#define A00_Narrator_Opening					330	//	100 years ago, a satellite detected an object under the sands of the Great Desert.
#define A00_Narrator_Opening2					331	//	An expedition was sent.
#define A00_Narrator_Opening3					332	//	An ancient starship, buried in the sand.
#define A00_Narrator_Opening4					333	//	Deep inside the ruin was a single stone that would change the course of our history forever.
#define A00_Narrator_Opening5					334	//	On the stone was etched a galactic map ...
#define A00_Narrator_Opening6					335	//	and a single word more ancient than the clans themselves ...
#define A00_Narrator_Opening7					336	//	Higaara.
#define A00_Narrator_Opening8					337	//	Our home.
#define A00_Narrator_Opening9					338	//	The clans were united and a massive colony ship was designed.
#define A00_Narrator_Opening10					339	//	Construction would take 60 years.
#define A00_Narrator_Opening11   				340	//	It would demand new technologies, new industries and new sacrifices.
#define A00_Narrator_Opening12   				341	//	The greatest of these was made by the scientist Karan Sajet who had herself permanently integrated into the colony ship as its living core.
#define A00_Narrator_Opening13   				342	//	She is now Fleet Command.
#define A00_Narrator_Opening14   				343	//	The promise of the Guidestone united the entire population.
#define A00_Narrator_Opening15   				344	//	Every mind became focused on the true origin of our people...
#define A00_Narrator_Opening16   				345	//	...every effort on the construction of the ship that would seek it out among the stars.

/* 0.5 N01 - Opening [Planet of Exile System] - EZ01 */
#define N01_All_Misc_1							346	//	What a beautiful sight
#define N01_All_Misc_2							347	//	You are cleared to approach
#define N01_All_Misc_3							348	//	Bays five, six and seven sealed.
#define N01_All_Misc_4							349	//	Release crews standing by.
#define N01_All_Misc_6							350	//	Bays eight, nine and ten sealed.
#define N01_All_Misc_7							351	//	Scaffold decks A, B, C secure.
#define N01_All_Misc_8							352	//	You got it
#define N01_All_Misc_9							353	//	Roger that Control.
#define N01_All_Misc_10							354	//	…decks D and E secure. Scaffold secure.
#define N01_All_Misc_11							355	//  Scaffold control standing by.
#define N01_All_Misc_12							356	//	All systems green.
#define N01_All_Misc_13							357	//	Forty-two seven-oh-one please confirm.
#define N01_All_Misc_14							358	//  We're ready.
#define N01_All_Misc_15							359	//	How do we look? All right, take it to one-fifteen. Locked in.
#define N01_Fleet_Intro							360	//	This is Fleet Command.  Reporting Mothership pre-launch status.
#define N01_Fleet_Command						361	//	Command on-line...
#define N01_Fleet_Resourcing					362	//	Resourcing online...
#define N01_Fleet_Construction					363	//	Construction online...
#define N01_Fleet_Cryo_AtoJ						364	//	Cryogenic subsections A through J online...
#define N01_Fleet_Cryo_KtoS						365	//	K through S online...
#define N01_Fleet_Hull							366	//	Hull pressure stable.
#define N01_Fleet_ScaffoldAlign					367	//	Scaffold Control stand by for alignment...
#define N01_Fleet_AlignConfirmed				368	//	Alingment confirmed.  Stand by Release Sync Control.
#define N01_All_CaliperReleased					369	//	All caliper banks released...
#define N01_All_DownTheMiddle					370	//	...she's right down the middle.
#define N01_All_FleetClear						371	//	Initial Fleet is clear.
#define N01_Fleet_MoshipClear					372	//	The Mothership has cleared the Scaffold.
#define N01_Fleet_MoshipAway					373	//	We are away.
#define N01_All_CommandLineGreen				374	//	Command Line green.  Initial Fleet in position.
#define N01_Chatter_Mid							375	//	for NIS's, add to the atmosphere, relatively calm
							

/* 1 M01 - TRIALS [KHARAK SYSTEM] - EZ01 */
#define M01_Intel_GoingOnline					376	//	Fleet Intelligence going on-line.  Our task is to analyze all sensor data and generate mission objectives.  Before the hyperdrive test, several trials must be completed.
#define M01_Intel_ResearchObjective				377	//	Test construction by building the primary research ship.
#define M01_Intel_HavestingObjective			378	//	Test resource processing by harvesting the asteroids provided nearby.
#define M01_Intel_CombatTrials					379	//	Standby to begin combat trials.  First we will be monitoring formation performance.
#define M01_Intel_CombatTrialsObjective			380	//  Target Drones have been provided here.  Assign a formation to your Fighters and destroy the Drones.
#define M01_Intel_CombatUseFormation			381	//	Target Drones destroyed.  Replacement Drones are being sent to the same location.  Completion of this trial requires the use of a formation.  Begin again.
#define M01_Intel_FormationAnalysis				382	//	Formation trial complete.  Flight analysis shows a twenty-two percent increase in combat performance.
#define M01_Intel_TacticsTrial					383	//  The next trial will test the effectiveness of tactics.  Standby to begin tactics trial.  Use Aggressive or Evasive tactics and engage the Target Drones here.
#define M01_Intel_CombatUseTactics				384	//	Target Drones destroyed.  Replacement Drones are being sent to the same location.  Completion of this trial requires the use of tactics.  Begin again.
#define M01_Intel_TacticsComplete				385	//	Tactics trial complete.
#define M01_Intel_SalvageTrial					386	//  The next trial will test the performance of the Salvage Corvette.  Build one and capture the Target Drone here.
#define M01_Intel_SalvageTargetDestroyed		387	//	Target Drone destroyed.  A replacement Drone is being sent to the same location.  Completion of this trial requires a successful capture of the drone.  Begin again.
#define M01_Intel_BuildInterceptors				388	//	Interceptors are ready for construction. 
#define M01_Intel_AdvancedCombatTrials			389	//  Advanced Target Drones with heavier armor have been provided.  Using Interceptors, begin the mock combat exercise here.
#define M01_Intel_CombatUseInterceptors			390	//	Target Drones destroyed.  Replacement Drones are being sent to the same location.  Completion of this trial requires the use of Interceptors.  Begin again.
#define M01_Fleet_HyperspaceCharging			391	//	Hyperspace module charging.  35% capacity and rising.  The Mothership will be ready for the Hyperdrive test in 10 minutes.
#define M01_Intel_TrialsComplete				392	//	Standby for Hyperdrive test.  Internal pressure doors will be sealed in 2 minutes.  Abort systems standing by.
#define M01_Fleet_HyperspaceReady				393	//	Hyperspace module fully charged.  I am ready to initiate quantum wave generation on your mark.  Good luck everyone.
#define M01_Intel_HyperspaceReady				394	//	All sections have reported in.  Trigger the hyperspace drive at your discretion.

/* 1.19 ANIMATIC - M01à M02 */
#define A01_Fleet_HyperspaceFullPower			395	//	All Hyperspace systems operating at full power.
#define A01_Intel_TargetingSystem				396	//	If the hyperspace targeting system is accurate, we should emerge in close proximity to the support vessel Khar-Selim.
#define A01_Intel_2								397	//	This ship has spent the past 10 years travelling on conventional drives to reach the edge of the Kharak system.
#define A01_Intel_3								398	//	The Khar-Selim will monitor the quantum waveform as we return to normal space and assist in tuning our drive control systems.
#define A01_Intel_4								399	//	If the hyperspace module malfunctions, the Khar-Selim will offer assistance and re-supply.
#define A01_Intel_5								400	//	Mission objectives will be to dock with the service vessel and link up research teams in order to complete adjustments to the Mothership and her drives.


/******************************************
****            Mission 2              ****
******************************************/

/* 2 M02 - BATTLE W/ TURANIC RAIDERS [GREAT WASTELANDS] - EZ02 */
#define M02_Fleet_MadeIt						401	//	We made it.  Hyperspace jump complete!  All systems nominal and the quantum wave effect has dissipated.
#define M02_Intel_MissingScienceVessel			402	//	We have miss-jumped.  The support ship is not here.  Fleet Command will signal the Khar-Selim while we confirm our true position.
#define M02_Fleet_CallingScienceVessel			403	//	This is the Mothership calling Support Vessel Khar-Selim.  Come in please.  We have miss-jumped and are requesting your beacon. . .  This is the Mothership calling Support Vessel Khar-Selim.  Please Respond. . .
#define M02_Intel_PickedUpBeacon				404	//	Priority Alert.  We have picked up an automated beacon from the Khar-Selim.
#define M02_Intel_SendProbe						405	//	Send a probe to make contact and re-establish communications.
#define M02_Intel_ScienceVesselDestroyed		406	//	The Khar-Selim has been destroyed.  Heavy weapon damage is visible on the remaining fragment.  A Salvage Corvette must be sent to retrieve the mission data recorder.
#define M02_Fleet_MothershipUnderAttack			407	//	They're attacking?!. . . I'm under attack!  The Mothership is under attack!!
#define M02_Intel_DefendMothership				408	//	Engage incoming units.  The Mothership must be defended.
#define M02_Intel_EnemyDestroyed				409	//	Hostile units destroyed.  Threat eliminated.
#define M02_Fleet_MinorDamage					410	//	Mothership sustained minor hull damage.  Repairs are underway.
#define M02_Intel_MoreEnemies					411	//	Priority Alert!  Additional hostile units detected on an intercept course with the Khar-Selim.
#define M02_Intel_ProtectSalvageTeam			412	//  Protection of the Salvage team is a primary Objective.  It must return to Mothership with the mission recorder.  We need that data.
#define M02_Intel_SalCorDestroyed				413	//	Salvage Corvette has been destroyed.  Use an escort force to guard next team.  We need those records.
#define M02_Intel_SalvageDocked					414	//	Salvage Team docked safely.  Downloading Mission Data Recording.  Replaying last entry:
#define M02_All_ScienceVesselAttacked			415	//	What do you mean you detect a Hyperspace entry?!  The mothership isn't due for-...
#define M02_10_Intel_DefendMoShipAgain			416	//	We are detecting enemy units advancing on our position.  Organize a defense force to protect the Mothership.
#define M02_11_Intel_InvestigatePowerSource		417	//	Large power signature detected nearby. Recommend immediate investigation.
#define M02_12_Intel_EnemyCarrier				418	//	It's an enemy Carrier.  They appear to be reinforcing their squadrons of Fighters and Corvettes with it.
#define M02_Intel_EnemyInferior					419	//  We have determined that these enemy units are inferior to ours.
#define M02_13_Intel_DestroyAllP1				420	//	Commander, this race is clearly hostile. We can't allow them to secure a position in our system. I recommend that we destroy the force completely.
#define M02_Intel_ObjectivesComplete			421	//	Enemy units are retreating.  Objectives complete.  Standby for immediate return to Kharak.

/* 2.15 ANIMATIC - M02à M03 */
#define A02_Intel_AnalysisOfWreckage			422	//	Analysis of wreckage reveals the hostile units are using strike craft ranging from Fighters to combat Corvettes.
#define A02_Intel_2								423	//	All pilots will be briefed in case hostiles have penetrated further into the Kharak system.
#define A02_Intel_3								424	//	On our return to Kharak, the final outfit of the Mothership must be accelerated in order to defend against possible future attacks.
#define A02_Intel_4								425	//	Many major Mothership systems are still incomplete.
#define A02_Intel_5								426	//	We will notify the Kharak Missile Defense System of this possible threat.
#define A02_Intel_6								427	//	The Mothership will then dock with the Scaffold for repairs.
#define A02_Intel_7								428	//	Standby for Hyperspace exit to Kharak.


/******************************************
****            Mission 3              ****
******************************************/

/* 3 BACK TO KHARAK [KHARAK SYSTEM] - EZ01 */
#define M03_Fleet_ItsGone						429	//	No one's left. . .Everything's gone!. . .Kharak is burning!
#define M03_Intel_Analyzing						430	//	Kharak is being consumed by a firestorm.  The Scaffold has been destroyed.  All orbital facilities destroyed.  Significant debris ring in low Kharak orbit.  Receiving no communications from anywhere in the system... Not even beacons.
#define M03_Fleet_CryotraySignal				431	//	Wait!  On the maintenance frequency.  I'm getting a signal from the Cryo Tray systems in orbit.  One of them is suffering a massive malfunction.
#define M03_Intel_DispatchSalCorvettes			432	//	Dispatch Salvage Corvettes immediately to collect the trays.
#define M03_Intel_CryotrayUnderAttack			433	//	The Cryo Trays are under attack.  These ships are different from those we encountered at the Khar-Selim.  It is likely they were involved in the destruction of Kharak.
#define M03_Intel_CaptureEnemy					434	//	Capture at least one vessel for interrogation and destroy the rest.
#define M03_Fleet_SaveCryotray					435	//	Those trays contain all that remain of our people.  Without them, we will become extinct.
#define M03_Intel_EnemyCaptured					436	//	Hostile vessel captured.  Crew interned.  Interrogation is underway.  While searching the enemy ship's computer systems, we came across these flight recordings.  Standby for playback.

/* 3.5 N04 - Learn Of Sacking [Kharak System] EZ01 */
#define N04_All_Battle1							437	//	Group three moving in... allright now, stay together... ready... SPREAD!
#define N04_All_Battle2							438	//	Surface attack group to first positions...
#define N04_All_Battle3							439	//	Incoming missiles...
#define N04_All_Battle4							440	//	Evasive maneuvers!
#define N04_All_Battle5							441	//	We're hit! Damage report!
#define N04_All_Battle6							442	//	What the...?
#define N04_All_Battle7							443	//	There's another one...
#define N04_All_Battle8							444	//	Prepare for immediate surface bombardment!
#define N04_All_Battle9							445	//	Standing by.
#define N04_All_Battle10						446	//	Commence surface delivery.
#define N04_All_Battle11						447	//	Delivery confirmed.
#define N04_All_Battle12						448	//	All targets acquired.
#define N04_All_Battle13						449	//	Multiple impacts confirmed.
#define N04_All_Battle14						450	//	Surface temperature fifteen-twenty-three and climbing...
#define N04_All_Battle15						451	//	Estimated immediate casualties: 98 percent.
#define N04_All_Battle16						452	//	Surface temperature seventeen-forty-two and stable...
#define N04_All_Battle17						453	//	Orbital target acquired.
#define N04_All_Battle18						454	//	Watch the spread pattern... stay sharp.
#define N04_All_Battle19						455	//	Target destroyed.
#define N04_All_Battle20						456	//	Requesting clearance to dock.
#define N04_All_Battle21						457	//	Clearance granted.
#define N04_All_Battle22						458	//	Primary directive achieved.
#define N04_All_Battle23						459	//	Prepare for secondary directive.
#define N04_All_Battle24						460	//	Fleet trajectory calculated and locked in.
#define N04_All_Battle25						461	//	Prepare for hyperspace.

/* 3 BACK TO KHARAK [KHARAK SYSTEM] - EZ01 */
#define M03_Intel_RecordingAnalysis				462	//	...Analysis of the recording indicates that the Kharak missile defenses heavily damaged the attacking fleet.  However, we have concluded that at present they can still easily defeat us.  We have therefore plotted a course to a deep space asteroid belt.  There we can hide and prepare our fleet for an assault.
#define M03_Intel_ResearchAnalyzedFrigate		463	//	Our research division has analyzed the captured frigate.  We have reverse engineered the drive technology and developed two new ships.  Plans for a third vessel are underway, but will require Frigate Chassis research.
#define M03_Fleet_CryotraysLoaded100			464	//	Cryo tray loaded.  One hundred thousand people secured.
#define M03_Fleet_CryotraysLoaded200			465	//	Cryo tray loaded.  Two hundred thousand people secured.
#define M03_Fleet_CryotraysLoaded300			466	//	Cryo tray loaded.  Three hundred thousand people secured.
#define M03_Fleet_CryotraysLoaded400			467	//	Cryo tray loaded.  Four hundred thousand people secured.
#define M03_Fleet_CryotraysLoaded500			468	//	Cryo tray loaded.  Five hundred thousand people secured.
#define M02_Intel_DeploySalvageTeam				469	//  Withdraw attack and deploy a Salvage Team.  We need that ship.
#define M02_Intel_HyperspaceReady				470	//  Cryo trays loaded and secure.  Hyperspace module charged.  There's nothing left for us here.  Let's go.

/* 3.17 ANIMATIC - M03à M04 */
#define A03_Fleet_CryotraysProcessed			471	//	Cryogenic trays have been processed and our colonists are safe for now.
#define A03_Fleet_2								472	//	We are all that's left of our world, our culture, our people.
#define A03_Fleet_3								473	//	No one survived. . .
#define A03_Intel_InterrogationInfo				474	//	From the interrogation we learned that a frontier fleet patrolling the borders of a vast interstellar empire was dispatched to destroy our planet.
#define A03_Intel_2								475	//	The captain claimed our people violated a 4000 year old treaty forbidding us to develop hyperspace technology.
#define A03_Intel_3								476	//	Extermination of our planet was the consequence.
#define A03_Intel_4								477	//	The subject did not survive interrogation.


/******************************************
****            Mission 4              ****
******************************************/

/* 4 M04 - DEFEAT P1 [GREAT WASTELANDS] EZ04 */
#define M04_Fleet_HyperspaceSuccesful			478	//  Hyperspace jump successful.
#define M04_Intel_ScannerData					479	//  Scanner data indicates asteroid density is highest in this region.  Commence resource gathering.
#define M04_Intel_ResearchController			480	//  Resource Controllers are available for construction.  To improve the efficiency of collection, build one and position it near the asteroid vein.
#define M04_Intel_LongRangeSensors				481	//  Long range sensors indicate a Mothership-class mass signature.  It's coming in fast.  Power readings are off the scale.  Full combat alert.  Standby for contact.

/* 4.5 N03 - Traders Intro [Great Wastelands] - EZ03 */
#define N03_Intel_AnomalyDetected				482	//	Prepare the ambassador.
#define N03_Fleet_AmbassadorAway				483	//	Ambassador away.
#define N03_All_LockedIn						484	//	Trajectory locked in, hailing signal open on all channels.
#define N03_All_MagneticField					485	//	Entering magnetic field now... almost there...
#define N03_All_LostGuidance					486	//	Fleet, we've lost Guidance and are being drawn in...
#define N03_All_LotsOfLights					487	//	There's a lot of lights... uh... there seems to be... some kind of activity inside, I can see...
#define N03_Traders_Intro						488	//	The whole big speel here
#define N03_All_AmbassadorReturning				489	//	Fleet, this is ambassador.  We are clear of the Bentusi vessel, all systems green.  Harbor control has released Guidance and the exchange unit is secure.  Receiving crews, prep the quarantine chamber.
#define N03_All_QuarantineReady					490	//	Roger that.  Quarantine standing by.
#define N03_Fleet_TradingEstablished			491	//	Bentusi Trading link established.

/* 4 M04 - DEFEAT P1 [GREAT WASTELANDS] EZ04 */
#define M04_Traders_FarewellKushan				492	//	Turanic Raiders, servants of the Taiidan, are arriving.  They must not learn of our contact.  We must depart.  All that moves is easily heard in the void.  We will listen for you.  Farewell.
#define M04_Traders_FarewellTaiidan				493	//	Turanic Raiders, servants of the Kushan, are arriving.  They must not learn of our contact.  We must depart.  All that moves is easily heard in the void.  We will listen for you.  Farewell.
#define M04_Traders_StopAttackNow				494	//	The Bentusi wish only to trade and make contact.  Your attack is unwarranted and ill-advised.  Stop now.
#define M04_Intel_CeaseFire						495	//	Recommend immediate cease-fire.
#define M04_Traders_KickAss						496	//	You insist on conflict.  This is most unfortunate.
#define M04_Intel_DefendTheFleet				497	//  Turanic Raider fighters are attacking our resource operation.  Defend it and prepare for more hostile ships to arrive.
#define M04_Intel_DefendTheMothership			498	//  Several Turanic Raider capital ships have just emerged from hyperspace near the Mothership.
#define M04_Intel_DiscoveredCarrier				499	//	The Turanic Raider Carrier has been located.  It is reinforcing their squadrons of Fighters and Corvettes.
#define M04_Intel_CarrierRetreatingKushan		500	//	The Turanic Raider Carrier is retreating.  If it escapes they will warn the Taiidan fleet of our pursuit.  Do not allow them to hyperspace.
#define M04_Intel_CarrierRetreatingTaiidan		501	//	The Turanic Raider Carrier is retreating.  If it escapes they will warn the Kushan fleet of our pursuit.  Do not allow them to hyperspace.
#define M04_Intel_HyperspaceKushan				502	//  The Turanic Raiders have been defeated.  We can now return to our immediate goal: preparation for an attack on the Taiidan fleet that devastated Kharak.
													//	Using data from the Taiidan vessel captured at Kharak, we have been able to determine their location.  If we strike now we can take advantage of their damaged condition.
													//	Hyperspace co-ordinates have been transferred to Fleet Command.
#define M04_Intel_HyperspaceTaiidan				503	//  The Turanic Raiders have been defeated.  We can now return to our immediate goal: preparation for an attack on the Kushan fleet that devastated Kharak.
													//	Using data from the Kushan vessel captured at Kharak, we have been able to determine their location.  If we strike now we can take advantage of their damaged condition.
													//	Hyperspace co-ordinates have been transferred to Fleet Command.

/* 4.12 ANIMATIC - M04à M05 */
#define A04_Intel_SystemsOptimal				504	//	We have repaired the damage incurred by the Turanic Raiders.
#define A04_Intel_SystemsOptimal2				505	//	Combat and sensor systems returning to optimal functionality.
#define A04_Fleet_Whining						506	//	I can't believe this happened.
#define A04_Fleet_Whining2						507	//	Our first hyperdrive test led to near genocide.
#define A04_Fleet_Whining3						508	//	Kharak... destroyed.
#define A04_Fleet_Whining4						509	//	All of us, all that's left, hunted by two alien races...
#define A04_Intel_ApproachingCoordinates		510	//	We must focus on the matter at hand: elimination of the fleet that destroyed our world.
#define A04_Intel_ApproachingCoordinates2		511	//	Coming up on hyperspace co-ordinates.


/******************************************
****            Mission 5              ****
******************************************/

/* 5 M05 - BATTLE AGAINST SACKING FORCE [GARDEN VIEW] EZ05 */
#define M05_Fleet_HyperspaceSuccessful			512	//	Hyperspace successful.  Current position and pre-jump coordinates are in perfect alignment.  We are on target.
#define M05_Intel_SendProbesKushan				513	//	We detect large resource deposits but no vessels.  It is possible that the Taiidan could be hiding in the denser portions of the belt, which may cause interference with our sensors.
#define DummyIGNORE						514	//	Send probes to investigate.  All fleet assets should be kept on alert.
#define M05_Intel_SendProbesTaiidan				515	//	We detect large resource deposits but no vessels.  It is possible that the Kushan could be hiding in the denser portions of the belt, which may cause interference with our sensors.
#define DummyIGNORETOO						516	//	Send probes to investigate.  All fleet assets should be kept on alert.
#define M05_Intel_ProfilesMatch				517	//	Ship profiles and markings match those from the recording at Kharak.  There is no doubt that this is the fleet.  Destroy them.
#define M05_Intel_ResearchIonCannon				518	//  Research Division reports it is now equipped for Ion Cannon technology.  We advise commencing research immediately.
#define M05_Intel_ResearchPlasmaBomb			519	//	Research Division reports it is now equipped for Plasma Bomb technology.  We advise commencing research immediately.
#define M05_Intel_DestroyEnemyCollector			520	//  Sensors indicate the enemy is harvesting resources.  Destroy their collector to impair ship production.
#define M05_Intel_MoveAttack					521	//  Enemy capital ships appear to be most heavily armored on the front and sides.  Our capital ships should be issued move orders while attacking to take advantage of the more vulnerable sides.
#define M05_Intel_ResearchDefenseFighter			522	//  The enemy is using a new Fighter class ship with strong defensive capabilities but low maneuverability.  Our Research Division reports that it can produce a similar vessel.  Begin Research as soon as possible.
#define M05_Intel_HyperspaceKushan				523	//	Taiidan fleet destroyed.  There's nothing left for us to return to.  Our only option is to follow the path etched into the Guidestone.  Finding our ancient home is our only hope now.Hyperspace coordinates locked in.	
#define M05_Intel_HyperspaceTaiidan				524	//	Kushan fleet destroyed.  There's nothing left for us to return to.  Our only option is to follow the path etched into the Guidestone.  Finding our ancient home is our only hope now.Hyperspace coordinates locked in.


/* 5.9 ANIMATIC - M05à M06 */
#define A05_Intel_1								525	//	We've completed de-crypting data from the enemy frigate we captured in the Kharak system.
#define A05_Intel_2								526	//	It appears to be an imperial broadcast.
#define A05_Intel_3								527	//	In order to stay clear of these outposts, our course will take us into a turbulent asteroid field and through the heart of a nebula.


/******************************************
****            Mission 6              ****
******************************************/

/* 6 M06 - ASTEROIDS 3D [DIAMOND SHOALS] EZ06 */
#define M06_Fleet_HyperspaceSuccessful			528	//	Hyperspace jump successful.
#define M06_Intel_AsteroidCollision				529	//	We haven't cleared the asteroid field.  Prepare for collisions.
#define M06_Intel_DestroyAsteroids				530	//	Incoming asteroids must be destroyed before they impact with the Mothership.  Concentrate fire within this collision envelope.
#define M06_Intel_ResearchSuperCap				531	//	Research Division reports it is now equipped for Super Capital Ship Drive technology.  We advise commencing research immediately.
#define M06_Intel_StrikeCraftIneffective		532	//	Strike Craft are proving ineffective.  We recommend using primarily capital ships.
#define M06_Intel_ClearedField					533	//	We've cleared the field.
#define M06_Traders_ComeToTrade					534	//	Greetings.  We have come to trade.
#define M06_Fleet_AskTradersForInfo				535	//	This is a dangerous and unpredictable region.  Can you give us information that will guide us through the nebula ahead?
#define M06_Traders_FearNebulaKushan			536	//	We hear nothing there.  Even the Taiidan fear the Great Nebula.  No one returns.
#define M06_Traders_FearNebulaTaiidan			537	//	We hear nothing there.  Even the Kushan fear the Great Nebula.  No one returns.
#define M06_Intel_HyperspaceReady				538	//	Hyperspace Module fully charged.

/* 6.11 ANIMATIC - M06à M07 */
#define A06_Fleet								539	//	The Bentusi said "No one returns."


/******************************************
****            Mission 7              ****
******************************************/

/* 7 M07 - SWARMER BATTLE [GARDENS] EZ07 */
#define M07_Intel_HarvestNebula					540	//	The Nebula is incredibly rich in energy and resources.  Energy levels are so high that our sensors are having trouble compensating.  Begin harvesting the nebula while we address this problem.
#define M07_Intel_AlertStatus					541	//	There is a contact closing with the Mothership.  Sensors instability in this region makes it difficult to identify.
#define M07_Intel_PrepareAmbassador				542	//	Prepare the Ambassador.

/* 7.3 N05 - P2 Intro [Gardens] EZ07 */
#define N05_Fleet_AmbassadorAway				543	//	Ambassador away.
#define N05_P2_Intro1							544	//	This is the Garden of Kadesh.  For thirteen generations we have protected it from the unclean.
#define N05_P2_Intro2							545	//	The Turanic Raiders who came before you failed to join and were punished for this trespass. Like theirs, your ship has already defiled this holy place.
#define N05_P2_Intro3							546	//	If you have come to join we welcome you and will spare your ship until all have disembarked.  If you have come to consume the garden you will be removed at once.
#define N05_P2_Intro4							547	//	What are your intentions?
#define N05_All_UnawareOfSignificance			548	//	We were unaware of the significance of this location.  We mean you no conflict.  Please allow us time to prepare our engines so that we may withdraw as requested.
#define N05_P2_NotARequest						549	//	If you will not join, then die.  There is no withdrawal from the Garden.

/* 7 M07 - SWARMER BATTLE [GARDENS] EZ07 */
#define M07_Intel_DefendMothership				550	//	Delay the attacking ships while Fleet Command charges the hyperdrive module.  We should have the range to jump clear of the nebula.
#define M07_Fleet_HyperspaceCharging1			551	//	Hyperspace module at 35%, ready in 8 minutes.
#define M07_Fleet_HyperspaceCharging2			552	//	Hyperspace module at 90%, ready in 1 minute.
#define M07_Fleet_HyperspaceReady				553	//	Hyperspace module fully charged.  Get us out of this place.
#define M07_Intel_EngageHyperdrive				554	//	Co-ordinates set.  Engage hyperdrive!
#define M07_Fleet_HyperdriveFailed				555	//	Hyperdrive jump failed!  The Quantum waveform collapsed before the ship could enter hyperspace.
#define M07_Intel_AnalyzingMalfunction			556	//	Analyzing malfunction.  Continue to protect the Mothership until the problem is solved.
#define M07_Intel_DefendCollectors				557	//	Enemy forces are concentrating on our Resource Collectors.  Allocate combat vessels to protect them.
#define M07_Fleet_HypdriveOnline				558	//	The Hyperdrive is back on-line.
#define M07_Intel_Hyperspace					559	//	Let's put these fanatics behind us.

/* 7.12 ANIMATIC - M07à M08 */
#define A07_Intel_1								560	//	The enemy Mothership appeared to be equipped with a powerful field generator.
#define A07_Intel_2								561	//	This field deformed our quantum wavefront and prevented us from making a hyperspace jump.
#define A07_Intel_3								562	//	We also observed that the enemy's hyperspace module has an identical power signature to our own.
#define A07_Intel_4								563	//	This raises interesting questions considering our own technology was reverse-engineered from the wreck of the Khar-Toba.
#define A07_Intel_5								564	//	Our hyperspace systems are now functioning properly and this jump will carry us clear of the Nebula.


/******************************************
****            Mission 8              ****
******************************************/

/*8 M08 - FALKIRK [OUTER GARDENS] EZ08 */
#define M08_Fleet_HyperspaceInterrupted			565	//	Something's wrong.  We've been pulled out of hyperspace.  We're still inside the nebula.
#define M08_Intel_ItsATrap						566	//	It's a trap!
#define M08_Intel_DestroyInhibitors				567	//	Sensors detect hyperspace inhibitors in a triangular formation.  Even one can keep us from entering hyperspace.  All of them must be destroyed.
#define M08_Intel_DestroyHostiles				568	//	The nebula is still scrambling our sensors but it looks like we have incoming hostiles.
#define M08_Intel_ResearchDrones				569	//  Research Division reports it is now equipped for Drone technology.  We advise commencing research immediately.
#define M08_Intel_ResearchDefenseField			570	//	Research Division reports it is now equipped for Defense Field technology.  We advise commencing research immediately.
#define M08_P2_JoinUs							571	//	Again we offer you the chance to join us and live here in peace.
#define M08_Fleet_SomethingInCommon				572	//	Our future lies elsewhere, but we already have something in common.  The hyperdrive technology we use is identical to yours.  Our ancestors left it in a wreckage on our planet.  We're on a mission to find our Homeworld.
#define M08_P2_YouWillFail						573	//	You will fail.  The evil that drove us here will find and destroy you.  From you they will know of us and come here.  This cannot come to pass.
#define M08_Intel_DestroyAttackers				574	//	We have enemy units closing on multiple attack vectors.  Engage and destroy hostiles.
#define M08_Intel_EnemyRetreating				575	//	Enemy vessels retreating to this point.  This reading has been consistent despite sensor interference.  It has a friendly signature but it's not one of ours.
#define M08_Fleet_LooksJustLike					576	//	It looks just like the Khar-Toba.
#define M08_Intel_IdenticalMatch				577	//	Metallurgy and structural composition are an identical match to the Khar-Toba wreckage on Kharak.
#define M08_Fleet_WeAreBrothers					578	//	We are brothers.
#define M08_Intel_HyperspaceOnline				579	//	The inhibitor field has ceased.  Hyperspace module back on-line.

/* 8.10 ANIMATIC - M08à M09 */
#define A08_Intel_1								580	//	After analyzing the data we collected from the Khar-Toba's sister ship we've been able to determine what happened in the nebula.
#define A08_Intel_2								581	//	While the Khar-Toba was able to limp to Kharak, this ship must have tried to hide here in the nebula.
#define A08_Intel_3								582	//	They soon resorted to preying on ships passing through the nebula.
#define A08_Intel_4								583	//	In time the nebula became off limits to all shipping.
#define A08_Intel_5								584	//	They developed hyperspace inhibitor technology to trap prey from far away without leaving the safety of the nebula.
#define A08_Intel_6								585	//	Eventually it became the center of their existence.
#define A08_Intel_7								586	//	Their religion.


/******************************************
****            Mission 9              ****
******************************************/

/* 9 M09 - GHOSTSHIP [WHISPERING OCEAN] EZ09 */
#define M09_Fleet_AnomolyDetected				587	//	Anomaly detected.  Override engaged.
#define M09_Intel_VesselDetected				588	//	Sensors detect a vessel here.  It doesn't match any of the profiles we have encountered.  Send in a team to investigate.
#define M09_Intel_ShipsInactive					589	//	We are detecting various ships surrounding the alien vessel.  They appear to be inactive.
#define M09_Intel_NeutralizeVessel				590	//	Attention!  Those ships are operational.  We believe the control center is the alien vessel.  It should be neutralized.
#define M09_Intel_LostControl					591	//	We've lost control of capital ships in close proximity to the alien vessel.
#define M09_Intel_ControlField					592	//	We've determined that the alien control field covers this area.  No capital ships should cross into this zone or we will lose them.
#define M09_Intel_MinimalEffect					593	//	Our weapons are having minimal effect on the alien vessel but each strike causes a tiny fluctuation in the control field.
#define M09_Intel_DirectAllFire					594	//	Direct all fire at the alien ship in an attempt to disable the field.
#define M09_Intel_ConstructMissleDestroyer		595	//	After seizing the Missile Destroyer, construction reports we can now build a similar vessel.
#define M09_Intel_VesselNeutralized				596	//	Alien vessel neutralized.  Our crews have regained control.
#define M09_Intel_SalvageUnknownShip			597	//	While the field was up they were able to analyze the alien control system.  We now have control of the foreign vessels.  In addition, we detect no life signs aboard the alien vessel.  It is a derelict.
#define M09_Intel_ResearchGravWell				598	//	Research Division reports it has developed plans for gravity warping technology based on the alien control field.  We advise commencing research immediately.
#define M09_Traders_CouldNotApproach			599	//	We have known of this ship but could never approach it.  We are particularly vulnerable to its influence.
#define M09_Traders_ExchangeInfo				600	//	The Bentusi would like the information you have acquired.  It will be transferred automatically if you choose to trade.
#define M09_Traders_InfoTransfered				601	//	The information was successfully transferred.  Thank you.
#define M09_Fleet_HelpUsDefeatKushan			602	//	The Taiidan are determined to destroy us.  Will you help us defeat them?
#define M09_Fleet_HelpUsDefeatTaiidan			603	//	The Kushan are determined to destroy us.  Will you help us defeat them?
#define M09_Traders_CouncilKushan				604	//	Conflict is not our way.  We will bring your cause to the Galactic Council.  The Taiidan rule the Empire but even they must answer to the council.
#define M09_Traders_CouncilTaiidan				605	//	Conflict is not our way.  We will bring your cause to the Galactic Council.  The Kushan rule the Empire but even they must answer to the council.
#define M09_Intel_HyperspaceReady				606	//	Hyperspace Module fully charged.  Engage at your discretion.

/* 9.14 ANIMATIC - M09à M10 */
#define A09_Intel_Kushan						607	//	We are about to enter the outer limits of the Taiidan empire.
#define A09_Intel_Taiidan						608	//	We are about to enter the outer limits of the Kushan empire.
#define A09_Intel_2								609	//	As we approach the galactic core, resistance is expected to increase.
#define A09_Intel_3								610	//	We have identified a weak point in the enemy defenses.
#define A09_Intel_4								611	//	There is a remote research station located near an active supernova.
#define A09_Intel_5								612	//	It should only have a minor garrison protecting it.
#define A09_Intel_6								613	//	It is time to hunt the enemy as they have hunted us.


/******************************************
****            Mission 10             ****
******************************************/

/* 10 M10 - SUPERNOVA STATION [WHISPERING OCEAN] EZ10 */
#define M10_Fleet_HyperspaceSuccesful			614	//	Hyperspace jump successful.  We have cleared the Outer Rim dust bank.
#define M10_Fleet_Supernova						615	//	The supernova is two hundred and fifteen light years away.  It is emitting intense radiation.
#define M10_Intel_StikeCraftVulnerable			616	//	Strike Craft are especially vulnerable to this radiation.  Capital ships will be the most effective due to their heavy armor.
#define M10_Intel_DestroyOutpost				617	//	The Research Station is located here.  Assemble a heavy strike force and destroy it.
#define M10_Intel_RadiationHeavier				618	//	Radiation is much heavier than we expected.  Sensors indicate that asteroids may have shielding properties.
#define M10_Intel_AsteroidProtection			619	//	We recommend using the asteroid pockets for protection.
#define M10_Intel_DeployController				620	//	The radiation is disrupting our normal resource collecting operations.  Deploy a Resource Controller in the shielded asteroid pockets.
#define M10_Intel_ResearchProximitySensor		621	//	Research Division reports it is now equipped for Proximity Sensor technology.  We advise commencing research immediately.
#define M10_Intel_ResearchMinlayer				622	//	The enemy is using mines.  Research Division reports it can produce a Corvette-class minelaying ship.  We advise commencing research immediately.
#define M10_Intel_DestroyDefenseForce			623	//	We must destroy the garrison protecting the station.  Enemy units cannot be allowed to escape or they may alert the Empire to our presence.
#define M10_Intel_DestroyCarrierKushan			624	//	We are picking up a quantum wave effect.  A Taiidan Carrier is loading ships and powering up.  It must be destroyed before it can hyperspace.
#define M10_Intel_DestroyCarrierTaiidan			625	//	We are picking up a quantum wave effect.  A Kushan Carrier is loading ships and powering up.  It must be destroyed before it can hyperspace.
#define M10_Intel_HyperspaceReady				626	//	Enemy base and fleet destroyed.  Hyperspace drives online.

/* 10.11 ANIMATIC - M10à M11 */
#define A10_Intel_Kushan						627	//	We have intercepted a coded Taiidan transmission:
#define A10_Intel_Taiidan						628	//	We have intercepted a coded Kushan transmission:
#define A10_Admiral_1							629	//	You have failed to keep the Exiles from penetrating the outer perimeter.  This could be disastrous.  You will find and destroy them immediately.
#define A10_Admiral_2							630	//	Do not fail us again, or the Elite Guard will require a new commander.
#define A10_Admiral_3							631	//	Our spies believe that the Bentusi have interfered in this matter.  They must not be allowed to bring this matter to the Council and gain support for the Exiles.


/******************************************
****            Mission 11             ****
******************************************/

/* 11 M11 - P3 VS. TRADERS [TENHAUSER GATE] EZ11 */
#define M11_Fleet_HyperspaceDisengaged			632	//	Hyperdrive disengaged.  The Bentusi are here.  They're in distress.
#define M11_Intel_DestroyEnemyKushan			633	//	The Bentusi must be protected.  Draw the Taiidan fleet away and destroy them.
#define M11_Intel_DestroyEnemyTaiidan			634	//	The Bentusi must be protected.  Draw the Kushan fleet away and destroy them.
#define M11_Traders_BetusiInDebt				635	//	For the first time in memory, the Bentusi are in the debt of another.

/* 11.4 N08 - Awareness [Tenhauser Gate] Ez11 */
#define N08_Traders_ForbiddenInfo				636	//	It has been forbidden to possess this information for some time.  But after your intervention on our behalf, we feel compelled to share it with you.  Behold:
#define N08_Traders_Awareness1					637	//	In the First Time, a terrible war brought with it the collapse of your ancient empire.
#define N08_Traders_Awareness2					638	//	In an effort to sooth relations, the conquerors spared the lives of the defeated.  All survivors were sent into exile.
#define N08_Traders_Awareness3					639	//	None were permitted to follow or aid the fallen.  All memory of them was to be erased.
#define N08_Traders_Awareness4					640	//	For generations the convoy moved silently through space.  They endured great difficulties...
#define N08_Traders_Awareness5					641	//	...imperfect technology...
#define N08_Traders_Awareness6					642	//	In time, a suitable system to receive them was found.
#define N08_Traders_Awareness7					643	//	This barren world appeared to be salvation. Their true legacy forgotten, a new vision of destiny had grown out of captivity.
#define N08_Traders_Awareness8					644	//	A single artifact survived the journey.  The Guide Stone you now carry.  It was removed from the sacred "Angel Moon" of your Homeworld, a place long since reduced to myth and tale.
#define N08_Traders_Awareness9					645	//  Your progress is becoming known among the Inner Limb worlds and elsewhere.  Many cultures have prophesized your return.

/* 11 M11 - P3 VS. TRADERS [TENHAUSER GATE] EZ11 */
#define M11_Traders_ReachHomeworld				646	//	Reach your Homeworld.  Establish your claim.  We will summon the Council.
#define M11_Intel_EngageHyperdrive				647	//	Engage Hyperdrive.

/* 0.0 ANIMATIC - M11à M12 */
#define A11_Fleet_1								648	//	Emergency alert!!
#define A11_Fleet_2								649	//	Hyperdrive Malfunction.
#define A11_Fleet_3								650	//	The quantum waveform is collapsing.
#define A11_Fleet_4								651	//	Emergency interrupt engaged.
#define A11_Fleet_5								652	//	Prepare for immediate return to normal space. 


/******************************************
****            Mission 12             ****
******************************************/

/* 12 M12 - ELITE GUARD TRAP [GALACTIC CORE] EZ12 */
#define M12_Fleet_HyperspaceDisengaged			653	//	Dropping out of hyperspace.  All systems online.
#define M12_Intel_DestroyGravWell				654	//  We are caught in a gravity well.  Fighters and Corvettes will be unable to move.  Seek out the source of this field and destroy it.
#define M12_Intel_ResearchMissleDestroyer		655	//	The enemy is using a Missile Destroyer.  Research Division reports it can produce a similar ship.  We advise commencing research immediately.
#define M12_Intel_ResearchCloadkedFighter		656	//  Research Division reports it is now equipped for Cloaked Fighter technology.  We advise commencing research immediately.
#define M12_Intel_ResearchDefenseFighter		657	//	Research Division reports it is now equipped for Defense Fighter technology.  We advise commencing research immediately.
#define M12_Intel_UnderAttackKushan				658	//	We are under attack by Taiidan forces.  They are concentrating fire on our immobilized Strike Craft.
#define M12_Intel_UnderAttackTaiidan			659	//  We are under attack by Kushan forces.  They are concentrating fire on our immobilized Strike Craft.
#define M12_Defector_DefectingKushan			660	//	Attention Kushan Mothership!  This is Captain Elson of the Taiidan Elite Guard Destroyer Kapella.  We wish to defect and need assistance.  In return we are prepared to help you.  Please respond.
#define M12_Defector_DefectingTaiidan			661	//	Attention Taiidan Mothership!  This is Captain Elson of the Kushan Elite Guard Destroyer Kapella.  We wish to defect and need assistance.  In return we are prepared to help you.  Please respond.
#define M12_Intel_ProtectDefector				662	//	This could be a trap but the Kapella is clearly damaged.  Engage the pursuing fleet and draw it away from the defecting ship.
#define M12_Intel_EngageHyperdrive				663	//	The defecting captain has been brought aboard.  Engage hyperspace.

/* 12.8 ANIMATIC - M12à M13 */
#define A12_Defector_1							664	//  I am Group Captain Elson of the Destroyer Kapella.
#define A12_Defector_Kushan						665	//  The Taiidan empire has become decadent and corrupt over the centuries.
#define A12_Defector_Taiidan					666	//  The Kushan empire has become decadent and corrupt over the centuries.
#define A12_Defector_3							667	//  The use of the forbidden atmosphere-deprivation device on your planet finally triggered the Rebellion.
#define A12_Defector_4							668	//  Help me get access to the Rebellion's communication network.
#define A12_Defector_5							669	//  I will show you a way through the defenses surrounding your Homeworld.
#define A12_Defector_6							670	//  Take me to the ship graveyard at Karos.
#define A12_Defector_7							671	//  Hidden in a derelict there is a relay I can use with your help.
#define A12_Defector_8							672	//  The Rebellion waits for my sign to move into its next phase.
#define ShanesARealBigDummy!!							673	// heh heh.. ;^)

/******************************************
****            Mission 13             ****
******************************************/

/* 13 M13 - SHIP GRAVEYARD [SHINING HINTERLANDS] EZ13 */
#define M13_Fleet_HyperspaceDisengaged			674	//	Hyperspace transition complete.  We have arrived at Karos.
#define M13_Intel_CommunicationRelay			675	//	The communication relay is here.  A Fighter or Corvette must dock with it to establish the link.
#define M13_Intel_CapShipLost					676	//	Attention.  We have lost contact with one of our capital ships.  It's last recorded position is here.
#define M13_Intel_HyperspaceSignature			677	//	Hyperspace signatures have been found at these locations but we detect no new ships.
#define M13_Intel_FoundMissingShip				678	//	We've located one of the missing ships.  It appears that it can be salvaged and reactivated.
#define M13_Intel_GraveyardDefended				679	//	The Graveyard is defended by autoguns.  They will complicate our attempt to reach the communications array.
#define M13_All_CommLinkEstablished				680	//	I'm in!  Communication link established.  I've docked!  Communication link established.
#define M13_Defector_ResistanceInformedKushan	681	//	Patching into command node now... The Taiidan resistance has been informed of your actions and are preparing the fleets.  You have our thanks.
#define M13_Defector_ResistanceInformedTaiidan	682	//	Patching into command node now... The Kushan resistance has been informed of your actions and are preparing the fleets.  You have our thanks.
#define M13_Defector_CoordinatesTransfered		683	//	The hyperspace coordinates you require have been transferred to your Mothership.  Farewell.
#define M13_Intel_HyperspaceEnabled				684	//	Hyperspace Enabled.

/* 13.10 ANIMATIC - M13à M14 */
#define A13_Intel_1								685	//	According to the data we received from Captain Elson, the Homeworld system is surrounded by a network of hyperspace inhibitors.
#define A13_Intel_2								686	//	The inhibitors are all heavily shielded and do not show up on any sensors.
#define A13_Intel_3								687	//	Elson has provided us with co-ordinates of the most vulnerable inhibitor station.
#define A13_Intel_4								688	//	Our goal is to destroy the station and create our own access point.


/******************************************
****            Mission 14             ****
******************************************/

/* 14 M14 - MINING FACILITY [BRIDGE OF SIGHS] EZ14 */
#define M14_Fleet_HyperspaceDisengaged			689	//	Hyperspace successful.  We are at the edge of the Homeworld system.
#define M14_Intel_DestroyGenerator				690	//	Elson's information was correct.  This is the field generator.  We must destroy it.
#define M14_Intel_ResearchHeavyGuns				691	//	Research Division reports it is now equipped for Heavy Guns technology.  We advise commencing research immediately.
#define M14_Intel_ResearchSensors				692	//	Research Division reports advancements in sensor fidelity which would allow us to determine the location of enemy ships.  We advise commencing research immediately.
#define M14_Intel_DestroyHyperspaceGates		693	//	The enemy has activated several standing hyperspace gates.  Destroy the gates to prevent enemy reinforcement. 
#define M14_Intel_HyperdriveOnline				694	//	The field surrounding the Homeworld system has been shut down.  Hyperdrive on-line.
#define M14_Fleet_TakeUsHome					695	//	Take us home.

/* 14.8 ANIMATIC - M14à M15 */
#define A14_Intel_1								696	//	We have to assume that the Homeworld's defensive fleet must be alerted to our presence.
#define A14_Intel_2								697	//	All vessels and crew at maximum readiness.
#define A14_Intel_3								698	//	Weapons and tracking at 100% efficiency.
#define A14_Intel_4								699	//	The fleet is ready.
#define A14_Intel_5								700	//	There can be no retreat now.


/******************************************
****            Mission 15             ****
******************************************/

/* 15 M15 - HEADSHOT [CHAPEL PERILOUS] EZ15 */
#define M15_Fleet_HyperspaceInterrupted			701	//	Hyperspace interrupted.
#define M15_Intel_AlertedKushan					702	//	Shutting down the inhibitor field has alerted the Taiidan to our presence.
#define M15_Intel_AlertedTaiidan				703	//	Shutting down the inhibitor field has alerted the Kushan to our presence.
#define M15_Intel_DestroyHeadshot				704	//  We are on a collision course with a very large object.  It appears to have escorts.  It must be destroyed before it impacts the Mothership.
#define M15_Intel_ImpactIn20					705	//	Impact in twenty minutes.
#define M15_Intel_ImpactIn15					706	//	Impact in fifteen minutes.
#define M15_Intel_ImpactIn10					707	//	Impact in ten minutes.
#define M15_Intel_ImpactIn5						708	//	Impact in five minutes.
#define M15_Intel_ImpactIn4						709	//	Impact in four minutes.
#define M15_Intel_ImpactIn3						710	//	Impact in three minutes.
#define M15_Intel_ImpactIn2						711	//	Impact in two minutes.
#define M15_Intel_ImpactIn1						712	//	Impact in one minute.
#define M15_Fleet_StandbyForImpact				713	//	Standby for impact.
#define M15_Intel_HyperspaceReady				714	//	Hyperspace module charged and ready.

/* 15.14 ANIMATIC - M15à M16 */
#define A15_1									715	//	What's happening?  OR  The Enemy must have been desperate.
#define A15_2									716	//  Karan.  OR  The time we took to break up that attack has allowed them time to reinforce.
#define A15_3									717	//  You've taken one step too close to me.  OR  The chemical composition of this system matches that of the Guidestone.
#define A15_4									718	//  From here I can touch you.  OR  We're home.


/******************************************
****            Mission 16             ****
******************************************/

/* 16 M16 - FINAL BATTLE [HOMEWORLD SYSTEM] EZ16 */
#define M16_Intel_LostKaran						719	//	We've lost Karan.  Fleet Command is gone.  Emergency biotech teams are working to keep her alive.
#define M16_Intel_CollisionAsteroidKushan		720	//	The collision asteroid must have served its purpose as a delay tactic.  There is a large number of Taiidan ships located here.
#define M16_Intel_CollisionAsteroidTaiidan		721	//	The collision asteroid must have served its purpose as a delay tactic.  There is a large number of Kushan ships located here.
#define M16_Intel_EmperorsShip					722	//	A Mothership-class vessel is among them.
#define M16_Intel_DestroyAll					723	//	All of these forces must be destroyed.  Good luck.
#define M16_Intel_EnemyReinforcements			724	//	Enemy reinforcements emerging from hyperspace.
#define M16_Intel_AnotherFleet					725	//	Another fleet is coming out of Hyperspace right on top of us.  We are being overwhelmed!
#define M16_Defector_CaptainElson				726	//	This is Captain Elson.  We have been battling reinforcement fleets to get here and have lost many ships already.
#define M16_Defector_EmperorKushan				727	//  The Emperor's flagship is here.  Together we can defeat him and the Taiidan fleet.  I am placing squadrons Cor and Jasah under your command.
#define M16_Defector_EmperorTaiidan				728	//  The Emperor's flagship is here.  Together we can defeat him and the Kushan fleet.  I am placing squadrons Cor and Jasah under your command.
#define M16_Intel_DestroyFlagship				729	//	Here is the Emperor's flagship.  It must be destroyed.
#define M16_Fleet_BackOnline					730	//	...Fleet Command back online.  The Emperor is gone.
#define M16_Traders_BroughtTheCouncil			731	//	We have brought the Council.  This war is over.

/* 16.10 ANIMATIC - M15à M16 */
#define A16_Fleet_SigningOff					732	//	This is Fleet Command, signing off.

/* Quick Fixes */
#define N01_All_Misc_5							733	//  All stations green.
#define M01_Intel_ResearchOnline				734	//	The primary Research Ship has been constructed and the Research Division is online.  Begin Fighter Chassis research immediately.
#define M15_Intel_ImpactIn30secs				735	//	Impact in 30 seconds.
#define M15_Intel_ImpactIn10secs				736	//	Impact in 10 seconds.

#define A15_1_Alt								737	//	The Enemy must have been desperate.
#define A15_2									738	//  The time we took to break up that attack has allowed them time to reinforce.
#define A15_3									739	//  The chemical composition of this system matches that of the Guidestone.
#define A15_4									740	//  We're home.

#define STAT_F_CrateFoundResources				741	//	STATUS_CrateFound-ResourcesRecovered
#define STAT_F_CrateFoundShips					742	//	STATUS_CrateFound-ShipsRecovered
#define STAT_F_CrateFoundTech					743	//	STATUS_CrateFound-TechRecovered
#define COMM_F_NoMoreResearchTopic				744	//	COMMAND_Research-NoMoreResearchTopics
#define STAT_F_RelicPlayerDies					745	//	STATUS_RelicPlayerDies-EasterEgg
#define STAT_F_RelicAllianceFormed				746	//	STATUS_RelicAllianceFormed-EasterEgg
#define STAT_F_RelicAllianceBroken				747	//	STATUS_RelicAllianceBroken-EasterEgg

#define STAT_CloakedShips-Detected				748	//	STATUS_CloakedShips-Detected[Report] (ALL SHIPS)

#define N01_Chatter_BG							749
#define N01_Fleet_Standby						750	//	Stand by for Initial Fleet Command Line testing...


/* ALTERNATIVE VERSIONS FOR ADAM AND ERIN */

#define M02_Intel_ObjectivesCompleteSHORT		751	//	Objectives complete.  Standby for immediate return to Kharak.
#define M09_Intel_VesselNeutralizedSHORT		752	//	Alien vessel neutralized.
#define M12_Intel_UnderAttackKushanSHORT		753	//	We are under attack by Taiidan forces.
#define M12_Intel_UnderAttackTaiidanSHORT		754	//  We are under attack by Kushan forces.


/* PICKUPS - Fleet Intel Pickups.doc */

#define M01_Intel_HyperdriveTest				755	//	Standby for Hyperdrive test.  Internal pressure doors sealed.  Abort systems standing by.

#define M02_Intel_EnemyInferior					756	//	We have determined that these enemy units are inferior to ours.
#define M02_Intel_DestroyCompletely				757	//  To protect against penetration of the Kharak system, destroy the attacking force completely.
#define M02_Intel_EnemyHyperspacing				758	//	The enemy Mothership is hyperspacing.  Recall all Fighters and prepare for return to Kharak.

#define M03_Intel_DefendCryoTrays				759	//	The Cryo Trays are under attack.  Defend them.
#define M03_Intel_SalvageCryoTrays				760	//	Enemy units neutralized.  Begin salvaging the cryo trays.

#define M04_Intel_MoveController				761	//	To increase harvesting efficiency, move your Resource Controller as close to heavy resource areas as possible.

#define M05_Intel_MoveAttack					762	//	Enemy capital ships appear to have lighter armor on the top, bottom, and rear sides.  Our capital ships should be issued move orders while attacking to take advantage of this weakness.

#define M06_Intel_IonCannonTech					763	//	Research Division reports it is now equipped for Ion Cannon technology.  We advise commencing research immediately.
#define M06_Intel_PlasmaBombTech				764	//	Research Division reports it is now equipped for Plasma Bomb technology.  We advise commencing research immediately.
#define M06_Intel_DetectingBentusi				765	//	Detecting incoming Bentusi vessel from the clearing ahead.

#define M07_Intel_HyperdriveFailed				766	//	Hyperdrive jump failed!  The Quantum waveform collapsed due to some kind of inhibitor field.
#define M07_Intel_AnalyzingField				767	//	Analyzing field.  Continue to protect the Mothership until the source is located.
#define M07_Intel_FieldDisappeared				768	//	The inhibitor field has disappeared.
#define M07_Intel_HyperdriveFunctional			769	//	The Hyperdrive is fully functional.
#define M07_Intel_ResearchMultigunVette			770	//	The enemy is relying heavily on Fighter-class ships.  Our Research division reports it can design a new type of Corvette specially suited to combat multiple fighters.  Begin Research as soon as possible.

#define M08_Intel_DestroyHostiles				771	//	We have enemy units closing from all directions.  Engage and destroy hostiles.
#define M08_Intel_InhibitorCeased				772	//	The inhibitor field has ceased.  Hyperspace ability has been restored.
#define M08_Fleet_ItsTheKharToba				773	//	It's the Khar-Toba.
#define M08_Intel_IdenticalToKharToba			774	//	Metallurgy and structural composition of the ship are an identical match to the Khar-Toba wreckage on Kharak.  Our origins are the same.

#define M09_Intel_SalvageShip					775	//	While the field was up they were able to analyze the alien control system.  We now have control of the foreign vessels.
#define M09_Intel_SendSalvageTeam				776	//  Send a Salvage Team to further investigate the alien ship.
#define M09_Intel_VesselNeutralizedSHORT		777	//	Alien vessel neutralized.
#define M09_Intel_SalvageDocked					778	//	Salvage Team docked.  The alien ship is millions of years old.  Its purpose is unclear.
#define M09_Intel_ResearchGravWellNEW			779	//  However, Research Division reports it has developed plans for gravity warping technology based on composition of the alien hull.  We advise commencing research immediately.

#define M10_Intel_ResearchCloakGen				780	//	Research Division reports it is now equipped for Cloaked Generator technology.  We advise commencing research immediately.
#define M10_Intel_UseViensForProtection			781	//	Radiation is much heavier than we expected.  Sensors indicate that these veins of space dust may have shielding properties.  We recommend using the veins for protection.

#define M11_Intel_ResearchSuperHeavy			782	//	Research Division reports it is now equipped for Super Heavy Chassis technology.  We advise commencing research immediately.

#define SP_Intel_PrimaryObjectiveComplete		783	//	Primary objective complete.
#define SP_Intel_SecondaryObjectiveComplete		784	//	Secondary objective complete.
#define SP_Intel_ConsultObjectives				785	//	A mission objective remains incomplete. Consult objectives.
#define SP_Intel_MissionFailed					786	//	Mission Failed.

/* PICKUPS - Fleet Command Pickups.doc */
#define M05_Fleet_FollowTheGuidestone			787	//	There's nothing left for us to return to.  Our only option is to follow the path etched into the Guidestone.  Finding our ancient home is our only hope now.

#define M07_Fleet_Charging8Minutes				788	//	Hyperspace module charging, ready in 8 minutes.
#define M07_Fleet_Charging1Minute				789	//	Hyperspace module charging, ready in 1 minute.
#define M07_Fleet_HyperdriveFailed				790	//	Hyperdrive jump failed!  The Quantum waveform collapsed due to some kind of inhibitor field.

#define M08_Fleet_CannotStay					791	//	We cannot stay - we're on a journey.  But let there be peace between us, for we have something in common.  The hyperdrive technology left to us by our ancestors is identical to yours.  The Homeworld we seek may be yours as well.

#define COMM_F_HyperspaceDetected				792	//  0.8 STATUS_HyperspaceDetected - 1) Hyperspace signature detected.
#define COMM_F_PrimaryObjectiveComplete			793	//  0.9 STATUS_PrimaryObjectiveComplete - 1) Primary objective complete.
#define COMM_F_SecondaryObjectiveComplete		794	//	0.10 STATUS_SecondaryObjectiveComplete - 1) Secondary objective complete.
#define STAT_F_CapturedShipRetrofit				795	//	0.11 STATUS_CapturedShipBeingRetrofitted - 1) Captured ship secure.  Retrofit commencing.

/* PICKUPS - Final Animatic */
#define A16_Narrator_Ending						796	//	The Galactic Council recognized our claim to this world.
#define A16_Narrator_Ending2					797	//	The sacrifice of thousands has left a trail of destruction behind us, like a path across the galaxy
#define A16_Narrator_Ending3					798	//	..to  Higaara, our Homeworld.
#define A16_Narrator_Ending4					799	//	So much destruction, so many lives lost, for this place. 
#define A16_Narrator_Ending5					800	//	A place of wonder to those who knew only the sands of Kharak.
#define A16_Narrator_Ending6					801	//	Our colonists were released from their long sleep.
#define A16_Narrator_Ending7					802	//	All symbols of the old empire were destroyed.
#define A16_Narrator_Ending8					803	//	But the conflict will never be forgotten.
#define A16_Narrator_Ending9					804	//	A celebration marked the beginning of a new time. 
#define A16_Narrator_Ending10					805	//	No longer Fleet Command, Karan Sjet survived extraction from the Mothership's core.
#define A16_Narrator_Ending11					806	//  She insisted that she would be the last person to disembark and set foot on the Homeworld.

/* SCRAPPED EVENTS REQUESTED TO BE RE-ADDED */
#define SP_Pilot_SCVetteNotEnough				807	//	This is to tell the player they're not using enough SC Vettes.

/* damned OEM mission */
#define M04OEM_Intel_Hyperspace					808	//	We've located the origin of the Turanic Raider fleets.  They came from a small, isolated planetoid located in a remote area of the Great Wasteland.  Engage hyperspace.
#define M05OEM_Intel_PlanetaryDefense			809	//	Sensors indicate a heavily armed planetary defense system which we cannot penetrate.
#define M05OEM_Intel_HyperspacingIn				810	//  The planetary garrison must have alerted the main Turanic Raider Fleet which is currently Hyperspacing in here.  They are retreating to the planetoid.
#define M05OEM_Intel_DestroyCarriers			811	//  We must stop their Carriers from reaching the safety of the planetary defenses.
#define M05OEM_Intel_PrepareForAssault			812	//	The Turanic Carriers are launching Strike Craft.  Prepare for assault.
#define M05OEM_Intel_MissionFailed				813	//	Mission Failed.
#define M05OEM_Intel_StayOutOfRange				814	//  The planetary defense system covers this area. Any of our ships entering its range will be destroyed.  Avoid this area at all costs.
#define M05OEM_Intel_DestroyedRaiders			815	//	We have destroyed the body of the Turanic Raider fleet.  We can now proceed on our journey.  Hyperspace when ready.

/* Bug Fixes */
#define M04_Intel_DefendTheFleetShort			816	//	Turanic Raider fighters are attacking. Prepare for more hostile ships to arrive.
#define STAT_Group_LowOnFuel					817	//	115. STATUS_StrikeCraft-LowOnFuel (ALL SHIPS)
#define STAT_Group_OutOfFuel					818	//	116. STATUS_StrikeCraft-OutOfFuel (ALL SHIPS)

/* alternate tutorial event for Falko */
#define TUT_SelectInfoOverlayAlt				819	//	At the top right of the screen is the Info Overlay.  It displays all the selected ships.  Click Next to continue.

/* SCRAPPED EVENTS REQUESTED TO BE RE-ADDED */
#define SP_Pilot_P2Refuelling					820	//	This tells the player to go after the fuel pods

/*********  FINALLY DONE!!!!  Maybe :)  ******/


