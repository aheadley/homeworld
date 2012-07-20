//#if defined(__LITTLE_ENDIAN__)
#ifndef ENDIAN_BIG

/* Actor Flags */
#define ACTOR_PILOT_FLAG						1024        //  0x00000400
#define ACTOR_FLEETCOMMAND_FLAG					2048        //  0x00000800
#define ACTOR_FLEETINTEL_FLAG					4096        //  0x00001000
#define ACTOR_TRADERS_FLAG						8192        //  0x00002000
#define ACTOR_PIRATES2_FLAG						16384       //  0x00004000
#define ACTOR_ALLSHIPSENEMY_FLAG				32768       //  0x00008000
#define ACTOR_AMBASSADOR_FLAG					65536       //  0x00010000
#define ACTOR_NARRATOR_FLAG                     131072      //  0x00020000
#define ACTOR_DEFECTOR_FLAG						262144		//	0x00040000
#define ACTOR_GENERAL_FLAG						524288		//	0x00080000
#define ACTOR_EMPEROR_FLAG						1048576		//	0x00100000
#define ACTOR_KHARSELIM_FLAG					2097152		//	0x00200000

/* Speech Type Flags */
#define SPEECH_GROUP_FLAG						8388608		//	0x00800000
#define SPEECH_CHATTER_FLAG						16777216    //  0x01000000
#define SPEECH_STATUS_FLAG						33554432	//	0x02000000
#define SPEECH_COMMAND_FLAG						67108864    //  0x04000000
#define SPEECH_TUTORIAL_FLAG					134217728   //  0x08000000
#define SPEECH_SINGLEPLAYER_FLAG				268435456   //  0x10000000
#define SPEECH_NIS_FLAG							536870912   //  0x20000000
#define SPEECH_ANIMATIC_FLAG					1073741824  //  0x40000000
#define SPEECH_ALWAYSPLAY_FLAG					2147483648	//	0x80000000

/* Speech Masks */
#define SPEECH_EVENT_MASK						1023		//	0x000003FF
#define SPEECH_FLAG_MASK						4193280		//	0x003FFC00
#define SPEECH_TYPE_MASK						4026531840	//	0xFFC00000

#else // we are PPC/BIG_ENDIAN!!!

    /* Actor Flags */
    #define ACTOR_PILOT_FLAG						0x04000000
    #define ACTOR_FLEETCOMMAND_FLAG					0x08000000
    #define ACTOR_FLEETINTEL_FLAG					0x10000000
    #define ACTOR_TRADERS_FLAG						0x20000000
    #define ACTOR_PIRATES2_FLAG						0x40000000
    #define ACTOR_ALLSHIPSENEMY_FLAG				0x80000000
    #define ACTOR_AMBASSADOR_FLAG					0x00000001
    #define ACTOR_NARRATOR_FLAG                     0x00000002
    #define ACTOR_DEFECTOR_FLAG						0x00000004
    #define ACTOR_GENERAL_FLAG						0x00000008
    #define ACTOR_EMPEROR_FLAG						0x00000010
    #define ACTOR_KHARSELIM_FLAG					0x00000020
    
    /* Speech Type Flags */
    #define SPEECH_GROUP_FLAG						0x00000080
    #define SPEECH_CHATTER_FLAG						0x00000100
    #define SPEECH_STATUS_FLAG						0x00000200
    #define SPEECH_COMMAND_FLAG						0x00000400
    #define SPEECH_TUTORIAL_FLAG					0x00000800
    #define SPEECH_SINGLEPLAYER_FLAG				0x00001000
    #define SPEECH_NIS_FLAG							0x00002000
    #define SPEECH_ANIMATIC_FLAG					0x00004000
    #define SPEECH_ALWAYSPLAY_FLAG					0x00008000
    
    /* Speech Masks */
    #define SPEECH_EVENT_MASK						0x03FF0000
    #define SPEECH_FLAG_MASK						0xFC00003F
    #define SPEECH_TYPE_MASK						0x0000FFC0
    
#endif


/******************************************
****                                   ****
****             ALL SHIPS             ****
****                                   ****
******************************************/

#define FrickinForgotToStartAtZero				0

/* ALL SHIPS - Capital Ships */
#define STAT_Cap_Damaged_Specific				(1 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)		//	1. STATUS_CapitalShip-Damaged(Report) (ALL SHIPS)
#define STAT_Cap_Surroundings_Enemy				(2 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)		//	2. STATUS_CapitalShip-Surroundings(Report) (ALL SHIPS)
#define STAT_Cap_Surroundings_Resources			(3 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)		//	2. STATUS_CapitalShip-Surroundings(Report) (ALL SHIPS)
#define STAT_Cap_Surroundings_Nothing			(4 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)		//	2. STATUS_CapitalShip-Surroundings(Report) (ALL SHIPS)
#define STAT_Cap_LargeHit_Query					(5 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)		//	7. STATUS_CapitalShip-LargeHit(Query) (ALL SHIPS)
#define STAT_Cap_LargeHit_More_Query			(6 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)		//	7. STATUS_CapitalShip-LargeHit(Query) (ALL SHIPS)
#define STAT_Cap_LargeHit_Resp					(7 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)		//	8. STATUS_CapitalShip-LargeHit(Response) (ALL SHIPS)
#define STAT_Cap_LargeHit_More_Resp				(8 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)		//	8. STATUS_CapitalShip-LargeHit(Response) (ALL SHIPS)
#define STAT_Cap_Damaged						(9 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)		//	9. STATUS_CapitalShip-Damaged(Report) (ALL SHIPS)
#define STAT_Cap_Dies							(10 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	12. STATUS_CapitalShip_Dies (ALL SHIPS)
#define STAT_Cap_Dies_Report					(11 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	13. STATUS_CapitalShip-Dies('A'Report) (ALL SHIPS)
#define COMM_Cap_Retire							(12 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	15. COMMAND_CapitalShip-Retire (ALL SHIPS)

/* ALL SHIPS - Ship-Specific Events */
#define STAT_Strike_Damaged						(13 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	16. STATUS_StrikeCraft_Damaged (ALL SHIPS)
#define COMM_Strike_Retired						(14 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	17. COMMAND_StrikeCraft-Retired (ALL SHIPS)
#define COMM_SCVette_Salvage					(15 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	26. COMMAND_SalvageCorvette-Salvage
#define STAT_SCVette_TargetAcquired				(16 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	27. STATUS_SalvageCorvette-TargetAcquired (ALL SHIPS)
#define STAT_RepVette_StartedRepairs			(17 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	29. STATUS_RepairCorvette-StartedRepairs (ALL SHIPS)
#define STAT_RepVette_FinishedRepairs			(18 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	30. STATUS_RepairCorvette-FinishedRepairs (ALL SHIPS)
#define COMM_DDF_LaunchDrones					(19 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	33. COMMAND_DroneFrigate-LaunchDrones (ALL SHIPS)
#define COMM_DDF_RetractDrones					(20 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	34. COMMAND_DroneFrigate-RetractDrones (ALL SHIPS)
#define STAT_Carrier_Damaged					(21 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	36. STATUS_Carrier-Damaged(Report) (ALL SHIPS)
#define STAT_Cruiser_Damaged					(22 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	38. STATUS_HeavyCruiser-Damaged(Report) (ALL SHIPS)
#define STAT_MoShip_LargeAttack					(23 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	41. STATUS_MothershipUnderLargeAttack(Report) (ALL SHIPS)
#define STAT_MoShip_LargeAttack_Resp			(24 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	42. STATUS_MothershipUnderLargeAttack(Response) (ALL SHIPS)
#define COMM_Grav_On							(25 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	44. COMMAND_GravityWellGenerator-On (ALL SHIPS)
#define STAT_Grav_Collapse						(26 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	45. STATUS_GravityWellGenerator-Collapse (ALL SHIPS)
#define COMM_Cloak_CloakingOn					(27 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	47. COMMAND_CloakingShips-CloakingOn(Report) (ALL SHIPS)
#define STAT_Cloak_CloakingOn_Resp				(28 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	48. STATUS_CloakingShips-CloakingOn(Response) (ALL SHIPS)
#define COMM_Cloak_InsufficientPower			(29 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	49. COMMAND_CloakingShips-InsufficientPowerToCloak (ALL SHIPS)
#define STAT_Cloak_CloakPowerLow				(30 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	50. STATUS_CloakingShips-CloakingPowerLow(Report) (ALL SHIPS)
#define COMM_Cloak_Decloak						(31 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	51. COMMAND_CloakingShips-DeCloak (ALL SHIPS)

/* ALL SHIPS - Resourcing */
#define STAT_ResCol_Grp_Damaged					(32 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	52. STATUS_GroupCollectors-Damaged(Report) (ALL SHIPS)
#define STAT_ResCol_Destroyed					(33 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	53. STATUS_ResourceCollector-Destroyed(Report) (ALL SHIPS)
#define STAT_ResCol_ResourcesTransferred		(34 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	54. STATUS_ResourceCollector-ResourcesTransferred (ALL SHIPS)
#define COMM_ResCol_Harvest						(35 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	55. COMMAND_ResourceCollector-Harvest (ALL SHIPS)
#define STAT_ResCol_Full						(36 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	57. STATUS_ResourceCollector-Full (ALL SHIPS)
#define STAT_ResCol_Damaged						(37 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	58. STATUS_ResourceController-Damaged (ALL SHIPS)

/* ALL SHIPS - Dogfighting*/
#define STAT_Fighter_WingmanChased				(38 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	60. STATUS_Fighter-WingmanChased(Report) (ALL SHIPS)
#define STAT_Fighter_WingmanChased_Rsp			(39 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	61. STATUS_Fighter-WingmanChased(Response) (ALL SHIPS)
#define STAT_Fighter_WingmanHit					(40 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	62. STATUS_Fighter-WingmanHit(Report) (ALL SHIPS)
#define STAT_Fighter_WingmanHit_Rsp				(41 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	63. STATUS_Fighter-WingmanHit(Response) (ALL SHIPS)
#define STAT_Fighter_WingmanLethal				(42 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	65. STATUS_Fighter-WingmanLethallyDamaged(Response) (ALL SHIPS)
#define STAT_Fighter_WingmanDies				(43 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	66. STATUS_Fighter-WingmanDies(Report) (ALL SHIPS)
#define STAT_Fighter_LeaderChased				(44 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	68. STATUS_Fighter-LeaderBeingChased(Report) (ALL SHIPS)
#define CHAT_Fighter_Wingman					(45 + ACTOR_PILOT_FLAG + SPEECH_CHATTER_FLAG)	//	67. CHATTER_Fighter-Wingman(Report) (ALL SHIPS)
#define COMM_LInt_Kamikaze						(46 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	69. COMMAND-LightInterceptor-Kamikaze (ALL SHIPS)

/* ALL SHIPS - Battle Events */
#define STAT_AssGrp_UnderAttack					(47 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG + SPEECH_GROUP_FLAG)	//	70. STATUS_AssignedGroup-UnderAttack(Report) (ALL SHIPS)
#define STAT_Grp_UnderAttack					(48 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	71. STATUS_GenericGroup-UnderAttack(Report) (ALL SHIPS)
#define STAT_Grp_UnderAttack_Rsp				(49 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	72. STATUS_GenericGroup-UnderAttack(Response) (ALL SHIPS)
#define STAT_Grp_StartBattleDisadvant			(50 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	73. STATUS_GenericGroup-StartBattleDisadvantaged(Report) (ALL SHIPS)
#define STAT_Grp_StartBattleDisadvant_Rsp		(51 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	74. STATUS_GenericGroup-StartBattleDisadvantaged(Response) (ALL SHIPS)
#define STAT_Grp_StartBattleFair				(52 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	75. STATUS_GenericGroup-StartBattleFair(Report) (ALL SHIPS)
#define STAT_Grp_StartBattleFair_Rsp			(53 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	76. STATUS_GenericGroup-StartBattleFair(Response) (ALL SHIPS)
#define STAT_Grp_StartBattleAdvantaged			(54 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	77. STATUS_GenericGroup-StartBattleAdvantaged(Report) (ALL SHIPS)
#define STAT_Grp_StartBattleAdvantaged_Rsp		(55 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	78. STATUS_GenericGroup-StartBattleAdvantaged(Response) (ALL SHIPS)
#define COMM_Group_Retreat						(56 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	79. COMMAND_GenericGroup-Retreat (ALL SHIPS)
#define COMM_Group_Retreat_Rsp					(57 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	80. COMMAND_GenericGroup-Retreat(Response) (ALL SHIPS)
#define STAT_Grp_WinningBattle					(58 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	81. STATUS_GenericGroup-WinningBattle(Report) (ALL SHIPS)
#define STAT_Grp_WinningBattle_Rsp				(59 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	82. STATUS_GenericGroup-WinningBattle(Response) (ALL SHIPS)
#define STAT_Grp_KickingButt					(60 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	83. STATUS_GenericGroup-KickingButt(Report) (ALL SHIPS)
#define STAT_Grp_KickingButt_Rsp				(61 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	84. STATUS_GenericGroup-KickingButt(Response) (ALL SHIPS)
#define STAT_Grp_BattleWon						(62 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	85. STATUS_GenericGroup-BattleWon(Report) (ALL SHIPS)
#define STAT_Grp_LosingBattle					(63 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	86. STATUS_GenericGroup-LosingBattle(Report) (ALL SHIPS)
#define STAT_Grp_LosingBattle_More				(64 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	86. STATUS_GenericGroup-LosingBattle(Report) (ALL SHIPS)
#define STAT_Grp_LosingBattle_Rsp				(65 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	87. STATUS_GenericGroup-LosingBattle(Response) (ALL SHIPS)
#define STAT_Grp_LosingBadly					(66 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	88. STATUS_GenericGroup-LosingBadly(Report) (ALL SHIPS)
#define STAT_Grp_LosingBadly_More				(67 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	88. STATUS_GenericGroup-LosingBadly(Report) (ALL SHIPS)
#define STAT_Grp_LosingBadly_Rsp				(68 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	89. STATUS_GenericGroup-LosingBadly(Response) (ALL SHIPS)
#define CHAT_Grp_PositiveBattle					(69 + ACTOR_PILOT_FLAG + SPEECH_CHATTER_FLAG)	//	90. CHATTER_GenericGroup-PositiveBattle(Report) (ALL SHIPS)
#define CHAT_Grp_PositiveBattle_More			(70 + ACTOR_PILOT_FLAG + SPEECH_CHATTER_FLAG)	//	90. CHATTER_GenericGroup-PositiveBattle(Report) (ALL SHIPS)
#define CHAT_Grp_NegativeBattle					(71 + ACTOR_PILOT_FLAG + SPEECH_CHATTER_FLAG)	//	91. CHATTER_GenericGroup-NegativeBattle(Report) (ALL SHIPS)
#define CHAT_Grp_NegativeBattle_More			(72 + ACTOR_PILOT_FLAG + SPEECH_CHATTER_FLAG)	//	91. CHATTER_GenericGroup-NegativeBattle(Report) (ALL SHIPS)
#define CHAT_Grp_NegativeBattle_More2			(73 + ACTOR_PILOT_FLAG + SPEECH_CHATTER_FLAG)	//	91. CHATTER_GenericGroup-NegativeBattle(Report) (ALL SHIPS)
#define CHAT_Grp_NeutralBattle					(74 + ACTOR_PILOT_FLAG + SPEECH_CHATTER_FLAG)	//	92. CHATTER_GenericGroup-NeutralBattle(Report) (ALL SHIPS)
#define CHAT_Grp_NeutralBattle_More				(75 + ACTOR_PILOT_FLAG + SPEECH_CHATTER_FLAG)	//	92. CHATTER_GenericGroup-NeutralBattle(Report) (ALL SHIPS)
#define CHAT_Grp_FriendlyFire					(76 + ACTOR_PILOT_FLAG + SPEECH_CHATTER_FLAG)	//	93. CHATTER_GenericGroup-FriendlyFire (ALL SHIPS)

