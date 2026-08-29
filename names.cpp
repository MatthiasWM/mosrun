/*
 mosrun - the MacOS MPW runtime emulator
 Copyright (C) 2013-2020  Matthias Melcher

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The author can be contacted at mosrun AT matthiasm DOT com.
 The latest source code can be found at https://github.com/MatthiasWM/mosrun
 */

#include "names.h"
#include "main.h"
#include "traps.h"

#include <stdio.h>

const char* gToolboxTrapName[kToolboxTrapTableSize] = { // 1024

}; // gToolboxTrapName

const char* gOSTrapName[kOSTrapTableSize] = { // 256
}; // gOSTrapName

typedef struct {
    unsigned int id;
    const char *name;
} Traps;

Traps trapLUT[] = {
    { 0xA000, "_Open" },
    { 0xA001, "_Close" },
    { 0xA002, "_Read" },
    { 0xA003, "_Write" },
    { 0xA004, "_Control" },
    { 0xA005, "_Status" },
    { 0xA006, "_KillIO" },
    { 0xA007, "_GetVolInfo" },
    { 0xA008, "_Create" },
    { 0xA009, "_Delete" },
    { 0xA00A, "_OpenRF" },
    { 0xA00B, "_Rename" },
    { 0xA00C, "_GetFileInfo" },
    { 0xA00D, "_SetFileInfo" },
    { 0xA00E, "_UnmountVol" },
    { 0xA00F, "_MountVol" },
    { 0xA010, "_Allocate" },
    { 0xA011, "_GetEOF" },
    { 0xA012, "_SetEOF" },
    { 0xA013, "_FlushVol" },
    { 0xA014, "_GetVol" },
    { 0xA015, "_SetVol" },
    { 0xA016, "_FInitQueue" },
    { 0xA017, "_Eject" },
    { 0xA018, "_GetFPos" },
    { 0xA019, "_InitZone" },
    { 0xA01B, "_SetZone" },
    { 0xA01C, "_FreeMem" },
    { 0xA01F, "_DisposePtr" },
    { 0xA020, "_SetPtrSize" },
    { 0xA021, "_GetPtrSize" },
    { 0xA023, "_DisposeHandle" },
    { 0xA024, "_SetHandleSize" },
    { 0xA025, "_GetHandleSize" },
    { 0xA027, "_ReallocHandle" },
    { 0xA029, "_HLock" },
    { 0xA02A, "_HUnlock" },
    { 0xA02B, "_EmptyHandle" },
    { 0xA02C, "_InitApplZone" },
    { 0xA02D, "_SetApplLimit" },
    { 0xA02E, "_BlockMove" },
    { 0xA02F, "_PostEvent" },
    { 0xA030, "_OSEventAvail" },
    { 0xA031, "_GetOSEvent" },
    { 0xA032, "_FlushEvents" },
    { 0xA033, "_VInstall" },
    { 0xA034, "_VRemove" },
    { 0xA035, "_OffLine" },
    { 0xA036, "_MoreMasters" },
    { 0xA038, "_WriteParam" },
    { 0xA039, "_ReadDateTime" },
    { 0xA03A, "_SetDateTime" },
    { 0xA03B, "_Delay" },
    { 0xA03C, "_CmpString" },
    { 0xA03D, "_DrvrInstall" },
    { 0xA03E, "_DrvrRemove" },
    { 0xA03F, "_InitUtil" },
    { 0xA040, "_ResrvMem" },
    { 0xA043, "_SetFilType" },
    { 0xA044, "_SetFPos" },
    { 0xA045, "_FlushFile" },
    { 0xA047, "_SetTrapAddress" },
    { 0xA049, "_HPurge" },
    { 0xA04A, "_HNoPurge" },
    { 0xA04B, "_SetGrowZone" },
    { 0xA04C, "_CompactMem" },
    { 0xA04D, "_PurgeMem" },
    { 0xA04E, "_AddDrive" },
    { 0xA04F, "_RDrvrInstall" },
    { 0xA050, "_CompareString" },
    { 0xA051, "_ReadXPRam" },
    { 0xA052, "_WriteXPRam" },
    { 0xA054, "_UprString" },
    { 0xA055, "_StripAddress" },
    { 0xA056, "_LowerText" },
    { 0xA057, "_SetApplBase" },
    { 0xA058, "_InsTime" },
    { 0xA059, "_RmvTime" },
    { 0xA05A, "_PrimeTime" },
    { 0xA05B, "_PowerOff" },
    { 0xA05C, "_MemoryDispatch" },
    { 0xA05D, "_SwapMMUMode" },
    { 0xA05E, "_NMInstall" },
    { 0xA05F, "_NMRemove" },
    { 0xA060, "_FSDispatch" },
    { 0xA061, "_MaxBlock" },
    { 0xA063, "_MaxApplZone" },
    { 0xA064, "_MoveHHi" },
    { 0xA065, "_StackSpace" },
    { 0xA067, "_HSetRBit" },
    { 0xA068, "_HClrRBit" },
    { 0xA069, "_HGetState" },
    { 0xA06A, "_HSetState" },
    { 0xA06C, "_InitFS" },
    { 0xA06D, "_InitEvents" },
    { 0xA06E, "_SlotManager" },
    { 0xA06F, "_SlotVInstall" },
    { 0xA070, "_SlotVRemove" },
    { 0xA071, "_AttachVBL" },
    { 0xA072, "_DoVBLTask" },
    { 0xA075, "_SIntInstall" },
    { 0xA076, "_SIntRemove" },
    { 0xA077, "_CountADBs" },
    { 0xA078, "_GetIndADB" },
    { 0xA079, "_GetADBInfo" },
    { 0xA07A, "_SetADBInfo" },
    { 0xA07B, "_ADBReInit" },
    { 0xA07C, "_ADBOp" },
    { 0xA07D, "_GetDefaultStartup" },
    { 0xA07E, "_SetDefaultStartup" },
    { 0xA07F, "_InternalWait" },
    { 0xA080, "_GetVideoDefault" },
    { 0xA081, "_SetVideoDefault" },
    { 0xA082, "_DTInstall" },
    { 0xA083, "_SetOSDefault" },
    { 0xA084, "_GetOSDefault" },
    { 0xA085, "_PMgrOp" },
    { 0xA086, "_IOPInfoAccess" },
    { 0xA087, "_IOPMsgRequest" },
    { 0xA088, "_IOPMoveData" },
    { 0xA089, "_SCSIAtomic" },
    { 0xA08A, "_Sleep" },
    { 0xA08B, "_CommToolboxDispatch" },
    { 0xA08D, "_DebugUtil" },
    { 0xA08F, "_DeferUserFn" },
    { 0xA090, "_SysEnvirons" },
    { 0xA091, "_Translate24To32" },
    { 0xA092, "_EgretDispatch" },
    { 0xA09F, "_PowerDispatch" },
    { 0xA0A4, "_HeapDispatch" },
    { 0xA0AE, "_VADBProc" },
    { 0xA0DD, "_PPC" },
    { 0xA0FE, "_TEFindWord" },
    { 0xA0FF, "_TEFindLine" },
    { 0xA11A, "_GetZone" },
    { 0xA11D, "_MaxMem" },
    { 0xA11E, "_NewPtr" },
    { 0xA122, "_NewHandle" },
    { 0xA126, "_HandleZone" },
    { 0xA128, "_RecoverHandle" },
    { 0xA12F, "_PPostEvent" },
    { 0xA146, "_GetTrapAddress" },
    { 0xA148, "_PtrZone" },
    { 0xA15C, "_MemoryDispatchA0Result" },
    { 0xA162, "_PurgeSpace" },
    { 0xA166, "_NewEmptyHandle" },
    { 0xA193, "_Microseconds" },
    { 0xA198, "_HWPriv" },
    { 0xA1AD, "_Gestalt" },
    { 0xA200, "_HOpen" },
    { 0xA204, "_ControlImmed" },
    { 0xA207, "_HGetVInfo" },
    { 0xA208, "_HCreate" },
    { 0xA209, "_HDelete" },
    { 0xA20A, "_HOpenRF" },
    { 0xA20B, "_HRename" },
    { 0xA20C, "_HGetFileInfo" },
    { 0xA20D, "_HSetFileInfo" },
    { 0xA20E, "_HUnmountVol" },
    { 0xA210, "_AllocContig" },
    { 0xA214, "_HGetVol" },
    { 0xA215, "_HSetVol" },
    { 0xA22E, "_BlockMoveData" },
    { 0xA23C, "_CmpStringMarks" },
    { 0xA241, "_SetFilLock" },
    { 0xA242, "_RstFilLock" },
    { 0xA247, "_SetOSTrapAddress" },
    { 0xA254, "_UprStringMarks" },
    { 0xA256, "_StripText" },
    { 0xA260, "_HFSDispatch" },
    { 0xA285, "_IdleUpdate" },
    { 0xA28A, "_SleepQInstall" },
    { 0xA31E, "_NewPtrClear" },
    { 0xA322, "_NewHandleClear" },
    { 0xA346, "_GetOSTrapAddress" },
    { 0xA3AD, "_NewGestalt" },
    { 0xA402, "_ReadAsync" },
    { 0xA403, "_WriteAsync" },
    { 0xA404, "_ControlAsync" },
    { 0xA41C, "_FreeMemSys" },
    { 0xA43C, "_CmpStringCase" },
    { 0xA43D, "_DrvrInstallResrvMem" },
    { 0xA440, "_ResrvMemSys" },
    { 0xA44D, "_PurgeMemSys" },
    { 0xA456, "_UpperText" },
    { 0xA458, "_InsXTime" },
    { 0xA461, "_MaxBlockSys" },
    { 0xA485, "_IdleState" },
    { 0xA48A, "_SleepQRemove" },
    { 0xA51E, "_NewPtrSys" },
    { 0xA522, "_NewHandleSys" },
    { 0xA53D, "_DrvrInstallResrvMemA0Result" },
    { 0xA562, "_PurgeSpaceSys" },
    { 0xA5AD, "_ReplaceGestalt" },
    { 0xA63C, "_CmpStringCaseMarks" },
    { 0xA647, "_SetToolBoxTrapAddress" },
    { 0xA656, "_StripUpperText" },
    { 0xA685, "_SerialPower" },
    { 0xA71E, "_NewPtrSysClear" },
    { 0xA722, "_NewHandleSysClear" },
    { 0xA746, "_GetToolBoxTrapAddress" },
    { 0xA7AD, "_GetGestaltProcPtr" },
    { 0xA800, "_SoundDispatch" },
    { 0xA801, "_SndDisposeChannel" },
    { 0xA802, "_SndAddModifier" },
    { 0xA803, "_SndDoCommand" },
    { 0xA804, "_SndDoImmediate" },
    { 0xA805, "_SndPlay" },
    { 0xA806, "_SndControl" },
    { 0xA807, "_SndNewChannel" },
    { 0xA808, "_InitProcMenu" },
    { 0xA809, "_GetControlVariant" },
    { 0xA80A, "_GetWVariant" },
    { 0xA80B, "_PopUpMenuSelect" },
    { 0xA80C, "_RGetResource" },
    { 0xA80D, "_Count1Resources" },
    { 0xA80E, "_Get1IxResource" },
    { 0xA80F, "_Get1IxType" },
    { 0xA810, "_Unique1ID" },
    { 0xA811, "_TESelView" },
    { 0xA812, "_TEPinScroll" },
    { 0xA813, "_TEAutoView" },
    { 0xA814, "_SetFractEnable" },
    { 0xA815, "_SCSIDispatch" },
    { 0xA816, "_Pack8" },
    { 0xA817, "_CopyMask" },
    { 0xA818, "_FixATan2" },
    { 0xA819, "_XMunger" },
    { 0xA81A, "_HOpenResFile" },
    { 0xA81B, "_HCreateResFile" },
    { 0xA81C, "_Count1Types" },
    { 0xA81D, "_InvalMenuBar" },
    { 0xA81F, "_Get1Resource" },
    { 0xA820, "_Get1NamedResource" },
    { 0xA821, "_MaxSizeRsrc" },
    { 0xA822, "_ResourceDispatch" },
    { 0xA823, "_AliasDispatch" },
    { 0xA826, "_InsertMenuItem" },
    { 0xA827, "_HideDialogItem" },
    { 0xA828, "_ShowDialogItem" },
    { 0xA82A, "_ComponentDispatch" },
    { 0xA82B, "_Pack9" },
    { 0xA82C, "_Pack10" },
    { 0xA82D, "_Pack11" },
    { 0xA82E, "_Pack12" },
    { 0xA82F, "_Pack13" },
    { 0xA830, "_Pack14" },
    { 0xA831, "_Pack15" },
    { 0xA833, "_ScrnBitMap" },
    { 0xA834, "_SetFScaleDisable" },
    { 0xA835, "_FontMetrics" },
    { 0xA836, "_GetMaskTable" },
    { 0xA837, "_MeasureText" },
    { 0xA838, "_CalcMask" },
    { 0xA839, "_SeedFill" },
    { 0xA83A, "_ZoomWindow" },
    { 0xA83B, "_TrackBox" },
    { 0xA83C, "_TEGetOffset" },
    { 0xA83D, "_TEDispatch" },
    { 0xA83E, "_TEStyleNew" },
    { 0xA83F, "_Long2Fix" },
    { 0xA840, "_Fix2Long" },
    { 0xA841, "_Fix2Frac" },
    { 0xA842, "_Frac2Fix" },
    { 0xA843, "_Fix2X" },
    { 0xA844, "_X2Fix" },
    { 0xA845, "_Frac2X" },
    { 0xA846, "_X2Frac" },
    { 0xA847, "_FracCos" },
    { 0xA848, "_FracSin" },
    { 0xA849, "_FracSqrt" },
    { 0xA84A, "_FracMul" },
    { 0xA84B, "_FracDiv" },
    { 0xA84D, "_FixDiv" },
    { 0xA84E, "_GetItemCmd" },
    { 0xA84F, "_SetItemCmd" },
    { 0xA850, "_InitCursor" },
    { 0xA851, "_SetCursor" },
    { 0xA852, "_HideCursor" },
    { 0xA853, "_ShowCursor" },
    { 0xA854, "_FontDispatch" },
    { 0xA855, "_ShieldCursor" },
    { 0xA856, "_ObscureCursor" },
    { 0xA857, "_SetAppBase" },
    { 0xA858, "_BitAnd" },
    { 0xA859, "_BitXOr" },
    { 0xA85A, "_BitNot" },
    { 0xA85B, "_BitOr" },
    { 0xA85C, "_BitShift" },
    { 0xA85D, "_BitTst" },
    { 0xA85E, "_BitSet" },
    { 0xA85F, "_BitClr" },
    { 0xA860, "_WaitNextEvent" },
    { 0xA861, "_Random" },
    { 0xA862, "_ForeColor" },
    { 0xA863, "_BackColor" },
    { 0xA864, "_ColorBit" },
    { 0xA865, "_GetPixel" },
    { 0xA866, "_StuffHex" },
    { 0xA867, "_LongMul" },
    { 0xA868, "_FixMul" },
    { 0xA869, "_FixRatio" },
    { 0xA86A, "_HiWord" },
    { 0xA86B, "_LoWord" },
    { 0xA86C, "_FixRound" },
    { 0xA86D, "_InitPort" },
    { 0xA86E, "_InitGraf" },
    { 0xA86F, "_OpenPort" },
    { 0xA870, "_LocalToGlobal" },
    { 0xA871, "_GlobalToLocal" },
    { 0xA872, "_GrafDevice" },
    { 0xA873, "_SetPort" },
    { 0xA874, "_GetPort" },
    { 0xA875, "_SetPortBits" },
    { 0xA876, "_PortSize" },
    { 0xA877, "_MovePortTo" },
    { 0xA878, "_SetOrigin" },
    { 0xA879, "_SetClip" },
    { 0xA87A, "_GetClip" },
    { 0xA87B, "_ClipRect" },
    { 0xA87C, "_BackPat" },
    { 0xA87D, "_ClosePort" },
    { 0xA87E, "_AddPt" },
    { 0xA87F, "_SubPt" },
    { 0xA880, "_SetPt" },
    { 0xA881, "_EqualPt" },
    { 0xA882, "_StdText" },
    { 0xA883, "_DrawChar" },
    { 0xA884, "_DrawString" },
    { 0xA885, "_DrawText" },
    { 0xA886, "_TextWidth" },
    { 0xA887, "_TextFont" },
    { 0xA888, "_TextFace" },
    { 0xA889, "_TextMode" },
    { 0xA88A, "_TextSize" },
    { 0xA88B, "_GetFontInfo" },
    { 0xA88C, "_StringWidth" },
    { 0xA88D, "_CharWidth" },
    { 0xA88E, "_SpaceExtra" },
    { 0xA88F, "_OSDispatch" },
    { 0xA890, "_StdLine" },
    { 0xA891, "_LineTo" },
    { 0xA892, "_Line" },
    { 0xA893, "_MoveTo" },
    { 0xA894, "_Move" },
    { 0xA895, "_ShutDown" },
    { 0xA896, "_HidePen" },
    { 0xA897, "_ShowPen" },
    { 0xA898, "_GetPenState" },
    { 0xA899, "_SetPenState" },
    { 0xA89A, "_GetPen" },
    { 0xA89B, "_PenSize" },
    { 0xA89C, "_PenMode" },
    { 0xA89D, "_PenPat" },
    { 0xA89E, "_PenNormal" },
    { 0xA89F, "_Unimplemented" },
    { 0xA8A0, "_StdRect" },
    { 0xA8A1, "_FrameRect" },
    { 0xA8A2, "_PaintRect" },
    { 0xA8A3, "_EraseRect" },
    { 0xA8A4, "_InvertRect" },
    { 0xA8A5, "_FillRect" },
    { 0xA8A6, "_EqualRect" },
    { 0xA8A7, "_SetRect" },
    { 0xA8A8, "_OffsetRect" },
    { 0xA8A9, "_InsetRect" },
    { 0xA8AA, "_SectRect" },
    { 0xA8AB, "_UnionRect" },
    { 0xA8AC, "_Pt2Rect" },
    { 0xA8AD, "_PtInRect" },
    { 0xA8AE, "_EmptyRect" },
    { 0xA8AF, "_StdRRect" },
    { 0xA8B0, "_FrameRoundRect" },
    { 0xA8B1, "_PaintRoundRect" },
    { 0xA8B2, "_EraseRoundRect" },
    { 0xA8B3, "_InvertRoundRect" },
    { 0xA8B4, "_FillRoundRect" },
    { 0xA8B5, "_ScriptUtil" },
    { 0xA8B6, "_StdOval" },
    { 0xA8B7, "_FrameOval" },
    { 0xA8B8, "_PaintOval" },
    { 0xA8B9, "_EraseOval" },
    { 0xA8BA, "_InvertOval" },
    { 0xA8BB, "_FillOval" },
    { 0xA8BC, "_SlopeFromAngle" },
    { 0xA8BD, "_StdArc" },
    { 0xA8BE, "_FrameArc" },
    { 0xA8BF, "_PaintArc" },
    { 0xA8C0, "_EraseArc" },
    { 0xA8C1, "_InvertArc" },
    { 0xA8C2, "_FillArc" },
    { 0xA8C3, "_PtToAngle" },
    { 0xA8C4, "_AngleFromSlope" },
    { 0xA8C5, "_StdPoly" },
    { 0xA8C6, "_FramePoly" },
    { 0xA8C7, "_PaintPoly" },
    { 0xA8C8, "_ErasePoly" },
    { 0xA8C9, "_InvertPoly" },
    { 0xA8CA, "_FillPoly" },
    { 0xA8CB, "_OpenPoly" },
    { 0xA8CC, "_ClosePoly" },
    { 0xA8CD, "_KillPoly" },
    { 0xA8CE, "_OffsetPoly" },
    { 0xA8CF, "_PackBits" },
    { 0xA8D0, "_UnpackBits" },
    { 0xA8D1, "_StdRgn" },
    { 0xA8D2, "_FrameRgn" },
    { 0xA8D3, "_PaintRgn" },
    { 0xA8D4, "_EraseRgn" },
    { 0xA8D5, "_InvertRgn" },
    { 0xA8D6, "_FillRgn" },
    { 0xA8D7, "_BitMapToRegion" },
    { 0xA8D8, "_NewRgn" },
    { 0xA8D9, "_DisposeRgn" },
    { 0xA8DA, "_OpenRgn" },
    { 0xA8DB, "_CloseRgn" },
    { 0xA8DC, "_CopyRgn" },
    { 0xA8DD, "_SetEmptyRgn" },
    { 0xA8DE, "_SetRectRgn" },
    { 0xA8DF, "_RectRgn" },
    { 0xA8E0, "_OffsetRgn" },
    { 0xA8E1, "_InsetRgn" },
    { 0xA8E2, "_EmptyRgn" },
    { 0xA8E3, "_EqualRgn" },
    { 0xA8E4, "_SectRgn" },
    { 0xA8E5, "_UnionRgn" },
    { 0xA8E6, "_DiffRgn" },
    { 0xA8E7, "_XOrRgn" },
    { 0xA8E8, "_PtInRgn" },
    { 0xA8E9, "_RectInRgn" },
    { 0xA8EA, "_SetStdProcs" },
    { 0xA8EB, "_StdBits" },
    { 0xA8EC, "_CopyBits" },
    { 0xA8ED, "_StdTxMeas" },
    { 0xA8EE, "_StdGetPic" },
    { 0xA8EF, "_ScrollRect" },
    { 0xA8F0, "_StdPutPic" },
    { 0xA8F1, "_StdComment" },
    { 0xA8F2, "_PicComment" },
    { 0xA8F3, "_OpenPicture" },
    { 0xA8F4, "_ClosePicture" },
    { 0xA8F5, "_KillPicture" },
    { 0xA8F6, "_DrawPicture" },
    { 0xA8F7, "_Layout" },
    { 0xA8F8, "_ScalePt" },
    { 0xA8F9, "_MapPt" },
    { 0xA8FA, "_MapRect" },
    { 0xA8FB, "_MapRgn" },
    { 0xA8FC, "_MapPoly" },
    { 0xA8FD, "_PrGlue" },
    { 0xA8FE, "_InitFonts" },
    { 0xA8FF, "_GetFontName" },
    { 0xA900, "_GetFNum" },
    { 0xA901, "_FMSwapFont" },
    { 0xA902, "_RealFont" },
    { 0xA903, "_SetFontLock" },
    { 0xA904, "_DrawGrowIcon" },
    { 0xA905, "_DragGrayRgn" },
    { 0xA906, "_NewString" },
    { 0xA907, "_SetString" },
    { 0xA908, "_ShowHide" },
    { 0xA909, "_CalcVis" },
    { 0xA90A, "_CalcVBehind" },
    { 0xA90B, "_ClipAbove" },
    { 0xA90C, "_PaintOne" },
    { 0xA90D, "_PaintBehind" },
    { 0xA90E, "_SaveOld" },
    { 0xA90F, "_DrawNew" },
    { 0xA910, "_GetWMgrPort" },
    { 0xA911, "_CheckUpDate" },
    { 0xA912, "_InitWindows" },
    { 0xA913, "_NewWindow" },
    { 0xA914, "_DisposeWindow" },
    { 0xA915, "_ShowWindow" },
    { 0xA916, "_HideWindow" },
    { 0xA917, "_GetWRefCon" },
    { 0xA918, "_SetWRefCon" },
    { 0xA919, "_GetWTitle" },
    { 0xA91A, "_SetWTitle" },
    { 0xA91B, "_MoveWindow" },
    { 0xA91C, "_HiliteWindow" },
    { 0xA91D, "_SizeWindow" },
    { 0xA91E, "_TrackGoAway" },
    { 0xA91F, "_SelectWindow" },
    { 0xA920, "_BringToFront" },
    { 0xA921, "_SendBehind" },
    { 0xA922, "_BeginUpDate" },
    { 0xA923, "_EndUpDate" },
    { 0xA924, "_FrontWindow" },
    { 0xA925, "_DragWindow" },
    { 0xA926, "_DragTheRgn" },
    { 0xA927, "_InvalRgn" },
    { 0xA928, "_InvalRect" },
    { 0xA929, "_ValidRgn" },
    { 0xA92A, "_ValidRect" },
    { 0xA92B, "_GrowWindow" },
    { 0xA92C, "_FindWindow" },
    { 0xA92D, "_CloseWindow" },
    { 0xA92E, "_SetWindowPic" },
    { 0xA92F, "_GetWindowPic" },
    { 0xA930, "_InitMenus" },
    { 0xA931, "_NewMenu" },
    { 0xA932, "_DisposeMenu" },
    { 0xA933, "_AppendMenu" },
    { 0xA934, "_ClearMenuBar" },
    { 0xA935, "_InsertMenu" },
    { 0xA936, "_DeleteMenu" },
    { 0xA937, "_DrawMenuBar" },
    { 0xA938, "_HiliteMenu" },
    { 0xA939, "_EnableItem" },
    { 0xA93A, "_DisableItem" },
    { 0xA93B, "_GetMenuBar" },
    { 0xA93C, "_SetMenuBar" },
    { 0xA93D, "_MenuSelect" },
    { 0xA93E, "_MenuKey" },
    { 0xA93F, "_GetItmIcon" },
    { 0xA940, "_SetItmIcon" },
    { 0xA941, "_GetItmStyle" },
    { 0xA942, "_SetItmStyle" },
    { 0xA943, "_GetItmMark" },
    { 0xA944, "_SetItmMark" },
    { 0xA945, "_CheckItem" },
    { 0xA946, "_GetMenuItemText" },
    { 0xA947, "_SetMenuItemText" },
    { 0xA948, "_CalcMenuSize" },
    { 0xA949, "_GetMenuHandle" },
    { 0xA94A, "_SetMenuFlash" },
    { 0xA94B, "_PlotIcon" },
    { 0xA94C, "_FlashMenuBar" },
    { 0xA94D, "_AppendResMenu" },
    { 0xA94E, "_PinRect" },
    { 0xA94F, "_DeltaPoint" },
    { 0xA950, "_CountMItems" },
    { 0xA951, "_InsertResMenu" },
    { 0xA952, "_DeleteMenuItem" },
    { 0xA953, "_UpdtControl" },
    { 0xA954, "_NewControl" },
    { 0xA955, "_DisposeControl" },
    { 0xA956, "_KillControls" },
    { 0xA957, "_ShowControl" },
    { 0xA958, "_HideControl" },
    { 0xA959, "_MoveControl" },
    { 0xA95A, "_GetControlReference" },
    { 0xA95B, "_SetControlReference" },
    { 0xA95C, "_SizeControl" },
    { 0xA95D, "_HiliteControl" },
    { 0xA95E, "_GetControlTitle" },
    { 0xA95F, "_SetControlTitle" },
    { 0xA960, "_GetControlValue" },
    { 0xA961, "_GetControlMinimum" },
    { 0xA962, "_GetControlMaximum" },
    { 0xA963, "_SetControlValue" },
    { 0xA964, "_SetControlMinimum" },
    { 0xA965, "_SetControlMaximum" },
    { 0xA966, "_TestControl" },
    { 0xA967, "_DragControl" },
    { 0xA968, "_TrackControl" },
    { 0xA969, "_DrawControls" },
    { 0xA96A, "_GetControlAction" },
    { 0xA96B, "_SetControlAction" },
    { 0xA96C, "_FindControl" },
    { 0xA96D, "_Draw1Control" },
    { 0xA96E, "_Dequeue" },
    { 0xA96F, "_Enqueue" },
    { 0xA970, "_GetNextEvent" },
    { 0xA971, "_EventAvail" },
    { 0xA972, "_GetMouse" },
    { 0xA973, "_StillDown" },
    { 0xA974, "_Button" },
    { 0xA975, "_TickCount" },
    { 0xA976, "_GetKeys" },
    { 0xA977, "_WaitMouseUp" },
    { 0xA978, "_UpdtDialog" },
    { 0xA979, "_CouldDialog" },
    { 0xA97A, "_FreeDialog" },
    { 0xA97B, "_InitDialogs" },
    { 0xA97C, "_GetNewDialog" },
    { 0xA97D, "_NewDialog" },
    { 0xA97E, "_SelectDialogItemText" },
    { 0xA97F, "_IsDialogEvent" },
    { 0xA980, "_DialogSelect" },
    { 0xA981, "_DrawDialog" },
    { 0xA982, "_CloseDialog" },
    { 0xA983, "_DisposeDialog" },
    { 0xA984, "_FindDialogItem" },
    { 0xA985, "_Alert" },
    { 0xA986, "_StopAlert" },
    { 0xA987, "_NoteAlert" },
    { 0xA988, "_CautionAlert" },
    { 0xA989, "_CouldAlert" },
    { 0xA98A, "_FreeAlert" },
    { 0xA98B, "_ParamText" },
    { 0xA98C, "_ErrorSound" },
    { 0xA98D, "_GetDialogItem" },
    { 0xA98E, "_SetDialogItem" },
    { 0xA98F, "_SetDialogItemText" },
    { 0xA990, "_GetDialogItemText" },
    { 0xA991, "_ModalDialog" },
    { 0xA992, "_DetachResource" },
    { 0xA993, "_SetResPurge" },
    { 0xA994, "_CurResFile" },
    { 0xA995, "_InitResources" },
    { 0xA996, "_RsrcZoneInit" },
    { 0xA997, "_OpenResFile" },
    { 0xA998, "_UseResFile" },
    { 0xA999, "_UpdateResFile" },
    { 0xA99A, "_CloseResFile" },
    { 0xA99B, "_SetResLoad" },
    { 0xA99C, "_CountResources" },
    { 0xA99D, "_GetIndResource" },
    { 0xA99E, "_CountTypes" },
    { 0xA99F, "_GetIndType" },
    { 0xA9A0, "_GetResource" },
    { 0xA9A1, "_GetNamedResource" },
    { 0xA9A2, "_LoadResource" },
    { 0xA9A3, "_ReleaseResource" },
    { 0xA9A4, "_HomeResFile" },
    { 0xA9A5, "_SizeResource" },
    { 0xA9A6, "_GetResAttrs" },
    { 0xA9A7, "_SetResAttrs" },
    { 0xA9A8, "_GetResInfo" },
    { 0xA9A9, "_SetResInfo" },
    { 0xA9AA, "_ChangedResource" },
    { 0xA9AB, "_AddResource" },
    { 0xA9AC, "_AddReference" },
    { 0xA9AD, "_RmveResource" },
    { 0xA9AE, "_RmveReference" },
    { 0xA9AF, "_ResError" },
    { 0xA9B0, "_WriteResource" },
    { 0xA9B1, "_CreateResFile" },
    { 0xA9B2, "_SystemEvent" },
    { 0xA9B3, "_SystemClick" },
    { 0xA9B4, "_SystemTask" },
    { 0xA9B5, "_SystemMenu" },
    { 0xA9B6, "_OpenDeskAcc" },
    { 0xA9B7, "_CloseDeskAcc" },
    { 0xA9B8, "_GetPattern" },
    { 0xA9B9, "_GetCursor" },
    { 0xA9BA, "_GetString" },
    { 0xA9BB, "_GetIcon" },
    { 0xA9BC, "_GetPicture" },
    { 0xA9BD, "_GetNewWindow" },
    { 0xA9BE, "_GetNewControl" },
    { 0xA9BF, "_GetRMenu" },
    { 0xA9C0, "_GetNewMBar" },
    { 0xA9C1, "_UniqueID" },
    { 0xA9C2, "_SysEdit" },
    { 0xA9C3, "_KeyTranslate" },
    { 0xA9C4, "_OpenRFPerm" },
    { 0xA9C5, "_RsrcMapEntry" },
    { 0xA9C6, "_SecondsToDate" },
    { 0xA9C7, "_DateToSeconds" },
    { 0xA9C8, "_SysBeep" },
    { 0xA9C9, "_SysError" },
    { 0xA9CA, "_PutIcon" },
    { 0xA9CB, "_TEGetText" },
    { 0xA9CC, "_TEInit" },
    { 0xA9CD, "_TEDispose" },
    { 0xA9CE, "_TETextBox" },
    { 0xA9CF, "_TESetText" },
    { 0xA9D0, "_TECalText" },
    { 0xA9D1, "_TESetSelect" },
    { 0xA9D2, "_TENew" },
    { 0xA9D3, "_TEUpdate" },
    { 0xA9D4, "_TEClick" },
    { 0xA9D5, "_TECopy" },
    { 0xA9D6, "_TECut" },
    { 0xA9D7, "_TEDelete" },
    { 0xA9D8, "_TEActivate" },
    { 0xA9D9, "_TEDeactivate" },
    { 0xA9DA, "_TEIdle" },
    { 0xA9DB, "_TEPaste" },
    { 0xA9DC, "_TEKey" },
    { 0xA9DD, "_TEScroll" },
    { 0xA9DE, "_TEInsert" },
    { 0xA9DF, "_TESetAlignment" },
    { 0xA9E0, "_Munger" },
    { 0xA9E1, "_HandToHand" },
    { 0xA9E2, "_PtrToXHand" },
    { 0xA9E3, "_PtrToHand" },
    { 0xA9E4, "_HandAndHand" },
    { 0xA9E5, "_InitPack" },
    { 0xA9E6, "_InitAllPacks" },
    { 0xA9E7, "_Pack0" },
    { 0xA9E8, "_Pack1" },
    { 0xA9E9, "_Pack2" },
    { 0xA9EA, "_Pack3" },
    { 0xA9EB, "_FP68K" },
    { 0xA9EC, "_Elems68K" },
    { 0xA9ED, "_Pack6" },
    { 0xA9EE, "_DECSTR68K" },
    { 0xA9EF, "_PtrAndHand" },
    { 0xA9F0, "_LoadSeg" },
    { 0xA9F1, "_UnLoadSeg" },
    { 0xA9F2, "_Launch" },
    { 0xA9F3, "_Chain" },
    { 0xA9F4, "_ExitToShell" },
    { 0xA9F5, "_GetAppParms" },
    { 0xA9F6, "_GetResFileAttrs" },
    { 0xA9F7, "_SetResFileAttrs" },
    { 0xA9F8, "_MethodDispatch" },
    { 0xA9F9, "_InfoScrap" },
    { 0xA9FA, "_UnloadScrap" },
    { 0xA9FB, "_LoadScrap" },
    { 0xA9FC, "_ZeroScrap" },
    { 0xA9FD, "_GetScrap" },
    { 0xA9FE, "_PutScrap" },
    { 0xA9FF, "_Debugger" },
    { 0xAA00, "_OpenCPort" },
    { 0xAA01, "_InitCPort" },
    { 0xAA02, "_CloseCPort" },
    { 0xAA03, "_NewPixMap" },
    { 0xAA04, "_DisposePixMap" },
    { 0xAA05, "_CopyPixMap" },
    { 0xAA06, "_SetPortPix" },
    { 0xAA07, "_NewPixPat" },
    { 0xAA08, "_DisposePixPat" },
    { 0xAA09, "_CopyPixPat" },
    { 0xAA0A, "_PenPixPat" },
    { 0xAA0B, "_BackPixPat" },
    { 0xAA0C, "_GetPixPat" },
    { 0xAA0D, "_MakeRGBPat" },
    { 0xAA0E, "_FillCRect" },
    { 0xAA0F, "_FillCOval" },
    { 0xAA10, "_FillCRoundRect" },
    { 0xAA11, "_FillCArc" },
    { 0xAA12, "_FillCRgn" },
    { 0xAA13, "_FillCPoly" },
    { 0xAA14, "_RGBForeColor" },
    { 0xAA15, "_RGBBackColor" },
    { 0xAA16, "_SetCPixel" },
    { 0xAA17, "_GetCPixel" },
    { 0xAA18, "_GetCTable" },
    { 0xAA19, "_GetForeColor" },
    { 0xAA1A, "_GetBackColor" },
    { 0xAA1B, "_GetCCursor" },
    { 0xAA1C, "_SetCCursor" },
    { 0xAA1D, "_AllocCursor" },
    { 0xAA1E, "_GetCIcon" },
    { 0xAA1F, "_PlotCIcon" },
    { 0xAA20, "_OpenCPicture" },
    { 0xAA21, "_OpColor" },
    { 0xAA22, "_HiliteColor" },
    { 0xAA23, "_CharExtra" },
    { 0xAA24, "_DisposeCTable" },
    { 0xAA25, "_DisposeCIcon" },
    { 0xAA26, "_DisposeCCursor" },
    { 0xAA27, "_GetMaxDevice" },
    { 0xAA28, "_GetCTSeed" },
    { 0xAA29, "_GetDeviceList" },
    { 0xAA2A, "_GetMainDevice" },
    { 0xAA2B, "_GetNextDevice" },
    { 0xAA2C, "_TestDeviceAttribute" },
    { 0xAA2D, "_SetDeviceAttribute" },
    { 0xAA2E, "_InitGDevice" },
    { 0xAA2F, "_NewGDevice" },
    { 0xAA30, "_DisposeGDevice" },
    { 0xAA31, "_SetGDevice" },
    { 0xAA32, "_GetGDevice" },
    { 0xAA33, "_Color2Index" },
    { 0xAA34, "_Index2Color" },
    { 0xAA35, "_InvertColor" },
    { 0xAA36, "_RealColor" },
    { 0xAA37, "_GetSubTable" },
    { 0xAA38, "_UpdatePixMap" },
    { 0xAA39, "_MakeITable" },
    { 0xAA3A, "_AddSearch" },
    { 0xAA3B, "_AddComp" },
    { 0xAA3C, "_SetClientID" },
    { 0xAA3D, "_ProtectEntry" },
    { 0xAA3E, "_ReserveEntry" },
    { 0xAA3F, "_SetEntries" },
    { 0xAA40, "_QDError" },
    { 0xAA41, "_SetWinColor" },
    { 0xAA42, "_GetAuxWin" },
    { 0xAA43, "_SetControlColor" },
    { 0xAA44, "_GetAuxiliaryControlRecord" },
    { 0xAA45, "_NewCWindow" },
    { 0xAA46, "_GetNewCWindow" },
    { 0xAA47, "_SetDeskCPat" },
    { 0xAA48, "_GetCWMgrPort" },
    { 0xAA49, "_SaveEntries" },
    { 0xAA4A, "_RestoreEntries" },
    { 0xAA4B, "_NewColorDialog" },
    { 0xAA4C, "_DelSearch" },
    { 0xAA4D, "_DelComp" },
    { 0xAA4E, "_SetStdCProcs" },
    { 0xAA4F, "_CalcCMask" },
    { 0xAA50, "_SeedCFill" },
    { 0xAA51, "_CopyDeepMask" },
    { 0xAA52, "_HighLevelFSDispatch" },
    { 0xAA53, "_DictionaryDispatch" },
    { 0xAA54, "_TextServicesDispatch" },
    { 0xAA57, "_DockingDispatch" },
    { 0xAA59, "_MixedModeDispatch" },
    { 0xAA5A, "_CodeFragmentDispatch" },
    { 0xAA60, "_DeleteMCEntries" },
    { 0xAA61, "_GetMCInfo" },
    { 0xAA62, "_SetMCInfo" },
    { 0xAA63, "_DisposeMCInfo" },
    { 0xAA64, "_GetMCEntry" },
    { 0xAA65, "_SetMCEntries" },
    { 0xAA66, "_MenuChoice" },
    { 0xAA68, "_DialogDispatch" },
    { 0xAA6E, "???" },
    { 0xAA90, "_InitPalettes" },
    { 0xAA91, "_NewPalette" },
    { 0xAA92, "_GetNewPalette" },
    { 0xAA93, "_DisposePalette" },
    { 0xAA94, "_ActivatePalette" },
    { 0xAA95, "_SetPalette" },
    { 0xAA96, "_GetPalette" },
    { 0xAA97, "_PmForeColor" },
    { 0xAA98, "_PmBackColor" },
    { 0xAA99, "_AnimateEntry" },
    { 0xAA9A, "_AnimatePalette" },
    { 0xAA9B, "_GetEntryColor" },
    { 0xAA9C, "_SetEntryColor" },
    { 0xAA9D, "_GetEntryUsage" },
    { 0xAA9E, "_SetEntryUsage" },
    { 0xAA9F, "_CTab2Palette" },
    { 0xAAA0, "_Palette2CTab" },
    { 0xAAA1, "_CopyPalette" },
    { 0xAAA2, "_PaletteDispatch" },
    { 0xAADB, "_CursorDeviceDispatch" },
    { 0xAB1D, "_QDExtensions" },
    { 0xABC3, "_NQDMisc" },
    { 0xABC9, "_IconDispatch" },
    { 0xABCA, "_DeviceLoop" },
    { 0xABEB, "_DisplayDispatch" },
    { 0xABF2, "_ThreadDispatch" },
    { 0xABF8, "_StdOpcodeProc" },
    { 0xABFC, "_TranslationDispatch" },
    { 0xABFF, "_DebugStr" },
    { 0xAFFC, "_MOSRUN_Exit" },
    { 0xAFFD, "_MOSRUN_Dispatch" },
    { 0xAFFE, "_MOSRUN_Breakpoint" },
    { 0xAFFF, "_MOSRUN_Native" },
    { 0, 0 }
};


