/*
 ============================================================================
 Name        : netxFlashAnalyzer.c
 Author      : Dirk Fischer
 Version     : 1.1.0.0
 Copyright   : Hilscher Gesellschaft für Systemautomation mbH
               MIT License
 Description : netXFileChecker utility to analyse netX 90 flash memory dumps
               or single files like *.fdl, *.nxi, etc.
               utility provides FDL generator functionality

 ChangeLog:
               V1.1.0.0 2020-06-09  updated Hil_DeviceProductionData.h
                                    added functionality to CreateFDL()
                                      - works for use case A now
                                      - added CRC32 checksum calculation
                                      - no use case B/C support yet

               V1.0.0.0 2020-04-23 Initial

 ============================================================================
 */

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "netXFileChecker.h"
#include "Hil_FileHeaderV3.h"
#include "Hil_DeviceProductionData.h"

#define MAX_FILENAME_LEN 100

uint32_t PS_CRC32(uint32_t ulPrevCrc, const uint8_t* pabBuffer, size_t numBytes);



FILE_T tFlashDumpFile[][8] ={
		{
				{".hwc",0x00000,0x02000,0,},
				{".fdl",0x02000,0x01000,0,},
				{".nxi",0x03000,0x7D000,0,},
				{".upd",0x80000,0x5F000,0,},
				{".mwc",0xDF000,0x02000,0,},
				{".mxf",0xE1000,0x15000,0,},
				{".rdt",0xF6000,0x08000,0,},
				{".mng",0xFE000,0x02000,0,},
		},
		{
				{".hwc",0x00000,0x02000,0,},
				{".fdl",0x02000,0x01000,0,},
				{".nxi",0x03000,0xDC000,0,},
				{".mwc",0xDF000,0x02000,0,},
				{".mxf",0xE1000,0x15000,0,},
				{".rdt",0xF6000,0x08000,0,},
				{".mng",0xFE000,0x02000,0,},
		},
		{
				{".hwc",0x00000,0x02000,0,},
				{".fdl",0x02000,0x01000,0,},
				{".nxi",0x03000,0xDC000,0,},
				{".mwc",0xDF000,0x02000,0,},
				{".mxf",0xE1000,0x1F000,0,},
		},

};


FILE_T tSQIDumpFile[][3] ={
		{
		},
		{
				{".upd",0x000000,0x05F000,0,},
		},
		{
				{".fat",0x080000,0x23E000,0,},
				{".rdt",0x2BE000,0x040000,0,},
				{".mng",0x2FE000,0x002000,0,},
		},
};



void printHelp(char* szCommandName){
	printf("usage: %s options filename\n",szCommandName);
	printf("options: \n");
	printf("         -h    print help\n");
	printf("         -u    use case A|B|C default:A, used for flash dump analysis and creation\n");
	printf("         -s    split, in case of a flash dump analysis, separate files are created\n");
	printf("         -fdl  create a standard flash device label, parameter -u might be used\n");

	printf("\nflash image analysis requires specification of use case (command line parameter -u)\n"
			"no automatic evaluation of FDL supported\n");

	printf("\nflash image analysis requires an input file with suffix *.bin\n");

	printf("\nfile analysis depends on file suffix\n"
			"no automatic file type detection supported\n"
			"FDL generator supports use case A only\n");


	printf("\nfile suffixs: \n");
	printf("         .bin flash dump                               \n");
	printf("         .nxi firmware                                 \n");
	printf("         .fdl flash device label                       \n");
	printf("         .mxf maintenance firmware                     \n");
	printf("         .hwc hardware config                          \n");
	printf("         .mwc hardware config for maintenance firmware \n");
	printf("         .rdt remanent data                            \n");
	printf("         .mng managmenet data                          \n");
	printf("         .upd update area file                         \n");
	printf("         .nai user firmware on APP side                \n");
	printf("         .nxf legacy firmware netX 51, netx 52, etc.   \n");
}


FILE* getFileHandle(int iUseCase,char *szSuffix){
	int i=0;

	for(i=0;i<sizeof(tFlashDumpFile[iUseCase])/sizeof(FILE_T);i++){
		if(0 == strcmp(szSuffix,tFlashDumpFile[iUseCase][i].szSuffix)){
			return tFlashDumpFile[iUseCase][i].hFile;
		}
	}
	return NULL;
}


uint32_t getOffset(int iUseCase,char *szSuffix){
	int i=0;

	for(i=0;i<sizeof(tFlashDumpFile[iUseCase])/sizeof(FILE_T);i++){
		if(0 == strcmp(szSuffix,tFlashDumpFile[iUseCase][i].szSuffix)){
			return tFlashDumpFile[iUseCase][i].ulOffset;
		}
	}
	return 0;
}

uint32_t getLength(int iUseCase,char *szSuffix){
	int i=0;

	for(i=0;i<sizeof(tFlashDumpFile[iUseCase])/sizeof(FILE_T);i++){
		if(0 == strcmp(szSuffix,tFlashDumpFile[iUseCase][i].szSuffix)){
			return tFlashDumpFile[iUseCase][i].ulLength;
		}
	}
	return 0;
}




char* getSuffix(int iUseCase, FILE* hFile){
	int i=0;

	for(i=0;i<sizeof(tFlashDumpFile[iUseCase])/sizeof(FILE_T);i++){
		if(hFile==tFlashDumpFile[iUseCase][i].hFile){
			return tFlashDumpFile[iUseCase][i].szSuffix;
		}
	}
	return NULL;
}


int OpenOutFile(char *szFilename, char *szSuffix, FILE** hFile){
	char szNewFilename[MAX_FILENAME_LEN];
	strcpy(szNewFilename,szFilename);
	strcat(szNewFilename, szSuffix);
	*hFile=fopen(szNewFilename,"wb");
	if(*hFile == NULL){
		printf("error opening file %s\n",szNewFilename);
		return 1;
	}
	return 0;
}