/* ALL SHIPS - Selection, Movement and Other Commands */
#define COMM_AssGrp_Select						(77 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG + SPEECH_GROUP_FLAG)	//	95. COMMAND_AnyShip-SelectHotkeyGroup (ALL SHIPS)
#define COMM_Selection							(78 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	96. COMMAND_AnyShip-Selection (ALL SHIPS)
#define COMM_Move								(79 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	97. COMMAND_AnyShip-SpecifyDestination (ALL SHIPS)
#define COMM_MoveCancelled						(80 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	98. COMMAND_AnyShip-MoveCancelled (ALL SHIPS)
#define COMM_Attack								(81 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	102. COMMAND_AnyShip-Attack (ALL SHIPS)
#define COMM_AttackCancelled					(82 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	103. COMMAND_AnyShip-AttackCancelled (ALL SHIPS)
#define COMM_AttackPlayerUnits					(83 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	105. COMMAND_AnyShip-ForceAttackPlayerUnits (ALL SHIPS)
#define COMM_AttackPlayerUnits_Cond				(84 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	106. COMMAND_AnyShip-ForceAttackPlayerUnits(Conditional) (ALL SHIPS)
#define COMM_AttackResources					(85 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	107. COMMAND_AnyShip-ForceAttackResources (ALL SHIPS)
#define COMM_AttackResourcesGeneric				(86 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	108. COMMAND_AnyShip-ForceAttackResourcesGeneric (ALL SHIPS)
#define COMM_SetFormation						(87 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	111. COMMAND_AnyShip-SetFormation (ALL SHIPS)
#define COMM_Guard								(88 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	113. COMMAND_AnyShip-Guard (ALL SHIPS)
#define STAT_Strike_LowOnFuel					(89 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG + SPEECH_ALWAYSPLAY_FLAG)	//	115. STATUS_StrikeCraft-LowOnFuel (ALL SHIPS)
#define STAT_Strike_OutOfFuel					(90 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG + SPEECH_ALWAYSPLAY_FLAG)	//	116. STATUS_StrikeCraft-OutOfFuel (ALL SHIPS)
#define COMM_CannotComply						(91 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	119. COMMAND_AnyShip-CannotComply (ALL SHIPS)

/* ALL SHIPS - Hyperspace */
#define COMM_Hyper_Autodock						(92 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	120. COMMAND_AnyShip-Hyperspace&Autodock (ALL SHIPS)
#define STAT_Hyper_Abandoned					(93 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	122. STATUS_AnyShip-AbandonedByHyperspace (ALL SHIPS)

/* ALL SHIPS - Construction and Docking */
#define CHAT_Const_DuringBattle					(94 + ACTOR_PILOT_FLAG + SPEECH_CHATTER_FLAG)	//	123. CHATTER_CarrierOrMShip-ConstructionDuringBattle (ALL SHIPS)
#define COMM_Const_BuildCapShip					(95 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	124. COMMAND_CarrierOrMShip-BuildCapitalShip (ALL SHIPS)
#define CHAT_Const								(96 + ACTOR_PILOT_FLAG + SPEECH_CHATTER_FLAG)	//	125. CHATTER_CarrierOrMShip-Construction (ALL SHIPS)
#define CHAT_Const_More							(97 + ACTOR_PILOT_FLAG + SPEECH_CHATTER_FLAG)	//	125. CHATTER_CarrierOrMShip-Construction (ALL SHIPS)
#define STAT_Cap_Launched						(98 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	126. STATUS_CarrierOrMShip-CapitalShipLaunched (ALL SHIPS)
#define STAT_Cap_Welcomed						(99 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	127. STATUS_AnyShip-CapitalShipWelcomed (ALL SHIPS)
#define COMM_Strike_Dock						(100 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	128. COMMAND_StrikeCraft-Dock (ALL SHIPS)
#define CHAT_Docking							(101 + ACTOR_PILOT_FLAG + SPEECH_CHATTER_FLAG)	//	130. CHATTER_AnyShip-Docking (ALL SHIPS)
#define CHAT_Docking_More						(102 + ACTOR_PILOT_FLAG + SPEECH_CHATTER_FLAG)	//	130. CHATTER_AnyShip-Docking (ALL SHIPS)
#define STAT_Cap_ShipDocked						(103 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	131. STATUS_CapitalShips-ShipDocked (ALL SHIPS)
#define STAT_Research_Docking					(104 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	133. STATUS_ResearchShip-Docking (ALL SHIPS)
#define STAT_Research_Docked					(105 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	134. STATUS_ResearchShip-Docked (ALL SHIPS)

/* ALL SHIPS - New Events */
#define COMM_ResCol_NoMoreRUs					(106 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	136. COMMAND_ResourceCollector-NoMoreRUs
#define STAT_Strike_AlsoOutOfFuel				(107 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	138. STATUS_StrikeCraft-AlsoOutofFuel
#define STAT_Strike_GenericUsToo				(108 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	139. STATUS_StrikeCraft-GenericUsToo
#define	STAT_Strike_StuckInGrav					(109 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	140. STATUS_StrikeCraft-StuckbyGravWell
#define STAT_Strike_AttackComplete				(110 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	141. STATUS_ StrikeCraft-AttackComplete
#define	COMM_TacticsEvasive						(111 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	142. COMMAND_AnyShip-SetTacticsEvasive
#define COMM_TacticsNeutral						(112 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	143. COMMAND_AnyShip-SetTacticsNeutral
#define COMM_TacticsAggressive					(113 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	144. COMMAND_AnyShip-SetTacticsAggressive
#define COMM_Support							(114 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	145. COMMAND_SupportShip-Support
#define COMM_Grav_Off							(115 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	146. COMMAND_GravityWellGenerator-Off
#define COMM_HVette_BurstAttack					(116 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	147. COMMAND_HeavyCorvette-BurstAttack
#define COMM_MLVette_ForceDrop					(117 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	148. COMMAND_MinelayerCorvette-ForceDropMines
#define COMM_MissleDest_VolleyAttack			(118 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	149. COMMAND_MissileDestroyer-VolleyAttack
#define COMM_Strike_OutOfFuelMove				(119 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	150. COMMAND_StrikeCraft-OutOfFuelMove
#define COMM_Kamikaze_NoTargets					(120 + ACTOR_PILOT_FLAG + SPEECH_COMMAND_FLAG)	//	151. COMMAND_AnyShip-KamikazeNoTargets
#define STAT_Grp_RetreatSuccessful				(121 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	152. STATUS_GenericGroup-RetreatSuccessful
#define STAT_AssGrp_RetreatSuccessful			(122 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG + SPEECH_GROUP_FLAG)	//	153. STATUS_AssignedGroup-RetreatSuccessful
#define STAT_Grp_EnemyRetreated					(123 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	154. STATUS_GenericGroup-EnemyRetreated
#define STAT_AssGrp_EnemyRetreated				(124 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG + SPEECH_GROUP_FLAG)	//	155. STATUS_AssignedGroup-EnemyRetreated
#define STAT_Grp_AttackFromAlly					(125 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	156. STATUS_Group-UnderAttackFromAlly
#define STAT_AssGrp_FriendlyFire				(126 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG + SPEECH_GROUP_FLAG)	//	159. STATUS_AssignedGroup-FriendlyFire
#define STAT_SCVette_DroppingOff				(127 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	160. STATUS_SalvageCorvette-DroppingOff
#define STAT_Grp_AttackRetaliate				(128 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	161. STATUS_GenericGroup-UnderAttackRetaliate(Report)
#define STAT_Grp_AttackRetaliate_Repeat			(129 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	162. STATUS_GenericGroup-UnderAttackRetaliate(Repeat)
#define STAT_AssGrp_AttackRetaliate				(130 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG + SPEECH_GROUP_FLAG)	//	163. STATUS_AssignedGroup-UnderAttackRetaliate(Report)
#define STAT_AssGrp_AttackRetaliate_Repeat		(131 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG + SPEECH_GROUP_FLAG)	//	164. STATUS_AssignedGroup-UnderAttackRetaliate(Report)
#define STAT_Grp_InMineField					(132 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	166. STATUS_GenericGroup-InMineField
#define	STAT_AssGrp_InMineField					(133 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG + SPEECH_GROUP_FLAG)	//	167. STATUS_AssignedGroup-InMineField
#define STAT_Research_StationOnline				(134 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	170. STATUS_ResearchShip-StationOnline

/* ALL SHIPS - Appendix */
#define STAT_Grp_EnemyFightersDecloaking		(135 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	STATUS_GenericGroup_EnemyCloakedFightersDecloaking
#define STAT_Grp_EnemyGeneratorDecloaking		(136 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	STATUS_GenericGroup_EnemyCloakGeneratorDecloaking



/******************************************
****                                   ****
****           FLEET COMMAND           ****
****                                   ****
******************************************/

/* FLEET COMMAND - Dialogues and Warnings */
#define STAT_F_AssGrp_UnderAttack				(137 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG + SPEECH_GROUP_FLAG)	//	1. STATUS_AssignedGroup-UnderAttack (FLEET)
#define COMM_F_Group_Assigning					(138 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG)	//	2. COMMAND_HotkeyGroups-Assigning (FLEET)
#define STAT_F_AssGrp_Victory					(139 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG + SPEECH_GROUP_FLAG)	//	3. STATUS_HotkeyGroup-Victory (FLEET)
#define STAT_F_AssGrp_Defeat					(140 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG + SPEECH_GROUP_FLAG)	//	5. STATUS_HotkeyGroup-Defeat (FLEET)
#define COMM_F_Probe_Deployed					(141 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG)	//	6. COMMAND_Probe-Deployed (FLEET)
#define STAT_F_Probe_Arrived					(142 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	7. STATUS_Probe-Arrived (FLEET)
#define STAT_F_Probe_Expiring					(143 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	8. STATUS_Probe-Expiring (FLEET)
#define STAT_F_EnemyProbe_Detected				(144 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	9. STATUS_EnemyProbe-Detected (FLEET)
#define STAT_F_ProxSensor_Detection				(145 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	10. STATUS_ProximitySensor-Detection (FLEET)
#define STAT_F_ResCol_Damaged					(146 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	11. STATUS_ResourceCollector-Damaged(Singular) (FLEET)
#define STAT_F_ResCol_Dies						(147 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	12. STATUS_ResourceCollector-Dies (FLEET)
#define STAT_F_ResLevels_Nil					(148 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	14. STATUS_ResourceLevels-Nil (FLEET)
#define STAT_F_SuperCap_Damaged					(149 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	15. STATUS_SuperCapitalShip-Damaged (FLEET)
#define STAT_F_Cap_Dies							(150 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	16. STATUS_CapitalShip-Dies (FLEET)
#define STAT_F_MoShip_UnderAttack				(151 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	17. STATUS_Mothership-UnderAttack (FLEET)
#define COMM_F_MoShip_Move						(152 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG)	//	18. COMMAND_Mothership-Move (FLEET)
#define COMM_F_MoShip_Arrived					(153 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG + SPEECH_ALWAYSPLAY_FLAG)	//	19. COMMAND_Mothership-ArrivesAtDestination (FLEET)
#define COMM_F_Hyper_Engage						(154 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG)	//	20. COMMAND_Hyperspace_Engage (FLEET)
#define STAT_F_Hyper_Autodock					(155 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	21. STATUS_Hyperspace-Autodock (FLEET)
#define STAT_F_Hyper_TakingOff					(156 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	22. STATUS_Hyperspace-TakingOff (FLEET)
#define COMM_F_Hyper_Abort						(157 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG)	//	23. COMMAND_Hyperspace-Abort (FLEET)

/* FLEET COMMAND - Construction */
#define COMM_F_Const_Start						(158 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG)	//	27. COMMAND_StartConstruction (FLEET)
#define COMM_F_Const_Pause						(159 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG)	//	28. COMMAND_PauseConstruction (FLEET)
#define STAT_F_ConstInProgs_ResInsufficient		(160 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	29. STATUS_ConstructionInProgress-ResourceInsufficient (FLEET)
#define STAT_F_Const_UnitLimit					(161 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	31. STATUS_Construction-ShipUnitLimitHit (FLEET)
#define STAT_F_Const_TotalLimit					(162 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	32. STATUS_Construction-TotalUnitLimitHit (FLEET)
#define COMM_F_Const_CancelAll					(163 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG)	//	33. COMMAND_CancelAllJobs (FLEET)
#define COMM_F_Const_CancelBatch				(164 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG)	//	34. COMMAND_CancelBatch (FLEET)
#define COMM_F_Const_Resume						(165 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG)	//	36. COMMAND_ResumeConstruction (FLEET)
#define STAT_F_Const_Complete					(166 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG + SPEECH_ALWAYSPLAY_FLAG)	//	37. STATUS_ConstructionComplete (FLEET)
#define STAT_F_Const_CapShipComplete			(167 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	38. STATUS_Construction-CapitalShipComplete (FLEET)

/* FLEET COMMAND - Launching */
#define COMM_F_Launch							(168 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG)	//	39. COMMAND_Launch (FLEET)
#define COMM_F_AutolaunchOn						(169 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG)	//	40. COMMAND_Autolaunching-On (FLEET)

/* FLEET COMMAND - Research */
#define COMM_F_Research_R1_Start				(170 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG)	//	41. COMMAND_Research-Start (FLEET)
#define COMM_F_Research_R2_Start				(171 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG)	//	41. COMMAND_Research-Start (FLEET)
#define COMM_F_Research_Stop					(172 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG)	//	42. COMMAND_Research-Stop (FLEET)
#define STAT_F_Research_R1_NearCompletion		(173 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	43. STATUS_Research-NearingCompletion (FLEET)
#define STAT_F_Research_R2_NearCompletion		(174 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	43. STATUS_Research-NearingCompletion (FLEET)
#define STAT_F_Research_R1_Completed			(175 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG + SPEECH_ALWAYSPLAY_FLAG)	//	44. STATUS_Research-Completed (FLEET)
#define STAT_F_Research_R2_Completed			(176 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG + SPEECH_ALWAYSPLAY_FLAG)	//	44. STATUS_Research-Completed (FLEET)
#define STAT_F_Research_CompletedShip			(177 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG + SPEECH_ALWAYSPLAY_FLAG)	//	45. STATUS_Research-CompletedShip (FLEET)

/* FLEET COMMAND - New Events */
#define COMM_F_ScuttleReaffirm					(178 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG)	//	48. COMMAND_ScuttleReaffirm
#define COMM_F_AssGrp_AddingShips				(179 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG + SPEECH_GROUP_FLAG)	//	49. COMMAND_AssignedGroup-AddingShips
#define STAT_F_ResourcesAllHarvested			(180 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG + SPEECH_ALWAYSPLAY_FLAG)	//	50. STATUS_ResourcesAllHarvested
#define STAT_F_ReassignedToCarrier				(181 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG + SPEECH_ALWAYSPLAY_FLAG)	//	51. STATUS_FleetCommandReassignedtoCarrier
#define STAT_F_GravWellDetected					(182 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	52. STATUS_GravWellDetected
#define STAT_F_CloakedShipsDetected				(183 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	53. STATUS_CloakedShipsDetected
#define STAT_F_EnemyShipCaptured				(184 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	54. STATUS_EnemyShipCaptured
#define STAT_F_ShipStolen						(185 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	55. STATUS_ShipStolen
#define STAT_F_ResourceInjectionRecieved		(186 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG + SPEECH_ALWAYSPLAY_FLAG)	//	56. STATUS_ResourceInjectionReceived
#define STAT_F_UnderAttackFromAlly				(187 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	58. STATUS_UnderAttackFromAlly
#define STAT_F_Cap_Retired						(188 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	59. STATUS_CapitalShipRetired
#define STAT_F_Cap_Retired_PercentRecovered		(189 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	59. STATUS_CapitalShipRetired
#define COMM_F_AllianceFormed					(190 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG + SPEECH_ALWAYSPLAY_FLAG)	//	61. COMMAND_AllianceFormed
#define COMM_F_AllianceBroken					(191 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG + SPEECH_ALWAYSPLAY_FLAG)	//	62. COMMAND_AllianceBroken
#define COMM_F_ResourcesTransferred				(192 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG + SPEECH_ALWAYSPLAY_FLAG)	//	63. COMMAND_ResourcesTransferred
#define COMM_F_ResourcesReceived				(193 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG + SPEECH_ALWAYSPLAY_FLAG)	//	64. COMMAND_ResourcesReceived

