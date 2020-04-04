/*
 * Written by ZhongYao Luo <luozhongyao@gmail.com>
 *
 * Copyright 2020 ZhongYao Luo
 */

#ifndef __FAT_H__
#define __FAT_H__

#define BS_JmpBoot			0		/* x86 jump instruction (3-byte) */
#define BS_OEMName			3		/* OEM name (8-byte) */
#define BPB_BytsPerSec		11		/* Sector size [byte] (WORD) */
#define BPB_SecPerClus		13		/* Cluster size [sector] (BYTE) */
#define BPB_RsvdSecCnt		14		/* Size of reserved area [sector] (WORD) */
#define BPB_NumFATs			16		/* Number of FATs (BYTE) */
#define BPB_RootEntCnt		17		/* Size of root directory area for FAT12/16 [entry] (WORD) */
#define BPB_TotSec16		19		/* Volume size (16-bit) [sector] (WORD) */
#define BPB_Media			21		/* Media descriptor byte (BYTE) */
#define BPB_FATSz16			22		/* FAT size (16-bit) [sector] (WORD) */
#define BPB_SecPerTrk		24		/* Track size for int13h [sector] (WORD) */
#define BPB_NumHeads		26		/* Number of heads for int13h (WORD) */
#define BPB_HiddSec			28		/* Volume offset from top of the drive (DWORD) */
#define BPB_TotSec32		32		/* Volume size (32-bit) [sector] (DWORD) */
#define BS_DrvNum			36		/* Physical drive number for int13h (BYTE) */
#define BS_NTres			37		/* Error flag (BYTE) */
#define BS_BootSig			38		/* Extended boot signature (BYTE) */
#define BS_VolID			39		/* Volume serial number (DWORD) */
#define BS_VolLab			43		/* Volume label string (8-byte) */
#define BS_FilSysType		54		/* File system type string (8-byte) */
#define BS_BootCode			62		/* Boot code (448-byte) */
#define BS_55AA				510		/* Signature word (WORD) */

#define BPB_FATSz32			36		/* FAT32: FAT size [sector] (DWORD) */
#define BPB_ExtFlags32		40		/* FAT32: Extended flags (WORD) */
#define BPB_FSVer32			42		/* FAT32: File system version (WORD) */
#define BPB_RootClus32		44		/* FAT32: Root directory cluster (DWORD) */
#define BPB_FSInfo32		48		/* FAT32: Offset of FSINFO sector (WORD) */
#define BPB_BkBootSec32		50		/* FAT32: Offset of backup boot sector (WORD) */
#define BS_DrvNum32			64		/* FAT32: Physical drive number for int13h (BYTE) */
#define BS_NTres32			65		/* FAT32: Error flag (BYTE) */
#define BS_BootSig32		66		/* FAT32: Extended boot signature (BYTE) */
#define BS_VolID32			67		/* FAT32: Volume serial number (DWORD) */
#define BS_VolLab32			71		/* FAT32: Volume label string (8-byte) */
#define BS_FilSysType32		82		/* FAT32: File system type string (8-byte) */
#define BS_BootCode32		90		/* FAT32: Boot code (420-byte) */

#define BPB_ZeroedEx		11		/* exFAT: MBZ field (53-byte) */
#define BPB_VolOfsEx		64		/* exFAT: Volume offset from top of the drive [sector] (QWORD) */
#define BPB_TotSecEx		72		/* exFAT: Volume size [sector] (QWORD) */
#define BPB_FatOfsEx		80		/* exFAT: FAT offset from top of the volume [sector] (DWORD) */
#define BPB_FatSzEx			84		/* exFAT: FAT size [sector] (DWORD) */
#define BPB_DataOfsEx		88		/* exFAT: Data offset from top of the volume [sector] (DWORD) */
#define BPB_NumClusEx		92		/* exFAT: Number of clusters (DWORD) */
#define BPB_RootClusEx		96		/* exFAT: Root directory cluster (DWORD) */
#define BPB_VolIDEx			100		/* exFAT: Volume serial number (DWORD) */
#define BPB_FSVerEx			104		/* exFAT: File system version (WORD) */
#define BPB_VolFlagEx		106		/* exFAT: Volume flags (BYTE) */
#define BPB_ActFatEx		107		/* exFAT: Active FAT flags (BYTE) */
#define BPB_BytsPerSecEx	108		/* exFAT: Log2 of sector size in byte (BYTE) */
#define BPB_SecPerClusEx	109		/* exFAT: Log2 of cluster size in sector (BYTE) */
#define BPB_NumFATsEx		110		/* exFAT: Number of FATs (BYTE) */
#define BPB_DrvNumEx		111		/* exFAT: Physical drive number for int13h (BYTE) */
#define BPB_PercInUseEx		112		/* exFAT: Percent in use (BYTE) */
#define	BPB_RsvdEx			113		/* exFAT: Reserved (7-byte) */
#define BS_BootCodeEx		120		/* exFAT: Boot code (390-byte) */