int WriteData(int iUseCase, char szSuffix[], FILE* hInFile, char* szFilename){

	uint8_t *abBuffer=0;

	fpos_t offset=getOffset(iUseCase,szSuffix);
	size_t size=getLength(iUseCase,szSuffix);
	FILE* hOutFile=getFileHandle(iUseCase,szSuffix);

	printf("write offset:0x%05x, size:0x%05x [%dKB] into %s%s\n",(int)offset,(int)size,(int)size/1024,szFilename,getSuffix(iUseCase, hOutFile));

	abBuffer=malloc(size);
	if(abBuffer==NULL){
		printf("error malloc\n");
		return EXIT_FAILURE;
	}
	fsetpos(hInFile,&offset);
	fread(abBuffer,sizeof(uint8_t),size,hInFile);
	fwrite(abBuffer,sizeof(uint8_t),size,hOutFile);
	free(abBuffer);

	return 0;
}





int AnalyzeNxfBootHeader(fpos_t offset, FILE* hInFile){
	uint8_t *abBuffer=0;
	size_t size=sizeof(HIL_FILE_BOOT_HEADER_V1_0_T);
	uint8_t*abSignature;

	HIL_FILE_BOOT_HEADER_V1_0_T* ptBootHeader=0;


	abBuffer=malloc(size);
	if(abBuffer==NULL){
		printf("error malloc\n");
		return EXIT_FAILURE;
	}
	fsetpos(hInFile,&offset);
	fread(abBuffer,sizeof(uint8_t),size,hInFile);
	ptBootHeader=(HIL_FILE_BOOT_HEADER_V1_0_T*)abBuffer;
	abSignature=(uint8_t*)&ptBootHeader->ulSignature;

	printf("\n--------------------------------------\nV3 BOOT HEADER ANALYSIS \n");
	printf("check offset:   0x%05x\n",(int)offset);
	printf("--------------------------------------\n");

	printf("Cookie:          0x%08x - ",ptBootHeader->ulMagicCookie);
	printf("%s\n",LookupCode(ptBootHeader->ulMagicCookie));
	printf("Signature:       0x%08x - ",ptBootHeader->ulSignature);
	printf("%c%c%c%c\n",(char)abSignature[0],(char)abSignature[1],(char)abSignature[2],(char)abSignature[3]);
	printf("Entry Point:     0x%08x\n",ptBootHeader->ulAppEntryPoint);
	printf("Start Address:   0x%08x\n",ptBootHeader->ulAppStartAddress);
	printf("AppFileSize DW:  %d [%dKB]\n",ptBootHeader->ulAppFileSize,ptBootHeader->ulAppFileSize/1024*4);
	printf("App Checksum:    0x%08x\n",ptBootHeader->ulAppChecksum);
	printf("Header Checksum: 0x%08x\n",ptBootHeader->ulBootHeaderChecksum);
	printf("DeviceType:      %d - ",ptBootHeader->ulSrcDeviceType);
	printf("%s\n",LookupDevTypeCode(ptBootHeader->ulSrcDeviceType));
	printf("SerialNumber:    %d\n",ptBootHeader->ulSerialNumber);



	free(abBuffer);
	return 0;
}



int AnalyzeNaiBootHeader(fpos_t offset, FILE* hInFile){
	uint8_t *abBuffer=0;
	size_t size=sizeof(HIL_FILE_BOOT_HEADER_NAI_NAE_V1_0_T);
	uint8_t*abSignature;


	HIL_FILE_BOOT_HEADER_NAI_NAE_V1_0_T* ptBootHeader=0;


	abBuffer=malloc(size);
	if(abBuffer==NULL){
		printf("error malloc\n");
		return EXIT_FAILURE;
	}
	fsetpos(hInFile,&offset);
	fread(abBuffer,sizeof(uint8_t),size,hInFile);
	ptBootHeader=(HIL_FILE_BOOT_HEADER_NAI_NAE_V1_0_T*)abBuffer;
	abSignature=(uint8_t*)&ptBootHeader->ulSignature;

	printf("\n--------------------------------------\nV3 BOOT HEADER ANALYSIS \n");
	printf("check offset:   0x%05x\n",(int)offset);
	printf("--------------------------------------\n");

	printf("Cookie:          0x%08x - ",ptBootHeader->ulMagicCookie);
	printf("%s\n",LookupCode(ptBootHeader->ulMagicCookie));
	printf("Signature:       0x%08x - ",ptBootHeader->ulSignature);
	printf("%c%c%c%c\n",(char)abSignature[0],(char)abSignature[1],(char)abSignature[2],(char)abSignature[3]);
	printf("AppFileSize DW:  %d [%dKB]\n",ptBootHeader->ulAppFileSize,ptBootHeader->ulAppFileSize/1024*4);
	printf("App Checksum:    0x%08x\n",ptBootHeader->ulAppChecksum);
	printf("Header Checksum: 0x%08x\n",ptBootHeader->ulBootHeaderChecksum);

	free(abBuffer);
	return 0;
}