/* FLEET COMMAND - Single-Player Events:*/
#define STAT_F_HyperspaceSuccessful				(194 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	70. STATUS_HyperspaceSuccessful NEW
#define STAT_F_HyperspaceInterrupted			(195 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	70.5 STATUS_HyperspaceInterrupted NEW
#define STAT_F_HyperspaceDrivesAvailable		(196 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	71. STATUS_HyperspaceDrivesAvailable NEW

/* FLEET COMMAND - Appendix */
#define COMM_F_BuildCenterSelected				(197 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG)	//	COMMAND_BuildCentreSelected
#define COMM_F_MoShip_Selected					(198 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG)	//	COMMAND_MothershipSelected
#define COMM_F_MoShip_CantMove					(199 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG)	//	COMMAND_MothershipCantMove


/******************************************
****                                   ****
****             TUTORIAL              ****
****                                   ****
******************************************/

#define TUT_Intro								(200 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Welcome to the Homeworld Training Mission.  Here we will teach you all the basic controls needed to play Homeworld. Lets begin!  Leftclick on the Next button to proceed.

/* Lesson 1: Using the Camera */
#define TUT_SaveCamera							(201 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Using the camera.  Click the Next button to begin.
#define TUT_CameraMoveDescription				(202 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	To rotate the camera, hold the right mouse button and drag the mouse.  Click Next to continue.
#define TUT_CameraMove							(203 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	To advance to the next lesson, rotate the camera 180 degrees around your Mothership.
#define TUT_CameraMoveUp						(204 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Next rotate the camera upward to view the Mothership from above.
#define TUT_CameraMoveDown						(205 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Now view the Mothership from below.
#define TUT_CameraBackToMiddle					(206 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Great!  Move the camera back to the horizontal.  Then I'll show you how to zoom in and out.
#define TUT_CameraZoomDescription				(207 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	To zoom in or out, hold both mouse buttons and drag the mouse up or down.  If your mouse has a wheel, you can use the wheel to zoom as well.  Click Next to continue.
#define TUT_CameraZoomOut						(208 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Zoom the camera out as far as you can.
#define TUT_CameraZoomIn						(209 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	From this position, we get a good overview of the action.  Now zoom the camera in as far as you can.
#define TUT_CameraPractice						(210 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Excellent.  From this position we get a good close-up view of the action.  Practice rotating and zooming the camera.  When you're ready to move on, click the Next button.

/* Lesson 2: TitleBuild */
#define TUT_TaskbarSave							(211 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Building Ships.  Click Next to begin.
#define TUT_TaskbarIntro						(212 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Most of Homeworld's user controls are accessible from a popup taskbar.  Move your cursor to the bottom of the screen to see the taskbar.
#define TUT_TaskbarDescription					(213 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	There it is.  Normally the taskbar would disappear when you move your cursor away from it, but let's keep it up to take a closer look.  Click Next to continue.
#define TUT_TaskbarBuildManagerButton			(214 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	This button brings up the Build Manager.  Leftclick on the Build button now.

/* Lesson 3: Building Ships */
#define TUT_BuildManagerIntro					(215 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Welcome to the Build Manager.  Here you can build new ships for the fleet.  Click Next to continue.
#define TUT_BuildManagerRUCount					(216 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	This is your RU count.  It displays the amount of Resource Units available for building.  Click Next to continue.
#define TUT_BuildManagerShipList				(217 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	This is the ship list.  It displays the ships available for building, their cost, and the number of ships in each build queue.  Click Next to continue.
#define TUT_BuildManagerAddResCollector			(218 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Let's build some ships.  Leftclick on the Resource Collector to select it.
#define TUT_3_4_BuildManagerIncrease			(219 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Leftclick again on the Resource Collector to add one to the build queue.
#define TUT_BuildManagerBuildRC					(220 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Leftclick on the Build button to begin construction.
#define TUT_BuildManagerIDProgressBar			(221 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Notice the progress bar.  This indicates how much of the ship has been built.  Click Next to continue.
#define TUT_BuildManagerBuildResearchShip		(222 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Now add a Research Ship to the build queue.  Leftclick twice on the Research Ship...
#define TUT_BuildManagerHitBuildRS				(223 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	...then leftclick on the Build button.
#define TUT_BuildManagerBuildScouts				(224 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Now add 10 scouts to the build queue.
#define TUT_BuildManagerHitBuildScouts			(225 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Good.  Now leftclick on the Build button to begin construction.
#define TUT_BuildManagerProgressBars			(226 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Take a close look at the progress bar.  The darker top bar shows the progress of the ship currently being built. Click Next to continue.
#define TUT_3_12_BuildManagerBottomProgressBar	(227 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	And the lighter bottom bar shows the progress of the entire batch. Click Next to continue.
#define TUT_BuildManagerClose					(228 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	To exit the Build Manager, leftclick on the Close button or press (ESCAPE).  Construction will continue.

/* Lesson 4: Selecting Ships */
#define TUT_SaveSelectingShips					(229 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Selecting and Focusing.   Click Next to begin.
#define TUT_4_1_SelectIntroZoomOut				(230 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Zoom out to bring the entire Mothership into view.
#define TUT_SelectIntro							(231 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Select the Mothership by leftclicking on it.
#define TUT_4_5_DeselectMothership				(232 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	De-select it by leftclicking on any empty space.
#define TUT_BandboxMothership					(233 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Hold down the left mouse button and drag a selection box around the Mothership.
#define TUT_SelectInfoOverlay					(234 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	At the top right of the screen is the Info Overlay.  It displays all the selected ships.  To proceed, zoom the camera all the way out.
#define TUT_SelectBandboxAll					(235 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Now drag a single selection box around as many ships as you can.
#define TUT_SelectOverlayClick					(236 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Notice how the Info Overlay lists all the selected ships.  Click on the Resource Collector in the list to select it.
#define TUT_SelectFocusResCollect				(237 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Now the Resource Collector is selected.  Click next to continue.
#define Shanes_A_Dummy							238
#define Shanes_A_Big_Dummy						239
#define TUT_4_12_FocusResCollect				(240 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Homeworld's camera always rotates around a ship or group of ships.  This is called the camera's focus.  To focus the camera on the Resource Collector, click your middle mouse button or press the (F) key.
#define TUT_SelectMoveCameraFocus				(241 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Good.  Rotate the camera.  Zoom it in and out.  Notice the camera is centered around the Resource Collector.  Click next to continue.
#define TUT_4_14_CameraFocusAgain				(242 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Click the middle mouse button again, or press the (F) key again to zoom in as close as possible.
#define TUT_SelectFocusIn						(243 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Zoom the camera out again to continue.
#define TUT_4_16_SelectScouts					(244 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	You have a line of Scouts in front of the Mothership.  Hold down the left mouse button and drag a selection box around all of them.
#define TUT_4_17_SelectFocusScouts				(245 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Click the middle mouse button or press the (F) key.
#define TUT_4_18_ScoutsCenteredFocusIn			(246 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Now the camera is focused on all your Scouts.  Click the middle mouse button or press the (F) key again.
#define TUT_4_19_ScoutsCloseup					(247 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Now we're zoomed in on the Scouts.  Move the camera around to take a look at them.
#define TUT_SelectFocusPractice					(248 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Take some time to practice selecting and focusing.  When the action heats up, it's important to be comfortable with these controls.  When you're ready to move on, click the Next button.
#define TUT_SelectResourceCollectorFirst		(249 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Select the Resource Collector, focus on it and zoom in until you're reasonably close.
#define TUT_SelectResCollHealthBar				(250 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	This is the ship's health bar.  It indicates the ship is selected.  If the ship gets damaged, the green bar will shrink and change color.  Click Next to continue.
#define TUT_SelectResCollContextMenu			(251 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	When selected, a ship's functions are accessible through its rightclick menu.  Right click on the Resource Collector to bring up this menu.
#define TUT_SelectResCollDescribeContextMenu	(252 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Begin harvesting by leftclicking on Harvest in the menu.
#define TUT_SelectResCollCollecting				(253 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	The Resource Collector will now find the nearest resources to harvest.  Note that you can still rotate and zoom the camera while it moves.  Click next to continue.
#define TUT_SelectFocusResearchShip				(254 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	To move on to the next lesson, select and focus on the Research Ship.  To help you find it, I have drawn a pointer to it.

/* Lesson 5: TitleResearch */
#define TUT_SaveResearch						(255 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Researching Technology.  Click Next to begin.
#define TUT_ResearchIntro						(256 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	This is a Research Ship.  The more you have of these, the faster you can research.  To access the Research Manager, bring up the taskbar by moving your cursor to the bottom of the screen.
#define TUT_TaskbarResearchManagerButton		(257 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	This button brings up the Research Manager.  Leftclick on it now.
#define TUT_ResearchManagerScreen				(258 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Welcome to the Research Manager.  Here you can research technology which will make new ship types available for construction.  Click Next to continue.
#define TUT_ResearchManagerTopicWindow			(259 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	This window is the technology list window.  Here you select the technologies to research.  Click Next to continue.
#define TUT_5_4_ResearchManagerGreenDot			(260 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	The green dot indicates that Capital Ship Drive technology has already been researched.
#define TUT_5_5_ResearchManagerSelectTech		(261 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Leftclick on the Capital Ship Chassis technology to select it.
#define TUT_ResearchManagerTopicDescription		(262 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Leftclick on the Research button.
#define TUT_ResearchManagerResearching			(263 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Notice that the technology has a progress bar.  This indicates how much research has been done.  Click Next to continue.
#define TUT_5_8_ResearchManagerClose			(264 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Leftclick on the Close button to exit the Research Manager.

/* Lesson 6: Moving Ships */
#define TUT_SaveMovingShips						(265 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Moving Ships.  Click Next to begin.
#define TUT_MoveSelectScouts					(266 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Now it's time to move your 10 Scouts.  Zoom out and select them all.
#define TUT_MoveFocusScouts						(267 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Focus on the scouts.
#define TUT_MoveCameraOverview					(268 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Now rotate the camera so we're looking straight down on the Scouts.
#define TUT_MoveCameraZoomOut					(269 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Lastly, zoom out as far as you can.
#define TUT_MoveContextMenu						(270 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	This gives us a nice view for moving the ships.  Rightclick on one of the Scouts to bring up its menu.
#define TUT_MoveSelectMove						(271 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Select the Move option from the menu.
#define TUT_MovePizzaDishIntro					(272 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	This is the Movement Disc.  Leftclick anywhere on the screen to move the Scouts there.
#define TUT_MoveScoutsMove						(273 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Moving ships is very important, so let's try it again.  Click Next to continue.
#define TUT_MoveHitM							(274 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	As a shortcut, bring up the Movement Disc by pressing the (M) key.
#define TUT_MoveCameraPizzaDish					(275 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	This time, rotate and zoom the camera before issuing the move command.
#define TUT_6_10_MoveHitM2						(276 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Now let's try some 3D movement.  Bring up the Movement Disc again.
#define TUT_MoveUp								(277 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Hold the (SHIFT) key and drag the mouse up or down to change the height of your destination point.  When you have found an acceptable height, release the (SHIFT) key.
#define TUT_6_14_MoveUpRelease					(278 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Now the height of the destination is fixed.  Drag the mouse to move the destination cursor at this level.
#define TUT_6_15_MovePractice					(279 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Nice work!  Take some time to practice horizontal and vertical movement.  When you feel comfortable with these controls, click Next.

/* Lesson 7: TitleSM */
#define TUT_SMSave								(280 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	The Sensors Manager.  Click Next to begin.
#define TUT_SMIntro								(281 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	The Sensors Manager is Homeworld's large-scale map.  To access it, first bring up the taskbar at the bottom of your screen.
#define TUT_SMPressSensors						(282 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Leftclick on this button to enter the Sensors Manager.
#define TUT_SMSensorsManager					(283 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Welcome to the Sensors Manager.  >From here you can get an overview of your surroundings and move your ships long distances.  Notice that you can still rotate and zoom the camera.  Click the Next button to continue.
#define TUT_SMDescription						(284 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	This is your Mothership.
#define TUT_7_4_SMDescriptionGreenDot			(285 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	The green points represent your ships.
#define TUT_7_5_SMDescriptionRedDot				(286 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	The red points represent enemy ships.
#define TUT_7_6_SMDescriptionLgBrownDot			(287 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	The large brown points represent harvestable resources.
#define TUT_7_7_SMDescriptionSmBrownDot			(288 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	The small brown points represent space dust that cannot be collected.
#define TUT_SMBandBox							(289 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Leftclick on any of your ships to exit the Sensors Manager and move the camera to that position.
#define TUT_SMExit								(290 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	In this way you can cover large distances quickly.  Use the taskbar or press (SPACE) to get back into the Sensors Manager.
#define TUT_SMSelected							(291 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Selected ships are displayed as flashing points in the Sensors Manager.  Click Next to continue.
#define TUT_7_12_SMMoveButton					(292 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	The Sensors Manager allows you to move ships the same way you do normally.
#define TUT_SMFocus								(293 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Focusing also works normally in the Sensors Manager.  Press (F) or click the middle mouse button to focus on the selected ships.
#define TUT_SMPractice							(294 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	You've learned a lot.  Practice moving your Scouts and using the Sensors Manager.  When you are ready, focus on the Scouts and click Next.

/* Lesson 8: TitleFormations */
#define TUT_SaveFormation						(295 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Formations. Click Next to begin.
#define TUT_FormationIntro						(296 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Right click on the scouts to bring up their menu.
#define TUT_FormationDelta						(297 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Move your cursor to Formation and then leftclick on Delta.
#define TUT_FormationDeltaBroad					(298 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Now the fighters are in the Delta formation.  Use the rightclick menu again to assign Broad formation.
#define TUT_FormationBroadX						(299 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Good.  Next try Formation X.
#define TUT_FormationXClaw						(300 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Formations can also be assigned using the (TAB) key.  Press (TAB) to set Claw formation.
#define TUT_FormationClawWall					(301 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Notice that (TAB) will cycle to the next formation.  Press (TAB) again to see how.
#define TUT_FormationWallSphere					(302 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	This is the Wall.  Press (TAB) for Sphere formation.
#define TUT_FormationPractice					(303 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Take some time to familiarize yourself with the formations.  When you're ready to continue, put the Scouts into Claw formation and leftclick on the Next button.
#define TUT_8_9_LookForEnemy					(304 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	To begin the combat lesson, use the Sensors Manager to move your Scouts near the enemy ships.
#define TUT_8_10_RedDots						(305 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	These are the enemies.  Move your Scouts here.
#define TUT_8_11_LeaveSensors					(306 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Once you have issued the move, you can leave the Sensors Manager.

