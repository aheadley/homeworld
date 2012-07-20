
#----------------------------------------------------------------------
# Assembler offsets for SSTREGS struct
#----------------------------------------------------------------------



#----------------------------------------------------------------------
# Assembler offsets for GC struct
#----------------------------------------------------------------------

base_ptr		 .EQU          0
reg_ptr			 .EQU          4
lfb_ptr			 .EQU         12
cull_mode		 .EQU        584
regDataList		 .EQU         20
tsuDataList		 .EQU        396
triPacketHdr	 .EQU        848
cullStripHdr	 .EQU        852
paramMask	 .EQU        856
fifoStart	 .EQU        860
fifoEnd	 .EQU        864
fifoOffset	 .EQU        868
fifoSize	 .EQU        872
fifoJmpHdr	 .EQU        876
fifoPtr	 .EQU        880
fifoRead	 .EQU        884
fifoRoom	 .EQU        888
roomToReadPtr	 .EQU        892
roomToEnd	 .EQU        896
fifoLfbP	 .EQU        900
lfbLockCount	 .EQU        904
SIZEOF_GrState		 .EQU        264
SIZEOF_GrHwConfiguration	 .EQU        148
SIZEOF_GC		 .EQU     201800


#----------------------------------------------------------------------
# Assembler offsets for GlideRoot struct
#----------------------------------------------------------------------

p6Fencer		 .EQU          0
current_sst		 .EQU          4
CPUType			 .EQU          8
curGC			 .EQU         12
curTriSize		 .EQU         24
trisProcessed		 .EQU        116
trisDrawn		 .EQU        120
SIZEOF_GlideRoot	 .EQU     807532


#----------------------------------------------------------------------
# Assembler offsets for GrVertex struct
#----------------------------------------------------------------------

x	 .EQU          0
y	 .EQU          4
r	 .EQU         12
g	 .EQU         16
b	 .EQU         20
a	 .EQU         28
SIZEOF_GrVertex	 .EQU         60