int AnalyzeNaiHBoot_BootHeader(fpos_t offset, FILE* hInFile){
	uint8_t *abBuffer=0;
	size_t size=sizeof(HIL_FILE_HBOOT_BOOT_HEADER_NAI_NAE_V1_0_T);
	uint8_t*abSignature;


	HIL_FILE_HBOOT_BOOT_HEADER_NAI_NAE_V1_0_T* ptBootHeader=0;


	abBuffer=malloc(size);
	if(abBuffer==NULL){
		printf("error malloc\n");
		return EXIT_FAILURE;
	}
	fsetpos(hInFile,&offset);
	fread(abBuffer,sizeof(uint8_t),size,hInFile);
	ptBootHeader=(HIL_FILE_HBOOT_BOOT_HEADER_NAI_NAE_V1_0_T*)abBuffer;
	abSignature=(uint8_t*)&ptBootHeader->ulSignature;

	printf("\n--------------------------------------\nV3 HBOOT HEADER ANALYSIS \n");
	printf("check offset:   0x%05x\n",(int)offset);
	printf("--------------------------------------\n");

	printf("Magic:          0x%08x - ",ptBootHeader->ulMagic);
	printf("%s\n",LookupCode(ptBootHeader->ulMagic));
	printf("Next Header:    0x%08x\n",ptBootHeader->pulNextHeader);
	printf("Destination:    0x%08x\n",ptBootHeader->pulDestination);
	printf("Flash Offset:   0x%08x\n",ptBootHeader->ulFlashOffsetBytes);
	printf("FlashSelection: 0x%08x\n",ptBootHeader->ulFlashSelection);
	printf("Signature:      0x%08x - ",ptBootHeader->ulSignature);
	printf("%c%c%c%c\n",(char)abSignature[0],(char)abSignature[1],(char)abSignature[2],(char)abSignature[3]);
	printf("Boot Checksum:  0x%08x\n",ptBootHeader->ulBootChksm);

	free(abBuffer);
	return 0;
}






int AnalyzeFHV3CommonHeader(fpos_t offset, FILE* hInFile){
	int i=0;
	uint8_t *abBuffer=0;
	uint8_t bNumModuleInfos=0;
	size_t size=sizeof(HIL_FILE_COMMON_HEADER_V3_0_T);


	HIL_FILE_COMMON_HEADER_V3_0_T* ptCommonHeader=0;
	HIL_FILE_MODULE_INFO_V1_0_T* ptModuleInfo=0;


	abBuffer=malloc(size);
	if(abBuffer==NULL){
		printf("error malloc\n");
		return EXIT_FAILURE;
	}
	fsetpos(hInFile,&offset);
	fread(abBuffer,sizeof(uint8_t),size,hInFile);
	ptCommonHeader=(HIL_FILE_COMMON_HEADER_V3_0_T*)abBuffer;

	printf("\n--------------------------------------\nV3 COMMON HEADER ANALYSIS \n");
	printf("check offset:   0x%05x\n",(int)offset);
	printf("--------------------------------------\n");
	printf("Header CRC32:   0x%08x\n",ptCommonHeader->ulHeaderCRC32);
	printf("Common CRC32:   0x%08x\n",ptCommonHeader->ulCommonCRC32);
	printf("Number Modules: %d\n",ptCommonHeader->bNumModuleInfos);

	bNumModuleInfos=ptCommonHeader->bNumModuleInfos;
	free(abBuffer);


	if(bNumModuleInfos){
		offset=offset+sizeof(HIL_FILE_COMMON_HEADER_V3_0_T)+sizeof(HIL_FILE_DEVICE_INFO_V1_0_T);
		size=bNumModuleInfos * sizeof(HIL_FILE_MODULE_INFO_V1_0_T);
		abBuffer=malloc(size);
		if(abBuffer==NULL){
			printf("error malloc\n");
			return EXIT_FAILURE;
		}
		fsetpos(hInFile,&offset);
		fread(abBuffer,sizeof(uint8_t),size,hInFile);
		ptModuleInfo=(HIL_FILE_MODULE_INFO_V1_0_T*)abBuffer;

		printf("--------\nMODULES\n--------\n");
		if(bNumModuleInfos>6) {
			bNumModuleInfos=6;
		}

		for(i=0;i<bNumModuleInfos;i++){
			if(ptModuleInfo[i].usProtocolClass!=0 && ptModuleInfo[i].usProtocolClass!=0xFFFF){
				printf("module %d\n",i);
				printf("Protocol Class: 0x%04x - ",ptModuleInfo[i].usProtocolClass);
				printf("%s\n",LookupProtClassCode(ptModuleInfo[i].usProtocolClass));
				printf("Comm. Class:    0x%04x - ",ptModuleInfo[i].usCommunicationClass);
				printf("%s\n",LookupComClassCode(ptModuleInfo[i].usCommunicationClass));
			}
		}
		free(abBuffer);
	}



	return 0;

}



int AnalyzeFHV3DeviceInfo(fpos_t offset, FILE* hInFile){
	uint8_t *abBuffer=0;
	size_t size=sizeof(HIL_FILE_DEVICE_INFO_V1_0_T);


	HIL_FILE_DEVICE_INFO_V1_0_T* ptDeviceInfo=0;


	abBuffer=malloc(size);
	if(abBuffer==NULL){
		printf("error malloc\n");
		return EXIT_FAILURE;
	}
	fsetpos(hInFile,&offset);
	fread(abBuffer,sizeof(uint8_t),size,hInFile);
	ptDeviceInfo=(HIL_FILE_DEVICE_INFO_V1_0_T*)abBuffer;

	printf("\n--------------------------------------\nV3 DEVICE INFO ANALYSIS \n");
	printf("check offset:   0x%05x\n",(int)offset);
	printf("--------------------------------------\n");
	printf("Manufacturer:   0x%04x\n",ptDeviceInfo->usManufacturer);
	printf("Device Class:   0x%04x - ",ptDeviceInfo->usDeviceClass);
	printf("%s\n",LookupDevClassCode(ptDeviceInfo->usDeviceClass));
	printf("ChipType:       0x%02x - ",ptDeviceInfo->bChipType);
	printf("%s\n",LookupChipTypeCode(ptDeviceInfo->bChipType));
	printf("HW Compatib:    0x%02x\n",ptDeviceInfo->bHwCompatibility);
	printf("HW Options:     0x%04x 0x%04x 0x%04x 0x%04x \n",ptDeviceInfo->ausHwOptions[0],ptDeviceInfo->ausHwOptions[1],ptDeviceInfo->ausHwOptions[2],ptDeviceInfo->ausHwOptions[3]);
	printf("FW Version:     %d.%d.%d.%d\n",(int)ptDeviceInfo->ausFwVersion[0],(int)ptDeviceInfo->ausFwVersion[1],(int)ptDeviceInfo->ausFwVersion[2],(int)ptDeviceInfo->ausFwVersion[3]);
	printf("FW Number:      %d\n",ptDeviceInfo->ulFwNumber);
	printf("Device Number:  %d\n",ptDeviceInfo->ulDeviceNumber);
	printf("Serial Number:  %d\n",ptDeviceInfo->ulSerialNumber);


	free(abBuffer);

	return 0;

}