#define	FSI_LeadSig			0		/* FAT32 FSI: Leading signature (DWORD) */
#define	FSI_StrucSig		484		/* FAT32 FSI: Structure signature (DWORD) */
#define	FSI_Free_Count		488		/* FAT32 FSI: Number of free clusters (DWORD) */
#define	FSI_Nxt_Free		492		/* FAT32 FSI: Last allocated cluster (DWORD) */

#define MBR_Table			446		/* MBR: Offset of partition table in the MBR */
#define	SZ_PTE				16		/* MBR: Size of a partition table entry */
#define PTE_Boot			0		/* MBR PTE: Boot indicator */
#define PTE_StHead			1		/* MBR PTE: Start head */
#define PTE_StSec			2		/* MBR PTE: Start sector */
#define PTE_StCyl			3		/* MBR PTE: Start cylinder */
#define PTE_System			4		/* MBR PTE: System ID */
#define PTE_EdHead			5		/* MBR PTE: End head */
#define PTE_EdSec			6		/* MBR PTE: End sector */
#define PTE_EdCyl			7		/* MBR PTE: End cylinder */
#define PTE_StLba			8		/* MBR PTE: Start in LBA */
#define PTE_SizLba			12		/* MBR PTE: Size in LBA */

#define	DIR_Name			0		/* Short file name (11-byte) */
#define	DIR_Attr			11		/* Attribute (BYTE) */
#define	DIR_NTres			12		/* Lower case flag (BYTE) */
#define DIR_CrtTime10		13		/* Created time sub-second (BYTE) */
#define	DIR_CrtTime			14		/* Created time (DWORD) */
#define DIR_LstAccDate		18		/* Last accessed date (WORD) */
#define	DIR_FstClusHI		20		/* Higher 16-bit of first cluster (WORD) */
#define	DIR_ModTime			22		/* Modified time (DWORD) */
#define	DIR_FstClusLO		26		/* Lower 16-bit of first cluster (WORD) */
#define	DIR_FileSize		28		/* File size (DWORD) */
#define	LDIR_Ord			0		/* LFN entry order and LLE flag (BYTE) */
#define	LDIR_Attr			11		/* LFN attribute (BYTE) */
#define	LDIR_Type			12		/* LFN type (BYTE) */
#define	LDIR_Chksum			13		/* Checksum of the SFN entry (BYTE) */
#define	LDIR_FstClusLO		26		/* Must be zero (WORD) */
#define	XDIR_Type			0		/* Type of exFAT directory entry (BYTE) */
#define	XDIR_NumLabel		1		/* Number of volume label characters (BYTE) */
#define	XDIR_Label			2		/* Volume label (11-WORD) */
#define	XDIR_CaseSum		4		/* Sum of case conversion table (DWORD) */
#define	XDIR_NumSec			1		/* Number of secondary entries (BYTE) */
#define	XDIR_SetSum			2		/* Sum of the set of directory entries (WORD) */
#define	XDIR_Attr			4		/* File attribute (WORD) */
#define	XDIR_CrtTime		8		/* Created time (DWORD) */
#define	XDIR_ModTime		12		/* Modified time (DWORD) */
#define	XDIR_AccTime		16		/* Last accessed time (DWORD) */
#define	XDIR_CrtTime10		20		/* Created time subsecond (BYTE) */
#define	XDIR_ModTime10		21		/* Modified time subsecond (BYTE) */
#define	XDIR_CrtTZ			22		/* Created timezone (BYTE) */
#define	XDIR_ModTZ			23		/* Modified timezone (BYTE) */
#define	XDIR_AccTZ			24		/* Last accessed timezone (BYTE) */
#define	XDIR_GenFlags		33		/* Gneral secondary flags (WORD) */
#define	XDIR_NumName		35		/* Number of file name characters (BYTE) */
#define	XDIR_NameHash		36		/* Hash of file name (WORD) */
#define XDIR_ValidFileSize	40		/* Valid file size (QWORD) */
#define	XDIR_FstClus		52		/* First cluster of the file data (DWORD) */
#define	XDIR_FileSize		56		/* File/Directory size (QWORD) */

#define	SZDIRE				32		/* Size of a directory entry */
#define	LLEF				0x40	/* Last long entry flag in LDIR_Ord */
#define	DDEM				0xE5	/* Deleted directory entry mark set to DIR_Name[0] */
#define	RDDEM				0x05	/* Replacement of the character collides with DDEM */

#endif /* __FAT_H__*/