/**
 * Return the name of a trap.
 */
const char *trapName(unsigned int id)
{
    static char buf[64];
    if ((id&0xf000)!=0xa000) {
        return "NOT_A_VALID_TRAP";
    }
    Traps *t = trapLUT;
    for (;;) {
        if (t->id == 0)
            break;
        if (t->id == id)
            return t->name;
        t++;
    }
    if ( (id&0xf800)==0xa800) {
        // 1010.1a0x.xxxx.xxxx: Toolbox call: x = trap #, if a is set, pop the extra return address from the stack
        for (t=trapLUT;;) {
            if (t->id == 0)
                break;
            if (t->id == (id^0x0400)) {
                snprintf(buf, 64, "%s (POP)", t->name);
                return buf;
            }
            t++;
        }
    } else {
        // 1010.0ffa.xxxx.xxxx: OS call: x = trap #, ff are extra flags that can be used by the traps
        for (t=trapLUT;;) {
            if (t->id == 0)
                break;
            if ((t->id&0xf8ff) == (id&0xf8ff)) {
                snprintf(buf, 64, "%s (+flags)", t->name);
                return buf;
            }
            t++;
        }
    }

    return "Unknown";
}



typedef struct {
    const char *name;
    unsigned int id;
    const char *comment;
} GVars;