int AnalyzeNxfFileHeader(fpos_t offset, FILE* hInFile){

	AnalyzeNxfBootHeader(offset,hInFile);
	offset+=sizeof(HIL_FILE_BOOT_HEADER_V1_0_T);
	AnalyzeFHV3CommonHeader(offset,hInFile);
	offset+=sizeof(HIL_FILE_COMMON_HEADER_V3_0_T);
	AnalyzeFHV3DeviceInfo(offset,hInFile);

	return 0;
}


int AnalyzeNaiFileHeader(fpos_t offset, FILE* hInFile){


	offset+=448; // first 448 bytes: vector table
	AnalyzeNaiHBoot_BootHeader(offset,hInFile);
	offset+=sizeof(HIL_FILE_HBOOT_BOOT_HEADER_NAI_NAE_V1_0_T);
	AnalyzeNaiBootHeader(offset,hInFile);
	offset+=sizeof(HIL_FILE_BOOT_HEADER_NAI_NAE_V1_0_T);
	AnalyzeFHV3CommonHeader(offset,hInFile);
	offset+=sizeof(HIL_FILE_COMMON_HEADER_V3_0_T);
	AnalyzeFHV3DeviceInfo(offset,hInFile);

	return 0;
}









int AnalyzeFDL(fpos_t offset, FILE* hInFile){
	int i=0;
	uint8_t *abBuffer=0;
	size_t size=sizeof(HIL_PRODUCT_DATA_LABEL_T);

	HIL_PRODUCT_DATA_LABEL_T* ptFDL=0;

	abBuffer=malloc(size);
	if(abBuffer==NULL){
		printf("error malloc\n");
		return EXIT_FAILURE;
	}
	fsetpos(hInFile,&offset);
	fread(abBuffer,sizeof(uint8_t),size,hInFile);
	ptFDL=(HIL_PRODUCT_DATA_LABEL_T*)abBuffer;

	printf("\n--------------------------------------\nFDL ANALYSIS\n");
	printf("check offset:   0x%05x\n",(int)offset);
	printf("--------------------------------------\n");

	printf("Manufacturer ID:     0x%04x\n",ptFDL->tProductData.tBasicDeviceData.usManufacturer);
	printf("Device Number:       %d\n",ptFDL->tProductData.tBasicDeviceData.ulDeviceNumber);
	printf("Serial Number:       %d\n",ptFDL->tProductData.tBasicDeviceData.ulSerialNumber);
	printf("HW Revision:         0x%02x\n",ptFDL->tProductData.tBasicDeviceData.bHwRevision);
	printf("Production Date:     0x%04x [year %d - week %d]\n",ptFDL->tProductData.tBasicDeviceData.usProductionDate, ((ptFDL->tProductData.tBasicDeviceData.usProductionDate >> 8) & 0x00ff) + 2000, ((ptFDL->tProductData.tBasicDeviceData.usProductionDate >> 0) & 0x00ff));
	printf("HW Compatibility:    0x%02x \n",ptFDL->tProductData.tBasicDeviceData.bHwCompatibility);
	printf("Device Class:        0x%04x [%s]\n",ptFDL->tProductData.tBasicDeviceData.usDeviceClass,LookupDevClassCode(ptFDL->tProductData.tBasicDeviceData.usDeviceClass));

	for(i=0;i<8;i++){
		printf("MAC Address %d COM:   %02x:%02x:%02x:%02x:%02x:%02x\n",i+1\
				,ptFDL->tProductData.tMACAddressesCom.atMAC[i].abMacAddress[0]\
				,ptFDL->tProductData.tMACAddressesCom.atMAC[i].abMacAddress[1]\
				,ptFDL->tProductData.tMACAddressesCom.atMAC[i].abMacAddress[2]\
				,ptFDL->tProductData.tMACAddressesCom.atMAC[i].abMacAddress[3]\
				,ptFDL->tProductData.tMACAddressesCom.atMAC[i].abMacAddress[4]\
				,ptFDL->tProductData.tMACAddressesCom.atMAC[i].abMacAddress[5]\
		);
	}

	for(i=0;i<4;i++){
		printf("MAC Address %d APP:   %02x:%02x:%02x:%02x:%02x:%02x\n",i+1\
				,ptFDL->tProductData.tMACAddressesApp.atMAC[i].abMacAddress[0]\
				,ptFDL->tProductData.tMACAddressesApp.atMAC[i].abMacAddress[1]\
				,ptFDL->tProductData.tMACAddressesApp.atMAC[i].abMacAddress[2]\
				,ptFDL->tProductData.tMACAddressesApp.atMAC[i].abMacAddress[3]\
				,ptFDL->tProductData.tMACAddressesApp.atMAC[i].abMacAddress[4]\
				,ptFDL->tProductData.tMACAddressesApp.atMAC[i].abMacAddress[5]\
		);
	}
	
	if(ptFDL->tProductData.tOEMIdentification.ulOemDataOptionFlags){
		printf("OEM Serial Number:    %s\n",ptFDL->tProductData.tOEMIdentification.szSerialNumber);
		printf("OEM Order Number:     %s\n",ptFDL->tProductData.tOEMIdentification.szOrderNumber);
		printf("OEM HW Revision:      %s\n",ptFDL->tProductData.tOEMIdentification.szHardwareRevision);
		printf("OEM Production Date:  %s\n",ptFDL->tProductData.tOEMIdentification.szProductionDate);
	}
	else {
		printf("No OEM Data enabled\n");
	}

	printf("\nFlash Chips\n");
	for(i=0;i<4;i++){
		if(ptFDL->tProductData.tFlashLayout.atChip[i].ulFlashSize==0)
			break;
		printf(" %16s",ptFDL->tProductData.tFlashLayout.atChip[i].szFlashName);
		printf(" 0x%02x",ptFDL->tProductData.tFlashLayout.atChip[i].ulChipNumber);
		printf(" 0x%08x [%6dKB]",ptFDL->tProductData.tFlashLayout.atChip[i].ulFlashSize,ptFDL->tProductData.tFlashLayout.atChip[i].ulFlashSize/1024);
		printf(" 0x%08x",ptFDL->tProductData.tFlashLayout.atChip[i].ulBlockSize);
		printf(" %d",ptFDL->tProductData.tFlashLayout.atChip[i].ulMaxEnduranceCycles);
		printf("\n");
	}
	printf("\nFlash Areas\n");
	for(i=0;i<10;i++){
		if(ptFDL->tProductData.tFlashLayout.atArea[i].ulContentType==0)
			break;
		printf(" %16s",ptFDL->tProductData.tFlashLayout.atArea[i].szName);
		printf(" 0x%02x",ptFDL->tProductData.tFlashLayout.atArea[i].ulContentType);
		printf(" %1d",ptFDL->tProductData.tFlashLayout.atArea[i].ulChipNumber);
		printf(" 0x%08x",ptFDL->tProductData.tFlashLayout.atArea[i].ulAreaStart);
		printf(" 0x%08x [%4dKB]",ptFDL->tProductData.tFlashLayout.atArea[i].ulAreaSize,ptFDL->tProductData.tFlashLayout.atArea[i].ulAreaSize/1024);
		printf(" 0x%02x",ptFDL->tProductData.tFlashLayout.atArea[i].bAccessTyp);
		printf("\n");
	}


	free(abBuffer);

	return 0;
}