/* Lesson 9: TitleAttacking */
#define TUT_AttackingSave						(307 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Combat.  Click Next to begin.
#define TUT_AttackingMouseOver					(308 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Rotate the camera so you can see the enemy ships.  Leftclick on one of them to attack it.
#define TUT_9_2_AttackingBandbox				(309 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Good.  To attack them all at once, hold down (CTRL) and drag a selection box around the enemy ships.
#define TUT_9_3_AttackingTimeStop				(310 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	I've paused the action to teach you how to focus the camera without selecting.

/* Lesson 10: TitleAltFocus */
#define TUT_10_ALT_Focusing						(311 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//  Lesson 10: (ALT)-Focusing.  Click Next to begin.
#define TUT_10_1_AltFocus						(312 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Hold down the (ALT) key and leftclick on any ship.  You can even use this to focus on enemy ships.
#define TUT_10_2_AltFocusBandbox				(313 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	You can also focus on multiple ships at once by holding (ALT) and dragging a selection box around them.  Try it now.
#define TUT_10_3_PracticeAndUnPause				(314 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Good.  Now practice focusing until you're ready to move on.  Press (P) to unpause the action.
#define TUT_10_4_BuildFrigate					(315 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	The Capital Ship Drive research you started earlier has been completed.  You can now build an Assault Frigate.  Go into the Build Manager.
#define TUT_10_5_BuildFrigateHintRetard			(316 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Try using the taskbar.
#define TUT_10_6_BuildFrigateStart				(317 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Build an Assault Frigate.
#define TUT_10_7_BuildFrigateUnderway			(318 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Let's return to your Scouts.  Leftclick on the Close button.

/* Lesson 11: TitleDock */
#define TUT_11_Docking							(319 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//  Lesson 11: Docking.  Click Next to begin.
#define TUT_11_0_Docking						(320 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Some of your scouts might be damaged or low on fuel.  To dock them with the Mothership, first bring up the rightclick menu.
#define TUT_11_1_SelectDock						(321 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Now select Dock.
#define TUT_11_2_ScoutsDocking					(322 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	The Scouts are on their way to the Mothership to be repaired and refueled.

/* Lesson 12: Cancelling Commands */
#define TUT_12_CancellingOrders					(323 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//  Lesson 12: Cancelling Orders.  Click Next to begin.
#define TUT_12_0_CommandCancelIntro				(324 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	You've learned how to give orders.  Now let's learn how to cancel one.
#define TUT_12_1_CommandCancelContextMenu		(325 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Rightclick on the Scouts and bring up their menu.
#define TUT_12_2_CancelCommand					(326 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Select cancel.
#define TUT_12_3_CancelCommandDescription		(327 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Notice that your Scouts have stopped.  We actually do want them to dock, so issue the dock order again.
#define TUT_12_4_TutorialPractice				(328 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	I have a feeling a battle is imminent.  When your Frigate is built, I bet you will be attacked by a small group of Corvettes and Scouts.
#define TUT_12_7_TutorialFinish					(329 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	Congratulations! You have now finished the Homeworld tutorial.


/******************************************
****                                   ****
****          Single Player            ****
****                                   ****
******************************************/

/******************************************
****            Mission 1              ****
******************************************/

/* 0.3 ANIMATIC - Opening */
#define A00_Narrator_Opening					(330 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	100 years ago, a satellite detected an object under the sands of the Great Desert.
#define A00_Narrator_Opening2					(331 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	An expedition was sent.
#define A00_Narrator_Opening3					(332 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	An ancient starship, buried in the sand.
#define A00_Narrator_Opening4					(333 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	Deep inside the ruin was a single stone that would change the course of our history forever.
#define A00_Narrator_Opening5					(334 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	On the stone was etched a galactic map ...
#define A00_Narrator_Opening6					(335 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	and a single word more ancient than the clans themselves ...
#define A00_Narrator_Opening7					(336 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	Higaara.
#define A00_Narrator_Opening8					(337 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	Our home.
#define A00_Narrator_Opening9					(338 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	The clans were united and a massive colony ship was designed.
#define A00_Narrator_Opening10					(339 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	Construction would take 60 years.
#define A00_Narrator_Opening11   				(340 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	It would demand new technologies, new industries and new sacrifices.
#define A00_Narrator_Opening12   				(341 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	The greatest of these was made by the scientist Karan Sajet who had herself permanently integrated into the colony ship as its living core.
#define A00_Narrator_Opening13   				(342 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	She is now Fleet Command.
#define A00_Narrator_Opening14   				(343 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	The promise of the Guidestone united the entire population.
#define A00_Narrator_Opening15   				(344 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	Every mind became focused on the true origin of our people...
#define A00_Narrator_Opening16   				(345 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	...every effort on the construction of the ship that would seek it out among the stars.

/* 0.5 N01 - Opening (Planet of Exile System) - EZ01 */
#define N01_All_Misc_1							(346 + ACTOR_PILOT_FLAG + SPEECH_NIS_FLAG)	//	What a beautiful sight
#define N01_All_Misc_2							(347 + ACTOR_PILOT_FLAG + SPEECH_NIS_FLAG)	//	You are cleared to approach
#define N01_All_Misc_3							(348 + ACTOR_PILOT_FLAG + SPEECH_NIS_FLAG)	//	Bays five, six and seven sealed.
#define N01_All_Misc_4							(349 + ACTOR_PILOT_FLAG + SPEECH_NIS_FLAG)	//	Release crews standing by.
#define N01_All_Misc_6							(350 + ACTOR_PILOT_FLAG + SPEECH_NIS_FLAG)	//	Bays eight, nine and ten sealed.
#define N01_All_Misc_7							(351 + ACTOR_PILOT_FLAG + SPEECH_NIS_FLAG)	//	Scaffold decks A, B, C secure.
#define N01_All_Misc_8							(352 + ACTOR_PILOT_FLAG + SPEECH_NIS_FLAG)	//	You got it
#define N01_All_Misc_9							(353 + ACTOR_PILOT_FLAG + SPEECH_NIS_FLAG)	//	Roger that Control.
#define N01_All_Misc_10							(354 + ACTOR_PILOT_FLAG + SPEECH_NIS_FLAG)	//	…decks D and E secure. Scaffold secure.
#define N01_All_Misc_11							(355 + ACTOR_PILOT_FLAG + SPEECH_NIS_FLAG)	//  Scaffold control standing by.
#define N01_All_Misc_12							(356 + ACTOR_PILOT_FLAG + SPEECH_NIS_FLAG)	//	All systems green.
#define N01_All_Misc_13							(357 + ACTOR_PILOT_FLAG + SPEECH_NIS_FLAG)	//	Forty-two seven-oh-one please confirm.
#define N01_All_Misc_14							(358 + ACTOR_PILOT_FLAG + SPEECH_NIS_FLAG)	//  We're ready.
#define N01_All_Misc_15							(359 + ACTOR_PILOT_FLAG + SPEECH_NIS_FLAG)	//	How do we look? All right, take it to one-fifteen. Locked in.
#define N01_Fleet_Intro							(360 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_NIS_FLAG)	//	This is Fleet Command.  Reporting Mothership pre-launch status.
#define N01_Fleet_Command						(361 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_NIS_FLAG)	//	Command on-line...
#define N01_Fleet_Resourcing					(362 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_NIS_FLAG)	//	Resourcing online...
#define N01_Fleet_Construction					(363 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_NIS_FLAG)	//	Construction online...
#define N01_Fleet_Cryo_AtoJ						(364 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_NIS_FLAG)	//	Cryogenic subsections A through J online...
#define N01_Fleet_Ctyo_KtoS						(365 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_NIS_FLAG)	//	K through S online...
#define N01_Fleet_Hull							(366 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_NIS_FLAG)	//	Hull pressure stable.
#define N01_Fleet_ScaffoldAlign					(367 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_NIS_FLAG)	//	Scaffold Control stand by for alignment...
#define N01_Fleet_AlignConfirmed				(368 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_NIS_FLAG)	//	Alingment confirmed.  Stand by Release Sync Control.
#define N01_All_CaliperReleased					(369 + ACTOR_PILOT_FLAG + SPEECH_NIS_FLAG)	//	All caliper banks released...
#define N01_All_DownTheMiddle					(370 + ACTOR_PILOT_FLAG + SPEECH_NIS_FLAG)	//	...she's right down the middle.
#define N01_All_FleetClear						(371 + ACTOR_PILOT_FLAG + SPEECH_NIS_FLAG)	//	Initial Fleet is clear.
#define N01_Fleet_MoshipClear					(372 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_NIS_FLAG)	//	The Mothership has cleared the Scaffold.
#define N01_Fleet_MoshipAway					(373 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_NIS_FLAG)	//	We are away.
#define N01_All_CommandLineGreen				(374 + ACTOR_PILOT_FLAG + SPEECH_NIS_FLAG)	//	Command Line green.  Initial Fleet in position.
#define N01_Chatter_Mid							(375 + ACTOR_PILOT_FLAG + SPEECH_NIS_FLAG)	//	for NIS's, add to the atmosphere, relatively calm
							

/* 1 M01 - TRIALS (KHARAK SYSTEM) - EZ01 */
#define M01_Intel_GoingOnline					(376 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Fleet Intelligence going on-line.  Our task is to analyze all sensor data and generate mission objectives.  Before the hyperdrive test, several trials must be completed.
#define M01_Intel_ResearchObjective				(377 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Test construction by building the primary research ship.
#define M01_Intel_HavestingObjective			(378 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Test resource processing by harvesting the asteroids provided nearby.
#define M01_Intel_CombatTrials					(379 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Standby to begin combat trials.  First we will be monitoring formation performance.
#define M01_Intel_CombatTrialsObjective			(380 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  Target Drones have been provided here.  Assign a formation to your Fighters and destroy the Drones.
#define M01_Intel_CombatUseFormation			(381 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Target Drones destroyed.  Replacement Drones are being sent to the same location.  Completion of this trial requires the use of a formation.  Begin again.
#define M01_Intel_FormationAnalysis				(382 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Formation trial complete.  Flight analysis shows a twenty-two percent increase in combat performance.
#define M01_Intel_TacticsTrial					(383 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  The next trial will test the effectiveness of tactics.  Standby to begin tactics trial.  Use Aggressive or Evasive tactics and engage the Target Drones here.
#define M01_Intel_CombatUseTactics				(384 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Target Drones destroyed.  Replacement Drones are being sent to the same location.  Completion of this trial requires the use of tactics.  Begin again.
#define M01_Intel_TacticsComplete				(385 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Tactics trial complete.
#define M01_Intel_SalvageTrial					(386 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  The next trial will test the performance of the Salvage Corvette.  Build one and capture the Target Drone here.
#define M01_Intel_SalvageTargetDestroyed		(387 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Target Drone destroyed.  A replacement Drone is being sent to the same location.  Completion of this trial requires a successful capture of the drone.  Begin again.
#define M01_Intel_BuildInterceptors				(388 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Interceptors are ready for construction.
#define M01_Intel_AdvancedCombatTrials			(389 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  Advanced Target Drones with heavier armor have been provided.  Using Interceptors, begin the mock combat exercise here.
#define M01_Intel_CombatUseInterceptors			(390 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Target Drones destroyed.  Replacement Drones are being sent to the same location.  Completion of this trial requires the use of Interceptors.  Begin again.
#define M01_Fleet_HyperspaceCharging			(391 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Hyperspace module charging.  35% capacity and rising.  The Mothership will be ready for the Hyperdrive test in 10 minutes.
#define M01_Intel_TrialsComplete				(392 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Standby for Hyperdrive test.  Internal pressure doors will be sealed in 2 minutes.  Abort systems standing by.
#define M01_Fleet_HyperspaceReady				(393 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Hyperspace module fully charged.  I am ready to initiate quantum wave generation on your mark.  Good luck everyone.
#define M01_Intel_HyperspaceReady				(394 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	All sections have reported in.  Trigger the hyperspace drive at your discretion.

/* 1.19 ANIMATIC - M01à M02 */
#define A01_Fleet_HyperspaceFullPower			(395 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_ANIMATIC_FLAG)	//	All Hyperspace systems operating at full power.
#define A01_Intel_TargetingSystem				(396 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	If the hyperspace targeting system is accurate, we should emerge in close proximity to the support vessel Khar-Selim.
#define A01_Intel_2								(397 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	This ship has spent the past 10 years travelling on conventional drives to reach the edge of the Kharak system.
#define A01_Intel_3								(398 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	The Khar-Selim will monitor the quantum waveform as we return to normal space and assist in tuning our drive control systems.
#define A01_Intel_4								(399 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	If the hyperspace module malfunctions, the Khar-Selim will offer assistance and re-supply.
#define A01_Intel_5								(400 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	Mission objectives will be to dock with the service vessel and link up research teams in order to complete adjustments to the Mothership and her drives.


/******************************************
****            Mission 2              ****
******************************************/

/* 2 M02 - BATTLE W/ TURANIC RAIDERS (GREAT WASTELANDS) - EZ02 */
#define M02_Fleet_MadeIt						(401 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We made it.  Hyperspace jump complete!  All systems nominal and the quantum wave effect has dissipated.
#define M02_Intel_MissingScienceVessel			(402 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We have miss-jumped.  The support ship is not here.  Fleet Command will signal the Khar-Selim while we confirm our true position.
#define M02_Fleet_CallingScienceVessel			(403 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	This is the Mothership calling Support Vessel Khar-Selim.  Come in please.  We have miss-jumped and are requesting your beacon. . .  This is the Mothership calling Support Vessel Khar-Selim.  Please Respond. . .
#define M02_Intel_PickedUpBeacon				(404 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Priority Alert.  We have picked up an automated beacon from the Khar-Selim.
#define M02_Intel_SendProbe						(405 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Send a probe to make contact and re-establish communications.
#define M02_Intel_ScienceVesselDestroyed		(406 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The Khar-Selim has been destroyed.  Heavy weapon damage is visible on the remaining fragment.  A Salvage Corvette must be sent to retrieve the mission data recorder.
#define M02_Fleet_MothershipUnderAttack			(407 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	They're attacking?!. . . I'm under attack!  The Mothership is under attack!!
#define M02_Intel_DefendMothership				(408 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Engage incoming units.  The Mothership must be defended.
#define M02_Intel_EnemyDestroyed				(409 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Hostile units destroyed.  Threat eliminated.
#define M02_Fleet_MinorDamage					(410 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Mothership sustained minor hull damage.  Repairs are underway.
#define M02_Intel_MoreEnemies					(411 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Priority Alert!  Additional hostile units detected on an intercept course with the Khar-Selim.
#define M02_Intel_ProtectSalvageTeam			(412 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  Protection of the Salvage team is a primary Objective.  It must return to Mothership with the mission recorder.  We need that data.
#define M02_Intel_SalCorDestroyed				(413 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Salvage Corvette has been destroyed.  Use an escort force to guard next team.  We need those records.
#define M02_Intel_SalvageDocked					(414 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Salvage Team docked safely.  Downloading Mission Data Recording.  Replaying last entry:
#define M02_All_ScienceVesselAttacked			(415 + ACTOR_KHARSELIM_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	What do you mean you detect a Hyperspace entry?!  The mothership isn't due for-...
#define M02_10_Intel_DefendMoShipAgain			(416 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We are detecting enemy units advancing on our position.  Organize a defense force to protect the Mothership.
#define M02_11_Intel_InvestigatePowerSource		(417 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Large power signature detected nearby. Recommend immediate investigation.
#define M02_12_Intel_EnemyCarrier				(418 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	It's an enemy Carrier.  They appear to be reinforcing their squadrons of Fighters and Corvettes with it.
#define M02_Intel_EnemyInferior					(419 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  We have determined that these enemy units are inferior to ours.
#define M02_13_Intel_DestroyAllP1				(420 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Commander, this race is clearly hostile. We can't allow them to secure a position in our system. I recommend that we destroy the force completely.
#define M02_Intel_ObjectivesComplete			(421 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Enemy units are retreating.  Objectives complete.  Standby for immediate return to Kharak.