GVars gvarLUT[] = {
    { "ResetSP", 0x00, "[EXCEPTION VECTOR]  (long) reset initial interrupt SP" },
    { "ResetPC", 0x04, "[EXCEPTION VECTOR]  (long) reset initial Program counter" },
    { "BusErrVct", 0x08, "[EXCEPTION VECTOR]  (long) bus error vector" },
    { "AddrVector", 0x0C, "[EXCEPTION VECTOR]  (long) address error" },
    { "IlglVector", 0x10, "[EXCEPTION VECTOR]  (long) illegal instruction" },
    { "ZeroVector", 0x14, "[EXCEPTION VECTOR]  (long) divide by 0" },
    { "CheckVector", 0x18, "[EXCEPTION VECTOR]  (long) check bounds error" },
    { "TrapVVector", 0x1C, "[EXCEPTION VECTOR]  (long) trapv location" },
    { "PrivlgVector", 0x20, "[EXCEPTION VECTOR]  (long) privilege violation" },
    { "TraceVector", 0x24, "[EXCEPTION VECTOR]  (long) trace exception vector" },
    { "Line1010", 0x28, "[EXCEPTION VECTOR]  (long) 1010 emulator trap (system routines)" },
    { "Line1111", 0x2C, "[EXCEPTION VECTOR]  (long) 1111 emulator trap (reserved)" },
    { "DebugVector", 0x2C, "[EXCEPTION VECTOR]  (long) debug instructions" },
    { "FmtErrVect", 0x38, "[EXCEPTION VECTOR]  (long) format error vector for 68010/68020" },
    { "BadIntVector", 0x60, "[EXCEPTION VECTOR]  (long) spurious interrupt" },
    { "AutoInt1", 0x64, "[EXCEPTION VECTOR]  (long) level 1 auto-vector" },
    { "AutoInt2", 0x68, "[EXCEPTION VECTOR]  (long) level 2 auto-vector" },
    { "AutoInt3", 0x6C, "[EXCEPTION VECTOR]  (long) level 3 auto-vector" },
    { "AutoInt4", 0x70, "[EXCEPTION VECTOR]  (long) level 4 auto-vector" },
    { "AutoInt5", 0x74, "[EXCEPTION VECTOR]  (long) level 5 auto-vector" },
    { "AutoInt6", 0x78, "[EXCEPTION VECTOR]  (long) level 6 auto-vector" },
    { "AutoInt7", 0x7C, "[EXCEPTION VECTOR]  (long) level 7 auto-vector" },
    { "BkptVector", 0xB8, "[EXCEPTION VECTOR]  (long) break loc" },
    { "BSUNVector", 0xC0, "[EXCEPTION VECTOR]  (long) BSUN for FP Emulation Code" },
    { "UNFLVector", 0xCC, "[EXCEPTION VECTOR]  (long) Underflow for FP Emulation Code" },
    { "OPERRVector", 0xD0, "[EXCEPTION VECTOR]  (long) Operand Error for FP Emulation Code" },
    { "OVFLVector", 0xD4, "[EXCEPTION VECTOR]  (long) Overflow for FP Emulation Code" },
    { "SNANVector", 0xD8, "[EXCEPTION VECTOR]  (long) Signal NAN for FP Emulation Code" },
    { "UNIMPDTVector", 0xDC, "[EXCEPTION VECTOR]  (long) 040 Unimplemented Data Type for FP Emulation Code" },
    { "PCDeskPat", 0x20B, "[GLOBAL VAR]  desktop pat, top bit only! others are in use" },
    { "HiKeyLast", 0x216, "[GLOBAL VAR]  Same as KbdVars" },
    { "KbdLast", 0x218, "[GLOBAL VAR]  Same as KbdVars+2" },
    { "ExpandMem", 0x2B6, "[GLOBAL VAR]  pointer to expanded memory block" },
    { "SCSIBase", 0x0C00, "[GLOBAL VAR]  (long) base address for SCSI chip read" },
    { "SCSI2Base", 0x1FF0, "[GLOBAL VAR]  (long) base address for 2nd SCSI chip read" },
    { "SCSIDMA", 0x0C04, "[GLOBAL VAR]  (long) base address for SCSI DMA" },
    { "SCSIHsk", 0x0C08, "[GLOBAL VAR]  (long) base address for SCSI handshake" },
    { "SCSIGlobals", 0x0C0C, "[GLOBAL VAR]  (long) ptr for SCSI mgr locals" },
    { "RGBBlack", 0x0C10, "[GLOBAL VAR]  (6 bytes) the black field for color" },
    { "RGBWhite", 0x0C16, "[GLOBAL VAR]  (6 bytes) the white field for color" },
    { "RowBits", 0x0C20, "[GLOBAL VAR]  (word) screen horizontal pixels" },
    { "ColLines", 0x0C22, "[GLOBAL VAR]  (word) screen vertical pixels" },
    { "ScreenBytes", 0x0C24, "[GLOBAL VAR]  (long) total screen bytes" },
    { "NMIFlag", 0x0C2C, "[GLOBAL VAR]  (byte) flag for NMI debounce" },
    { "VidType", 0x0C2D, "[GLOBAL VAR]  (byte) video board type ID" },
    { "VidMode", 0x0C2E, "[GLOBAL VAR]  (byte) video mode (4=4bit color)" },
    { "SCSIPoll", 0x0C2F, "[GLOBAL VAR]  (byte) poll for device zero only once." },
    { "SEVarBase", 0x0C30, "[GLOBAL VAR] " },
    { "MMUFlags", 0x0CB0, "[GLOBAL VAR]  (byte) cleared to zero (reserved for future use)" },
    { "MMUType", 0x0CB1, "[GLOBAL VAR]  (byte) kind of MMU present" },
    { "MMU32bit", 0x0CB2, "[GLOBAL VAR]  (byte) boolean reflecting current machine MMU mode" },
    { "MMUFluff", 0x0CB3, "[GLOBAL VAR]  (byte) fluff byte forced by reducing MMUMode to MMU32bit." },
    { "MMUTbl", 0x0CB4, "[GLOBAL VAR]  (long) pointer to MMU Mapping table" },
    { "MMUTblSize", 0x0CB8, "[GLOBAL VAR]  (long) size of the MMU mapping table" },
    { "SInfoPtr", 0x0CBC, "[GLOBAL VAR]  (long) pointer to Slot manager information" },
    { "ASCBase", 0x0CC0, "[GLOBAL VAR]  (long) pointer to Sound Chip" },
    { "SMGlobals", 0x0CC4, " (long) pointer to Sound Manager Globals" },
    { "TheGDevice", 0x0CC8, "[GLOBAL VAR]  (long) the current graphics device" },
    { "CQDGlobals", 0x0CCC, " (long) quickDraw global extensions" },
    { "ADBBase", 0x0CF8, "[GLOBAL VAR]  (long) pointer to Front Desk Buss Variables" },
    { "WarmStart", 0x0CFC, "[GLOBAL VAR]  (long) flag to indicate it is a warm start" },
    { "TimeDBRA", 0x0D00, "[GLOBAL VAR]  (word) number of iterations of DBRA per millisecond" },
    { "TimeSCCDB", 0x0D02, "[GLOBAL VAR]  (word) number of iter's of SCC access & DBRA." },
    { "SlotQDT", 0x0D04, "[GLOBAL VAR]  ptr to slot queue table" },
    { "SlotPrTbl", 0x0D08, "[GLOBAL VAR]  ptr to slot priority table" },
    { "SlotVBLQ", 0x0D0C, "[GLOBAL VAR]  ptr to slot VBL queue table" },
    { "ScrnVBLPtr", 0x0D10, "[GLOBAL VAR]  save for ptr to main screen VBL queue" },
    { "SlotTICKS", 0x0D14, "[GLOBAL VAR]  ptr to slot tickcount table" },
    { "TableSeed", 0x0D20, "[GLOBAL VAR]  (long) seed value for color table ID's" },
    { "SRsrcTblPtr", 0x0D24, "[GLOBAL VAR]  (long) pointer to slot resource table." },
    { "JVBLTask", 0x0D28, "[GLOBAL VAR]  vector to slot VBL task interrupt handler" },
    { "WMgrCPort", 0x0D2C, "[GLOBAL VAR]  window manager color port " },
    { "VertRRate", 0x0D30, "[GLOBAL VAR]  (word) Vertical refresh rate for start manager. " },
    { "ChunkyDepth", 0x0D60, "[GLOBAL VAR]  depth of the pixels" },
    { "CrsrPtr", 0x0D62, "[GLOBAL VAR]  pointer to cursor save area" },
    { "PortList", 0x0D66, "[GLOBAL VAR]  list of grafports" },
    { "MickeyBytes", 0x0D6A, "[GLOBAL VAR]  long pointer to cursor stuff" },
    { "QDErrLM", 0x0D6E, "[GLOBAL VAR] QDErr has name conflict w/ type. QuickDraw error code [word]" },
    { "VIA2DT", 0x0D70, "[GLOBAL VAR]  32 bytes for VIA2 dispatch table for NuMac" },
    { "SInitFlags", 0x0D90, "[GLOBAL VAR]  StartInit.a flags [word]" },
    { "DTQueue", 0x0D92, "[GLOBAL VAR]  (10 bytes) deferred task queue header" },
    { "DTQFlags", 0x0D92, "[GLOBAL VAR]  flag word for DTQueue" },
    { "DTskQHdr", 0x0D94, "[GLOBAL VAR]  ptr to head of queue" },
    { "DTskQTail", 0x0D98, "[GLOBAL VAR]  ptr to tail of queue" },
    { "JDTInstall", 0x0D9C, "[GLOBAL VAR]  (long) ptr to deferred task install routine" },
    { "HiliteRGB", 0x0DA0, "[GLOBAL VAR]  6 bytes: rgb of hilite color" },
    { "HWCfgFlags", 0x0B22, "[GLOBAL VAR]  (word) hardware configuration flags" },
    { "TimeSCSIDB", 0x0B24, "[GLOBAL VAR]  (word) number of iter's of SCSI access & DBRA" },
    { "DSCtrAdj", 0x0DA8, "[GLOBAL VAR]  (long) Center adjust for DS rect." },
    { "IconTLAddr", 0x0DAC, "[GLOBAL VAR]  (long) pointer to where start icons are to be put." },
    { "VideoInfoOK", 0x0DB0, "[GLOBAL VAR]  (long) Signals to CritErr that the Video card is ok" },
    { "EndSRTPtr", 0x0DB4, "[GLOBAL VAR]  (long) Pointer to the end of the Slot Resource Table (Not the SRT buffer)." },
    { "SDMJmpTblPtr", 0x0DB8, "[GLOBAL VAR]  (long) Pointer to the SDM jump table" },
    { "JSwapMMU", 0x0DBC, "[GLOBAL VAR]  (long) jump vector to SwapMMU routine" },
    { "SdmBusErr", 0x0DC0, "[GLOBAL VAR]  (long) Pointer to the SDM busErr handler" },
    { "LastTxGDevice", 0x0DC4, "[GLOBAL VAR]  (long) copy of TheGDevice set up for fast text measure" },
    { "NewCrsrJTbl", 0x88C, "[GLOBAL VAR]  location of new crsr jump vectors" },
    { "JAllocCrsr", 0x88C, "[GLOBAL VAR]  (long) vector to routine that allocates cursor" },
    { "JSetCCrsr", 0x890, "[GLOBAL VAR]  (long) vector to routine that sets color cursor" },
    { "JOpcodeProc", 0x894, "[GLOBAL VAR]  (long) vector to process new picture opcodes" },
    { "CrsrBase", 0x898, "[GLOBAL VAR]  (long) scrnBase for cursor" },
    { "CrsrDevice", 0x89C, "[GLOBAL VAR]  (long) current cursor device" },
    { "SrcDevice", 0x8A0, "[GLOBAL VAR]  (LONG) Src device for Stretchbits" },
    { "MainDevice", 0x8A4, "[GLOBAL VAR]  (long) the main screen device" },
    { "DeviceList", 0x8A8, "[GLOBAL VAR]  (long) list of display devices" },
    { "CrsrRow", 0x8AC, "[GLOBAL VAR]  (word) rowbytes for current cursor screen" },
    { "QDColors", 0x8B0, "[GLOBAL VAR]  (long) handle to default colors" },
    { "HiliteMode", 0x938, "[GLOBAL VAR]  used for color highlighting" },
    { "RestProc", 0xA8C, "[GLOBAL VAR]  Resume procedure f InitDialogs [pointer]" },
    { "ROM85", 0x28E, "[GLOBAL VAR]  (word) actually high bit - 0 for ROM vers $75 (sic) and later" },
    { "ntsc", 0xB3E, "[GLOBAl VAR] (byte) Decides fate of menubar�s squareness: 0 for square, non-zero for round." },
    { "ROMMapHndl", 0xB06, "[GLOBAL VAR]  (long) handle of ROM resource map" },
    { "ScrVRes", 0x102, "[GLOBAL VAR] Pixels per inch vertically (word) screen vertical dots/inch [word]" },
    { "ScrHRes", 0x104, "[GLOBAL VAR] Pixels per inch horizontally (word) screen horizontal dots/inch [word]" },
    { "ScrnBase", 0x824, "[GLOBAL VAR] Address of main screen buffer Screen Base [pointer]" },
    { "ScreenRow", 0x106, "[GLOBAL VAR]  rowBytes of screen [word]" },
    { "MBTicks", 0x16E, "[GLOBAL VAR]  tick count @ last mouse button [long]" },
    { "JKybdTask", 0x21A, "[GLOBAL VAR]  keyboard VBL task hook [pointer]" },
    { "KeyLast", 0x184, "[GLOBAL VAR]  ASCII for last valid keycode [word]" },
    { "KeyTime", 0x186, "[GLOBAL VAR]  tickcount when KEYLAST was rec'd [long]" },
    { "KeyRepTime", 0x18A, "[GLOBAL VAR]  tickcount when key was last repeated [long]" },
    { "SPConfig", 0x1FB, "[GLOBAL VAR] Use types for serial ports (byte) config bits: 4-7 A, 0-3 B (see use type below)" },
    { "SPPortA", 0x1FC, "[GLOBAL VAR] Modem port configuration (word) SCC port A configuration [word]" },
    { "SPPortB", 0x1FE, "[GLOBAL VAR] Printer port configuration (word) SCC port B configuration [word]" },
    { "SCCRd", 0x1D8, "[GLOBAL VAR] SCC read base address SCC base read address [pointer]" },
    { "SCCWr", 0x1DC, "[GLOBAL VAR] SCC write base address SCC base write address [pointer]" },
    { "DoubleTime", 0x2F0, "[GLOBAL VAR] Double-click interval in ticks (long) double click ticks [long]" },
    { "CaretTime", 0x2F4, "[GLOBAL VAR] Caret-blink interval in ticks (long) caret blink ticks [long]" },
    { "KeyThresh", 0x18E, "[GLOBAL VAR] Auto-key threshold (word) threshold for key repeat [word]" },
    { "KeyRepThresh", 0x190, "[GLOBAL VAR] Auto-key rate (word) key repeat speed [word]" },
    { "SdVolume", 0x260, "[GLOBAL VAR] Current speaker volume (byte:  low-order three bits only) Global volume(sound) control [byte]" },
    { "Ticks", 0x16A, "[GLOBAL VAR] Current number of ticks since system startup (long) Tick count, time since boot [unsigned long]" },
    { "TimeLM", 0x20C, "[GLOBAL VAR] Time has name conflict w/ type. Clock time (extrapolated) [long]" },
    { "MonkeyLives", 0x100, "[GLOBAL VAR]  monkey lives if >= 0 [word]" },
    { "SEvtEnb", 0x15C, "[GLOBAL VAR] 0 if SystemEvent should return FALSE (byte) enable SysEvent calls from GNE [byte]" },
    { "JournalFlag", 0x8DE, "[GLOBAL VAR] Journaling mode (word) journaling state [word]" },
    { "JournalRef", 0x8E8, "[GLOBAL VAR] Reference number of journaling device driver (word) Journalling driver's refnum [word]" },
    { "BufPtr", 0x10C, "[GLOBAL VAR] Address of end of jump table top of application memory [pointer]" },
    { "StkLowPt", 0x110, "[GLOBAL VAR]  Lowest stack as measured in VBL task [pointer]" },
    { "TheZone", 0x118, "[GLOBAL VAR] Address of current heap zone current heap zone [pointer]" },
    { "ApplLimit", 0x130, "[GLOBAL VAR] Application heap limit application limit [pointer]" },
    { "SysZone", 0x2A6, "[GLOBAL VAR] Address of system heap zone system heap zone [pointer]" },
    { "ApplZone", 0x2AA, "[GLOBAL VAR] Address of application heap zone application heap zone [pointer]" },
    { "HeapEnd", 0x114, "[GLOBAL VAR] Address of end of application heap zone end of heap [pointer]" },
    { "HiHeapMark", 0xBAE, "[GLOBAL VAR]  (long) highest address used by a zone below sp<01Nov85 JTC>" },
    { "MemErr", 0x220, "[GLOBAL VAR]  last memory manager error [word]" },
    { "UTableBase", 0x11C, "[GLOBAL VAR] Base address of unit table unit I/O table [pointer]" },
    { "UnitNtryCnt", 0x1D2, "[GLOBAL VAR]  count of entries in unit table [word]" },
    { "JFetch", 0x8F4, "[GLOBAL VAR] Jump vector for Fetch function fetch a byte routine for drivers [pointer]" },
    { "JStash", 0x8F8, "[GLOBAL VAR] Jump vector for Stash function stash a byte routine for drivers [pointer]" },
    { "JIODone", 0x8FC, "[GLOBAL VAR] Jump vector for IODone function IODone entry location [pointer]" },
    { "DrvQHdr", 0x308, "[GLOBAL VAR] Drive queue header (10 bytes) queue header of drives in system [10 bytes]" },
    { "BootDrive", 0x210, "[GLOBAL VAR]  drive number of boot drive [word]" },
    { "EjectNotify", 0x338, "[GLOBAL VAR]  eject notify procedure [pointer]" },
    { "IAZNotify", 0x33C, "[GLOBAL VAR]  world swaps notify procedure [pointer]" },
    { "SFSaveDisk", 0x214, "[GLOBAL VAR] Negative of volume reference number used by Standard File Package (word) last vRefNum seen by standard file [word]" },
    { "CurDirStore", 0x398, "[GLOBAL VAR]  save dir across calls to Standard File [long]" },
    { "OneOne", 0xA02, "[GLOBAL VAR] $00010001 constant $00010001 [long]" },
    { "MinusOne", 0xA06, "[GLOBAL VAR] $FFFFFFFF constant $FFFFFFFF [long]" },
    { "Lo3Bytes", 0x31A, "[GLOBAL VAR] $00FFFFFF constant $00FFFFFF [long]" },
    { "ROMBase", 0x2AE, "[GLOBAL VAR] Base address of ROM ROM base address [pointer]" },
    { "RAMBase", 0x2B2, "[GLOBAL VAR] Trap dispatch table's base address for routines in RAM RAM base address [pointer]" },
    { "SysVersion", 0x15A, "[GLOBAL VAR]  version # of RAM-based system [word]" },
    { "RndSeed", 0x156, "[GLOBAL VAR] Random number seed (long) random seed/number [long]" },
    { "Scratch20", 0x1E4, "[GLOBAL VAR] 20-byte scratch area scratch [20 bytes]" },
    { "Scratch8", 0x9FA, "[GLOBAL VAR] 8-byte scratch area scratch [8 bytes]" },
    { "ToolScratch", 0x9CE, "[GLOBAL VAR] 8-byte scratch area scratch [8 bytes]" },
    { "ApplScratch", 0xA78, "[GLOBAL VAR] 12-byte application scratch area scratch [12 bytes]" },
    { "ScrapSize", 0x960, "[GLOBAL VAR] Size in bytes of desk scrap (long) scrap length [long]" },
    { "ScrapHandle", 0x964, "[GLOBAL VAR] Handle to desk scrap in memory memory scrap [handle]" },
    { "ScrapCount", 0x968, "[GLOBAL VAR] Count changed by ZeroScrap (word) validation byte [word]" },
    { "ScrapState", 0x96A, "[GLOBAL VAR] Tells where desk scrap is (word) scrap state [word]" },
    { "ScrapName", 0x96C, "[GLOBAL VAR] Pointer to scrap file name (preceded by length byte) pointer to scrap name [pointer]" },
    { "IntlSpec", 0xBA0, "[GLOBAL VAR]  (long) - ptr to extra Intl data " },
    { "SwitcherTPtr", 0x286, "[GLOBAL VAR]  Switcher's switch table " },
    { "CPUFlag", 0x12F, "[GLOBAL VAR]  $00=68000, $01=68010, $02=68020 (old ROM inits to $00)" },
    { "VIA", 0x1D4, "[GLOBAL VAR] VIA base address VIA base address [pointer]" },
    { "IWM", 0x1E0, "[GLOBAL VAR]  IWM base address [pointer]" },
    { "Lvl1DT", 0x192, "[GLOBAL VAR] Level-1 secondary interrupt vector table (32 bytes) Interrupt level 1 dispatch table [32 bytes]" },
    { "Lvl2DT", 0x1B2, "[GLOBAL VAR] Level-2 secondary interrupt vector table (32 bytes) Interrupt level 2 dispatch table [32 bytes]" },
    { "ExtStsDT", 0x2BE, "[GLOBAL VAR] External/status interrupt vector table (16 bytes) SCC ext/sts secondary dispatch table [16 bytes]" },
    { "SPValid", 0x1F8, "[GLOBAL VAR] Validity status (byte) validation field ($A7) [byte]" },
    { "SPATalkA", 0x1F9, "[GLOBAL VAR] AppleTalk node ID hint for modem port (byte) AppleTalk node number hint for port A" },
    { "SPATalkB", 0x1FA, "[GLOBAL VAR] AppleTalk node ID hint for printer port (byte) AppleTalk node number hint for port B" },
    { "SPAlarm", 0x200, "[GLOBAL VAR] Alarm setting (long) alarm time [long]" },
    { "SPFont", 0x204, "[GLOBAL VAR] Application font number minus 1 (word) default application font number minus 1 [word]" },
    { "SPKbd", 0x206, "[GLOBAL VAR] Auto-key threshold and rate (byte) kbd repeat thresh in 4/60ths [2 4-bit]" },
    { "SPPrint", 0x207, "[GLOBAL VAR] Printer connection (byte) print stuff [byte]" },
    { "SPVolCtl", 0x208, "[GLOBAL VAR] Speaker volume setting in parameter RAM (byte) volume control [byte]" },
    { "SPClikCaret", 0x209, "[GLOBAL VAR] Double-click and caret-blink times (byte) double click/caret time in 4/60ths[2 4-bit]" },
    { "SPMisc1", 0x20A, "[GLOBAL VAR]  miscellaneous [1 byte]" },
    { "SPMisc2", 0x20B, "[GLOBAL VAR] Mouse scaling, system startup disk, menu blink (byte) miscellaneous [1 byte]" },
    { "GetParam", 0x1E4, "[GLOBAL VAR]  system parameter scratch [20 bytes]" },
    { "SysParam", 0x1F8, "[GLOBAL VAR] Low-memory copy of parameter RAM (20 bytes) system parameter memory [20 bytes]" },
    { "CrsrThresh", 0x8EC, "[GLOBAL VAR] Mouse-scaling threshold (word) delta threshold for mouse scaling [word]" },
    { "JCrsrTask", 0x8EE, "[GLOBAL VAR]  address of CrsrVBLTask [long]" },
    { "MTemp", 0x828, "[GLOBAL VAR]  Low-level interrupt mouse location [long]" },
    { "RawMouse", 0x82C, "[GLOBAL VAR]  un-jerked mouse coordinates [long]" },
    { "CrsrRect", 0x83C, "[GLOBAL VAR]  Cursor hit rectangle [8 bytes]" },
    { "TheCrsr", 0x844, "[GLOBAL VAR]  Cursor data, mask & hotspot [68 bytes]" },
    { "CrsrAddr", 0x888, "[GLOBAL VAR]  Address of data under cursor [long]" },
    { "CrsrSave", 0x88C, "[GLOBAL VAR]  data under the cursor [64 bytes]" },
    { "CrsrVis", 0x8CC, "[GLOBAL VAR]  Cursor visible? [byte]" },
    { "CrsrBusy", 0x8CD, "[GLOBAL VAR]  Cursor locked out? [byte]" },
    { "CrsrNew", 0x8CE, "[GLOBAL VAR]  Cursor changed? [byte]" },
    { "CrsrState", 0x8D0, "[GLOBAL VAR]  Cursor nesting level [word]" },
    { "CrsrObscure", 0x8D2, "[GLOBAL VAR]  Cursor obscure semaphore [byte]" },
    { "KbdVars", 0x216, "[GLOBAL VAR]  Keyboard manager variables [4 bytes]" },
    { "KbdType", 0x21E, "[GLOBAL VAR]  keyboard model number [byte]" },
    { "MBState", 0x172, "[GLOBAL VAR]  current mouse button state [byte]" },
    { "KeyMapLM", 0x174, "[GLOBAL VAR] KeyMap has name conflict w/ type. Bitmap of the keyboard [4 longs]" },
    { "KeypadMap", 0x17C, "[GLOBAL VAR]  bitmap for numeric pad-18bits [long]" },
    { "Key1Trans", 0x29E, "[GLOBAL VAR]  keyboard translator procedure [pointer]" },
    { "Key2Trans", 0x2A2, "[GLOBAL VAR]  numeric keypad translator procedure [pointer]" },
    { "JGNEFilter", 0x29A, "[GLOBAL VAR]  GetNextEvent filter proc [pointer]" },
    { "KeyMVars", 0xB04, "[GLOBAL VAR]  (word) for ROM KEYM proc state" },
    { "Mouse", 0x830, "[GLOBAL VAR]  processed mouse coordinate [long]" },
    { "CrsrPin", 0x834, "[GLOBAL VAR]  cursor pinning rectangle [8 bytes]" },
    { "CrsrCouple", 0x8CF, "[GLOBAL VAR]  cursor coupled to mouse? [byte]" },
    { "CrsrScale", 0x8D3, "[GLOBAL VAR]  cursor scaled? [byte]" },
    { "MouseMask", 0x8D6, "[GLOBAL VAR]  V-H mask for ANDing with mouse [long]" },
    { "MouseOffset", 0x8DA, "[GLOBAL VAR]  V-H offset for adding after ANDing [long]" },
    { "AlarmState", 0x21F, "[GLOBAL VAR]  Bit7=parity, Bit6=beeped, Bit0=enable [byte]" },
    { "VBLQueue", 0x160, "[GLOBAL VAR] Vertical retrace queue header (10 bytes) VBL queue header [10 bytes]" },
    { "SysEvtMask", 0x144, "[GLOBAL VAR] System event mask (word) system event mask [word]" },
    { "SysEvtBuf", 0x146, "[GLOBAL VAR]  system event queue element buffer [pointer]" },
    { "EventQueue", 0x14A, "[GLOBAL VAR] Event queue header (10 bytes) event queue header [10 bytes]" },
    { "EvtBufCnt", 0x154, "[GLOBAL VAR]  max number of events in SysEvtBuf - 1 [word]" },
    { "GZRootHnd", 0x328, "[GLOBAL VAR] Handle to relocatable block not to be moved by grow zone function root handle for GrowZone [handle]" },
    { "GZRootPtr", 0x32C, "[GLOBAL VAR]  root pointer for GrowZone [pointer]" },
    { "GZMoveHnd", 0x330, "[GLOBAL VAR]  moving handle for GrowZone [handle]" },
    { "MemTop", 0x108, "[GLOBAL VAR] Address of end of RAM (on Macintosh XL, end of RAM available to applications) top of memory [pointer]" },
    { "MmInOK", 0x12E, "[GLOBAL VAR]  initial memory mgr checks ok? [byte]" },
    //  { "HpChk", 0x316, "[GLOBAL VAR]  heap check RAM code [pointer]" },
    { "MaskBC", 0x31A, "[GLOBAL VAR]  Memory Manager Byte Count Mask [long]" },
    { "MaskHandle", 0x31A, "[GLOBAL VAR]  Memory Manager Handle Mask [long]" },
    { "MaskPtr", 0x31A, "[GLOBAL VAR]  Memory Manager Pointer Mask [long]" },
    { "MinStack", 0x31E, "[GLOBAL VAR] Minimum space allotment for stack (long) min stack size used in InitApplZone [long]" },
    { "DefltStack", 0x322, "[GLOBAL VAR] Default space allotment for stack (long) default size of stack [long]" },
    { "MMDefFlags", 0x326, "[GLOBAL VAR]  default zone flags [word]" },
    { "DSAlertTab", 0x2BA, "[GLOBAL VAR] Pointer to system error alert table in use system error alerts [pointer]" },
    { "DSAlertRect", 0x3F8, "[GLOBAL VAR] Rectangle enclosing system error alert (8 bytes) rectangle for disk-switch alert [8 bytes]" },
    { "DSDrawProc", 0x334, "[GLOBAL VAR]  alternate syserror draw procedure [pointer]" },
    { "DSWndUpdate", 0x15D, "[GLOBAL VAR]  GNE not to paintBehind DS AlertRect? [byte]" },
    { "WWExist", 0x8F2, "[GLOBAL VAR]  window manager initialized? [byte]" },
    { "QDExist", 0x8F3, "[GLOBAL VAR]  quickdraw is initialized [byte]" },
    { "ResumeProc", 0xA8C, "[GLOBAL VAR] Address of resume procedure Resume procedure from InitDialogs [pointer]" },
    { "DSErrCode", 0xAF0, "[GLOBAL VAR] Current system error ID (word) last system error alert ID" },
    { "IntFlag", 0x15F, "[GLOBAL VAR]  reduce interrupt disable time when bit 7 = 0" },
    { "SerialVars", 0x2D0, "[GLOBAL VAR]  async driver variables [16 bytes]" },
    { "ABusVars", 0x2D8, "[GLOBAL VAR] Pointer to AppleTalk variables ;Pointer to AppleTalk local variables" },
    { "ABusDCE", 0x2DC, "[GLOBAL VAR] ;Pointer to AppleTalk DCE" },
    { "PortAUse", 0x290, "[GLOBAL VAR]  bit 7: 1 = not in use, 0 = in use" },
    { "PortBUse", 0x291, "[GLOBAL VAR] Current availability of serial port B (byte) port B use, same format as PortAUse" },
    { "SCCASts", 0x2CE, "[GLOBAL VAR]  SCC read reg 0 last ext/sts rupt - A [byte]" },
    { "SCCBSts", 0x2CF, "[GLOBAL VAR]  SCC read reg 0 last ext/sts rupt - B [byte]" },
    { "DskErr", 0x142, "[GLOBAL VAR]  disk routine result code [word]" },
    { "PWMBuf2", 0x312, "[GLOBAL VAR]  PWM buffer 1 (or 2 if sound) [pointer]" },
    { "SoundPtr", 0x262, "[GLOBAL VAR] Pointer to four-tone record 4VE sound definition table [pointer]" },
    { "SoundBase", 0x266, "[GLOBAL VAR] Pointer to free-form synthesizer buffer sound bitMap [pointer]" },
    { "SoundVBL", 0x26A, "[GLOBAL VAR]  vertical retrace control element [16 bytes]" },
    { "SoundDCE", 0x27A, "[GLOBAL VAR]  sound driver DCE [pointer]" },
    { "SoundActive", 0x27E, "[GLOBAL VAR]  sound is active? [byte]" },
    { "SoundLevel", 0x27F, "[GLOBAL VAR] Amplitude in 740-byte buffer (byte) current level in buffer [byte]" },
    { "CurPitch", 0x280, "[GLOBAL VAR] Value of count in square-wave synthesizer buffer (word) current pitch value [word]" },
    { "DskVerify", 0x12C, "[GLOBAL VAR]  used by 3.5 disk driver for read/verify [byte]" },
    { "TagData", 0x2FA, "[GLOBAL VAR]  sector tag info for disk drivers [14 bytes]" },
    { "BufTgFNum", 0x2FC, "[GLOBAL VAR] File tags buffer:  file number (long) file number [long]" },
    { "BufTgFFlg", 0x300, "[GLOBAL VAR] File tags buffer:  flags (word:  bit 1=1 if resource fork) flags [word]" },
    { "BufTgFBkNum", 0x302, "[GLOBAL VAR] File tags buffer:  logical block number (word) logical block number [word]" },
    { "BufTgDate", 0x304, "[GLOBAL VAR] File tags buffer:  date and time of last modification (long) time stamp [word]" },
    { "ScrDmpEnb", 0x2F8, "[GLOBAL VAR] 0 if GetNextEvent shouldn't process Command-Shift-number combinations (byte) screen dump enabled? [byte]" },
    { "ScrDmpType", 0x2F9, "[GLOBAL VAR]  FF dumps screen, FE dumps front window [byte]" },
    { "ScrapVars", 0x960, "[GLOBAL VAR]  scrap manager variables [32 bytes]" },
    { "ScrapInfo", 0x960, "[GLOBAL VAR]  scrap length [long]" },
    { "ScrapEnd", 0x980, "[GLOBAL VAR]  end of scrap vars" },
    { "ScrapTag", 0x970, "[GLOBAL VAR]  scrap file name [STRING[15]]" },
    { "LaunchFlag", 0x902, "[GLOBAL VAR]  from launch or chain [byte]" },
    { "SaveSegHandle", 0x930, "[GLOBAL VAR]  seg 0 handle [handle]" },
    { "CurJTOffset", 0x934, "[GLOBAL VAR] Offset to jump table from location pointed to by A5 (word) current jump table offset [word]" },
    { "CurPageOption", 0x936, "[GLOBAL VAR] Sound/screen buffer configuration passed to Chain or Launch (word) current page 2 configuration [word]" },
    { "LoaderPBlock", 0x93A, "[GLOBAL VAR]  param block for ExitToShell [10 bytes]" },
    { "CurApRefNum", 0x900, "[GLOBAL VAR] Reference number of current application's resource file (word) refNum of application's resFile [word]" },
    { "CurrentA5", 0x904, "[GLOBAL VAR] Address of boundary between application globals and application parameters current value of A5 [pointer]" },
    { "CurStackBase", 0x908, "[GLOBAL VAR] Address of base of stack; start of application globals current stack base [pointer]" },
    { "CurApName", 0x910, "[GLOBAL VAR] Name of current application (length byte followed by up to 31 characters) name of application [STRING[31]]" },
    { "LoadTrap", 0x12D, "[GLOBAL VAR]  trap before launch? [byte]" },
    { "SegHiEnable", 0xBB2, "[GLOBAL VAR]  (byte) 0 to disable MoveHHi in LoadSeg" },
    { "WindowList", 0x9D6, "[GLOBAL VAR] Pointer to first window in window list; 0 if using events but not windows Z-ordered linked list of windows [pointer]" },
    { "PaintWhite", 0x9DC, "[GLOBAL VAR] Flag for whether to paint window white before update event (word) erase newly drawn windows? [word]" },
    { "WMgrPort", 0x9DE, "[GLOBAL VAR] Pointer to Window Manager port window manager's grafport [pointer]" },
    { "GrayRgn", 0x9EE, "[GLOBAL VAR] Handle to region drawn as desktop rounded gray desk region [handle]" },
    { "CurActivate", 0xA64, "[GLOBAL VAR] Pointer to window to receive activate event window slated for activate event [pointer]" },
    { "CurDeactive", 0xA68, "[GLOBAL VAR] Pointer to window to receive deactivate event window slated for deactivate event [pointer]" },
    { "DragHook", 0x9F6, "[GLOBAL VAR] Address of procedure to execute during TrackGoAway, DragWindow, GrowWindow, DragGrayRgn, TrackControl, and DragControl user hook during dragging [pointer]" },
    { "DeskPattern", 0xA3C, "[GLOBAL VAR] Pattern with which desktop is painted (8 bytes) desk pattern [8 bytes]" },
    { "DeskHook", 0xA6C, "[GLOBAL VAR] Address of procedure for painting desktop or responding to clicks on desktop hook for painting the desk [pointer]" },
    { "GhostWindow", 0xA84, "[GLOBAL VAR] Pointer to window never to be considered frontmost window hidden from FrontWindow [pointer]" },
    { "TEDoText", 0xA70, "[GLOBAL VAR] Address of TextEdit multi-purpose routine textEdit doText proc hook [pointer]" },
    { "TERecal", 0xA74, "[GLOBAL VAR] Address of routine to recalculate line starts for TextEdit textEdit recalText proc hook [pointer]" },
    { "TEScrapLength", 0xAB0, "[GLOBAL VAR] Size in bytes of TextEdit scrap (long) textEdit Scrap Length [word]" },
    { "TEScrpLength", 0xAB0, "[GLOBAL VAR] Size in bytes of TextEdit scrap (long) textEdit Scrap Length [word]" },
    { "TEScrpHandle", 0xAB4, "[GLOBAL VAR] Handle to TextEdit scrap textEdit Scrap [handle]" },
    { "TEWdBreak", 0xAF6, "[GLOBAL VAR] default word break routine [pointer]" },
    { "TEWordBreak", 0xAF6, "[GLOBAL VAR] default word break routine [pointer]" },
    { "WordRedraw", 0xBA5, "[GLOBAL VAR] (byte) - used by TextEdit RecalDraw" },
    { "TESysJust", 0xBAC, "[GLOBAL VAR] (word) system justification (intl. textEdit)       obsolete " },
    { "SysDirection", 0xBAC, "[GLOBAL VAR] (word) system justification (intl. textEdit)" },
    { "TopMapHndl", 0xA50, "[GLOBAL VAR] Handle to resource map of most recently opened resource file topmost map in list [handle]" },
    { "SysMapHndl", 0xA54, "[GLOBAL VAR] Handle to map of system resource file system map [handle]" },
    { "SysMap", 0xA58, "[GLOBAL VAR] Reference number of system resource file (word) reference number of system map [word]" },
    { "CurMap", 0xA5A, "[GLOBAL VAR] Reference number of current resource file (word) reference number of current map [word]" },
    { "ResReadOnly", 0xA5C, "[GLOBAL VAR] Read only flag [word]" },
    { "ResLoad", 0xA5E, "[GLOBAL VAR] Current SetResLoad state (word) Auto-load feature [word]" },
    { "ResErr", 0xA60, "[GLOBAL VAR] Current value of ResError (word) Resource error code [word]" },
    { "ResErrProc", 0xAF2, "[GLOBAL VAR] Address of resource error procedure Resource error procedure [pointer]" },
    { "SysResName", 0xAD8, "[GLOBAL VAR] Name of system resource file (length byte followed by up to 19 characters) Name of system resource file [STRING[19]]" },
    { "RomMapInsert", 0xB9E, "[GLOBAL VAR] (byte) determines if we should link in map" },
    { "TmpResLoad", 0xB9F, "[GLOBAL VAR] second byte is temporary ResLoad value." },
    { "MBarHeight", 0xBAA, "[GLOBAL VAR] height of the menu bar" },
    { "CommToolboxGlobals", 0x0BB4, "[GLOBAL VAR] pointer to CommToolbox globals " },
    // alternative names in case the ones above are not complete
    { "unassigned", 0x0000, nullptr },
    { "ResetSPPC", 0x0004, nullptr },
    { "BusError", 0x0008, nullptr },
    { "AddrErr", 0x000C, nullptr },
    { "Illegal", 0x0010, nullptr },
    { "ZeroDiv", 0x0014, nullptr },
    { "ChkError", 0x0018, nullptr },
    { "TrapVErr", 0x001C, nullptr },
    { "Privileg", 0x0020, nullptr },
    { "Trace", 0x0024, nullptr },
    { "Line1010", 0x0028, nullptr },
    { "Line 1111", 0x002C, nullptr },
    { "unassigned", 0x0030, nullptr },
    { "Coproces", 0x0034, nullptr },
    { "FmtErrVect", 0x0038, nullptr },
    { "Uninited", 0x003C, nullptr },
    { "Unassig2", 0x0040, nullptr },
    { "Spurious", 0x0060, nullptr },
    { "AutoInt1", 0x0064, nullptr },
    { "AutoInt2", 0x0068, nullptr },
    { "AutoInt3", 0x006C, nullptr },
    { "AutoInt4", 0x0070, nullptr },
    { "AutoInt5", 0x0074, nullptr },
    { "AutoInt6", 0x0078, nullptr },
    { "AutoInt7", 0x007C, nullptr },
    { "TRAPtble", 0x0080, nullptr },
    { "FP-68881", 0x00C0, nullptr },
    { "reserved", 0x00DC, nullptr },
    { "PMMU", 0x00E0, nullptr },
    { "SMgrOldCore", 0x00E0, nullptr },
    { "reserved", 0x00EC, nullptr },
    { "MonkeyLives", 0x0100, nullptr },
    { "SysCom", 0x0100, nullptr },
    { "ScrVRes", 0x0102, nullptr },
    { "ScrHRes", 0x0104, nullptr },
    { "ScreenRow", 0x0106, nullptr },
    { "MemTop", 0x0108, nullptr },
    { "BufPtr", 0x010C, nullptr },
    { "StkLowPtr", 0x0110, nullptr },
    { "HeapEnd", 0x0114, nullptr },
    { "TheZone", 0x0118, nullptr },
    { "UTableBase", 0x011C, nullptr },
    { "MacJmp", 0x0120, nullptr },
    { "DskRtnAdr", 0x0124, nullptr },
    { "PollRtnAdr", 0x0128, nullptr },
    { "DskVerify", 0x012C, nullptr },
    { "LoadTrap", 0x012D, nullptr },
    { "MmInOK", 0x012E, nullptr },
    { "CPUFlag", 0x012F, nullptr },
    { "DskWr11", 0x012F, nullptr },
    { "ApplLimit", 0x0130, nullptr },
    { "SonyVars", 0x0134, nullptr },
    { "PWMValue", 0x0138, nullptr },
    { "PollStack", 0x013A, nullptr },
    { "PollProc", 0x013E, nullptr },
    { "DskErr", 0x0142, nullptr },
    { "SysEvtMask", 0x0144, nullptr },
    { "SysEvtBuf", 0x0146, nullptr },
    { "EventQueue", 0x014A, nullptr },
    { "EvtBufCnt", 0x0154, nullptr },
    { "RndSeed", 0x0156, nullptr },
    { "SysVersion", 0x015A, nullptr },
    { "SEvtEnb", 0x015C, nullptr },
    { "DSWndUpdate", 0x015D, nullptr },
    { "FontFlag", 0x015E, nullptr },
    { "IntFlag", 0x015F, nullptr },
    { "VBLQueue", 0x0160, nullptr },
    { "Ticks", 0x016A, nullptr },
    { "MBTicks", 0x016E, nullptr },
    { "MBState", 0x0172, nullptr },
    { "Tocks", 0x0173, nullptr },
    { "KeyMap", 0x0174, nullptr },
    { "KeyPadMap", 0x017C, nullptr },
    { "unknown", 0x0180, nullptr },
    { "KeyLast", 0x0184, nullptr },
    { "KeyTime", 0x0186, nullptr },
    { "KeyRepTime", 0x018A, nullptr },
    { "KeyThresh", 0x018E, nullptr },
    { "KeyRepThresh", 0x0190, nullptr },
    { "Lvl1DT", 0x0192, nullptr },
    { "Lvl2DT", 0x01B2, nullptr },
    { "UnitNtryCnt", 0x01D2, nullptr },
    { "VIA", 0x01D4, nullptr },
    { "SCCRd", 0x01D8, nullptr },
    { "SCCWr", 0x01DC, nullptr },
    { "IWM", 0x01E0, nullptr },
    { "GetParam", 0x01E4, nullptr },
    { "SysParam", 0x01F8, nullptr },
    { "SPValid", 0x01F8, nullptr },
    { "SPATalkA", 0x01F9, nullptr },
    { "SPATalkB", 0x01FA, nullptr },
    { "SPConfig", 0x01FB, nullptr },
    { "SPPortA", 0x01FC, nullptr },
    { "SPPortB", 0x01FE, nullptr },
    { "SPAlarm", 0x0200, nullptr },
    { "SPFont", 0x0204, nullptr },
    { "SPKbd", 0x0206, nullptr },
    { "SPPrint", 0x0207, nullptr },
    { "SPVolCtl", 0x0208, nullptr },
    { "SPClikCaret", 0x0209, nullptr },
    { "SPMisc1", 0x020A, nullptr },
    { "CDeskPat", 0x020B, nullptr },
    { "Time", 0x020C, nullptr },
    { "BootDrive", 0x0210, nullptr },
    { "JShell", 0x0212, nullptr },
    { "SFSaveDisk", 0x0214, nullptr },
    { "KbdVars", 0x0216, nullptr },
    { "KbdLast", 0x0218, nullptr },
    { "JKybdTask", 0x021A, nullptr },
    { "KbdType", 0x021E, nullptr },
    { "AlarmState", 0x021F, nullptr },
    { "MemErr", 0x0220, nullptr },
    { "JFigTrkSpd", 0x0222, nullptr },
    { "JDiskPrime", 0x0226, nullptr },
    { "JRdAddr", 0x022A, nullptr },
    { "JRdData", 0x022E, nullptr },
    { "JWrData", 0x0232, nullptr },
    { "JSeek", 0x0236, nullptr },
    { "JSetUpPoll", 0x023A, nullptr },
    { "JRecal", 0x023E, nullptr },
    { "JControl", 0x0242, nullptr },
    { "JWakeUp", 0x0246, nullptr },
    { "JReSeek", 0x024A, nullptr },
    { "JMakeSpdTbl", 0x024E, nullptr },
    { "JAdrDisk", 0x0252, nullptr },
    { "JSetSpeed", 0x0256, nullptr },
    { "NiblTbl", 0x025A, nullptr },
    { "FlEvtMask", 0x025E, nullptr },
    { "SdVolume", 0x0260, nullptr },
    { "Finder", 0x0261, nullptr },
    { "SoundVars", 0x0262, nullptr },
    { "SoundPtr", 0x0262, nullptr },
    { "SoundBase", 0x0266, nullptr },
    { "SoundVBL", 0x026A, nullptr },
    { "SoundDCE", 0x027A, nullptr },
    { "SoundActive", 0x027E, nullptr },
    { "SoundLevel", 0x027F, nullptr },
    { "CurPitch", 0x0280, nullptr },
    { "SoundLast", 0x0282, nullptr },
    { "Switcher", 0x0282, nullptr },
    { "SwitcherTPtr", 0x0286, nullptr },
    { "RSDHndl", 0x028A, nullptr },
    { "ROM85", 0x028E, nullptr },
    { "PortAUse", 0x0290, nullptr },
    { "PortBUse", 0x0291, nullptr },
    { "ScreenVars", 0x0292, nullptr },
    { "JGNEFilter", 0x029A, nullptr },
    { "Key1Trans", 0x029E, nullptr },
    { "Key2Trans", 0x02A2, nullptr },
    { "SysZone", 0x02A6, nullptr },
    { "ApplZone", 0x02AA, nullptr },
    { "ROMBase", 0x02AE, nullptr },
    { "RAMBase", 0x02B2, nullptr },
    { "BasicGlob", 0x02B6, nullptr },
    { "ExpandMem", 0x02B6, nullptr },
    { "DSAlertTab", 0x02BA, nullptr },
    { "ExtStsDT", 0x02BE, nullptr },
    { "SCCASts", 0x02CE, nullptr },
    { "SCCBSts", 0x02CF, nullptr },
    { "SerialVars", 0x02D0, nullptr },
    { "ABusVars", 0x02D8, nullptr },
    { "FinderName", 0x02E0, nullptr },
    { "DoubleTime", 0x02F0, nullptr },
    { "CaretTime", 0x02F4, nullptr },
    { "ScrDmpEnb", 0x02F8, nullptr },
    { "ScrDmpType", 0x02F9, nullptr },
    { "TagData", 0x02FA, nullptr },
    { "BufTgFNum", 0x02FC, nullptr },
    { "BufTgFFlg", 0x0300, nullptr },
    { "BufTgFBkNum", 0x0302, nullptr },
    { "BufTgDate", 0x0304, nullptr },
    { "DrvQHdr", 0x0308, nullptr },
    { "PWMBuf2", 0x0312, nullptr },
    { "MacPgm", 0x0316, "used as handle to pointers by MPW" },
    { "Lo3Bytes", 0x031A, nullptr },
    { "MinStack", 0x031E, nullptr },
    { "DefltStack", 0x0322, nullptr },
    { "MMDefFlags", 0x0326, nullptr },
    { "GZRootHnd", 0x0328, nullptr },
    { "GZRootPtr", 0x032C, nullptr },
    { "GZMoveHnd", 0x0330, nullptr },
    { "DSDrawProc", 0x0334, nullptr },
    { "EjectNotify", 0x0338, nullptr },
    { "IAZNotify", 0x033C, nullptr },
    { "CkdDB", 0x0340, nullptr },
    { "NxtDB", 0x0342, nullptr },
    { "MaxDB", 0x0344, nullptr },
    { "FlushOnly", 0x0346, nullptr },
    { "RegRscr", 0x0347, nullptr },
    { "FLckUnlck", 0x0348, nullptr },
    { "FrcSync", 0x0349, nullptr },
    { "NewMount", 0x034A, nullptr },
    { "DrMstrBlk", 0x034C, nullptr },
    { "FCBSPtr", 0x034E, nullptr },
    { "DefVCBPtr", 0x0352, nullptr },
    { "VCBQHdr", 0x0356, nullptr },
    { "FSBusy", 0x0360, nullptr },
    { "FSQHead", 0x0362, nullptr },
    { "FSQTail", 0x0366, nullptr },
    { "RgSvArea", 0x036A, nullptr },
    { "WDCBsPtr", 0x0372, nullptr },
    { "HFSVars", 0x0376, nullptr },
    { "DefVRefnum", 0x0384, nullptr },
    { "HFSDSErr", 0x0392, nullptr },
    { "CurDirStore", 0x0398, nullptr },
    { "ErCode", 0x03A2, nullptr },
    { "Params", 0x03A4, nullptr },
    { "FSTemp8", 0x03D6, nullptr },
    { "FSTemp4", 0x03DE, nullptr },
    { "FSQueueHook", 0x03E2, nullptr },
    { "ExtFSHook", 0x03E6, nullptr },
    { "DskSwtchHook", 0x03EA, nullptr },
    { "RegstVol", 0x03EE, nullptr },
    { "ToExtFS", 0x03F2, nullptr },
    { "FSFCBLen", 0x03F6, nullptr },
    { "DSAlertRect", 0x03F8, nullptr },
    { "OSTable", 0x0400, nullptr },
    { "GrafBegin", 0x0800, nullptr },
    { "JHideCursor", 0x0800, nullptr },
    { "JShowCursor", 0x0804, nullptr },
    { "JShieldCursor", 0x0808, nullptr },
    { "JScrnAddr", 0x080C, nullptr },
    { "JScrnSize", 0x0810, nullptr },
    { "JInitCursor", 0x0814, nullptr },
    { "JSetCrsr", 0x0818, nullptr },
    { "JCrsrObscure", 0x081C, nullptr },
    { "JUpdateProc", 0x0820, nullptr },
    { "ScrnBase", 0x0824, nullptr },
    { "MTemp", 0x0828, nullptr },
    { "RawMouse", 0x082C, nullptr },
    { "Mouse", 0x0830, nullptr },
    { "CrsrPin", 0x0834, nullptr },
    { "CrsrRect", 0x083C, nullptr },
    { "TheCrsr", 0x0844, nullptr },
    { "CrsrAddr", 0x0888, nullptr },
    { "JAllocCrsr", 0x088C, nullptr },
    { "JSetCCrsr", 0x0890, nullptr },
    { "JOpcodeProc", 0x0894, nullptr },
    { "CrsrBase", 0x0898, nullptr },
    { "CrsrDevice", 0x089C, nullptr },
    { "SrcDevice", 0x08A0, nullptr },
    { "MainDevice", 0x08A4, nullptr },
    { "DeviceList", 0x08A8, nullptr },
    { "CrsrRow", 0x08AC, nullptr },
    { "unknown", 0x08AE, nullptr },
    { "QDColors", 0x08B0, nullptr },
    { "CrsrVis", 0x08CC, nullptr },
    { "CrsrBusy", 0x08CD, nullptr },
    { "CrsrNew", 0x08CE, nullptr },
    { "CrsrCouple", 0x08CF, nullptr },
    { "CrsrState", 0x08D0, nullptr },
    { "CrsrObscure", 0x08D2, nullptr },
    { "CrsrScale", 0x08D3, nullptr },
    { "unknown", 0x08D4, nullptr },
    { "MouseMask", 0x08D6, nullptr },
    { "MouseOffset", 0x08DA, nullptr },
    { "JournalFlag", 0x08DE, nullptr },
    { "JSwapFont", 0x08E0, nullptr },
    { "WidthListHand", 0x08E4, nullptr },
    { "JournalRef", 0x08E8, nullptr },
    { "unknown", 0x08EA, nullptr },
    { "CrsrThresh", 0x08EC, nullptr },
    { "JCrsrTask", 0x08EE, nullptr },
    { "WWExist", 0x08F2, nullptr },
    { "QDExist", 0x08F3, nullptr },
    { "JFetch", 0x08F4, nullptr },
    { "JStash", 0x08F8, nullptr },
    { "JIODone", 0x08FC, nullptr },
    { "CurApRefNum", 0x0900, nullptr },
    { "LaunchFlag", 0x0902, nullptr },
    { "FondState", 0x0903, nullptr },
    { "CurrentA5", 0x0904, nullptr },
    { "CurStackBase", 0x0908, nullptr },
    { "LoadFiller", 0x090C, nullptr },
    { "CurApName", 0x0910, nullptr },
    { "SaveSegHandle", 0x0930, nullptr },
    { "CurJTOffset", 0x0934, nullptr },
    { "CurPageOption", 0x0936, nullptr },
    { "HiliteMode", 0x0938, nullptr },
    { "unknown", 0x0939, nullptr },
    { "LoaderPBlock", 0x093A, nullptr },
    { "PrintErr", 0x0944, nullptr },
    { "ChooserBits", 0x0946, nullptr },
    { "CoreEditVars", 0x0954, nullptr },
    { "ScrapSize", 0x0960, nullptr },
    { "ScrapHandle", 0x0964, nullptr },
    { "ScrapCount", 0x0968, nullptr },
    { "ScrapState", 0x096A, nullptr },
    { "ScrapName", 0x096C, nullptr },
    { "ScrapTag", 0x0970, nullptr },
    { "RomFont0", 0x0980, nullptr },
    { "ApFontID", 0x0984, nullptr },
    { "SaveFondFlags", 0x0986, nullptr },
    { "FMDefaultSize", 0x0987, nullptr },
    { "CurFMInput", 0x0988, nullptr },
    { "CurFMSize", 0x098A, nullptr },
    { "CurFMFace", 0x098C, nullptr },
    { "CurFMNeedBits", 0x098D, nullptr },
    { "CurFMDevice", 0x098E, nullptr },
    { "CurFMNumer", 0x0990, nullptr },
    { "CurFMDenom", 0x0994, nullptr },
    { "FOutError", 0x0998, nullptr },
    { "FOutFontHandle", 0x099A, nullptr },
    { "FOutBold", 0x099E, nullptr },
    { "FOutItalic", 0x099F, nullptr },
    { "FOutULOffset", 0x09A0, nullptr },
    { "FOutULShadow", 0x09A1, nullptr },
    { "FOutULThick", 0x09A2, nullptr },
    { "FOutShadow", 0x09A3, nullptr },
    { "FOutExtra", 0x09A4, nullptr },
    { "FOutAccent", 0x09A5, nullptr },
    { "FOutDescent", 0x09A6, nullptr },
    { "FOutWidMax", 0x09A7, nullptr },
    { "FOutLeading", 0x09A8, nullptr },
    { "FOutUnused", 0x09A9, nullptr },
    { "FOutNumer", 0x09AA, nullptr },
    { "FOutDenom", 0x09AE, nullptr },
    { "FMDotsPerInch", 0x09B2, nullptr },
    { "FMStyleTab", 0x09B6, nullptr },
    { "ToolScratch", 0x09CE, nullptr },
    { "WindowList", 0x09D6, nullptr },
    { "SaveUpdate", 0x09DA, nullptr },
    { "PaintWhite", 0x09DC, nullptr },
    { "WMgrPort", 0x09DE, nullptr },
    { "DeskPort", 0x09E2, nullptr },
    { "OldStructure", 0x09E6, nullptr },
    { "OldContent", 0x09EA, nullptr },
    { "GrayRgn", 0x09EE, nullptr },
    { "SaveVisRgn", 0x09F2, nullptr },
    { "DragHook", 0x09F6, nullptr },
    { "TempRect", 0x09FA, nullptr },
    { "OneOne", 0x0A02, nullptr },
    { "MinusOne", 0x0A06, nullptr },
    { "TopMenuItem", 0x0A0A, nullptr },
    { "AtMenuBotom", 0x0A0C, nullptr },
    { "IconBitMap", 0x0A0E, nullptr },
    { "MenuList", 0x0A1C, nullptr },
    { "MBarEnable", 0x0A20, nullptr },
    { "CurDeKind", 0x0A22, nullptr },
    { "MenuFlash", 0x0A24, nullptr },
    { "TheMenu", 0x0A26, nullptr },
    { "SavedHandle", 0x0A28, nullptr },
    { "MBarHook", 0x0A2C, nullptr },
    { "MenuHook", 0x0A30, nullptr },
    { "DragPattern", 0x0A34, nullptr },
    { "DeskPattern", 0x0A3C, nullptr },
    { "DragFlag", 0x0A44, nullptr },
    { "CurDragAction", 0x0A46, nullptr },
    { "FPState", 0x0A4A, nullptr },
    { "TopMapHndl", 0x0A50, nullptr },
    { "SysMapHndl", 0x0A54, nullptr },
    { "SysMap", 0x0A58, nullptr },
    { "CurMap", 0x0A5A, nullptr },
    { "ResReadOnly", 0x0A5C, nullptr },
    { "ResLoad", 0x0A5E, nullptr },
    { "ResErr", 0x0A60, nullptr },
    { "TaskLock", 0x0A62, nullptr },
    { "FScaleDisable", 0x0A63, nullptr },
    { "CurActivate", 0x0A64, nullptr },
    { "CurDeactive", 0x0A68, nullptr },
    { "DeskHook", 0x0A6C, nullptr },
    { "TEDoText", 0x0A70, nullptr },
    { "TERecal", 0x0A74, nullptr },
    { "ApplScratch", 0x0A78, nullptr },
    { "GhostWindow", 0x0A84, nullptr },
    { "CloseOrnHook", 0x0A88, nullptr },
    { "ResumeProc", 0x0A8C, nullptr },
    { "SaveProc", 0x0A90, nullptr },
    { "SaveSP", 0x0A94, nullptr },
    { "ANumber", 0x0A98, nullptr },
    { "ACount", 0x0A9A, nullptr },
    { "DABeeper", 0x0A9C, nullptr },
    { "DAStrings", 0x0AA0, nullptr },
    { "TEScrpLength", 0x0AB0, nullptr },
    { "unknown", 0x0AB2, nullptr },
    { "TEScrpHandle", 0x0AB4, nullptr },
    { "AppPacks", 0x0AB8, nullptr },
    { "SysResName", 0x0AD8, nullptr },
    { "SoundGlue", 0x0AE8, nullptr },
    { "AppParmHandle", 0x0AEC, nullptr },
    { "DSErrCode", 0x0AF0, nullptr },
    { "ResErrProc", 0x0AF2, nullptr },
    { "TEWdBreak", 0x0AF6, nullptr },
    { "DlgFont", 0x0AFA, nullptr },
    { "LastTGlobal", 0x0AFC, nullptr },
    { "TrapAgain", 0x0B00, nullptr },
    { "KeyMVars", 0x0B04, nullptr },
    { "ROMMapHndl", 0x0B06, nullptr },
    { "PWMBuf1", 0x0B0A, nullptr },
    { "BootMask", 0x0B0E, nullptr },
    { "WidthPtr", 0x0B10, nullptr },
    { "AtalkHk1", 0x0B14, nullptr },
    { "AtalkHk2", 0x0B18, nullptr },
    { "FourDHack", 0x0B1C, nullptr },
    { "unknown", 0x0B20, nullptr },
    { "HWCfgFlags", 0x0B22, nullptr },
    { "TimeSCSIDB", 0x0B24, nullptr },
    { "TopMenuItem", 0x0B26, nullptr },
    { "AtMenuBottom", 0x0B28, nullptr },
    { "WidthTabHandle", 0x0B2A, nullptr },
    { "SCSIDrvrs", 0x0B2E, nullptr },
    { "TimeVars", 0x0B30, nullptr },
    { "BtDskRfn", 0x0B34, nullptr },
    { "BootTmp8", 0x0B36, nullptr },
    { "unknown", 0x0B3E, nullptr },
    { "T1Arbitrate", 0x0B3F, nullptr },
    { "JDiskSel", 0x0B4C, nullptr },
    { "JSendCmd", 0x0B44, nullptr },
    { "JDCDReset", 0x0B48, nullptr },
    { "LastSPExtra", 0x0B4C, nullptr },
    { "AppleShare", 0x0B50, nullptr },
    { "MenuDisable", 0x0B54, nullptr },
    { "MBDFHndl", 0x0B58, nullptr },
    { "MBSaveLoc", 0x0B5C, nullptr },
    { "BNMQHd", 0x0B60, nullptr },
    { "Twitcher1", 0x0B64, nullptr },
    { "unknown", 0x0B68, nullptr },
    { "Twitcher2", 0x0B7C, nullptr },
    { "RMgrHiVars", 0x0B80, nullptr },
    { "RomMapInsert", 0x0B9E, nullptr },
    { "TmpResLoad", 0x0B9F, nullptr },
    { "IntlSpec", 0x0BA0, nullptr },
    { "SMgrCore", 0x0BA0, nullptr },
    { "RMgrPerm", 0x0BA4, nullptr },
    { "WordRedraw", 0x0BA5, nullptr },
    { "SysFontFam", 0x0BA6, nullptr },
    { "SysFontSize", 0x0BA8, nullptr },
    { "MBarHeight", 0x0BAA, nullptr },
    { "TESysJust", 0x0BAC, nullptr },
    { "HiHeapMask", 0x0BAE, nullptr },
    { "SegHiEnable", 0x0BB2, nullptr },
    { "FDevDisable", 0x0BB3, nullptr },
    { "CMVector", 0x0BB4, nullptr },
    { "XFSGlobs", 0x0BB8, nullptr },
    { "ShutDownQHdr", 0x0BBC, nullptr },
    { "NewUnused", 0x0BC0, nullptr },
    { "LastFOND", 0x0BC2, nullptr },
    { "FONDID", 0x0BC6, nullptr },
    { "App2Packs", 0x0BC8, nullptr },
    { "MAErrProc", 0x0BE8, nullptr },
    { "MASuperTab", 0x0BEC, nullptr },
    { "MimeGlobs", 0x0BF0, nullptr },
    { "FractEnable", 0x0BF4, nullptr },
    { "UsedWidths", 0x0BF5, nullptr },
    { "FScaleHFact", 0x0BF6, nullptr },
    { "FScaleVFact", 0x0BFA, nullptr },
    { "unknown", 0x0BFE, nullptr },
    { "SCSIBase", 0x0C00, nullptr },
    { "SCSIDMA", 0x0C04, nullptr },
    { "SCSIHsk", 0x0C08, nullptr },
    { "SCSIGlobals", 0x0C0C, nullptr },
    { "RGBBlack", 0x0C10, nullptr },
    { "RGBWhite", 0x0C16, nullptr },
    { "unknown", 0x0C1C, nullptr },
    { "RowBits", 0x0C20, nullptr },
    { "ColLines", 0x0C22, nullptr },
    { "ScreenBytes", 0x0C24, nullptr },
    { "IOPMgrVars", 0x0C28, nullptr },
    { "NMIFlag", 0x0C2C, nullptr },
    { "VidType", 0x0C2D, nullptr },
    { "VidMode", 0x0C2E, nullptr },
    { "SCSIPoll", 0x0C2F, nullptr },
    { "SEVarBase", 0x0C30, nullptr },
    { "MMUFlags", 0x0CB0, nullptr },
    { "MMUType", 0x0CB1, nullptr },
    { "MMU32bit", 0x0CB2, nullptr },
    { "WhichBox", 0x0CB3, nullptr },
    { "MMUTbl", 0x0CB4, nullptr },
    { "MMUTblSize", 0x0CB8, nullptr },
    { "SInfoPtr", 0x0CBC, nullptr },
    { "ASCBase", 0x0CC0, nullptr },
    { "SMGlobals", 0x0CC4, nullptr },
    { "TheGDevice", 0x0CC8, nullptr },
    { "CQDGlobals", 0x0CCC, nullptr },
    { "AuxWinHead", 0x0CD0, nullptr },
    { "AuxCtlHead", 0x0CD4, nullptr },
    { "DeskCPat", 0x0CD8, nullptr },
    { "SetOSDefKey", 0x0CDC, nullptr },
    { "LastBinPat", 0x0CE0, nullptr },
    { "DeskPatEnable", 0x0CE8, nullptr },
    { "unknown", 0x0CEA, nullptr },
    { "ADBBase", 0x0CF8, nullptr },
    { "WarmStart", 0x0CFC, nullptr },
    { "TimeDBRA", 0x0D00, nullptr },
    { "TimeSCCDB", 0x0D02, nullptr },
    { "SlotQDT", 0x0D04, nullptr },
    { "SlotPrTbl", 0x0D08, nullptr },
    { "SlotVBLQ", 0x0D0C, nullptr },
    { "ScrnVBLPtr", 0x0D10, nullptr },
    { "SlotTICKS", 0x0D14, nullptr },
    { "PowerMgrVars", 0x0D18, nullptr },
    { "AGBHandle", 0x0D1C, nullptr },
    { "TableSeed", 0x0D20, nullptr },
    { "SRsrcTblPtr", 0x0D24, nullptr },
    { "JVBLTask", 0x0D28, nullptr },
    { "WMgrCPort", 0x0D2C, nullptr },
    { "VertRRate", 0x0D30, nullptr },
    { "SynListHandle", 0x0D32, nullptr },
    { "LastFore", 0x0D36, nullptr },
    { "LastMode", 0x0D3E, nullptr },
    { "LastDepth", 0x0D40, nullptr },
    { "FMExist", 0x0D42, nullptr },
    { "SavedHilite", 0x0D43, nullptr },
    { "unknown", 0x0D44, nullptr },
    { "MenuCInfo", 0x0D50, nullptr },
    { "MBProcHndl", 0x0D54, nullptr },
    { "MBSaveLoc", 0x0D58, nullptr },
    { "MRect", 0x0D58, nullptr },
    { "MBFlash", 0x0D5C, nullptr },
    { "MenuCInfo", 0x0D5C, nullptr },
    { "ChunkyDepth", 0x0D60, nullptr },
    { "CrsrPtr", 0x0D62, nullptr },
    { "unknown", 0x0D64, nullptr },
    { "PortList", 0x0D66, nullptr },
    { "MickeyBytes", 0x0D6A, nullptr },
    { "QDErr", 0x0D6E, nullptr },
    { "VIA2DT", 0x0D70, nullptr },
    { "SInitFlags", 0x0D90, nullptr },
    { "DTQFlags", 0x0D92, nullptr },
    { "DTQueue", 0x0D92, nullptr },
    { "DtskQHdr", 0x0D94, nullptr },
    { "DTskQTail", 0x0D98, nullptr },
    { "JDTInstall", 0x0D9C, nullptr },
    { "HiliteRGB", 0x0DA0, nullptr },
    { "TimeSCSIDB", 0x0DA6, nullptr },
    { "DSCtrAdj", 0x0DA8, nullptr },
    { "IconTLAddr", 0x0DAC, nullptr },
    { "VideoInfoOK", 0x0DB0, nullptr },
    { "EndSRTPtr", 0x0DB4, nullptr },
    { "SDMJmpTblPtr", 0x0DB8, nullptr },
    { "JSwapMMU", 0x0DBC, nullptr },
    { "SdmBusErr", 0x0DC0, nullptr },
    { "LastTxGDevice", 0x0DC4, nullptr },
    { "PmgrHandle", 0x0DC8, nullptr },
    { "LayerPalette", 0x0DCC, nullptr },
    { "ToolTable", 0x0E00, nullptr },
    { "SystemHeap", kSystemHeapStart, nullptr }, // 0x00001E00
    { 0, 0xffffffff, 0 }
};