/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
/* NOT COMPLETED YET*/
/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
int CreateFDL(char* szFileName, int iUseCase){
	int i=0;
	uint8_t *abBuffer=0;
	uint32_t ulChecksum=0;
	size_t size=sizeof(HIL_PRODUCT_DATA_LABEL_T);
	FILE* hOutFile=NULL;

	HIL_PRODUCT_DATA_LABEL_T* ptFDL=0;

	abBuffer=malloc(size);
	if(abBuffer==NULL){
		printf("error malloc\n");
		return EXIT_FAILURE;
	}

	memset(abBuffer,0x00,sizeof(HIL_PRODUCT_DATA_LABEL_T));

	ptFDL=(HIL_PRODUCT_DATA_LABEL_T*)abBuffer;


/*HEADER*/
	memcpy(ptFDL->tHeader.abStartToken,HIL_PRODUCT_DATA_START_TOKEN,strlen(HIL_PRODUCT_DATA_START_TOKEN));
	ptFDL->tHeader.usContentSize=sizeof(HIL_PRODUCT_DATA_T);
	ptFDL->tHeader.usLabelSize=ptFDL->tHeader.usContentSize+sizeof(HIL_PRODUCT_DATA_HEADER_T)+sizeof(HIL_PRODUCT_DATA_FOOTER_T);

/*BODY*/


/*	  HIL_PRODUCT_DATA_BASIC_DEVICE_DATA_T        tBasicDeviceData;*/

	ptFDL->tProductData.tBasicDeviceData.usManufacturer=0x0001;
	ptFDL->tProductData.tBasicDeviceData.usDeviceClass=HIL_HW_DEV_CLASS_CHIP_NETX_90_COM;
	ptFDL->tProductData.tBasicDeviceData.ulDeviceNumber=7833000;
	ptFDL->tProductData.tBasicDeviceData.ulSerialNumber=20000;
	ptFDL->tProductData.tBasicDeviceData.usProductionDate=0x112B; // Format is 0xYYWW, YY + 2000
	ptFDL->tProductData.tBasicDeviceData.bHwRevision=0x03;
	ptFDL->tProductData.tBasicDeviceData.bHwCompatibility=0x00;


/*	  HIL_PRODUCT_DATA_LIBSTORAGE_T               tFlashLayout;*/

	ptFDL->tProductData.tFlashLayout.atChip[0].ulChipNumber=0;
	ptFDL->tProductData.tFlashLayout.atChip[0].ulFlashSize=0x00080000; //512KB
	ptFDL->tProductData.tFlashLayout.atChip[0].ulBlockSize=0x00001000;
	ptFDL->tProductData.tFlashLayout.atChip[0].ulMaxEnduranceCycles=10000;

	ptFDL->tProductData.tFlashLayout.atChip[1].ulChipNumber=1;
	ptFDL->tProductData.tFlashLayout.atChip[1].ulFlashSize=0x00080000; //512KB
	ptFDL->tProductData.tFlashLayout.atChip[1].ulBlockSize=0x00001000;
	ptFDL->tProductData.tFlashLayout.atChip[1].ulMaxEnduranceCycles=10000;


	memcpy(ptFDL->tProductData.tFlashLayout.atArea[0].szName,"HWConfig",sizeof("HWConfig"));
	ptFDL->tProductData.tFlashLayout.atArea[0].ulContentType=HIL_PRODUCT_DATA_FLASH_LAYOUT_CONTENT_TYPE_HWCONFIG;
	ptFDL->tProductData.tFlashLayout.atArea[0].ulChipNumber=0;
	ptFDL->tProductData.tFlashLayout.atArea[0].ulAreaStart=0x00100000;
	ptFDL->tProductData.tFlashLayout.atArea[0].ulAreaSize=0x00002000;
	ptFDL->tProductData.tFlashLayout.atArea[0].bAccessTyp=0x00; // read only

	memcpy(ptFDL->tProductData.tFlashLayout.atArea[1].szName,"FDL",sizeof("FDL"));
	ptFDL->tProductData.tFlashLayout.atArea[1].ulContentType=HIL_PRODUCT_DATA_FLASH_LAYOUT_CONTENT_TYPE_FDL;
	ptFDL->tProductData.tFlashLayout.atArea[1].ulChipNumber=0;
	ptFDL->tProductData.tFlashLayout.atArea[1].ulAreaStart=0x00102000;
	ptFDL->tProductData.tFlashLayout.atArea[1].ulAreaSize=0x00001000;
	ptFDL->tProductData.tFlashLayout.atArea[1].bAccessTyp=0x00; // read only

	memcpy(ptFDL->tProductData.tFlashLayout.atArea[2].szName,"FW",sizeof("FW"));
	ptFDL->tProductData.tFlashLayout.atArea[2].ulContentType=HIL_PRODUCT_DATA_FLASH_LAYOUT_CONTENT_TYPE_FW;
	ptFDL->tProductData.tFlashLayout.atArea[2].ulChipNumber=0;
	ptFDL->tProductData.tFlashLayout.atArea[2].ulAreaStart=0x00103000;
	ptFDL->tProductData.tFlashLayout.atArea[2].ulAreaSize=0x0007D000;
	ptFDL->tProductData.tFlashLayout.atArea[2].bAccessTyp=0x00; // read only



	memcpy(ptFDL->tProductData.tFlashLayout.atArea[3].szName,"FWUpdate",sizeof("FWUpdate"));
	ptFDL->tProductData.tFlashLayout.atArea[3].ulContentType=HIL_PRODUCT_DATA_FLASH_LAYOUT_CONTENT_TYPE_FWUPDATE;
	ptFDL->tProductData.tFlashLayout.atArea[3].ulChipNumber=1;
	ptFDL->tProductData.tFlashLayout.atArea[3].ulAreaStart=0x00180000;
	ptFDL->tProductData.tFlashLayout.atArea[3].ulAreaSize=0x0005F000;
	ptFDL->tProductData.tFlashLayout.atArea[3].bAccessTyp=0x02; // read write

	memcpy(ptFDL->tProductData.tFlashLayout.atArea[4].szName,"MFW_HWConfig",sizeof("MFW_HWConfig"));
	ptFDL->tProductData.tFlashLayout.atArea[4].ulContentType=HIL_PRODUCT_DATA_FLASH_LAYOUT_CONTENT_TYPE_MFW_HWCONFIG;
	ptFDL->tProductData.tFlashLayout.atArea[4].ulChipNumber=1;
	ptFDL->tProductData.tFlashLayout.atArea[4].ulAreaStart=0x001DF000;
	ptFDL->tProductData.tFlashLayout.atArea[4].ulAreaSize=0x00002000;
	ptFDL->tProductData.tFlashLayout.atArea[4].bAccessTyp=0x00; // read only

	memcpy(ptFDL->tProductData.tFlashLayout.atArea[5].szName,"Maintenance",sizeof("Maintenance"));
	ptFDL->tProductData.tFlashLayout.atArea[5].ulContentType=HIL_PRODUCT_DATA_FLASH_LAYOUT_CONTENT_TYPE_MFW;
	ptFDL->tProductData.tFlashLayout.atArea[5].ulChipNumber=1;
	ptFDL->tProductData.tFlashLayout.atArea[5].ulAreaStart=0x001E1000;
	ptFDL->tProductData.tFlashLayout.atArea[5].ulAreaSize=0x00015000;
	ptFDL->tProductData.tFlashLayout.atArea[5].bAccessTyp=0x00; // read only

	memcpy(ptFDL->tProductData.tFlashLayout.atArea[6].szName,"Remanent",sizeof("Remanent"));
	ptFDL->tProductData.tFlashLayout.atArea[6].ulContentType=HIL_PRODUCT_DATA_FLASH_LAYOUT_CONTENT_TYPE_REMANENT;
	ptFDL->tProductData.tFlashLayout.atArea[6].ulChipNumber=1;
	ptFDL->tProductData.tFlashLayout.atArea[6].ulAreaStart=0x001F6000;
	ptFDL->tProductData.tFlashLayout.atArea[6].ulAreaSize=0x00008000;
	ptFDL->tProductData.tFlashLayout.atArea[6].bAccessTyp=0x02; // read write

	memcpy(ptFDL->tProductData.tFlashLayout.atArea[7].szName,"Management",sizeof("Management"));
	ptFDL->tProductData.tFlashLayout.atArea[7].ulContentType=HIL_PRODUCT_DATA_FLASH_LAYOUT_CONTENT_TYPE_MANAGEMENT;
	ptFDL->tProductData.tFlashLayout.atArea[7].ulChipNumber=1;
	ptFDL->tProductData.tFlashLayout.atArea[7].ulAreaStart=0x001FE000;
	ptFDL->tProductData.tFlashLayout.atArea[7].ulAreaSize=0x00002000;
	ptFDL->tProductData.tFlashLayout.atArea[7].bAccessTyp=0x02; // read write



/*	  HIL_PRODUCT_DATA_MAC_ADDRESSES_COM_T        tMACAddressesCom;*/

	uint8_t abMacCom[4][6]={
			{0x02,0x00,0x00,0x1E,0x99,0x00},
			{0x02,0x00,0x00,0x1E,0x99,0x01},
			{0x02,0x00,0x00,0x1E,0x99,0x02},
			{0x02,0x00,0x00,0x1E,0x99,0x03},
	};

	for(i=0;i<4;i++){
		memcpy(ptFDL->tProductData.tMACAddressesCom.atMAC[i].abMacAddress,abMacCom[i],sizeof(abMacCom[i]));
	}


/*	  HIL_PRODUCT_DATA_MAC_ADDRESSES_APP_T        tMACAddressesApp;*/

	// UNUSED, keep 0x00


/*	  HIL_PRODUCT_DATA_OEM_IDENTIFICATION_T       tOEMIdentification;*/
#if(0)
	ptFDL->tProductData.tOEMIdentification.ulOemDataOptionFlags=0x0000000F;
	memcpy(ptFDL->tProductData.tOEMIdentification.szSerialNumber,"F-1234-5678",sizeof("F-1234-5678"));
	memcpy(ptFDL->tProductData.tOEMIdentification.szOrderNumber,"0815",sizeof("0815"));
	memcpy(ptFDL->tProductData.tOEMIdentification.szHardwareRevision,"01",sizeof("01"));
	memcpy(ptFDL->tProductData.tOEMIdentification.szProductionDate,"2020-04-01",sizeof("2020-04-01"));


/*	  HIL_PRODUCT_DATA_PRODUCT_IDENTIFICATION_T   tProductIdentification;*/

	ptFDL->tProductData.tProductIdentification.tUSBInfo.usUSBProductID=0x1234;
	ptFDL->tProductData.tProductIdentification.tUSBInfo.usUSBVendorID=0x1234;
	memcpy(ptFDL->tProductData.tProductIdentification.tUSBInfo.abUSBProductName,"ProductABC",sizeof("ProductABC"));
	memcpy(ptFDL->tProductData.tProductIdentification.tUSBInfo.abUSBVendorName,"VendorABC",sizeof("VendorABC"));
#endif


	/*calculate checksum CRC-32 (IEEE 802.3) of content ptFDL->tProductData*/
	ulChecksum=PS_CRC32(0x00000000,(uint8_t *)&ptFDL->tProductData,sizeof(HIL_PRODUCT_DATA_T));



/*FOOTER*/
	ptFDL->tFooter.ulChecksum=ulChecksum;
	memcpy(ptFDL->tFooter.abEndToken,HIL_PRODUCT_DATA_END_TOKEN,strlen(HIL_PRODUCT_DATA_END_TOKEN));





	hOutFile=fopen(szFileName,"wb");
	if(hOutFile==NULL){
		printf("\nError opening file %s\n",szFileName);
		return EXIT_FAILURE;
	}

	fwrite(ptFDL,sizeof(HIL_PRODUCT_DATA_LABEL_T),1,hOutFile);

	uint8_t bClear = 0xFF;
	for(i=0;i<(0x1000-sizeof(HIL_PRODUCT_DATA_LABEL_T));i++){
		fwrite((void*)&bClear, sizeof(uint8_t), 1, hOutFile);
	}
	fclose(hOutFile);

	free(abBuffer);
	return EXIT_SUCCESS;

}