/* 2.15 ANIMATIC - M02à M03 */
#define A02_Intel_AnalysisOfWreckage			(422 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	Analysis of wreckage reveals the hostile units are using strike craft ranging from Fighters to combat Corvettes.
#define A02_Intel_2								(423 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	All pilots will be briefed in case hostiles have penetrated further into the Kharak system.
#define A02_Intel_3								(424 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	On our return to Kharak, the final outfit of the Mothership must be accelerated in order to defend against possible future attacks.
#define A02_Intel_4								(425 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	Many major Mothership systems are still incomplete.
#define A02_Intel_5								(426 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	We will notify the Kharak Missile Defense System of this possible threat.
#define A02_Intel_6								(427 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	The Mothership will then dock with the Scaffold for repairs.
#define A02_Intel_7								(428 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	Standby for Hyperspace exit to Kharak.


/******************************************
****            Mission 3              ****
******************************************/

/* 3 BACK TO KHARAK (KHARAK SYSTEM) - EZ01 */
#define M03_Fleet_ItsGone						(429 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	No one's left. . .Everything's gone!. . .Kharak is burning!
#define M03_Intel_Analyzing						(430 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Kharak is being consumed by a firestorm.  The Scaffold has been destroyed.  All orbital facilities destroyed.  Significant debris ring in low Kharak orbit.  Receiving no communications from anywhere in the system... Not even beacons.
#define M03_Fleet_CryotraySignal				(431 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Wait!  On the maintenance frequency.  I'm getting a signal from the Cryo Tray systems in orbit.  One of them is suffering a massive malfunction.
#define M03_Intel_DispatchSalCorvettes			(432 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Dispatch Salvage Corvettes immediately to collect the trays.
#define M03_Intel_CryotrayUnderAttack			(433 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The Cryo Trays are under attack.  These ships are different from those we encountered at the Khar-Selim.  It is likely they were involved in the destruction of Kharak.
#define M03_Intel_CaptureEnemy					(434 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Capture at least one vessel for interrogation and destroy the rest.
#define M03_Fleet_SaveCryotray					(435 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Those trays contain all that remain of our people.  Without them, we will become extinct.
#define M03_Intel_EnemyCaptured					(436 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Hostile vessel captured.  Crew interned.  Interrogation is underway.  While searching the enemy ship's computer systems, we came across these flight recordings.  Standby for playback.

/* 3.5 N04 - Learn Of Sacking (Kharak System) EZ01 */
#define N04_All_Battle1							(437 + ACTOR_ALLSHIPSENEMY_FLAG + SPEECH_NIS_FLAG)	//	Group three moving in... allright now, stay together... ready... SPREAD!
#define N04_All_Battle2							(438 + ACTOR_ALLSHIPSENEMY_FLAG + SPEECH_NIS_FLAG)	//	Surface attack group to first positions...
#define N04_All_Battle3							(439 + ACTOR_ALLSHIPSENEMY_FLAG + SPEECH_NIS_FLAG)	//	Incoming missiles...
#define N04_All_Battle4							(440 + ACTOR_ALLSHIPSENEMY_FLAG + SPEECH_NIS_FLAG)	//	Evasive maneuvers!
#define N04_All_Battle5							(441 + ACTOR_ALLSHIPSENEMY_FLAG + SPEECH_NIS_FLAG)	//	We're hit! Damage report!
#define N04_All_Battle6							(442 + ACTOR_ALLSHIPSENEMY_FLAG + SPEECH_NIS_FLAG)	//	What the...?
#define N04_All_Battle7							(443 + ACTOR_ALLSHIPSENEMY_FLAG + SPEECH_NIS_FLAG)	//	There's another one...
#define N04_All_Battle8							(444 + ACTOR_ALLSHIPSENEMY_FLAG + SPEECH_NIS_FLAG)	//	Prepare for immediate surface bombardment!
#define N04_All_Battle9							(445 + ACTOR_ALLSHIPSENEMY_FLAG + SPEECH_NIS_FLAG)	//	Standing by.
#define N04_All_Battle10						(446 + ACTOR_ALLSHIPSENEMY_FLAG + SPEECH_NIS_FLAG)	//	Commence surface delivery.
#define N04_All_Battle11						(447 + ACTOR_ALLSHIPSENEMY_FLAG + SPEECH_NIS_FLAG)	//	Delivery confirmed.
#define N04_All_Battle12						(448 + ACTOR_ALLSHIPSENEMY_FLAG + SPEECH_NIS_FLAG)	//	All targets acquired.
#define N04_All_Battle13						(449 + ACTOR_ALLSHIPSENEMY_FLAG + SPEECH_NIS_FLAG)	//	Multiple impacts confirmed.
#define N04_All_Battle14						(450 + ACTOR_ALLSHIPSENEMY_FLAG + SPEECH_NIS_FLAG)	//	Surface temperature fifteen-twenty-three and climbing...
#define N04_All_Battle15						(451 + ACTOR_ALLSHIPSENEMY_FLAG + SPEECH_NIS_FLAG)	//	Estimated immediate casualties: 98 percent.
#define N04_All_Battle16						(452 + ACTOR_ALLSHIPSENEMY_FLAG + SPEECH_NIS_FLAG)	//	Surface temperature seventeen-forty-two and stable...
#define N04_All_Battle17						(453 + ACTOR_ALLSHIPSENEMY_FLAG + SPEECH_NIS_FLAG)	//	Orbital target acquired.
#define N04_All_Battle18						(454 + ACTOR_ALLSHIPSENEMY_FLAG + SPEECH_NIS_FLAG)	//	Watch the spread pattern... stay sharp.
#define N04_All_Battle19						(455 + ACTOR_ALLSHIPSENEMY_FLAG + SPEECH_NIS_FLAG)	//	Target destroyed.
#define N04_All_Battle20						(456 + ACTOR_ALLSHIPSENEMY_FLAG + SPEECH_NIS_FLAG)	//	Requesting clearance to dock.
#define N04_All_Battle21						(457 + ACTOR_ALLSHIPSENEMY_FLAG + SPEECH_NIS_FLAG)	//	Clearance granted.
#define N04_All_Battle22						(458 + ACTOR_ALLSHIPSENEMY_FLAG + SPEECH_NIS_FLAG)	//	Primary directive achieved.
#define N04_All_Battle23						(459 + ACTOR_ALLSHIPSENEMY_FLAG + SPEECH_NIS_FLAG)	//	Prepare for secondary directive.
#define N04_All_Battle24						(460 + ACTOR_ALLSHIPSENEMY_FLAG + SPEECH_NIS_FLAG)	//	Fleet trajectory calculated and locked in.
#define N04_All_Battle25						(461 + ACTOR_ALLSHIPSENEMY_FLAG + SPEECH_NIS_FLAG)	//	Prepare for hyperspace.

/* 3 BACK TO KHARAK (KHARAK SYSTEM) - EZ01 */
#define M03_Intel_RecordingAnalysis				(462 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	...Analysis of the recording indicates that the Kharak missile defenses heavily damaged the attacking fleet.  However, we have concluded that at present they can still easily defeat us.  We have therefore plotted a course to a deep space asteroid belt.  There we can hide and prepare our fleet for an assault.
#define M03_Intel_ResearchAnalyzedFrigate		(463 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Our research division has analyzed the captured frigate.  We have reverse engineered the drive technology and developed two new ships.  Plans for a third vessel are underway, but will require Frigate Chassis research.
#define M03_Fleet_CryotraysLoaded100			(464 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Cryo tray loaded.  One hundred thousand people secured.
#define M03_Fleet_CryotraysLoaded200			(465 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Cryo tray loaded.  Two hundred thousand people secured.
#define M03_Fleet_CryotraysLoaded300			(466 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Cryo tray loaded.  Three hundred thousand people secured.
#define M03_Fleet_CryotraysLoaded400			(467 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Cryo tray loaded.  Four hundred thousand people secured.
#define M03_Fleet_CryotraysLoaded500			(468 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Cryo tray loaded.  Five hundred thousand people secured.
#define M03_Intel_DeploySalvageTeam				(469 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  Withdraw attack and deploy a Salvage Team.  We need that ship.
#define M03_Intel_HyperspaceReady				(470 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  Cryo trays loaded and secure.  Hyperspace module charged.  There's nothing left for us here.  Let's go.

/* 3.17 ANIMATIC - M03à M04 */
#define A03_Fleet_CryotraysProcessed			(471 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_ANIMATIC_FLAG)	//	Cryogenic trays have been processed and our colonists are safe for now.
#define A03_Fleet_2								(472 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_ANIMATIC_FLAG)	//	We are all that's left of our world, our culture, our people.
#define A03_Fleet_3								(473 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_ANIMATIC_FLAG)	//	No one survived. . .
#define A03_Intel_InterrogationInfo				(474 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	From the interrogation we learned that a frontier fleet patrolling the borders of a vast interstellar empire was dispatched to destroy our planet.
#define A03_Intel_2								(475 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	The captain claimed our people violated a 4000 year old treaty forbidding us to develop hyperspace technology.
#define A03_Intel_3								(476 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	Extermination of our planet was the consequence.
#define A03_Intel_4								(477 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	The subject did not survive interrogation.


/******************************************
****            Mission 4              ****
******************************************/

/* 4 M04 - DEFEAT P1 (GREAT WASTELANDS) EZ04 */
#define M04_Fleet_HyperspaceSuccesful			(478 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  Hyperspace jump successful.
#define M04_Intel_ScannerData					(479 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  Scanner data indicates asteroid density is highest in this region.  Commence resource gathering.
#define M04_Intel_ResearchController			(480 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  Resource Controllers are available for construction.  To improve the efficiency of collection, build one and position it near the asteroid vein.
#define M04_Intel_LongRangeSensors				(481 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  Long range sensors indicate a Mothership-class mass signature.  It's coming in fast.  Power readings are off the scale.  Full combat alert.  Standby for contact.

/* 4.5 N03 - Traders Intro (Great Wastelands) - EZ03 */
#define N03_Intel_AnomalyDetected				(482 + ACTOR_FLEETINTEL_FLAG + SPEECH_NIS_FLAG)	//	Prepare the ambassador.
#define N03_Fleet_AmbassadorAway				(483 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_NIS_FLAG)	//	Ambassador away.
#define N03_All_LockedIn						(484 + ACTOR_AMBASSADOR_FLAG + SPEECH_NIS_FLAG)	//	Trajectory locked in, hailing signal open on all channels.
#define N03_All_MagneticField					(485 + ACTOR_AMBASSADOR_FLAG + SPEECH_NIS_FLAG)	//	Entering magnetic field now... almost there...
#define N03_All_LostGuidance					(486 + ACTOR_AMBASSADOR_FLAG + SPEECH_NIS_FLAG)	//	Fleet, we've lost Guidance and are being drawn in...
#define N03_All_LotsOfLights					(487 + ACTOR_AMBASSADOR_FLAG + SPEECH_NIS_FLAG)	//	There's a lot of lights... uh... there seems to be... some kind of activity inside, I can see...
#define N03_Traders_Intro						(488 + ACTOR_TRADERS_FLAG + SPEECH_NIS_FLAG)	//	The whole big speel here
#define N03_All_AmbassadorReturning				(489 + ACTOR_AMBASSADOR_FLAG + SPEECH_NIS_FLAG)	//	Fleet, this is ambassador.  We are clear of the Bentusi vessel, all systems green.  Harbor control has released Guidance and the exchange unit is secure.  Receiving crews, prep the quarantine chamber.
#define N03_All_QuarantineReady					(490 + ACTOR_PILOT_FLAG + SPEECH_NIS_FLAG)	//	Roger that.  Quarantine standing by.
#define N03_Fleet_TradingEstablished			(491 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_NIS_FLAG)	//	Bentusi Trading link established.

/* 4 M04 - DEFEAT P1 (GREAT WASTELANDS) EZ04 */
#define M04_Traders_FarewellKushan				(492 + ACTOR_TRADERS_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Turanic Raiders, servants of the Taiidan, are arriving.  They must not learn of our contact.  We must depart.  All that moves is easily heard in the void.  We will listen for you.  Farewell.
#define M04_Traders_FarewellTaiidan				(493 + ACTOR_TRADERS_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Turanic Raiders, servants of the Kushan, are arriving.  They must not learn of our contact.  We must depart.  All that moves is easily heard in the void.  We will listen for you.  Farewell.
#define M04_Traders_StopAttackNow				(494 + ACTOR_TRADERS_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The Bentusi wish only to trade and make contact.  Your attack is unwarranted and ill-advised.  Stop now.
#define M04_Intel_CeaseFire						(495 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Recommend immediate cease-fire.
#define M04_Traders_KickAss						(496 + ACTOR_TRADERS_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	You insist on conflict.  This is most unfortunate.
#define M04_Intel_DefendTheFleet				(497 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  Turanic Raider fighters are attacking our resource operation.  Defend it and prepare for more hostile ships to arrive.
#define M04_Intel_DefendTheMothership			(498 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  Several Turanic Raider capital ships have just emerged from hyperspace near the Mothership.
#define M04_Intel_DiscoveredCarrier				(499 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The Turanic Raider Carrier has been located.  It is reinforcing their squadrons of Fighters and Corvettes.
#define M04_Intel_CarrierRetreatingKushan		(500 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The Turanic Raider Carrier is retreating.  If it escapes they will warn the Taiidan fleet of our pursuit.  Do not allow them to hyperspace.
#define M04_Intel_CarrierRetreatingTaiidan		(501 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The Turanic Raider Carrier is retreating.  If it escapes they will warn the Kushan fleet of our pursuit.  Do not allow them to hyperspace.
#define M04_Intel_HyperspaceKushan				(502 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  The Turanic Raiders have been defeated.  We can now return to our immediate goal: preparation for an attack on the Taiidan fleet that devastated Kharak.
													//	Using data from the Taiidan vessel captured at Kharak, we have been able to determine their location.  If we strike now we can take advantage of their damaged condition.
													//	Hyperspace co-ordinates have been transferred to Fleet Command.
#define M04_Intel_HyperspaceTaiidan				(503 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  The Turanic Raiders have been defeated.  We can now return to our immediate goal: preparation for an attack on the Kushan fleet that devastated Kharak.
													//	Using data from the Kushan vessel captured at Kharak, we have been able to determine their location.  If we strike now we can take advantage of their damaged condition.
													//	Hyperspace co-ordinates have been transferred to Fleet Command.

/* 4.12 ANIMATIC - M04à M05 */
#define A04_Intel_SystemsOptimal				(504 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	We have repaired the damage incurred by the Turanic Raiders.
#define A04_Intel_SystemsOptimal2				(505 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	Combat and sensor systems returning to optimal functionality.
#define A04_Fleet_Whining						(506 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_ANIMATIC_FLAG)	//	I can't believe this happened.
#define A04_Fleet_Whining2						(507 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_ANIMATIC_FLAG)	//	Our first hyperdrive test led to near genocide.
#define A04_Fleet_Whining3						(508 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_ANIMATIC_FLAG)	//	Kharak... destroyed.
#define A04_Fleet_Whining4						(509 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_ANIMATIC_FLAG)	//	All of us, all that's left, hunted by two alien races...
#define A04_Intel_ApproachingCoordinates		(510 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	We must focus on the matter at hand: elimination of the fleet that destroyed our world.
#define A04_Intel_ApproachingCoordinates2		(511 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	Coming up on hyperspace co-ordinates.