/**
 * Return the name of the system variables.
 */
const char *gvarName(unsigned int id, const char ** description)
{
    if (description) *description = "";
    if (id>kSystemHeapStart) {
        return "NOT_A_LOCAL_VARIABLE";
    }
    GVars *t = gvarLUT;
    for (;;) {
        if (t->id == 0xffffffff)
            break;
        if (t->id == id) {
            if (description && t->comment)
                *description = t->comment;
            return t->name;
        }
        t++;
    }
    return "Unknown";
}


#if 0

/*

    ; QuickDraw

*/

#define _CopyMask 0xA817
#define _MeasureText 0xA837
#define _GetMaskTable 0xA836
#define _CalcMask 0xA838
#define _SeedFill 0xA839
#define _InitCursor 0xA850
#define _SetCursor 0xA851
#define _HideCursor 0xA852
#define _ShowCursor 0xA853
#define _ShieldCursor 0xA855
#define _ObscureCursor 0xA856
#define _BitAnd 0xA858
#define _BitXOr 0xA859
#define _BitNot 0xA85A
#define _BitOr 0xA85B
#define _BitShift 0xA85C
#define _BitTst 0xA85D
#define _BitSet 0xA85E
#define _BitClr 0xA85F
#define _Random 0xA861
#define _ForeColor 0xA862
#define _BackColor 0xA863
#define _ColorBit 0xA864
#define _GetPixel 0xA865
#define _StuffHex 0xA866
#define _LongMul 0xA867
#define _FixMul 0xA868
#define _FixRatio 0xA869
#define _HiWord 0xA86A
#define _LoWord 0xA86B
#define _FixRound 0xA86C
#define _InitPort 0xA86D
#define _InitGraf 0xA86E
#define _OpenPort 0xA86F
#define _LocalToGlobal 0xA870
#define _GlobalToLocal 0xA871
#define _GrafDevice 0xA872
#define _SetPort 0xA873
#define _GetPort 0xA874
#define _SetPBits 0xA875
#define _PortSize 0xA876
#define _MovePortTo 0xA877
#define _SetOrigin 0xA878
#define _SetClip 0xA879
#define _GetClip 0xA87A
#define _ClipRect 0xA87B
#define _BackPat 0xA87C
#define _ClosePort 0xA87D
#define _AddPt 0xA87E
#define _SubPt 0xA87F
#define _SetPt 0xA880
#define _EqualPt 0xA881
#define _StdText 0xA882
#define _DrawChar 0xA883
#define _DrawString 0xA884
#define _DrawText 0xA885
#define _TextWidth 0xA886
#define _TextFont 0xA887
#define _TextFace 0xA888
#define _TextMode 0xA889
#define _TextSize 0xA88A
#define _GetFontInfo 0xA88B
#define _StringWidth 0xA88C
#define _CharWidth 0xA88D
#define _SpaceExtra 0xA88E
#define _StdLine 0xA890
#define _LineTo 0xA891
#define _Line 0xA892
#define _MoveTo 0xA893
#define _Move 0xA894
#define _ShutDown 0xA895
#define _HidePen 0xA896
#define _ShowPen 0xA897
#define _GetPenState 0xA898
#define _SetPenState 0xA899
#define _GetPen 0xA89A
#define _PenSize 0xA89B
#define _PenMode 0xA89C
#define _PenPat 0xA89D
#define _PenNormal 0xA89E
#define _Unimplemented 0xA89F
#define _StdRect 0xA8A0
#define _FrameRect 0xA8A1
#define _PaintRect 0xA8A2
#define _EraseRect 0xA8A3
#define _InverRect 0xA8A4
#define _FillRect 0xA8A5
#define _EqualRect 0xA8A6
#define _SetRect 0xA8A7
#define _OffsetRect 0xA8A8
#define _OffSetRect 0xA8A8
#define _InsetRect 0xA8A9
#define _InSetRect 0xA8A9
#define _SectRect 0xA8AA
#define _UnionRect 0xA8AB
#define _Pt2Rect 0xA8AC
#define _PtInRect 0xA8AD
#define _EmptyRect 0xA8AE
#define _StdRRect 0xA8AF
#define _FrameRoundRect 0xA8B0
#define _PaintRoundRect 0xA8B1
#define _EraseRoundRect 0xA8B2
#define _InverRoundRect 0xA8B3
#define _FillRoundRect 0xA8B4
#define _StdOval 0xA8B6
#define _FrameOval 0xA8B7
#define _PaintOval 0xA8B8
#define _EraseOval 0xA8B9
#define _InvertOval 0xA8BA
#define _FillOval 0xA8BB
#define _SlopeFromAngle 0xA8BC
#define _StdArc 0xA8BD
#define _FrameArc 0xA8BE
#define _PaintArc 0xA8BF
#define _EraseArc 0xA8C0
#define _InvertArc 0xA8C1
#define _FillArc 0xA8C2
#define _PtToAngle 0xA8C3
#define _AngleFromSlope 0xA8C4
#define _StdPoly 0xA8C5
#define _FramePoly 0xA8C6
#define _PaintPoly 0xA8C7
#define _ErasePoly 0xA8C8
#define _InvertPoly 0xA8C9
#define _FillPoly 0xA8CA
#define _OpenPoly 0xA8CB
#define _ClosePgon 0xA8CC
#define _ClosePoly 0xA8CC
#define _KillPoly 0xA8CD
#define _OffsetPoly 0xA8CE
#define _OffSetPoly 0xA8CE
#define _PackBits 0xA8CF
#define _UnpackBits 0xA8D0
#define _StdRgn 0xA8D1
#define _FrameRgn 0xA8D2
#define _PaintRgn 0xA8D3
#define _EraseRgn 0xA8D4
#define _InverRgn 0xA8D5
#define _FillRgn 0xA8D6
#define _BitMapRgn 0xA8D7
#define _BitMapToRegion 0xA8D7
#define _NewRgn 0xA8D8
#define _DisposRgn 0xA8D9
#define _DisposeRgn 0xA8D9
#define _OpenRgn 0xA8DA
#define _CloseRgn 0xA8DB
#define _CopyRgn 0xA8DC
#define _SetEmptyRgn 0xA8DD
#define _SetRecRgn 0xA8DE
#define _RectRgn 0xA8DF
#define _OfsetRgn 0xA8E0
#define _OfSetRgn 0xA8E0
#define _OffsetRgn 0xA8E0
#define _InsetRgn 0xA8E1
#define _InSetRgn 0xA8E1
#define _EmptyRgn 0xA8E2
#define _EqualRgn 0xA8E3
#define _SectRgn 0xA8E4
#define _UnionRgn 0xA8E5
#define _DiffRgn 0xA8E6
#define _XOrRgn 0xA8E7
#define _PtInRgn 0xA8E8
#define _RectInRgn 0xA8E9
#define _SetStdProcs 0xA8EA
#define _StdBits 0xA8EB
#define _CopyBits 0xA8EC
#define _StdTxMeas 0xA8ED
#define _StdGetPic 0xA8EE
#define _ScrollRect 0xA8EF
#define _StdPutPic 0xA8F0
#define _StdComment 0xA8F1
#define _PicComment 0xA8F2
#define _OpenPicture 0xA8F3
#define _ClosePicture 0xA8F4
#define _KillPicture 0xA8F5
#define _DrawPicture 0xA8F6
#define _Layout 0xA8F7
#define _ScalePt 0xA8F8
#define _MapPt 0xA8F9
#define _MapRect 0xA8FA
#define _MapRgn 0xA8FB
#define _MapPoly 0xA8FC