/***
 *                      _        __ __
 *                     (_)      / / \ \
 *      _ __ ___   __ _ _ _ __ | |   | |
 *     | '_ ` _ \ / _` | | '_ \| |   | |
 *     | | | | | | (_| | | | | | |   | |
 *     |_| |_| |_|\__,_|_|_| |_| |   | |
 *                              \_\ /_/
 *
 *    http://patorjk.com/software/taag
 */

int main(int argc, char *argv[])
{
	FILE *hInFile=NULL;
	char *szFilename=NULL;
	int i;
	int iRes=0;
	int iUseCase=0;
	FILE_TYPE_E eFileType=FILETYPE_UNKNOWN;
	char szInFileSuffix[5]={0};


	bool bSplitFlashImage=false;
	bool bCreateFDL=false;


	if( argc == 1 )
	{
		printHelp(argv[0]);
		return EXIT_FAILURE;
	}
	else{
		for(i=1;i<argc;i++){
			if(!strcmp(argv[i],"-s")){
				bSplitFlashImage=true;
				continue;
			}
			if(!strcmp(argv[i],"-fdl")){
				bCreateFDL=true;
				continue;
			}
			if(!strcmp(argv[i],"-u")){
				i++;
				if (i == argc) {
					printf("error: no use case given\n");
					printHelp(argv[0]);
					return EXIT_FAILURE;
				}
				switch (argv[i][0]){
				case 'A':
					iUseCase=USE_CASE_A;
					break;
				case 'B':
					iUseCase=USE_CASE_B;
					break;
				case 'C':
					iUseCase=USE_CASE_C;
					break;
				default:
					printf("error: invalid use case\n");
					printHelp(argv[0]);
					return EXIT_FAILURE;
				}
				continue;
			}
			if(!strcmp(argv[i],"-h")){
				printHelp(argv[0]);
				return EXIT_SUCCESS;
			}
			if (argv[i][0] == '-'){
				printf("unknown option \"%s\" is ignored \n",argv[i]);
			} else {
				i--;
				break;
			}
		}
		if (i < argc) {
			szFilename=(char*)argv[argc-1]; // the last command line parameter is always the filename
		} else {
			printf("error: no filename given\n");
			printHelp(argv[0]);
			return EXIT_FAILURE;
		}
		if(strlen(szFilename)>4)
		{
			strcpy(szInFileSuffix, &szFilename[strlen(szFilename)-4]);
		}
		else {
			printf("Error: filename too short\n");
			return EXIT_FAILURE;
		}


		if(strcmp(szInFileSuffix,".bin")==0 || strcmp(szInFileSuffix,".BIN")==0){
			eFileType=FILETYPE_FLASHDUMP;
		}
		if(strcmp(szInFileSuffix,".fdl")==0 || strcmp(szInFileSuffix,".FDL")==0){
			eFileType=FILETYPE_FDL;
		}
		if(strcmp(szInFileSuffix,".nxi")==0 || strcmp(szInFileSuffix,".NXI")==0){
			eFileType=FILETYPE_NXI;
		}
		if(strcmp(szInFileSuffix,".mxf")==0 || strcmp(szInFileSuffix,".MXF")==0){
			eFileType=FILETYPE_MXF;
		}
		if(strcmp(szInFileSuffix,".hwc")==0 || strcmp(szInFileSuffix,".HWC")==0){
			eFileType=FILETYPE_HWC;
		}
		if(strcmp(szInFileSuffix,".mwc")==0 || strcmp(szInFileSuffix,".MWC")==0){
			eFileType=FILETYPE_MWC;
		}
		if(strcmp(szInFileSuffix,".rdt")==0 || strcmp(szInFileSuffix,".RDT")==0){
			eFileType=FILETYPE_RDT;
		}
		if(strcmp(szInFileSuffix,".mng")==0 || strcmp(szInFileSuffix,".MNG")==0){
			eFileType=FILETYPE_MNG;
		}
		if(strcmp(szInFileSuffix,".upd")==0 || strcmp(szInFileSuffix,".UPD")==0){
			eFileType=FILETYPE_UPD;
		}
		if(strcmp(szInFileSuffix,".nai")==0 || strcmp(szInFileSuffix,".NAI")==0){
			eFileType=FILETYPE_NAI;
		}

		if(strcmp(szInFileSuffix,".nxf")==0 || strcmp(szInFileSuffix,".NXF")==0){
			eFileType=FILETYPE_NXF;
		}


		if(eFileType==FILETYPE_UNKNOWN) {
			printf("Error: unknown file extension %s\n",szInFileSuffix);
			return EXIT_FAILURE;
		}

	}



	if(bCreateFDL){
		iRes=CreateFDL(szFilename, iUseCase);
		if(iRes){
			printf("Error: FDL creation failed: 0x%08x\n",iRes);
			return EXIT_FAILURE;
		}
		else {
			printf("OK: FDL created\n");
			return EXIT_SUCCESS;
		}
	}


	hInFile=fopen(szFilename,"rb");
	if(hInFile==NULL){
		printf("\nError opening file %s\n",szFilename);
		return EXIT_FAILURE;
	}

	printf("\nanalyze ");
	if(eFileType == FILETYPE_FLASHDUMP) {
		printf("FLASH DUMP file");
		switch (iUseCase){
		case USE_CASE_A:
			printf(" [use case A] ");
			break;
		case USE_CASE_B:
			printf(" [use case B] ");
			break;
		case USE_CASE_C:
			printf(" [use case C] ");
			break;
		default:
			printf(" [invalid use case] ");
			break;
		}

	}
	else
		printf("file");

	printf(" %s\n\n",szFilename);

	if(bSplitFlashImage==true && (eFileType == FILETYPE_FLASHDUMP)){

		printf("create separate files\n");

		for(i=0;i<sizeof(tFlashDumpFile[iUseCase])/sizeof(FILE_T);i++){
			iRes=OpenOutFile(szFilename,tFlashDumpFile[iUseCase][i].szSuffix,&tFlashDumpFile[iUseCase][i].hFile);
		}


		WriteData(iUseCase, ".hwc", hInFile, szFilename);
		WriteData(iUseCase, ".fdl", hInFile, szFilename);
		WriteData(iUseCase, ".nxi", hInFile, szFilename);
		WriteData(iUseCase, ".upd", hInFile, szFilename);
		WriteData(iUseCase, ".mwc", hInFile, szFilename);
		WriteData(iUseCase, ".mxf", hInFile, szFilename);
		WriteData(iUseCase, ".rdt", hInFile, szFilename);
		WriteData(iUseCase, ".mng", hInFile, szFilename);

		for(i=0;i<sizeof(tFlashDumpFile[iUseCase])/sizeof(FILE_T);i++){
			iRes=fclose(tFlashDumpFile[iUseCase][i].hFile);
			if(iRes){
				printf("error closing file %s\n",getSuffix(iUseCase, tFlashDumpFile[iUseCase][i].hFile));
			}
		}

	}



	switch(eFileType){
	case FILETYPE_FLASHDUMP:
		printf("\n-------------------------------- Flash Device Label FDL----------------------------------\n");
		AnalyzeFDL(getOffset(iUseCase, ".fdl"), hInFile); // FDL
		printf("\n-------------------------------------- Firmware NXI--------------------------------------\n");
		printf("offset: 0x%08x, areasize: 0x%08x [%dKB]\n",getOffset(iUseCase, ".nxi"),getLength(iUseCase, ".nxi"),getLength(iUseCase, ".nxi")/1024);
		AnalyzeNxfFileHeader(getOffset(iUseCase, ".nxi"),hInFile);

		if(iUseCase==USE_CASE_A || iUseCase==USE_CASE_B){
			printf("\n-------------------------------------- Update Area --------------------------------------\n");
			printf("offset: 0x%08x, areasize: 0x%08x [%dKB]\n",getOffset(iUseCase, ".upd"),getLength(iUseCase, ".upd"),getLength(iUseCase, ".upd")/1024);
			AnalyzeNxfFileHeader(getOffset(iUseCase, ".upd"),hInFile);
		}

		printf("\n-------------------------------- Maintenance Firmware MXF--------------------------------\n");
		printf("offset: 0x%08x, areasize: 0x%08x [%dKB]\n",getOffset(iUseCase, ".mxf"),getLength(iUseCase, ".mxf"),getLength(iUseCase, ".mxf")/1024);
		AnalyzeNxfFileHeader(getOffset(iUseCase, ".mxf"),hInFile);
		break;

	case FILETYPE_FDL:
		AnalyzeFDL(0x0000, hInFile); // FDL
		break;

	case FILETYPE_NXF:
	case FILETYPE_NXI:
	case FILETYPE_MXF:
	case FILETYPE_UPD:
		AnalyzeNxfFileHeader(0x0000,hInFile);
		break;

	case FILETYPE_NAI:
		AnalyzeNaiFileHeader(0x0000, hInFile);
		break;


	case FILETYPE_UNKNOWN:
		printf("Error: unknown file extension\n");
		return EXIT_FAILURE;
		break;

	default:
		printf("No file analyzer for %s\n",szInFileSuffix);
		break;

	}





	iRes=fclose(hInFile);
	if(iRes){
		printf("error closing file %s\n",szFilename);
	}


	return EXIT_SUCCESS;
}