/******************************************
****            Mission 5              ****
******************************************/

/* 5 M05 - BATTLE AGAINST SACKING FORCE (GARDEN VIEW) EZ05 */
#define M05_Fleet_HyperspaceSuccessful			(512 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Hyperspace successful.  Current position and pre-jump coordinates are in perfect alignment.  We are on target.
#define M05_Intel_SendProbesKushan				(513 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We detect large resource deposits but no vessels.  It is possible that the Taiidan could be hiding in the denser portions of the belt, which may cause interference with our sensors.
#define DummyIGNORE								514	//	Send probes to investigate.  All fleet assets should be kept on alert.
#define M05_Intel_SendProbesTaiidan				(515 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We detect large resource deposits but no vessels.  It is possible that the Kushan could be hiding in the denser portions of the belt, which may cause interference with our sensors.
#define DummyIGNORETOO							516	//	Send probes to investigate.  All fleet assets should be kept on alert.
#define M05_Intel_ProfilesMatch					(517 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Ship profiles and markings match those from the recording at Kharak.  There is no doubt that this is the fleet.  Destroy them.
#define M05_Intel_ResearchIonCannon				(518 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  Research Division reports it is now equipped for Ion Cannon technology.  We advise commencing research immediately.
#define M05_Intel_ResearchPlasmaBomb			(519 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Research Division reports it is now equipped for Plasma Bomb technology.  We advise commencing research immediately.
#define M05_Intel_DestroyEnemyCollector			(520 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  Sensors indicate the enemy is harvesting resources.  Destroy their collector to impair ship production.
#define M05_Intel_MoveAttack					(521 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  Enemy capital ships appear to be most heavily armored on the front and sides.  Our capital ships should be issued move orders while attacking to take advantage of the more vulnerable sides.
#define M05_Intel_ResearchDefenseFighter		(522 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  The enemy is using a new Fighter class ship with strong defensive capabilities but low maneuverability.  Our Research Division reports that it can produce a similar vessel.  Begin Research as soon as possible.
#define M05_Intel_HyperspaceKushan				(523 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Taiidan fleet destroyed.  There's nothing left for us to return to.  Our only option is to follow the path etched into the Guidestone.  Finding our ancient home is our only hope now.Hyperspace coordinates locked in.	
#define M05_Intel_HyperspaceTaiidan				(524 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Kushan fleet destroyed.  There's nothing left for us to return to.  Our only option is to follow the path etched into the Guidestone.  Finding our ancient home is our only hope now.Hyperspace coordinates locked in.


/* 5.9 ANIMATIC - M05à M06 */
#define A05_Intel_1								(525 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	We've completed de-crypting data from the enemy frigate we captured in the Kharak system.
#define A05_Intel_2								(526 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	It appears to be an imperial broadcast.
#define A05_Intel_3								(527 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	In order to stay clear of these outposts, our course will take us into a turbulent asteroid field and through the heart of a nebula.


/******************************************
****            Mission 6              ****
******************************************/

/* 6 M06 - ASTEROIDS 3D (DIAMOND SHOALS) EZ06 */
#define M06_Fleet_HyperspaceSuccessful			(528 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Hyperspace jump successful.
#define M06_Intel_AsteroidCollision				(529 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We haven't cleared the asteroid field.  Prepare for collisions.
#define M06_Intel_DestroyAsteroids				(530 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Incoming asteroids must be destroyed before they impact with the Mothership.  Concentrate fire within this collision envelope.
#define M06_Intel_ResearchSuperCap				(531 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Research Division reports it is now equipped for Super Capital Ship Drive technology.  We advise commencing research immediately.
#define M06_Intel_StrikeCraftIneffective		(532 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Strike Craft are proving ineffective.  We recommend using primarily capital ships.
#define M06_Intel_ClearedField					(533 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We've cleared the field.
#define M06_Traders_ComeToTrade					(534 + ACTOR_TRADERS_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Greetings.  We have come to trade.
#define M06_Fleet_AskTradersForInfo				(535 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	This is a dangerous and unpredictable region.  Can you give us information that will guide us through the nebula ahead?
#define M06_Traders_FearNebulaKushan			(536 + ACTOR_TRADERS_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We hear nothing there.  Even the Taiidan fear the Great Nebula.  No one returns.
#define M06_Traders_FearNebulaTaiidan			(537 + ACTOR_TRADERS_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We hear nothing there.  Even the Kushan fear the Great Nebula.  No one returns.
#define M06_Intel_HyperspaceReady				(538 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Hyperspace Module fully charged.

/* 6.11 ANIMATIC - M06à M07 */
#define A06_Fleet								(539 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_ANIMATIC_FLAG)	//	The Bentusi said "No one returns."


/******************************************
****            Mission 7              ****
******************************************/

/* 7 M07 - SWARMER BATTLE (GARDENS) EZ07 */
#define M07_Intel_HarvestNebula					(540 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The Nebula is incredibly rich in energy and resources.  Energy levels are so high that our sensors are having trouble compensating.  Begin harvesting the nebula while we address this problem.
#define M07_Intel_AlertStatus					(541 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	There is a contact closing with the Mothership.  Sensors instability in this region makes it difficult to identify.
#define M07_Intel_PrepareAmbassador				(542 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Prepare the Ambassador.

/* 7.3 N05 - P2 Intro (Gardens) EZ07 */
#define N05_Fleet_AmbassadorAway				(543 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_NIS_FLAG)	//	Ambassador away.
#define N05_P2_Intro1							(544 + ACTOR_PIRATES2_FLAG + SPEECH_NIS_FLAG)	//	This is the Garden of Kadesh.  For thirteen generations we have protected it from the unclean.
#define N05_P2_Intro2							(545 + ACTOR_PIRATES2_FLAG + SPEECH_NIS_FLAG)	//	The Turanic Raiders who came before you failed to join and were punished for this trespass. Like theirs, your ship has already defiled this holy place.
#define N05_P2_Intro3							(546 + ACTOR_PIRATES2_FLAG + SPEECH_NIS_FLAG)	//	If you have come to join we welcome you and will spare your ship until all have disembarked.  If you have come to consume the garden you will be removed at once.
#define N05_P2_Intro4							(547 + ACTOR_PIRATES2_FLAG + SPEECH_NIS_FLAG)	//	What are your intentions?
#define N05_All_UnawareOfSignificance			(548 + ACTOR_AMBASSADOR_FLAG + SPEECH_NIS_FLAG)	//	We were unaware of the significance of this location.  We mean you no conflict.  Please allow us time to prepare our engines so that we may withdraw as requested.
#define N05_P2_NotARequest						(549 + ACTOR_PIRATES2_FLAG + SPEECH_NIS_FLAG)	//	If you will not join, then die.  There is no withdrawal from the Garden.

/* 7 M07 - SWARMER BATTLE (GARDENS) EZ07 */
#define M07_Intel_DefendMothership				(550 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Delay the attacking ships while Fleet Command charges the hyperdrive module.  We should have the range to jump clear of the nebula.
#define M07_Fleet_HyperspaceCharging1			(551 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Hyperspace module at 35%, ready in 8 minutes.
#define M07_Fleet_HyperspaceCharging2			(552 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Hyperspace module at 90%, ready in 1 minute.
#define M07_Fleet_HyperspaceReady				(553 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Hyperspace module fully charged.  Get us out of this place.
#define M07_Intel_EngageHyperdrive				(554 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Co-ordinates set.  Engage hyperdrive!
#define M07_Fleet_HyperdriveFailed				(555 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Hyperdrive jump failed!  The Quantum waveform collapsed before the ship could enter hyperspace.
#define M07_Intel_AnalyzingMalfunction			(556 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Analyzing malfunction.  Continue to protect the Mothership until the problem is solved.
#define M07_Intel_DefendCollectors				(557 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Enemy forces are concentrating on our Resource Collectors.  Allocate combat vessels to protect them.
#define M07_Fleet_HypdriveOnline				(558 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The Hyperdrive is back on-line.
#define M07_Intel_Hyperspace					(559 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Let's put these fanatics behind us.

/* 7.12 ANIMATIC - M07à M08 */
#define A07_Intel_1								(560 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	The enemy Mothership appeared to be equipped with a powerful field generator.
#define A07_Intel_2								(561 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	This field deformed our quantum wavefront and prevented us from making a hyperspace jump.
#define A07_Intel_3								(562 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	We also observed that the enemy's hyperspace module has an identical power signature to our own.
#define A07_Intel_4								(563 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	This raises interesting questions considering our own technology was reverse-engineered from the wreck of the Khar-Toba.
#define A07_Intel_5								(564 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	Our hyperspace systems are now functioning properly and this jump will carry us clear of the Nebula.


/******************************************
****            Mission 8              ****
******************************************/

/*8 M08 - FALKIRK (OUTER GARDENS) EZ08 */
#define M08_Fleet_HyperspaceInterrupted			(565 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Something's wrong.  We've been pulled out of hyperspace.  We're still inside the nebula.
#define M08_Intel_ItsATrap						(566 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	It's a trap!
#define M08_Intel_DestroyInhibitors				(567 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Sensors detect hyperspace inhibitors in a triangular formation.  Even one can keep us from entering hyperspace.  All of them must be destroyed.
#define M08_Intel_DestroyHostiles				(568 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The nebula is still scrambling our sensors but it looks like we have incoming hostiles.
#define M08_Intel_ResearchDrones				(569 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  Research Division reports it is now equipped for Drone technology.  We advise commencing research immediately.
#define M08_Intel_ResearchDefenseField			(570 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Research Division reports it is now equipped for Defense Field technology.  We advise commencing research immediately.
#define M08_P2_JoinUs							(571 + ACTOR_PIRATES2_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Again we offer you the chance to join us and live here in peace.
#define M08_Fleet_SomethingInCommon				(572 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Our future lies elsewhere, but we already have something in common.  The hyperdrive technology we use is identical to yours.  Our ancestors left it in a wreckage on our planet.  We're on a mission to find our Homeworld.
#define M08_P2_YouWillFail						(573 + ACTOR_PIRATES2_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	You will fail.  The evil that drove us here will find and destroy you.  From you they will know of us and come here.  This cannot come to pass.
#define M08_Intel_DestroyAttackers				(574 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We have enemy units closing on multiple attack vectors.  Engage and destroy hostiles.
#define M08_Intel_EnemyRetreating				(575 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Enemy vessels retreating to this point.  This reading has been consistent despite sensor interference.  It has a friendly signature but it's not one of ours.
#define M08_Fleet_LooksJustLike					(576 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	It looks just like the Khar-Toba.
#define M08_Intel_IdenticalMatch				(577 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Metallurgy and structural composition are an identical match to the Khar-Toba wreckage on Kharak.
#define M08_Fleet_WeAreBrothers					(578 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We are brothers.
#define M08_Intel_HyperspaceOnline				(579 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The inhibitor field has ceased.  Hyperspace module back on-line.

/* 8.10 ANIMATIC - M08à M09 */
#define A08_Intel_1								(580 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	After analyzing the data we collected from the Khar-Toba's sister ship we've been able to determine what happened in the nebula.
#define A08_Intel_2								(581 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	While the Khar-Toba was able to limp to Kharak, this ship must have tried to hide here in the nebula.
#define A08_Intel_3								(582 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	They soon resorted to preying on ships passing through the nebula.
#define A08_Intel_4								(583 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	In time the nebula became off limits to all shipping.
#define A08_Intel_5								(584 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	They developed hyperspace inhibitor technology to trap prey from far away without leaving the safety of the nebula.
#define A08_Intel_6								(585 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	Eventually it became the center of their existence.
#define A08_Intel_7								(586 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	Their religion.


/******************************************
****            Mission 9              ****
******************************************/

/* 9 M09 - GHOSTSHIP (WHISPERING OCEAN) EZ09 */
#define M09_Fleet_AnomolyDetected				(587 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Anomaly detected.  Override engaged.
#define M09_Intel_VesselDetected				(588 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Sensors detect a vessel here.  It doesn't match any of the profiles we have encountered.  Send in a team to investigate.
#define M09_Intel_ShipsInactive					(589 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We are detecting various ships surrounding the alien vessel.  They appear to be inactive.
#define M09_Intel_NeutralizeVessel				(590 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Attention!  Those ships are operational.  We believe the control center is the alien vessel.  It should be neutralized.
#define M09_Intel_LostControl					(591 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We've lost control of capital ships in close proximity to the alien vessel.
#define M09_Intel_ControlField					(592 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We've determined that the alien control field covers this area.  No capital ships should cross into this zone or we will lose them.
#define M09_Intel_MinimalEffect					(593 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Our weapons are having minimal effect on the alien vessel but each strike causes a tiny fluctuation in the control field.
#define M09_Intel_DirectAllFire					(594 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Direct all fire at the alien ship in an attempt to disable the field.
#define M09_Intel_ConstructMissleDestroyer		(595 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	After seizing the Missile Destroyer, construction reports we can now build a similar vessel.
#define M09_Intel_VesselNeutralized				(596 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Alien vessel neutralized.  Our crews have regained control.
#define M09_Intel_SalvageUnknownShip			(597 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	While the field was up they were able to analyze the alien control system.  We now have control of the foreign vessels.  In addition, we detect no life signs aboard the alien vessel.  It is a derelict.
#define M09_Intel_ResearchGravWell				(598 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Research Division reports it has developed plans for gravity warping technology based on the alien control field.  We advise commencing research immediately.
#define M09_Traders_CouldNotApproach			(599 + ACTOR_TRADERS_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We have known of this ship but could never approach it.  We are particularly vulnerable to its influence.
#define M09_Traders_ExchangeInfo				(600 + ACTOR_TRADERS_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The Bentusi would like the information you have acquired.  It will be transferred automatically if you choose to trade.
#define M09_Traders_InfoTransfered				(601 + ACTOR_TRADERS_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The information was successfully transferred.  Thank you.
#define M09_Fleet_HelpUsDefeatKushan			(602 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The Taiidan are determined to destroy us.  Will you help us defeat them?
#define M09_Fleet_HelpUsDefeatTaiidan			(603 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The Kushan are determined to destroy us.  Will you help us defeat them?
#define M09_Traders_CouncilKushan				(604 + ACTOR_TRADERS_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Conflict is not our way.  We will bring your cause to the Galactic Council.  The Taiidan rule the Empire but even they must answer to the council.
#define M09_Traders_CouncilTaiidan				(605 + ACTOR_TRADERS_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Conflict is not our way.  We will bring your cause to the Galactic Council.  The Kushan rule the Empire but even they must answer to the council.
#define M09_Intel_HyperspaceReady				(606 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Hyperspace Module fully charged.  Engage at your discretion.

/* 9.14 ANIMATIC - M09à M10 */
#define A09_Intel_Kushan						(607 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	We are about to enter the outer limits of the Taiidan empire.
#define A09_Intel_Taiidan						(608 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	We are about to enter the outer limits of the Kushan empire.
#define A09_Intel_2								(609 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	As we approach the galactic core, resistance is expected to increase.
#define A09_Intel_3								(610 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	We have identified a weak point in the enemy defenses.
#define A09_Intel_4								(611 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	There is a remote research station located near an active supernova.
#define A09_Intel_5								(612 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	It should only have a minor garrison protecting it.
#define A09_Intel_6								(613 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	It is time to hunt the enemy as they have hunted us.


/******************************************
****            Mission 10             ****
******************************************/