/*

    ; Toolbox

*/

#define _Count1Resources 0xA80D
#define _Get1IxResource 0xA80E
#define _Get1IxType 0xA80F
#define _Unique1ID 0xA810
#define _TESelView 0xA811
#define _TEPinScroll 0xA812
#define _TEAutoView 0xA813
#define _Pack8 0xA816
#define _FixATan2 0xA818
#define _XMunger 0xA819
#define _HOpenResFile 0xA81A
#define _HCreateResFile 0xA81B
#define _Count1Types 0xA81C
#define _Get1Resource 0xA81F
#define _Get1NamedResource 0xA820
#define _MaxSizeRsrc 0xA821
#define _InsMenuItem 0xA826
#define _InsertMenuItem 0xA826
#define _HideDItem 0xA827
#define _HideDialogItem 0xA827
#define _ShowDItem 0xA828
#define _ShowDialogItem 0xA828
#define _LayerDispatch 0xA829
#define _Pack9 0xA82B
#define _Pack10 0xA82C
#define _Pack11 0xA82D
#define _Pack12 0xA82E
#define _Pack13 0xA82F
#define _Pack14 0xA830
#define _Pack15 0xA831
#define _ScrnBitMap 0xA833
#define _SetFScaleDisable 0xA834
#define _FontMetrics 0xA835
#define _ZoomWindow 0xA83A
#define _TrackBox 0xA83B
#define _PrGlue 0xA8FD
#define _InitFonts 0xA8FE
#define _GetFName 0xA8FF
#define _GetFNum 0xA900
#define _FMSwapFont 0xA901
#define _RealFont 0xA902
#define _SetFontLock 0xA903
#define _DrawGrowIcon 0xA904
#define _DragGrayRgn 0xA905
#define _NewString 0xA906
#define _SetString 0xA907
#define _ShowHide 0xA908
#define _CalcVis 0xA909
#define _CalcVBehind 0xA90A
#define _ClipAbove 0xA90B
#define _PaintOne 0xA90C
#define _PaintBehind 0xA90D
#define _SaveOld 0xA90E
#define _DrawNew 0xA90F
#define _GetWMgrPort 0xA910
#define _CheckUpDate 0xA911
#define _InitWindows 0xA912
#define _NewWindow 0xA913
#define _DisposWindow 0xA914
#define _DisposeWindow 0xA914
#define _ShowWindow 0xA915
#define _HideWindow 0xA916
#define _GetWRefCon 0xA917
#define _SetWRefCon 0xA918
#define _GetWTitle 0xA919
#define _SetWTitle 0xA91A
#define _MoveWindow 0xA91B
#define _HiliteWindow 0xA91C
#define _SizeWindow 0xA91D
#define _TrackGoAway 0xA91E
#define _SelectWindow 0xA91F
#define _BringToFront 0xA920
#define _SendBehind 0xA921
#define _BeginUpDate 0xA922
#define _EndUpDate 0xA923
#define _FrontWindow 0xA924
#define _DragWindow 0xA925
#define _DragTheRgn 0xA926
#define _InvalRgn 0xA927
#define _InvalRect 0xA928
#define _ValidRgn 0xA929
#define _ValidRect 0xA92A
#define _GrowWindow 0xA92B
#define _FindWindow 0xA92C
#define _CloseWindow 0xA92D
#define _SetWindowPic 0xA92E
#define _GetWindowPic 0xA92F
#define _InitMenus 0xA930
#define _NewMenu 0xA931
#define _DisposMenu 0xA932
#define _DisposeMenu 0xA932
#define _AppendMenu 0xA933
#define _ClearMenuBar 0xA934
#define _InsertMenu 0xA935
#define _DeleteMenu 0xA936
#define _DrawMenuBar 0xA937
#define _InvalMenuBar 0xA81D
#define _HiliteMenu 0xA938
#define _EnableItem 0xA939
#define _DisableItem 0xA93A
#define _GetMenuBar 0xA93B
#define _SetMenuBar 0xA93C
#define _MenuSelect 0xA93D
#define _MenuKey 0xA93E
#define _GetItmIcon 0xA93F
#define _SetItmIcon 0xA940
#define _GetItmStyle 0xA941
#define _SetItmStyle 0xA942
#define _GetItmMark 0xA943
#define _SetItmMark 0xA944
#define _CheckItem 0xA945
#define _GetItem 0xA946
#define _GetMenuItemText 0xA946
#define _SetItem 0xA947
#define _SetMenuItemText 0xA947
#define _CalcMenuSize 0xA948
#define _GetMHandle 0xA949
#define _GetMenuHandle 0xA949
#define _SetMFlash 0xA94A
#define _PlotIcon 0xA94B
#define _FlashMenuBar 0xA94C
#define _AddResMenu 0xA94D
#define _AppendResMenu 0xA94D
#define _PinRect 0xA94E
#define _DeltaPoint 0xA94F
#define _CountMItems 0xA950
#define _InsertResMenu 0xA951
#define _DelMenuItem 0xA952
#define _DeleteMenuItem 0xA952
#define _UpdtControl 0xA953
#define _NewControl 0xA954
#define _DisposControl 0xA955
#define _DisposeControl 0xA955
#define _KillControls 0xA956
#define _ShowControl 0xA957
#define _HideControl 0xA958
#define _MoveControl 0xA959
#define _GetCRefCon 0xA95A
#define _GetControlReference 0xA95A
#define _SetCRefCon 0xA95B
#define _SetControlReference 0xA95B
#define _SizeControl 0xA95C
#define _HiliteControl 0xA95D
#define _GetCTitle 0xA95E
#define _GetControlTitle 0xA95E
#define _SetCTitle 0xA95F
#define _SetControlTitle 0xA95F
#define _GetCtlValue 0xA960
#define _GetControlValue 0xA960
#define _GetMinCtl 0xA961
#define _GetControlMinimum 0xA961
#define _GetMaxCtl 0xA962
#define _GetControlMaximum 0xA962
#define _SetCtlValue 0xA963
#define _SetControlValue 0xA963
#define _SetMinCtl 0xA964
#define _SetControlMinimum 0xA964
#define _SetMaxCtl 0xA965
#define _SetControlMaximum 0xA965
#define _TestControl 0xA966
#define _DragControl 0xA967
#define _TrackControl 0xA968
#define _DrawControls 0xA969
#define _GetCtlAction 0xA96A
#define _GetControlAction 0xA96A
#define _SetCtlAction 0xA96B
#define _SetControlAction 0xA96B
#define _FindControl 0xA96C
#define _Draw1Control 0xA96D
#define _Dequeue 0xA96E
#define _Enqueue 0xA96F
#define _WaitNextEvent 0xA860
#define _GetNextEvent 0xA970
#define _EventAvail 0xA971
#define _GetMouse 0xA972
#define _StillDown 0xA973
#define _Button 0xA974
#define _TickCount 0xA975
#define _GetKeys 0xA976
#define _WaitMouseUp 0xA977
#define _UpdtDialog 0xA978
#define _InitDialogs 0xA97B
#define _GetNewDialog 0xA97C
#define _NewDialog 0xA97D
#define _SelIText 0xA97E
#define _SelectDialogItemText 0xA97E
#define _IsDialogEvent 0xA97F
#define _DialogSelect 0xA980
#define _DrawDialog 0xA981
#define _CloseDialog 0xA982
#define _DisposDialog 0xA983
#define _DisposeDialog 0xA983
#define _FindDItem 0xA984
#define _FindDialogItem 0xA984
#define _Alert 0xA985
#define _StopAlert 0xA986
#define _NoteAlert 0xA987
#define _CautionAlert 0xA988
#define _ParamText 0xA98B
#define _ErrorSound 0xA98C
#define _GetDItem 0xA98D
#define _GetDialogItem 0xA98D
#define _SetDItem 0xA98E
#define _SetDialogItem 0xA98E
#define _SetIText 0xA98F
#define _SetDialogItemText 0xA98F
#define _GetIText 0xA990
#define _GetDialogItemText 0xA990
#define _ModalDialog 0xA991
#define _DetachResource 0xA992
#define _SetResPurge 0xA993
#define _CurResFile 0xA994
#define _InitResources 0xA995
#define _RsrcZoneInit 0xA996
#define _OpenResFile 0xA997
#define _UseResFile 0xA998
#define _UpdateResFile 0xA999
#define _CloseResFile 0xA99A
#define _SetResLoad 0xA99B
#define _CountResources 0xA99C
#define _GetIndResource 0xA99D
#define _CountTypes 0xA99E
#define _GetIndType 0xA99F
#define _GetResource 0xA9A0
#define _GetNamedResource 0xA9A1
#define _LoadResource 0xA9A2
#define _ReleaseResource 0xA9A3
#define _HomeResFile 0xA9A4
#define _SizeRsrc 0xA9A5
#define _GetResAttrs 0xA9A6
#define _SetResAttrs 0xA9A7
#define _GetResInfo 0xA9A8
#define _SetResInfo 0xA9A9
#define _ChangedResource 0xA9AA
#define _AddResource 0xA9AB
#define _AddReference 0xA9AC
#define _RmveResource 0xA9AD
#define _RmveReference 0xA9AE
#define _ResError 0xA9AF
#define _WriteResource 0xA9B0
#define _CreateResFile 0xA9B1
#define _SystemEvent 0xA9B2
#define _SystemClick 0xA9B3
#define _SystemTask 0xA9B4
#define _SystemMenu 0xA9B5
#define _OpenDeskAcc 0xA9B6
#define _CloseDeskAcc 0xA9B7
#define _GetPattern 0xA9B8
#define _GetCursor 0xA9B9
#define _GetString 0xA9BA
#define _GetIcon 0xA9BB
#define _GetPicture 0xA9BC
#define _GetNewWindow 0xA9BD
#define _GetNewControl 0xA9BE
#define _GetRMenu 0xA9BF
#define _GetNewMBar 0xA9C0
#define _UniqueID 0xA9C1
#define _SysEdit 0xA9C2
#define _OpenRFPerm 0xA9C4
#define _RsrcMapEntry 0xA9C5
#define _Secs2Date 0xA9C6
#define _SecondsToDate 0xA9C6
#define _Date2Secs 0xA9C7
#define _DateToSeconds 0xA9C7
#define _SysBeep 0xA9C8
#define _SysError 0xA9C9
#define _PutIcon 0xA9CA
#define _Munger 0xA9E0
#define _HandToHand 0xA9E1
#define _PtrToXHand 0xA9E2
#define _PtrToHand 0xA9E3
#define _HandAndHand 0xA9E4
#define _InitPack 0xA9E5
#define _InitAllPacks 0xA9E6
#define _Pack0 0xA9E7
#define _Pack1 0xA9E8
#define _Pack2 0xA9E9
#define _Pack3 0xA9EA
#define _FP68K 0xA9EB
#define _Pack4 0xA9EB
#define _Elems68K 0xA9EC
#define _Pack5 0xA9EC
#define _Pack6 0xA9ED
#define _DECSTR68K 0xA9EE
#define _Pack7 0xA9EE
#define _PtrAndHand 0xA9EF
#define _LoadSeg 0xA9F0
#define _UnLoadSeg 0xA9F1
#define _Launch 0xA9F2
#define _Chain 0xA9F3
#define _ExitToShell 0xA9F4
#define _GetAppParms 0xA9F5
#define _GetResFileAttrs 0xA9F6
#define _SetResFileAttrs 0xA9F7
#define _MethodDispatch 0xA9F8
#define _InfoScrap 0xA9F9
#define _UnlodeScrap 0xA9FA
#define _UnloadScrap 0xA9FA
#define _LodeScrap 0xA9FB
#define _LoadScrap 0xA9FB
#define _ZeroScrap 0xA9FC
#define _GetScrap 0xA9FD
#define _PutScrap 0xA9FE
#define _Debugger 0xA9FF
#define _DisplayDispatch 0xABEB
#define _IconDispatch 0xABC9
#define _ThreadDispatch 0xABF2
#define _DebugStr 0xABFF

