/*
 * netxFlashAnalyzer.h
 *
 *  Created on: 04.03.2020
 *      Author: dirk
 */

#ifndef INC_NETXFLASHANALYZER_H_
#define INC_NETXFLASHANALYZER_H_

#include <stdio.h>
#include <stdint.h>
#include "Hil_Compiler.h"


#define HIL_HBOOT_STANDARD_COOKIE                       0xF3BEAF00
#define HIL_HBOOT_NO_AUTO_DETECTION_SQI_FLASHES_COOKIE  0xF3BEAF1A
#define HIL_HBOOT_ALTERNATIVE_IMAGE_COOKIE              0xF3AD9E00






/* HBOOT BOOT header netX90/4000 (64 bytes, used for NAI and NAE) */
typedef __HIL_PACKED_PRE struct __HIL_PACKED_POST HIL_FILE_HBOOT_BOOT_HEADER_NAI_NAE_V1_0_Ttag {
	uint32_t     ulMagic;
	uint32_t     ulFlashOffsetBytes;
	uint32_t     pulNextHeader;
	uint32_t     pulDestination;
	uint32_t     ulImageSizeDword;
	uint32_t     ulFlashSelection;
	uint32_t     ulSignature;
	uint32_t     ulReserved;
	uint32_t     aulHash[7];
	uint32_t     ulBootChksm;

} HIL_FILE_HBOOT_BOOT_HEADER_NAI_NAE_V1_0_T;




typedef struct FILE_Ttag {
	char szSuffix[5];
	uint32_t ulOffset;
	uint32_t ulLength;
	FILE* hFile;
} FILE_T;

typedef enum FILE_TYPE_Etag {
	FILETYPE_FLASHDUMP, // a COM side flash image
	FILETYPE_NXI,  // firmware for COM side
	FILETYPE_FDL,  // flash device label
	FILETYPE_MXF,  // maintenance firmware
	FILETYPE_HWC,  // hardware config
	FILETYPE_MWC,  // hardware config for maintenance firmware
	FILETYPE_RDT,  // remanent data
	FILETYPE_MNG,  // managmenet data
	FILETYPE_UPD, // update area file
	FILETYPE_NAI, // user firmware for APP side
	FILETYPE_NXF, // legacy firmware, netX 51, netX 52 , etc
	FILETYPE_UNKNOWN,
}FILE_TYPE_E;


#define USE_CASE_A 0
#define USE_CASE_B 1
#define USE_CASE_C 2


extern char* LookupCode(uint32_t ulCmd);
extern char* LookupComClassCode(uint16_t ulCmd);
extern char* LookupProtClassCode(uint16_t ulCmd);
extern char* LookupDevClassCode(uint16_t ulCmd);
extern char* LookupDevTypeCode(uint32_t ulCmd);
extern char* LookupChipTypeCode(uint8_t ulCmd);


#endif /* INC_NETXFLASHANALYZER_H_ */