/* 10 M10 - SUPERNOVA STATION (WHISPERING OCEAN) EZ10 */
#define M10_Fleet_HyperspaceSuccesful			(614 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Hyperspace jump successful.  We have cleared the Outer Rim dust bank.
#define M10_Fleet_Supernova						(615 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The supernova is two hundred and fifteen light years away.  It is emitting intense radiation.
#define M10_Intel_StikeCraftVulnerable			(616 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Strike Craft are especially vulnerable to this radiation.  Capital ships will be the most effective due to their heavy armor.
#define M10_Intel_DestroyOutpost				(617 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The Research Station is located here.  Assemble a heavy strike force and destroy it.
#define M10_Intel_RadiationHeavier				(618 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Radiation is much heavier than we expected.  Sensors indicate that asteroids may have shielding properties.
#define M10_Intel_AsteroidProtection			(619 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We recommend using the asteroid pockets for protection.
#define M10_Intel_DeployController				(620 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The radiation is disrupting our normal resource collecting operations.  Deploy a Resource Controller in the shielded asteroid pockets.
#define M10_Intel_ResearchProximitySensor		(621 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Research Division reports it is now equipped for Proximity Sensor technology.  We advise commencing research immediately.
#define M10_Intel_ResearchMinlayer				(622 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The enemy is using mines.  Research Division reports it can produce a Corvette-class minelaying ship.  We advise commencing research immediately.
#define M10_Intel_DestroyDefenseForce			(623 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We must destroy the garrison protecting the station.  Enemy units cannot be allowed to escape or they may alert the Empire to our presence.
#define M10_Intel_DestroyCarrierKushan			(624 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We are picking up a quantum wave effect.  A Taiidan Carrier is loading ships and powering up.  It must be destroyed before it can hyperspace.
#define M10_Intel_DestroyCarrierTaiidan			(625 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We are picking up a quantum wave effect.  A Kushan Carrier is loading ships and powering up.  It must be destroyed before it can hyperspace.
#define M10_Intel_HyperspaceReady				(626 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Enemy base and fleet destroyed.  Hyperspace drives online.

/* 10.11 ANIMATIC - M10à M11 */
#define A10_Intel_Kushan						(627 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	We have intercepted a coded Taiidan transmission:
#define A10_Intel_Taiidan						(628 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	We have intercepted a coded Kushan transmission:
#define A10_Admiral_1							(629 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	You have failed to keep the Exiles from penetrating the outer perimeter.  This could be disastrous.  You will find and destroy them immediately.
#define A10_Admiral_2							(630 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	Do not fail us again, or the Elite Guard will require a new commander.
#define A10_Admiral_3							(631 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	Our spies believe that the Bentusi have interfered in this matter.  They must not be allowed to bring this matter to the Council and gain support for the Exiles.


/******************************************
****            Mission 11             ****
******************************************/

/* 11 M11 - P3 VS. TRADERS (TENHAUSER GATE) EZ11 */
#define M11_Fleet_HyperspaceDisengaged			(632 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Hyperdrive disengaged.  The Bentusi are here.  They're in distress.
#define M11_Intel_DestroyEnemyKushan			(633 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The Bentusi must be protected.  Draw the Taiidan fleet away and destroy them.
#define M11_Intel_DestroyEnemyTaiidan			(634 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The Bentusi must be protected.  Draw the Kushan fleet away and destroy them.
#define M11_Traders_BetusiInDebt				(635 + ACTOR_TRADERS_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	For the first time in memory, the Bentusi are in the debt of another.

/* 11.4 N08 - Awareness (Tenhauser Gate) Ez11 */
#define N08_Traders_ForbiddenInfo				(636 + ACTOR_TRADERS_FLAG + SPEECH_NIS_FLAG)	//	It has been forbidden to possess this information for some time.  But after your intervention on our behalf, we feel compelled to share it with you.  Behold:
#define N08_Traders_Awareness1					(637 + ACTOR_TRADERS_FLAG + SPEECH_NIS_FLAG)	//	In the First Time, a terrible war brought with it the collapse of your ancient empire.
#define N08_Traders_Awareness2					(638 + ACTOR_TRADERS_FLAG + SPEECH_NIS_FLAG)	//	In an effort to sooth relations, the conquerors spared the lives of the defeated.  All survivors were sent into exile.
#define N08_Traders_Awareness3					(639 + ACTOR_TRADERS_FLAG + SPEECH_NIS_FLAG)	//	None were permitted to follow or aid the fallen.  All memory of them was to be erased.
#define N08_Traders_Awareness4					(640 + ACTOR_TRADERS_FLAG + SPEECH_NIS_FLAG)	//	For generations the convoy moved silently through space.  They endured great difficulties...
#define N08_Traders_Awareness5					(641 + ACTOR_TRADERS_FLAG + SPEECH_NIS_FLAG)	//	...imperfect technology...
#define N08_Traders_Awareness6					(642 + ACTOR_TRADERS_FLAG + SPEECH_NIS_FLAG)	//	In time, a suitable system to receive them was found.
#define N08_Traders_Awareness7					(643 + ACTOR_TRADERS_FLAG + SPEECH_NIS_FLAG)	//	This barren world appeared to be salvation. Their true legacy forgotten, a new vision of destiny had grown out of captivity.
#define N08_Traders_Awareness8					(644 + ACTOR_TRADERS_FLAG + SPEECH_NIS_FLAG)	//	A single artifact survived the journey.  The Guide Stone you now carry.  It was removed from the sacred "Angel Moon" of your Homeworld, a place long since reduced to myth and tale.
#define N08_Traders_Awareness9					(645 + ACTOR_TRADERS_FLAG + SPEECH_NIS_FLAG)	//  Your progress is becoming known among the Inner Limb worlds and elsewhere.  Many cultures have prophesized your return.

/* 11 M11 - P3 VS. TRADERS (TENHAUSER GATE) EZ11 */
#define M11_Traders_ReachHomeworld				(646 + ACTOR_TRADERS_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Reach your Homeworld.  Establish your claim.  We will summon the Council.
#define M11_Intel_EngageHyperdrive				(647 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Engage Hyperdrive.

/* 0.0 ANIMATIC - M11à M12 */
#define A11_Fleet_1								(648 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_ANIMATIC_FLAG)	//	Emergency alert!!
#define A11_Fleet_2								(649 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_ANIMATIC_FLAG)	//	Hyperdrive Malfunction.
#define A11_Fleet_3								(650 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_ANIMATIC_FLAG)	//	The quantum waveform is collapsing.
#define A11_Fleet_4								(651 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_ANIMATIC_FLAG)	//	Emergency interrupt engaged.
#define A11_Fleet_5								(652 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_ANIMATIC_FLAG)	//	Prepare for immediate return to normal space.
												

/******************************************
****            Mission 12             ****
******************************************/

/* 12 M12 - ELITE GUARD TRAP (GALACTIC CORE) EZ12 */
#define M12_Fleet_HyperspaceDisengaged			(653 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Dropping out of hyperspace.  All systems online.
#define M12_Intel_DestroyGravWell				(654 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  We are caught in a gravity well.  Fighters and Corvettes will be unable to move.  Seek out the source of this field and destroy it.
#define M12_Intel_ResearchMissleDestroyer		(655 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The enemy is using a Missile Destroyer.  Research Division reports it can produce a similar ship.  We advise commencing research immediately.
#define M12_Intel_ResearchCloadkedFighter		(656 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  Research Division reports it is now equipped for Cloaked Fighter technology.  We advise commencing research immediately.
#define M12_Intel_ResearchDefenseFighter		(657 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Research Division reports it is now equipped for Defense Fighter technology.  We advise commencing research immediately.
#define M12_Intel_UnderAttackKushan				(658 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We are under attack by Taiidan forces.  They are concentrating fire on our immobilized Strike Craft.
#define M12_Intel_UnderAttackTaiidan			(659 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  We are under attack by Kushan forces.  They are concentrating fire on our immobilized Strike Craft.
#define M12_Defector_DefectingKushan			(660 + ACTOR_DEFECTOR_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Attention Kushan Mothership!  This is Captain Elson of the Taiidan Elite Guard Destroyer Kapella.  We wish to defect and need assistance.  In return we are prepared to help you.  Please respond.
#define M12_Defector_DefectingTaiidan			(661 + ACTOR_DEFECTOR_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Attention Taiidan Mothership!  This is Captain Elson of the Kushan Elite Guard Destroyer Kapella.  We wish to defect and need assistance.  In return we are prepared to help you.  Please respond.
#define M12_Intel_ProtectDefector				(662 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	This could be a trap but the Kapella is clearly damaged.  Engage the pursuing fleet and draw it away from the defecting ship.
#define M12_Intel_EngageHyperdrive				(663 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The defecting captain has been brought aboard.  Engage hyperspace.

/* 12.8 ANIMATIC - M12à M13 */
#define A12_Defector_1							(664 + ACTOR_DEFECTOR_FLAG + SPEECH_ANIMATIC_FLAG)	//  I am Group Captain Elson of the Destroyer Kapella.
#define A12_Defector_Kushan						(665 + ACTOR_DEFECTOR_FLAG + SPEECH_ANIMATIC_FLAG)	//  The Taiidan empire has become decadent and corrupt over the centuries.
#define A12_Defector_Taiidan					(666 + ACTOR_DEFECTOR_FLAG + SPEECH_ANIMATIC_FLAG)	//  The Kushan empire has become decadent and corrupt over the centuries.
#define A12_Defector_3							(667 + ACTOR_DEFECTOR_FLAG + SPEECH_ANIMATIC_FLAG)	//  The use of the forbidden atmosphere-deprivation device on your planet finally triggered the Rebellion.
#define A12_Defector_4							(668 + ACTOR_DEFECTOR_FLAG + SPEECH_ANIMATIC_FLAG)	//  Help me get access to the Rebellion's communication network.
#define A12_Defector_5							(669 + ACTOR_DEFECTOR_FLAG + SPEECH_ANIMATIC_FLAG)	//  I will show you a way through the defenses surrounding your Homeworld.
#define A12_Defector_6							(670 + ACTOR_DEFECTOR_FLAG + SPEECH_ANIMATIC_FLAG)	//  Take me to the ship graveyard at Karos.
#define A12_Defector_7							(671 + ACTOR_DEFECTOR_FLAG + SPEECH_ANIMATIC_FLAG)	//  Hidden in a derelict there is a relay I can use with your help.
#define A12_Defector_8							(672 + ACTOR_DEFECTOR_FLAG + SPEECH_ANIMATIC_FLAG)	//  The Rebellion waits for my sign to move into its next phase.
#define ShanesARealBigDummy						673	// heh heh.. ;^)

/******************************************
****            Mission 13             ****
******************************************/

/* 13 M13 - SHIP GRAVEYARD (SHINING HINTERLANDS) EZ13 */
#define M13_Fleet_HyperspaceDisengaged			(674 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Hyperspace transition complete.  We have arrived at Karos.
#define M13_Intel_CommunicationRelay			(675 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The communication relay is here.  A Fighter or Corvette must dock with it to establish the link.
#define M13_Intel_CapShipLost					(676 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Attention.  We have lost contact with one of our capital ships.  It's last recorded position is here.
#define M13_Intel_HyperspaceSignature			(677 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Hyperspace signatures have been found at these locations but we detect no new ships.
#define M13_Intel_FoundMissingShip				(678 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We've located one of the missing ships.  It appears that it can be salvaged and reactivated.
#define M13_Intel_GraveyardDefended				(679 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The Graveyard is defended by autoguns.  They will complicate our attempt to reach the communications array.
#define M13_All_CommLinkEstablished				(680 + ACTOR_PILOT_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	I'm in!  Communication link established.  I've docked!  Communication link established.
#define M13_Defector_ResistanceInformedKushan	(681 + ACTOR_DEFECTOR_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Patching into command node now... The Taiidan resistance has been informed of your actions and are preparing the fleets.  You have our thanks.
#define M13_Defector_ResistanceInformedTaiidan	(682 + ACTOR_DEFECTOR_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Patching into command node now... The Kushan resistance has been informed of your actions and are preparing the fleets.  You have our thanks.
#define M13_Defector_CoordinatesTransfered		(683 + ACTOR_DEFECTOR_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The hyperspace coordinates you require have been transferred to your Mothership.  Farewell.
#define M13_Intel_HyperspaceEnabled				(684 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Hyperspace Enabled.

/* 13.10 ANIMATIC - M13à M14 */
#define A13_Intel_1								(685 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	According to the data we received from Captain Elson, the Homeworld system is surrounded by a network of hyperspace inhibitors.
#define A13_Intel_2								(686 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	The inhibitors are all heavily shielded and do not show up on any sensors.
#define A13_Intel_3								(687 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	Elson has provided us with co-ordinates of the most vulnerable inhibitor station.
#define A13_Intel_4								(688 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	Our goal is to destroy the station and create our own access point.


/******************************************
****            Mission 14             ****
******************************************/

/* 14 M14 - MINING FACILITY (BRIDGE OF SIGHS) EZ14 */
#define M14_Fleet_HyperspaceDisengaged			(689 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Hyperspace successful.  We are at the edge of the Homeworld system.
#define M14_Intel_DestroyGenerator				(690 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Elson's information was correct.  This is the field generator.  We must destroy it.
#define M14_Intel_ResearchHeavyGuns				(691 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Research Division reports it is now equipped for Heavy Guns technology.  We advise commencing research immediately.
#define M14_Intel_ResearchSensors				(692 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Research Division reports advancements in sensor fidelity which would allow us to determine the location of enemy ships.  We advise commencing research immediately.
#define M14_Intel_DestroyHyperspaceGates		(693 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The enemy has activated several standing hyperspace gates.  Destroy the gates to prevent enemy reinforcement.
#define M14_Intel_HyperdriveOnline				(694 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The field surrounding the Homeworld system has been shut down.  Hyperdrive on-line.
#define M14_Fleet_TakeUsHome					(695 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Take us home.

/* 14.8 ANIMATIC - M14à M15 */
#define A14_Intel_1								(696 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	We have to assume that the Homeworld's defensive fleet must be alerted to our presence.
#define A14_Intel_2								(697 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	All vessels and crew at maximum readiness.
#define A14_Intel_3								(698 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	Weapons and tracking at 100% efficiency.
#define A14_Intel_4								(699 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	The fleet is ready.
#define A14_Intel_5								(700 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	There can be no retreat now.


/******************************************
****            Mission 15             ****
******************************************/

/* 15 M15 - HEADSHOT (CHAPEL PERILOUS) EZ15 */
#define M15_Fleet_HyperspaceInterrupted			(701 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Hyperspace interrupted.
#define M15_Intel_AlertedKushan					(702 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Shutting down the inhibitor field has alerted the Taiidan to our presence.
#define M15_Intel_AlertedTaiidan				(703 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Shutting down the inhibitor field has alerted the Kushan to our presence.
#define M15_Intel_DestroyHeadshot				(704 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  We are on a collision course with a very large object.  It appears to have escorts.  It must be destroyed before it impacts the Mothership.
#define M15_Intel_ImpactIn20					(705 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Impact in twenty minutes.
#define M15_Intel_ImpactIn15					(706 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Impact in fifteen minutes.
#define M15_Intel_ImpactIn10					(707 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Impact in ten minutes.
#define M15_Intel_ImpactIn5						(708 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Impact in five minutes.
#define M15_Intel_ImpactIn4						(709 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Impact in four minutes.
#define M15_Intel_ImpactIn3						(710 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Impact in three minutes.
#define M15_Intel_ImpactIn2						(711 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Impact in two minutes.
#define M15_Intel_ImpactIn1						(712 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Impact in one minute.
#define M15_Fleet_StandbyForImpact				(713 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Standby for impact.
#define M15_Intel_HyperspaceReady				(714 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Hyperspace module charged and ready.