/*

    ; Resource Manager

*/

#define _ResourceDispatch 0xA822

/*

    ; PPCToolbox

*/

#define _PPC 0xA0DD

/*

    ; Alias Manager

*/

#define _AliasDispatch 0xA823

/*

    ; Device Manager (some shared by the File Manager)

*/

#define _Open 0xA000
#define _Close 0xA001
#define _Read 0xA002
#define _Write 0xA003
#define _Control 0xA004
#define _Status 0xA005
#define _KillIO 0xA006

/*

    ; File Manager

*/

#define _GetVolInfo 0xA007
#define _Create 0xA008
#define _Delete 0xA009
#define _OpenRF 0xA00A
#define _Rename 0xA00B
#define _ReName 0xA00B
#define _GetFileInfo 0xA00C
#define _SetFileInfo 0xA00D
#define _UnmountVol 0xA00E
#define _UnMountVol 0xA00E
#define _HUnmountVol 0xA20E
#define _MountVol 0xA00F
#define _Allocate 0xA010
#define _GetEOF 0xA011
#define _SetEOF 0xA012
#define _FlushVol 0xA013
#define _GetVol 0xA014
#define _SetVol 0xA015
#define _FInitQueue 0xA016
#define _Eject 0xA017
#define _GetFPos 0xA018
#define _SetFilLock 0xA041
#define _RstFilLock 0xA042
#define _SetFilType 0xA043
#define _SetFPos 0xA044
#define _FlushFile 0xA045
#define _HOpen 0xA200
#define _HGetVInfo 0xA207
#define _HCreate 0xA208
#define _HDelete 0xA209
#define _HOpenRF 0xA20A
#define _HRename 0xA20B
#define _HGetFileInfo 0xA20C
#define _HSetFileInfo 0xA20D
#define _AllocContig 0xA210
#define _HSetVol 0xA215
#define _HGetVol 0xA214
#define _HSetFLock 0xA241
#define _HRstFLock 0xA242

/*

    ; dispatch trap for remaining File Manager (and Desktop Manager) calls

*/

#define _FSDispatch 0xA060
#define _HFSDispatch 0xA260

/*

    ; High level FSSpec calls

*/

#define _HighLevelFSDispatch 0xAA52

/*

    ; Memory Manager

*/

#define _InitZone 0xA019
#define _GetZone 0xA11A
#define _SetZone 0xA01B
#define _FreeMem 0xA01C
#define _MaxMem 0xA11D
#define _NewPtr 0xA11E
#define _NewPtrSys 0xA51E
#define _NewPtrClear 0xA31E
#define _NewPtrSysClear 0xA71E
#define _DisposPtr 0xA01F
#define _DisposePtr 0xA01F
#define _SetPtrSize 0xA020
#define _GetPtrSize 0xA021
#define _NewHandle 0xA122
#define _NewHandleClear 0xA322
#define _DisposHandle 0xA023
#define _DisposeHandle 0xA023
#define _SetHandleSize 0xA024
#define _GetHandleSize 0xA025
#define _HandleZone 0xA126
#define _ReallocHandle 0xA027
#define _RecoverHandle 0xA128
#define _HLock 0xA029
#define _HUnlock 0xA02A
#define _EmptyHandle 0xA02B
#define _InitApplZone 0xA02C
#define _SetApplLimit 0xA02D
#define _BlockMove 0xA02E
#define _BlockMoveData 0xA22E
#define _MemoryDispatch 0xA05C
#define _MemoryDispatchA0Result 0xA15C
#define _DeferUserFn 0xA08F
#define _DebugUtil 0xA08D