/* 15.14 ANIMATIC - M15à M16 */
#define A15_1									(715 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_ANIMATIC_FLAG)	//	What's happening?  OR  The Enemy must have been desperate.
#define A15_2									(716 + ACTOR_EMPEROR_FLAG + SPEECH_ANIMATIC_FLAG)	//  Karan.  OR  The time we took to break up that attack has allowed them time to reinforce.
#define A15_3									(717 + ACTOR_EMPEROR_FLAG + SPEECH_ANIMATIC_FLAG)	//  You've taken one step too close to me.  OR  The chemical composition of this system matches that of the Guidestone.
#define A15_4									(718 + ACTOR_EMPEROR_FLAG + SPEECH_ANIMATIC_FLAG)	//  From here I can touch you.  OR  We're home.


/******************************************
****            Mission 16             ****
******************************************/

/* 16 M16 - FINAL BATTLE (HOMEWORLD SYSTEM) EZ16 */
#define M16_Intel_LostKaran						(719 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We've lost Karan.  Fleet Command is gone.  Emergency biotech teams are working to keep her alive.
#define M16_Intel_CollisionAsteroidKushan		(720 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The collision asteroid must have served its purpose as a delay tactic.  There is a large number of Taiidan ships located here.
#define M16_Intel_CollisionAsteroidTaiidan		(721 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The collision asteroid must have served its purpose as a delay tactic.  There is a large number of Kushan ships located here.
#define M16_Intel_EmperorsShip					(722 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	A Mothership-class vessel is among them.
#define M16_Intel_DestroyAll					(723 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	All of these forces must be destroyed.  Good luck.
#define M16_Intel_EnemyReinforcements			(724 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Enemy reinforcements emerging from hyperspace.
#define M16_Intel_AnotherFleet					(725 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Another fleet is coming out of Hyperspace right on top of us.  We are being overwhelmed!
#define M16_Defector_CaptainElson				(726 + ACTOR_DEFECTOR_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	This is Captain Elson.  We have been battling reinforcement fleets to get here and have lost many ships already.
#define M16_Defector_EmperorKushan				(727 + ACTOR_DEFECTOR_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  The Emperor's flagship is here.  Together we can defeat him and the Taiidan fleet.  I am placing squadrons Cor and Jasah under your command.
#define M16_Defector_EmperorTaiidan				(728 + ACTOR_DEFECTOR_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  The Emperor's flagship is here.  Together we can defeat him and the Kushan fleet.  I am placing squadrons Cor and Jasah under your command.
#define M16_Intel_DestroyFlagship				(729 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Here is the Emperor's flagship.  It must be destroyed.
#define M16_Fleet_BackOnline					(730 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	...Fleet Command back online.  The Emperor is gone.
#define M16_Traders_BroughtTheCouncil			(731 + ACTOR_TRADERS_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We have brought the Council.  This war is over.

/* 16.10 ANIMATIC - M15à M16 */
#define A16_Fleet_SigningOff					(732 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_ANIMATIC_FLAG)	//	This is Fleet Command, signing off.

/* Quick Fixes */
#define N01_All_Misc_5							(733 + ACTOR_PILOT_FLAG + SPEECH_NIS_FLAG)	//  All stations green.
#define M01_Intel_ResearchOnline				(734 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The primary Research Ship has been constructed and the Research Division is online.  Begin Fighter Chassis research immediately.
#define M15_Intel_ImpactIn30secs				(735 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Impact in 30 seconds.
#define M15_Intel_ImpactIn10secs				(736 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Impact in 10 seconds.

#define A15_1_Alt								(737 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//	The Enemy must have been desperate.
#define A15_2_Alt								(738 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//  The time we took to break up that attack has allowed them time to reinforce.
#define A15_3_Alt								(739 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//  The chemical composition of this system matches that of the Guidestone.
#define A15_4_Alt								(740 + ACTOR_FLEETINTEL_FLAG + SPEECH_ANIMATIC_FLAG)	//  We're home.

#define STAT_F_CrateFoundResources				(741 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	STATUS_CrateFound-ResourcesRecovered
#define STAT_F_CrateFoundShips					(742 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	STATUS_CrateFound-ShipsRecovered
#define STAT_F_CrateFoundTech					(743 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	STATUS_CrateFound-TechRecovered
#define COMM_F_NoMoreResearchTopic				(744 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG)	//	COMMAND_Research-NoMoreResearchTopics
#define STAT_F_RelicPlayerDies					(745 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	STATUS_RelicPlayerDies-EasterEgg
#define COMM_F_RelicAllianceFormed				(746 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	STATUS_RelicAllianceFormed-EasterEgg
#define COMM_F_RelicAllianceBroken				(747 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	STATUS_RelicAllianceBroken-EasterEgg

#define STAT_CloakedShips_Detected				(748 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG)	//	STATUS_CloakedShips-Detected(Report) (ALL SHIPS)
#define N01_Chatter_BG							(749 + ACTOR_PILOT_FLAG + SPEECH_NIS_FLAG)
#define N01_Fleet_Standby						(750 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_NIS_FLAG)		//	Stand by for Initial Fleet Command Line testing...


/* ALTERNATIVE VERSIONS FOR ADAM AND ERIN */

#define M02_Intel_ObjectivesCompleteSHORT		(751 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Objectives complete.  Standby for immediate return to Kharak.
#define M09_Intel_VesselNeutralizedSHORT		(752 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Alien vessel neutralized.
#define M12_Intel_UnderAttackKushanSHORT		(753 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We are under attack by Taiidan forces.
#define M12_Intel_UnderAttackTaiidanSHORT		(754 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  We are under attack by Kushan forces.

/* PICKUPS - Fleet Intel Pickups.doc */

#define M01_Intel_HyperdriveTest				(755 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Standby for Hyperdrive test.  Internal pressure doors sealed.  Abort systems standing by.

#define M02_Intel_EnemyInferior2				(756 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We have determined that these enemy units are inferior to ours.
#define M02_Intel_DestroyCompletely				(757 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  To protect against penetration of the Kharak system, destroy the attacking force completely.
#define M02_Intel_EnemyHyperspacing				(758 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The enemy Mothership is hyperspacing.  Recall all Fighters and prepare for return to Kharak.

#define M03_Intel_DefendCryoTrays				(759 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The Cryo Trays are under attack.  Defend them.
#define M03_Intel_SalvageCryoTrays				(760 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Enemy units neutralized.  Begin salvaging the cryo trays.

#define M04_Intel_MoveController				(761 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	To increase harvesting efficiency, move your Resource Controller as close to heavy resource areas as possible.

#define M05_Intel_MoveAttackNEW					(762 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Enemy capital ships appear to have lighter armor on the top, bottom, and rear sides.  Our capital ships should be issued move orders while attacking to take advantage of this weakness.

#define M06_Intel_IonCannonTech					(763 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Research Division reports it is now equipped for Ion Cannon technology.  We advise commencing research immediately.
#define M06_Intel_PlasmaBombTech				(764 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Research Division reports it is now equipped for Plasma Bomb technology.  We advise commencing research immediately.
#define M06_Intel_DetectingBentusi				(765 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Detecting incoming Bentusi vessel from the clearing ahead.

#define M07_Intel_HyperdriveFailed				(766 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Hyperdrive jump failed!  The Quantum waveform collapsed due to some kind of inhibitor field.
#define M07_Intel_AnalyzingField				(767 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Analyzing field.  Continue to protect the Mothership until the source is located.
#define M07_Intel_FieldDisappeared				(768 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The inhibitor field has disappeared.
#define M07_Intel_HyperdriveFunctional			(769 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The Hyperdrive is fully functional.
#define M07_Intel_ResearchMultigunVette			(770 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The enemy is relying heavily on Fighter-class ships.  Our Research division reports it can design a new type of Corvette specially suited to combat multiple fighters.  Begin Research as soon as possible.

#define M08_Intel_DestroyHostilesNEW			(771 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We have enemy units closing from all directions.  Engage and destroy hostiles.
#define M08_Intel_InhibitorCeased				(772 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The inhibitor field has ceased.  Hyperspace ability has been restored.
#define M08_Fleet_ItsTheKharToba				(773 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	It's the Khar-Toba.
#define M08_Intel_IdenticalToKharToba			(774 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Metallurgy and structural composition of the ship are an identical match to the Khar-Toba wreckage on Kharak.  Our origins are the same.

#define M09_Intel_SalvageShip					(775 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	While the field was up they were able to analyze the alien control system.  We now have control of the foreign vessels.
#define M09_Intel_SendSalvageTeam				(776 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  Send a Salvage Team to further investigate the alien ship.
#define M09_Intel_VesselNeutralizedSHORTNEW		(777 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Alien vessel neutralized.
#define M09_Intel_SalvageDocked					(778 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Salvage Team docked.  The alien ship is millions of years old.  Its purpose is unclear.
#define M09_Intel_ResearchGravWellNEW			(779 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  However, Research Division reports it has developed plans for gravity warping technology based on composition of the alien hull.  We advise commencing research immediately.

#define M10_Intel_ResearchCloakGen				(780 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Research Division reports it is now equipped for Cloaked Generator technology.  We advise commencing research immediately.
#define M10_Intel_UseViensForProtection			(781 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Radiation is much heavier than we expected.  Sensors indicate that these veins of space dust may have shielding properties.  We recommend using the veins for protection.

#define M11_Intel_ResearchSuperHeavy			(782 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Research Division reports it is now equipped for Super Heavy Chassis technology.  We advise commencing research immediately.

#define SP_Intel_PrimaryObjectiveComplete		(783 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Primary objective complete.
#define SP_Intel_SecondaryObjectiveComplete		(784 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Secondary objective complete.
#define SP_Intel_ConsultObjectives				(785 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	A mission objective remains incomplete. Consult objectives.
#define SP_Intel_MissionFailed					(786 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Mission Failed.

/* PICKUPS - Fleet Command Pickups.doc */
#define M05_Fleet_FollowTheGuidestone			(787 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	There's nothing left for us to return to.  Our only option is to follow the path etched into the Guidestone.  Finding our ancient home is our only hope now.

#define M07_Fleet_Charging8Minutes				(788 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Hyperspace module charging, ready in 8 minutes.
#define M07_Fleet_Charging1Minute				(789 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Hyperspace module charging, ready in 1 minute.
#define M07_Fleet_HyperdriveFailedNEW			(790 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Hyperdrive jump failed!  The Quantum waveform collapsed due to some kind of inhibitor field.

#define M08_Fleet_CannotStay					(791 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We cannot stay - we're on a journey.  But let there be peace between us, for we have something in common.  The hyperdrive technology left to us by our ancestors is identical to yours.  The Homeworld we seek may be yours as well.

#define COMM_F_HyperspaceDetected				(792 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG)	//  0.8 STATUS_HyperspaceDetected - 1) Hyperspace signature detected.
#define COMM_F_PrimaryObjectiveComplete			(793 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG)	//  0.9 STATUS_PrimaryObjectiveComplete - 1) Primary objective complete.
#define COMM_F_SecondaryObjectiveComplete		(794 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_COMMAND_FLAG)	//	0.10 STATUS_SecondaryObjectiveComplete - 1) Secondary objective complete.
#define STAT_F_CapturedShipRetrofit				(795 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_STATUS_FLAG)	//	0.11 STATUS_CapturedShipBeingRetrofitted - 1) Captured ship secure.  Retrofit commencing.

/* PICKUPS - Final Animatic */
#define A16_Narrator_Ending						(796 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	The Galactic Council recognized our claim to this world.
#define A16_Narrator_Ending2					(797 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	The sacrifice of thousands has left a trail of destruction behind us, like a path across the galaxy
#define A16_Narrator_Ending3					(798 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	..to  Higaara, our Homeworld.
#define A16_Narrator_Ending4					(799 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	So much destruction, so many lives lost, for this place.
#define A16_Narrator_Ending5					(800 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	A place of wonder to those who knew only the sands of Kharak.
#define A16_Narrator_Ending6					(801 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	Our colonists were released from their long sleep.
#define A16_Narrator_Ending7					(802 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	All symbols of the old empire were destroyed.
#define A16_Narrator_Ending8					(803 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	But the conflict will never be forgotten.
#define A16_Narrator_Ending9					(804 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	A celebration marked the beginning of a new time.
#define A16_Narrator_Ending10					(805 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//	No longer Fleet Command, Karan Sjet survived extraction from the Mothership's core.
#define A16_Narrator_Ending11					(806 + ACTOR_NARRATOR_FLAG + SPEECH_ANIMATIC_FLAG)	//  She insisted that she would be the last person to disembark and set foot on the Homeworld.

/* SCRAPPED EVENTS REQUESTED TO BE RE-ADDED */
#define SP_Pilot_SCVetteNotEnough				(807 + ACTOR_PILOT_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	This is to tell the player they're not using enough SC Vettes.

/* damned OEM mission */
#define M04OEM_Intel_Hyperspace					(808 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We've located the origin of the Turanic Raider fleets.  They came from a small, isolated planetoid located in a remote area of the Great Wasteland.  Engage hyperspace.
#define M05OEM_Intel_PlanetaryDefense			(809 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Sensors indicate a heavily armed planetary defense system which we cannot penetrate.
#define M05OEM_Intel_HyperspacingIn				(810 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  The planetary garrison must have alerted the main Turanic Raider Fleet which is currently Hyperspacing in here.  They are retreating to the planetoid.
#define M05OEM_Intel_DestroyCarriers			(811 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  We must stop their Carriers from reaching the safety of the planetary defenses.
#define M05OEM_Intel_PrepareForAssault			(812 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	The Turanic Carriers are launching Strike Craft.  Prepare for assault.
#define M05OEM_Intel_MissionFailed				(813 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Mission Failed.
#define M05OEM_Intel_StayOutOfRange				(814 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//  The planetary defense system covers this area. Any of our ships entering its range will be destroyed.  Avoid this area at all costs.
#define M05OEM_Intel_DestroyedRaiders			(815 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	We have destroyed the body of the Turanic Raider fleet.  We can now proceed on our journey.  Hyperspace when ready.

/* Bug Fixes */
#define M04_Intel_DefendTheFleetShort			(816 + ACTOR_FLEETINTEL_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	Turanic Raider fighters are attacking. Prepare for more hostile ships to arrive.
#define STAT_Group_LowOnFuel					(817 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG + SPEECH_GROUP_FLAG + SPEECH_ALWAYSPLAY_FLAG)	//	115. STATUS_StrikeCraft-LowOnFuel (ALL SHIPS)
#define STAT_Group_OutOfFuel					(818 + ACTOR_PILOT_FLAG + SPEECH_STATUS_FLAG + SPEECH_GROUP_FLAG + SPEECH_ALWAYSPLAY_FLAG)	//	116. STATUS_StrikeCraft-OutOfFuel (ALL SHIPS)

/* alternate tutorial event for Falko */
#define TUT_SelectInfoOverlayAlt				(819 + ACTOR_FLEETCOMMAND_FLAG + SPEECH_TUTORIAL_FLAG)	//	At the top right of the screen is the Info Overlay.  It displays all the selected ships.  Click Next to continue.

/* SCRAPPED EVENTS REQUESTED TO BE RE-ADDED */
#define SP_Pilot_P2Refuelling					(820 + ACTOR_PILOT_FLAG + SPEECH_SINGLEPLAYER_FLAG)	//	This tells the player to go after the fuel pods

/*********  FINALLY DONE!!!!  Maybe :)  ******/

#define SPEECH_LAST_EVENT						(SP_Pilot_P2Refuelling & SPEECH_EVENT_MASK)

#define NUM_SPEECH_EVENTS						(SP_Pilot_P2Refuelling & SPEECH_EVENT_MASK + 1)

#define SPEECH_FIRST_SP_EVENT					(TUT_Intro & SPEECH_EVENT_MASK)