/*

    ; Event Manager

*/

#define _PostEvent 0xA02F
#define _PPostEvent 0xA12F
#define _OSEventAvail 0xA030
#define _GetOSEvent 0xA031
#define _FlushEvents 0xA032
#define _VInstall 0xA033
#define _VRemove 0xA034
#define _OffLine 0xA035
#define _MoreMasters 0xA036
#define _WriteParam 0xA038
#define _ReadDateTime 0xA039
#define _SetDateTime 0xA03A
#define _Delay 0xA03B
#define _CmpString 0xA03C
#define _DrvrInstall 0xA03D
#define _DrvrRemove 0xA03E
#define _InitUtil 0xA03F
#define _ResrvMem 0xA040
#define _GetTrapAddress 0xA146
#define _SetTrapAddress 0xA047
#define _GetOSTrapAddress 0xA346
#define _SetOSTrapAddress 0xA247
#define _GetToolTrapAddress 0xA746
#define _SetToolTrapAddress 0xA647
#define _GetToolBoxTrapAddress 0xA746
#define _SetToolBoxTrapAddress 0xA647
#define _PtrZone 0xA148
#define _HPurge 0xA049
#define _HNoPurge 0xA04A
#define _SetGrowZone 0xA04B
#define _CompactMem 0xA04C
#define _PurgeMem 0xA04D
#define _AddDrive 0xA04E
#define _RDrvrInstall 0xA04F
#define _LwrString 0xA056
#define _UprString 0xA054
#define _SetApplBase 0xA057
#define _HWPriv 0xA198

/*

    ; New names for (mostly) new flavors of old LwrString trap (redone <13>)
*/

#define _LowerText 0xA056
#define _StripText 0xA256
#define _UpperText 0xA456
#define _StripUpperText 0xA656

/*

    ; Temporary Memory routines

*/

#define _OSDispatch 0xA88F
#define _RelString 0xA050
#define _CompareString 0xA050
#define _ReadXPRam 0xA051
#define _WriteXPRam 0xA052
#define _InsTime 0xA058
#define _InsXTime 0xA458
#define _RmvTime 0xA059
#define _PrimeTime 0xA05A
#define _PowerOff 0xA05B
#define _MaxBlock 0xA061
#define _PurgeSpace 0xA162
#define _PurgeSpaceSys 0xA562
#define _MaxApplZone 0xA063
#define _MoveHHi 0xA064
#define _StackSpace 0xA065
#define _NewEmptyHandle 0xA166
#define _HSetRBit 0xA067
#define _HClrRBit 0xA068
#define _HGetState 0xA069
#define _HSetState 0xA06A
#define _InitFS 0xA06C
#define _InitEvents 0xA06D
#define _StripAddress 0xA055
#define _Translate24To32 0xA091
#define _SetAppBase 0xA057
#define _SwapMMUMode 0xA05D
#define _SlotVInstall 0xA06F
#define _SlotVRemove 0xA070
#define _AttachVBL 0xA071
#define _DoVBLTask 0xA072
#define _SIntInstall 0xA075
#define _SIntRemove 0xA076
#define _CountADBs 0xA077
#define _GetIndADB 0xA078
#define _GetADBInfo 0xA079
#define _SetADBInfo 0xA07A
#define _ADBReInit 0xA07B
#define _ADBOp 0xA07C
#define _GetDefaultStartup 0xA07D
#define _SetDefaultStartup 0xA07E
#define _InternalWait 0xA07F
#define _RGetResource 0xA80C
#define _GetVideoDefault 0xA080
#define _SetVideoDefault 0xA081
#define _DTInstall 0xA082
#define _SetOSDefault 0xA083
#define _GetOSDefault 0xA084
#define _IOPInfoAccess 0xA086
#define _IOPMsgRequest 0xA087
#define _IOPMoveData 0xA088

/*

    ; Power Manager

*/

#define _PowerDispatch 0xA09F
#define _PMgrOp 0xA085
#define _IdleUpdate 0xA285
#define _IdleState 0xA485
#define _SerialPower 0xA685
#define _Sleep 0xA08A
#define _SleepQInstall 0xA28A
#define _SlpQInstall 0xA28A
#define _SleepQRemove 0xA48A
#define _SlpQRemove 0xA48A

/*

    ; Comm. Toolbox

*/

#define _CommToolboxDispatch 0xA08B
#define _SysEnvirons 0xA090

/*

    ; Egret Manager

*/

#define _EgretDispatch 0xA092
#define _Gestalt 0xA1AD
#define _NewGestalt 0xA3AD
#define _ReplaceGestalt 0xA5AD
#define _GetGestaltProcPtr 0xA7AD
#define _InitProcMenu 0xA808
#define _GetItemCmd 0xA84E
#define _SetItemCmd 0xA84F
#define _PopUpMenuSelect 0xA80B
#define _KeyTrans 0xA9C3
#define _KeyTranslate 0xA9C3

/*

    ; TextEdit

*/

#define _TEGetText 0xA9CB
#define _TEInit 0xA9CC
#define _TEDispose 0xA9CD
#define _TextBox 0xA9CE
#define _TETextBox 0xA9CE
#define _TESetText 0xA9CF
#define _TECalText 0xA9D0
#define _TESetSelect 0xA9D1
#define _TENew 0xA9D2
#define _TEUpdate 0xA9D3
#define _TEClick 0xA9D4
#define _TECopy 0xA9D5
#define _TECut 0xA9D6
#define _TEDelete 0xA9D7
#define _TEActivate 0xA9D8
#define _TEDeactivate 0xA9D9
#define _TEIdle 0xA9DA
#define _TEPaste 0xA9DB
#define _TEKey 0xA9DC
#define _TEScroll 0xA9DD
#define _TEInsert 0xA9DE
#define _TESetJust 0xA9DF
#define _TESetAlignment 0xA9DF
#define _TEGetOffset 0xA83C
#define _TEDispatch 0xA83D
#define _TEStyleNew 0xA83E

/*

    ; Color Quickdraw

*/

#define _OpenCPort 0xAA00
#define _OpenCport 0xAA00
#define _InitCPort 0xAA01
#define _InitCport 0xAA01
#define _CloseCPort 0xA87D
#define _CloseCport 0xA87D
#define _NewPixMap 0xAA03
#define _DisposPixMap 0xAA04
#define _DisposePixMap 0xAA04
#define _CopyPixMap 0xAA05
#define _SetPortPix 0xAA06
#define _NewPixPat 0xAA07
#define _DisposPixPat 0xAA08
#define _DisposePixPat 0xAA08
#define _CopyPixPat 0xAA09
#define _PenPixPat 0xAA0A
#define _BackPixPat 0xAA0B
#define _GetPixPat 0xAA0C
#define _MakeRGBPat 0xAA0D
#define _FillCRect 0xAA0E
#define _FillCOval 0xAA0F
#define _FillCRoundRect 0xAA10
#define _FillCArc 0xAA11
#define _FillCRgn 0xAA12
#define _FillCPoly 0xAA13
#define _RGBForeColor 0xAA14
#define _RGBBackColor 0xAA15
#define _SetCPixel 0xAA16
#define _GetCPixel 0xAA17
#define _GetCTable 0xAA18
#define _GetForeColor 0xAA19
#define _GetBackColor 0xAA1A
#define _GetCCursor 0xAA1B
#define _SetCCursor 0xAA1C
#define _AllocCursor 0xAA1D
#define _GetCIcon 0xAA1E
#define _PlotCIcon 0xAA1F
#define _OpenCPicture 0xAA20
#define _OpColor 0xAA21
#define _HiliteColor 0xAA22
#define _CharExtra 0xAA23
#define _DisposCTable 0xAA24
#define _DisposeCTable 0xAA24
#define _DisposCIcon 0xAA25
#define _DisposeCIcon 0xAA25
#define _DisposCCursor 0xAA26
#define _DisposeCCursor 0xAA26
#define _SeedCFill 0xAA50
#define _CalcCMask 0xAA4F
#define _CopyDeepMask 0xAA51

/*

    ; Routines for video devices

*/

#define _GetMaxDevice 0xAA27
#define _GetCTSeed 0xAA28
#define _GetDeviceList 0xAA29
#define _GetMainDevice 0xAA2A
#define _GetNextDevice 0xAA2B
#define _TestDeviceAttribute 0xAA2C
#define _SetDeviceAttribute 0xAA2D
#define _InitGDevice 0xAA2E
#define _NewGDevice 0xAA2F
#define _DisposGDevice 0xAA30
#define _DisposeGDevice 0xAA30
#define _SetGDevice 0xAA31
#define _GetGDevice 0xAA32
#define _DeviceLoop 0xABCA

/*

    ; Color Manager

*/

#define _Color2Index 0xAA33
#define _Index2Color 0xAA34
#define _InvertColor 0xAA35
#define _RealColor 0xAA36
#define _GetSubTable 0xAA37
#define _UpdatePixMap 0xAA38

/*

    ; Dialog Manager

*/

#define _NewCDialog 0xAA4B
#define _NewColorDialog 0xAA4B
#define _MakeITable 0xAA39
#define _AddSearch 0xAA3A
#define _AddComp 0xAA3B
#define _SetClientID 0xAA3C
#define _ProtectEntry 0xAA3D
#define _ReserveEntry 0xAA3E
#define _SetEntries 0xAA3F
#define _QDError 0xAA40
#define _SaveEntries 0xAA49
#define _RestoreEntries 0xAA4A
#define _DelSearch 0xAA4C
#define _DelComp 0xAA4D
#define _SetStdCProcs 0xAA4E
#define _StdOpcodeProc 0xABF8

/*

    ; added to Toolbox for color

*/

#define _SetWinColor 0xAA41
#define _GetAuxWin 0xAA42
#define _SetCtlColor 0xAA43
#define _SetControlColor 0xAA43
#define _GetAuxCtl 0xAA44
#define _GetAuxiliaryControlRecord 0xAA44
#define _NewCWindow 0xAA45
#define _GetNewCWindow 0xAA46
#define _SetDeskCPat 0xAA47
#define _GetCWMgrPort 0xAA48
#define _GetCVariant 0xA809
#define _GetControlVariant 0xA809
#define _GetWVariant 0xA80A

/*

    ; added to Menu Manager for color

*/

#define _DelMCEntries 0xAA60
#define _DeleteMCEntries 0xAA60
#define _GetMCInfo 0xAA61
#define _SetMCInfo 0xAA62
#define _DispMCInfo 0xAA63
#define _DisposeMCInfo 0xAA63
#define _GetMCEntry 0xAA64
#define _SetMCEntries 0xAA65

/*

    ; Menu Manager

*/

#define _MenuChoice 0xAA66

/*

    ; Dialog Manager?

*/

#define _ModalDialogMenuSetup 0xAA67
#define _DialogDispatch 0xAA68

/*

    ; Font Manager

*/

#define _SetFractEnable 0xA814
#define _FontDispatch 0xA854

/*

    ; Palette Manager

*/

#define _InitPalettes 0xAA90
#define _NewPalette 0xAA91
#define _GetNewPalette 0xAA92
#define _DisposePalette 0xAA93
#define _ActivatePalette 0xAA94
#define _SetPalette 0xAA95
#define _NSetPalette 0xAA95
#define _GetPalette 0xAA96
#define _PmForeColor 0xAA97
#define _PmBackColor 0xAA98
#define _AnimateEntry 0xAA99
#define _AnimatePalette 0xAA9A
#define _GetEntryColor 0xAA9B
#define _SetEntryColor 0xAA9C
#define _GetEntryUsage 0xAA9D
#define _SetEntryUsage 0xAA9E
#define _CTab2Palette 0xAA9F
#define _Palette2CTab 0xAAA0
#define _CopyPalette 0xAAA1
#define _PaletteDispatch 0xAAA2

/*

    ; Sound Manager

*/

#define _SoundDispatch 0xA800
#define _SndDisposeChannel 0xA801
#define _SndAddModifier 0xA802
#define _SndDoCommand 0xA803
#define _SndDoImmediate 0xA804
#define _SndPlay 0xA805
#define _SndControl 0xA806
#define _SndNewChannel 0xA807
#define _SlotManager 0xA06E
#define _ScriptUtil 0xA8B5
#define _SCSIAtomic 0xA089
#define _SCSIDispatch 0xA815
#define _Long2Fix 0xA83F
#define _Fix2Long 0xA840
#define _Fix2Frac 0xA841
#define _Frac2Fix 0xA842
#define _Fix2X 0xA843
#define _X2Fix 0xA844
#define _Frac2X 0xA845
#define _X2Frac 0xA846
#define _FracCos 0xA847
#define _FracSin 0xA848
#define _FracSqrt 0xA849
#define _FracMul 0xA84A
#define _FracDiv 0xA84B
#define _FixDiv 0xA84D
#define _NMInstall 0xA05E
#define _NMRemove 0xA05F

/*

    ; All QDOffscreen Routines go through one trap with a selector

*/

#define _QDExtensions 0xAB1D

/*

    ; UserDelay

*/

#define _UserDelay 0xA84C

/*

    ; Component Manager

*/

#define _ComponentDispatch 0xA82A


/*

	; PowerPC Mac

*/

#define _MixedModeMagic 		0xAAFE
#define _MixedModeDispatch		0xAA59
#define _CodeFragmentDispatch	0xAA5A

/*

    ; Translation Manager

*/

#define _TranslationDispatch 0xABFC


/*
	; Human Interface Utilities
*/

#define	_HumanInterfaceUtilsDispatch	0xAADD


#define _InitDogCow 0xA89F
#define _EnableDogCow 0xA89F
#define _DisableDogCow 0xA89F
#define _Moof 0xA89F
#define _HFSPinaforeDispatch 0xAA52

#endif