#include "appInfo.h"
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "spi_arduino.h"
#include <semaphore.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>

static emt_state_t emt_state;
int driver_data_offset = 0;

void handleSignal(int n, siginfo_t *info, void *unused) {
	driver_data_offset = info->si_int;
	emt_state = EMT_STATE_SIGNAL_RECEIVED;
}

int generateDataForSending(cBmpFile *inFile, applicationParameters *appParam)
{
	unsigned int idx, idy, idz, ids;
	uint8_t **bitArrayRS;

	/* BMP Design Length */
	appParam->designLength = inFile->bmInfoHeader.biHeight;
	printf("BMP Design Length = %d\n", appParam->designLength);

	/* # of equipements = Width / 8 */
	appParam->totalNoEquipments = inFile->bmInfoHeader.biWidth/8;
	if(appParam->totalNoEquipments > MAXIMUM_SUPPORTED_S2P_CARDS*MAXIMUM_EQUIPMENTS_ON_CARDS) {
		printf("This design is bigger than HW system!");
		exit(0);
	}
	appParam->totalNoS2PCards = (appParam->totalNoEquipments/MAXIMUM_EQUIPMENTS_ON_CARDS);
	if(appParam->totalNoEquipments % MAXIMUM_EQUIPMENTS_ON_CARDS != 0)
		appParam->totalNoS2PCards += 1;

	printf("Total # of Equipments required = %d\n", appParam->totalNoEquipments);
	printf("Total # of S2P Cards required = %d\n", appParam->totalNoS2PCards);

	/* Create 2D Array for Reshuffling */
	bitArrayRS = (uint8_t **) malloc(inFile->bmInfoHeader.biHeight * sizeof(uint8_t *));
	for(idx = 0; idx < inFile->bmInfoHeader.biHeight; idx++) {
		bitArrayRS[idx] = (uint8_t *) malloc( inFile->bmInfoHeader.biWidth * sizeof(uint8_t));
	}

	/* Reshuffle 2D bitArray */
	for(idx=0; idx<inFile->bmInfoHeader.biHeight; idx++) {
		for(idy=0; idy<inFile->bmInfoHeader.biWidth;) {
#if 0
			for(idz=0,ids=idy; idz<NO_OF_EQUIPMENTS_IN_A_LINE; idz++, ids+=2, idy+=8) {
				/* TODO: Change this logic, if Pixel arrangement is required */
				bitArrayRS[idx][idy+0] = appParam->bitArray[idx][ids+0];
				bitArrayRS[idx][idy+1] = appParam->bitArray[idx][ids+1];
				bitArrayRS[idx][idy+2] = appParam->bitArray[idx][ids+0+NO_OF_EQUIPMENTS_IN_A_LINE*2];
				bitArrayRS[idx][idy+3] = appParam->bitArray[idx][ids+1+NO_OF_EQUIPMENTS_IN_A_LINE*2];
				bitArrayRS[idx][idy+4] = appParam->bitArray[idx][ids+0+NO_OF_EQUIPMENTS_IN_A_LINE*4];
				bitArrayRS[idx][idy+5] = appParam->bitArray[idx][ids+1+NO_OF_EQUIPMENTS_IN_A_LINE*4];
				bitArrayRS[idx][idy+6] = appParam->bitArray[idx][ids+0+NO_OF_EQUIPMENTS_IN_A_LINE*6];
				bitArrayRS[idx][idy+7] = appParam->bitArray[idx][ids+1+NO_OF_EQUIPMENTS_IN_A_LINE*6];
				#if DEBUG_ENABLED
				printf("%3d %3d %3d %3d %3d %3d %3d %3d\n",
								ids+0, ids+1,
								ids+0+NO_OF_EQUIPMENTS_IN_A_LINE*2, ids+1+NO_OF_EQUIPMENTS_IN_A_LINE*2,
								ids+0+NO_OF_EQUIPMENTS_IN_A_LINE*4, ids+1+NO_OF_EQUIPMENTS_IN_A_LINE*4,
								ids+0+NO_OF_EQUIPMENTS_IN_A_LINE*6, ids+1+NO_OF_EQUIPMENTS_IN_A_LINE*6);
				#endif
			}
#endif
#if 1
			for(idz=0,ids=idy; idz<NO_OF_EQUIPMENTS_IN_A_LINE; idz++, ids+=1, idy+=8) {
				/* TODO: Change this logic, if Pixel arrangement is required */
				bitArrayRS[idx][idy+0] = appParam->bitArray[idx][ids+0];
				bitArrayRS[idx][idy+1] = appParam->bitArray[idx][ids+NO_OF_EQUIPMENTS_IN_A_LINE*1];
				bitArrayRS[idx][idy+2] = appParam->bitArray[idx][ids+NO_OF_EQUIPMENTS_IN_A_LINE*2];
				bitArrayRS[idx][idy+3] = appParam->bitArray[idx][ids+NO_OF_EQUIPMENTS_IN_A_LINE*3];
				bitArrayRS[idx][idy+4] = appParam->bitArray[idx][ids+NO_OF_EQUIPMENTS_IN_A_LINE*4];
				bitArrayRS[idx][idy+5] = appParam->bitArray[idx][ids+NO_OF_EQUIPMENTS_IN_A_LINE*5];
				bitArrayRS[idx][idy+6] = appParam->bitArray[idx][ids+NO_OF_EQUIPMENTS_IN_A_LINE*6];
				bitArrayRS[idx][idy+7] = appParam->bitArray[idx][ids+NO_OF_EQUIPMENTS_IN_A_LINE*7];
				#if DEBUG_ENABLED
				printf("%3d %3d %3d %3d %3d %3d %3d %3d\n",
								ids+0, ids+NO_OF_EQUIPMENTS_IN_A_LINE*1,
								ids+NO_OF_EQUIPMENTS_IN_A_LINE*2, ids+NO_OF_EQUIPMENTS_IN_A_LINE*3,
								ids+NO_OF_EQUIPMENTS_IN_A_LINE*4, ids+NO_OF_EQUIPMENTS_IN_A_LINE*5,
								ids+NO_OF_EQUIPMENTS_IN_A_LINE*6, ids+NO_OF_EQUIPMENTS_IN_A_LINE*7);
				#endif
			}
#endif

			#if DEBUG_ENABLED
			printf("idx=%d, idy=%d, ids=%d\n", idx, idy, ids);
			#endif
		}
	}

	/* Total Bytes to send for this BMP image = Design Length/Height x No of Bytes/Equipments for each Row */
	appParam->dataForEquipments = (uint8_t *) malloc ( appParam->designLength * appParam->totalNoEquipments );
	for(idx=0, ids=0; idx<inFile->bmInfoHeader.biHeight; idx++) {
		for(idy=0; idy<inFile->bmInfoHeader.biWidth; idy+=8) {
			/* TODO: Change the order here, if MSB <--> LSB need to switch on Equipment LEDs */
			appParam->dataForEquipments[ids++] =
#if 1
				(bitArrayRS[idx][idy+0] << 7) |
				(bitArrayRS[idx][idy+1] << 6) |
				(bitArrayRS[idx][idy+2] << 5) |
				(bitArrayRS[idx][idy+3] << 4) |
				(bitArrayRS[idx][idy+4] << 3) |
				(bitArrayRS[idx][idy+5] << 2) |
				(bitArrayRS[idx][idy+6] << 1) |
				(bitArrayRS[idx][idy+7] << 0);
#endif
#if 0
				(bitArrayRS[idx][idy+0] << 1) |
				(bitArrayRS[idx][idy+1] << 0) |
				(bitArrayRS[idx][idy+2] << 3) |
				(bitArrayRS[idx][idy+3] << 2) |
				(bitArrayRS[idx][idy+4] << 5) |
				(bitArrayRS[idx][idy+5] << 4) |
				(bitArrayRS[idx][idy+6] << 7) |
				(bitArrayRS[idx][idy+7] << 6);
#endif
#if 0
				(bitArrayRS[idx][idy+0] << 0) |
				(bitArrayRS[idx][idy+1] << 1) |
				(bitArrayRS[idx][idy+2] << 2) |
				(bitArrayRS[idx][idy+3] << 3) |
				(bitArrayRS[idx][idy+4] << 4) |
				(bitArrayRS[idx][idy+5] << 5) |
				(bitArrayRS[idx][idy+6] << 6) |
				(bitArrayRS[idx][idy+7] << 7);

#endif
			#if DEBUG_ENABLED
			for(idz=0; idz<8; idz++) {
				printf("%d", bitArrayRS[idx][idy+idz]);
			}
			printf("\nByte[%d] = 0x%x\n", ids-1, appParam->dataForEquipments[ids-1]);
			#endif
		}
	}
	appParam->totalBytes = ids;
	printf("Total # of Bytes to Send = %d\n", appParam->totalBytes);
	return 0;
}

int generateBitArray(cBmpFile *inFile, FILE *fpInput, applicationParameters *appParam)
{
	unsigned int idx, idy;
	int idb;
	uint8_t rByte = 0x00;
	unsigned int nColors = 0;
	int retVal = 0;
	int localWidth = 0;
	int localHeight = 0;

	if(inFile->bmInfoHeader.biClrUsed != 0) {
		/* No of Actual colors used in entries */
		nColors = inFile->bmInfoHeader.biClrUsed;

	} else {
		/* Index images */
		nColors = (int) pow(2, inFile->bmInfoHeader.biBitCount);
	}
	printf("\n# of CLU (color look up) table entries: %d\n", nColors);

	inFile->colorTable = (rgbQuad *) malloc (nColors * sizeof(rgbQuad));
	if(NULL == inFile->colorTable) {
		printf("failed to allocate buffer for CLU table\n");
		return -1;
	}


	retVal = fread(inFile->colorTable, sizeof(rgbQuad), nColors, fpInput);
	if(0 > retVal) {
		printf("failed to read CLU table info\n");
		return -1;
	}
	printf("%d colors copied in CLU!!\n", nColors);

	printf("CLU Table\n");
	for(idx=0; idx<nColors; idx++) {
		printf("[%3d]: 0x%2x 0x%2x 0x%2x 0x%2x\n", idx,
					inFile->colorTable[idx].rgbBlue,
					inFile->colorTable[idx].rgbGreen,
					inFile->colorTable[idx].rgbRed,
					inFile->colorTable[idx].rgbReserved);
	}

	/* Skip Header */
	fseek(fpInput, inFile->bmFileHeader.bfOffBits, SEEK_SET);

	switch(inFile->bmInfoHeader.biBitCount) {
		case 1:
			printf("1-bit BMP Image\n");

			inFile->bmPixelData = (uint8_t *) malloc ((inFile->bmInfoHeader.biWidth/8) * inFile->bmInfoHeader.biHeight);
			if(NULL == inFile->bmPixelData) {
				printf("failed to allocate buffer for pixel data i/p\n");
				return -1;
			}

			fread(inFile->bmPixelData, ((inFile->bmInfoHeader.biWidth/8) * inFile->bmInfoHeader.biHeight), 1, fpInput);

			// Create 2D Pixel Array for storing BMP pixel data in TOP-BOTTOM (LEFT-RIGHT) order
			// inFile->bmInfoHeader.biWidth : No of columns
			// inFile->bmInfoHeader.biHeight : No of rows
			appParam->pixArray = (uint8_t **) malloc(inFile->bmInfoHeader.biHeight * sizeof(uint8_t *));
			for(idx = 0; idx < inFile->bmInfoHeader.biHeight; idx++) {
				appParam->pixArray[idx] = (uint8_t *) malloc((inFile->bmInfoHeader.biWidth/8) * sizeof(uint8_t));
			}
			// Fill BMP data to 2D Pixel Array
			localWidth = inFile->bmInfoHeader.biWidth/8;
			localHeight = inFile->bmInfoHeader.biHeight-1;
			for(idx=0; idx<inFile->bmInfoHeader.biHeight; idx++) {
				for(idy=0; idy<inFile->bmInfoHeader.biWidth/8; idy++) {
					appParam->pixArray[idx][idy] = inFile->bmPixelData[localWidth*localHeight+idy];
				}
				localHeight--;
			}
			#if DEBUG_ENABLED
			printf("T-L 8 Pixels = 0x%x\n", appParam->pixArray[0][0]);
			printf("T-R 8 Pixels = 0x%x\n", appParam->pixArray[0][(inFile->bmInfoHeader.biWidth/8)-1]);
			printf("B-L 8 Pixels = 0x%x\n", appParam->pixArray[inFile->bmInfoHeader.biHeight-1][0]);
			printf("B-R 8 Pixels = 0x%x\n", appParam->pixArray[inFile->bmInfoHeader.biHeight-1][(inFile->bmInfoHeader.biWidth/8)-1]);
			#endif

			// Create 2D bit array
			appParam->bitArray = (uint8_t **) malloc(inFile->bmInfoHeader.biHeight * sizeof(uint8_t *));
			for(idx = 0; idx < inFile->bmInfoHeader.biHeight; idx++) {
				appParam->bitArray[idx] = (uint8_t *) malloc( inFile->bmInfoHeader.biWidth * sizeof(uint8_t));
			}
			// Fill BMP data to 2D bit Array according WHITE v/s COLOR/BLACK pixel value
			for(idx=0; idx<inFile->bmInfoHeader.biHeight; idx++) {
				for(idy=0; idy<inFile->bmInfoHeader.biWidth/8; idy++) {
					rByte = appParam->pixArray[idx][idy];
					for(idb=7; idb>=0; idb--) {
						/* For each bit # idb */
						/* Check pixel value : if WHITE, put 0. If Any other COLOR, put 1 */
						if(inFile->colorTable[rByte & 0x01].rgbBlue == 0xFF &&
								inFile->colorTable[rByte & 0x01].rgbGreen == 0xFF &&
								inFile->colorTable[rByte & 0x01].rgbRed == 0xFF) {
							appParam->bitArray[idx][idy*8+idb] = 0;
						} else {
							appParam->bitArray[idx][idy*8+idb] = 1;
						}
						rByte >>= 1;
					}
					rByte = 0x00;
				}
			}

			#if DEBUG_ENABLED
			for(idb=0; idb<40; idb++) {
				if(idb%8 == 0) printf(" ");
				printf("%d", appParam->bitArray[0][idb]);
			}
			printf("\n");
			#endif
			break;

		case 4:
			printf("4-bit BMP Image\n");

			inFile->bmPixelData = (uint8_t *) malloc ((inFile->bmInfoHeader.biWidth/2) * inFile->bmInfoHeader.biHeight);
			if(NULL == inFile->bmPixelData) {
				printf("failed to allocate buffer for pixel data i/p\n");
				return -1;
			}

			fread(inFile->bmPixelData, ((inFile->bmInfoHeader.biWidth/2) * inFile->bmInfoHeader.biHeight), 1, fpInput);

			// Create 2D Pixel Array for storing BMP pixel data in TOP-BOTTOM (LEFT-RIGHT) order
			// inFile->bmInfoHeader.biWidth : No of columns
			// inFile->bmInfoHeader.biHeight : No of rows
			appParam->pixArray = (uint8_t **) malloc(inFile->bmInfoHeader.biHeight * sizeof(uint8_t *));
			for(idx = 0; idx < inFile->bmInfoHeader.biHeight; idx++) {
				appParam->pixArray[idx] = (uint8_t *) malloc((inFile->bmInfoHeader.biWidth/2) * sizeof(uint8_t));
			}
			// Fill BMP data to 2D Pixel Array
			localWidth = inFile->bmInfoHeader.biWidth/2;
			localHeight = inFile->bmInfoHeader.biHeight-1;
			for(idx=0; idx<inFile->bmInfoHeader.biHeight; idx++) {
				for(idy=0; idy<inFile->bmInfoHeader.biWidth/2; idy++) {
					appParam->pixArray[idx][idy] = inFile->bmPixelData[localWidth*localHeight+idy];
				}
				localHeight--;
			}

			#if DEBUG_ENABLED
			for(idx=0; idx<1; idx++) {
				for(idy=0; idy<inFile->bmInfoHeader.biWidth/2; idy++) {
					printf("%d ", appParam->pixArray[idx][idy] >> 4);
					printf("%d ", appParam->pixArray[idx][idy] & 0x0F);
				}
				printf("\n");
			}
			printf("\n");
			#endif
			// Create 2D bit array
			appParam->bitArray = (uint8_t **) malloc(inFile->bmInfoHeader.biHeight * sizeof(uint8_t *));
			for(idx = 0; idx < inFile->bmInfoHeader.biHeight; idx++) {
				appParam->bitArray[idx] = (uint8_t *) malloc( inFile->bmInfoHeader.biWidth * sizeof(uint8_t));
			}
			// Fill BMP data to 2D bit Array according WHITE v/s COLOR/BLACK pixel value
			for(idx=0; idx<inFile->bmInfoHeader.biHeight; idx++) {
				for(idy=0; idy<inFile->bmInfoHeader.biWidth/2; idy++) {
					rByte = appParam->pixArray[idx][idy];
					for(idb=1; idb>=0; idb--) {
						/* For each nibble # idb */
						/* Check pixel value : if WHITE, put 0. If Any other COLOR, put 1 */
						if(inFile->colorTable[rByte & 0x0F].rgbBlue == 0xFF &&
								inFile->colorTable[rByte & 0x0F].rgbGreen == 0xFF &&
								inFile->colorTable[rByte & 0x0F].rgbRed == 0xFF) {
							appParam->bitArray[idx][idy*2+idb] = 0;
						} else {
							appParam->bitArray[idx][idy*2+idb] = 1;
						}
						rByte >>= 4;
					}
					rByte = 0x00;
				}
			}
			#if DEBUG_ENABLED
			for(idb=0; idb<inFile->bmInfoHeader.biWidth/2; idb++) {
				printf("%d", appParam->bitArray[0][idb]);
			}
			printf("\n");
			#endif
			break;

		case 8:
			printf("8-bit BMP Image\n");

			inFile->bmPixelData = (uint8_t *) malloc (inFile->bmInfoHeader.biWidth * inFile->bmInfoHeader.biHeight);
			if(NULL == inFile->bmPixelData) {
				printf("failed to allocate buffer for pixel data i/p\n");
				return -1;
			}

			fread(inFile->bmPixelData, inFile->bmInfoHeader.biWidth * inFile->bmInfoHeader.biHeight, 1, fpInput);

			// Create 2D Pixel Array for storing BMP pixel data in TOP-BOTTOM (LEFT-RIGHT) order
			// inFile->bmInfoHeader.biWidth : No of columns
			// inFile->bmInfoHeader.biHeight : No of rows
			appParam->pixArray = (uint8_t **) malloc(inFile->bmInfoHeader.biHeight * sizeof(uint8_t *));
			for(idx = 0; idx < inFile->bmInfoHeader.biHeight; idx++) {
				appParam->pixArray[idx] = (uint8_t *) malloc(inFile->bmInfoHeader.biWidth * sizeof(uint8_t));
			}
			// Fill BMP data to 2D Pixel Array
			localWidth = inFile->bmInfoHeader.biWidth;
			localHeight = inFile->bmInfoHeader.biHeight-1;
			for(idx=0; idx<inFile->bmInfoHeader.biHeight; idx++) {
				for(idy=0; idy<inFile->bmInfoHeader.biWidth; idy++) {
					appParam->pixArray[idx][idy] = inFile->bmPixelData[localWidth*localHeight+idy];
				}
				localHeight--;
			}

			#if DEBUG_ENABLED
			for(idx=0; idx<1; idx++) {
				for(idy=0; idy<inFile->bmInfoHeader.biWidth; idy++) {
					printf("%d ", appParam->pixArray[idx][idy]);
				}
			}
			printf("\n");
			#endif
			// Create 2D bit array
			appParam->bitArray = (uint8_t **) malloc(inFile->bmInfoHeader.biHeight * sizeof(uint8_t *));
			for(idx = 0; idx < inFile->bmInfoHeader.biHeight; idx++) {
				appParam->bitArray[idx] = (uint8_t *) malloc( inFile->bmInfoHeader.biWidth * sizeof(uint8_t));
			}
			// Fill BMP data to 2D bit Array according WHITE v/s COLOR/BLACK pixel value
			for(idx=0; idx<inFile->bmInfoHeader.biHeight; idx++) {
				for(idy=0; idy<inFile->bmInfoHeader.biWidth; idy++) {
					rByte = appParam->pixArray[idx][idy];
					/* For each byte # */
					/* Check pixel value : if WHITE, put 0. If Any other COLOR, put 1 */
					if(inFile->colorTable[rByte].rgbBlue == 0xFF &&
							inFile->colorTable[rByte].rgbGreen == 0xFF &&
							inFile->colorTable[rByte].rgbRed == 0xFF) {
						appParam->bitArray[idx][idy] = 0;
					} else {
						appParam->bitArray[idx][idy] = 1;
					}
					rByte = 0x00;
				}
			}
			#if DEBUG_ENABLED
			for(idb=0; idb<inFile->bmInfoHeader.biWidth; idb++) {
				printf("%d", appParam->bitArray[0][idb]);
			}
			printf("\n");
			#endif
			break;
		default:
			printf("Invalid BMP Image\n");
	}

	return 0;
}

void displayBMPMetaData(cBmpFile *inFile)
{
	printf("inFile->bmFileHeader.bfType 		= 0x%x\n", inFile->bmFileHeader.bfType);
	printf("inFile->bmFileHeader.bfSize 		= %d\n", inFile->bmFileHeader.bfSize);
	printf("inFile->bmFileHeader.bfOffBits		= %d\n", inFile->bmFileHeader.bfOffBits);
	printf("inFile->bmInfoHeader.biSize		= %d\n", inFile->bmInfoHeader.biSize);
	printf("inFile->bmInfoHeader.biWidth		= %d\n", inFile->bmInfoHeader.biWidth);
	printf("inFile->bmInfoHeader.biHeight		= %d\n", inFile->bmInfoHeader.biHeight);
	printf("inFile->bmInfoHeader.biPlanes		= %d\n", inFile->bmInfoHeader.biPlanes);
	printf("inFile->bmInfoHeader.biBitCount		= %d\n", inFile->bmInfoHeader.biBitCount);
	printf("inFile->bmInfoHeader.biCompression	= %d\n", inFile->bmInfoHeader.biCompression);
	printf("inFile->bmInfoHeader.biSizeImage	= %d\n", inFile->bmInfoHeader.biSizeImage);
	printf("inFile->bmInfoHeader.biXPelsPerMeter	= %d\n", inFile->bmInfoHeader.biXPelsPerMeter);
	printf("inFile->bmInfoHeader.biYPelsPerMeter	= %d\n", inFile->bmInfoHeader.biYPelsPerMeter);
	printf("inFile->bmInfoHeader.biClrUsed		= %d\n", inFile->bmInfoHeader.biClrUsed);
	printf("inFile->bmInfoHeader.biClrImportant	= %d\n", inFile->bmInfoHeader.biClrImportant);
}
int loadActualImage(char *filename, cBmpFile *inFile, applicationParameters *appParams)
{
	int retVal = 0;
	FILE *fpInput = NULL;

	if ((inFile == NULL) || (appParams == NULL) || (filename == NULL)) {
		printf("Invalid null Parameters received\n");
		return -1;
	}

	fpInput = fopen(filename, "rb+");
	if(NULL == fpInput) {
		printf("failed to open %s input file. Not exist?\n", filename);
		return -1;
	}

	/* clear inFile parameters */
	memset(inFile, 0x0, sizeof(cBmpFile));

	retVal = fread(&inFile->bmFileHeader, sizeof(inFile->bmFileHeader), 1, fpInput);
	if(0 > retVal) {
		printf("failed to read file header\n");
		return -1;
	}

	if (inFile->bmFileHeader.bfType != 0x4D42) {
		printf("Invalid bmp image found\n");
		return -1;
	}
	printf("Valid bmp File Header with %d bytes size found!!\n", sizeof(inFile->bmFileHeader));

	retVal = fread(&inFile->bmInfoHeader, sizeof(inFile->bmInfoHeader), 1, fpInput);
	if(0 > retVal) {
		printf("failed to read info header\n");
		return -1;
	}
	printf("Valid bmp Information Header with %d bytes size found!!\n", sizeof(inFile->bmInfoHeader));

	displayBMPMetaData(inFile);

	if(inFile->bmInfoHeader.biBitCount > 8) {
		printf("%s input file is having more than 256 colors, BitCount=%d\n", filename, inFile->bmInfoHeader.biBitCount);
		return -1;
	}

	retVal = generateBitArray(inFile, fpInput, appParams);
	if(0 > retVal) {
		printf("Unsupported BMP image\n");
		return -1;
	}

	fclose(fpInput);
	printf("Done!!\n");
	return 0;
}

int processImage(cBmpFile *inFile, applicationParameters *appParam)
{
	int retVal = 0;
	if ((inFile == NULL) || (appParam == NULL)) {
		printf("Invalid null Parameters received: %s\n", __func__);
		return -1;
	}

	retVal = generateDataForSending(inFile, appParam);

	printf("Done!!\n");

	return retVal;
}

int initializeDevice(applicationParameters *appParam)
{
	struct sigaction sa;

	if (appParam == NULL) {
		printf("Invalid null parameter received:%s\n", __func__);
		return -1;
	}

	sa.sa_handler = &handleSignal;
	sa.sa_flags = SA_SIGINFO;

	if (sigaction(38, &sa, NULL) == -1) {
		printf("Failed to register signal handler\n");
		return -1;
	}

	appParam->iDeviceHandle = open("/dev/arduino",  O_WRONLY );
	if( -1 == appParam->iDeviceHandle) {
		printf("Failed to open /dev/arduino device\n");
		return -1;
	}

	return 0;
}

void *emtAppStateMachine(void *app) {

	int retVal;
	applicationParameters *appParam = (applicationParameters *)app;
	printf("state machine start\n");

	while (1)
	{
		switch(emt_state) {
			case EMT_STATE_NONE:
				break;
			case EMT_STATE_SIGNAL_RECEIVED:
				appParam->dataOffset = driver_data_offset;
				appParam->dataLines = driver_data_offset/appParam->totalNoEquipments + 1;
				//printf("dataLines %d\n", appParam->dataLines);
#if 0
				retVal = ioctl(appParam->iDeviceHandle, ARDUINO_SEND_DATA,
						&appParam->dataForEquipments[appParam->dataOffset]);
				if(retVal < 0) {
					printf("Failed to send data\n");
					return NULL;
				}
#endif
				emt_state = EMT_STATE_DATA_SENT;
				break;
			case EMT_STATE_DATA_SENT:
#if 0
				appParam->dataOffset += appParam->totalNoEquipments;
				if (appParam->dataOffset >= appParam->totalBytes) {
					appParam->dataOffset = 0;
				}
#endif
				emt_state = EMT_STATE_NONE;
				break;
			case EMT_STATE_OFFSET_SAVED:
				emt_state = EMT_STATE_NONE;
				break;
			case EMT_STATE_UPDATE_GUI:
				emt_state = EMT_STATE_NONE;
				break;
			case EMT_STATE_CLOSE_APP:
				retVal = ioctl(appParam->iDeviceHandle, ARDUINO_STOP_DATA, NULL);
				if(retVal < 0) {
					printf("Failed to stop data\n");
					return NULL;
				}
				emt_state = EMT_STATE_NONE;
				return NULL;
			default:
				printf("Invalid application State\n");
				return NULL;		
		}
		usleep(10000);
	};
	return NULL;
}

int setDataLines(applicationParameters *appParam, unsigned int value)
{
	int retVal = 0;
	if (appParam == NULL) {
		printf("Invalid null parameter received:%s\n", __func__);
		return -1;
	}
	retVal = ioctl(appParam->iDeviceHandle, ARDUINO_CHANGE_DATALINES, (value - 1)*appParam->totalNoEquipments);
	if(retVal < 0) {
		printf("Failed to set data Lines\n");
		return -1;
	}
	return 0;
}

int configureData(applicationParameters *appParam)
{
	int retVal = 0;
	spi_arduino_config config;
	pthread_t deviceThread;

	if (appParam == NULL) {
		printf("Invalid null parameter received:%s\n", __func__);
		return -1;
	}

	emt_state = EMT_STATE_NONE;
	retVal = pthread_create( &deviceThread, NULL, emtAppStateMachine, appParam);
	if (retVal) {
		printf("Failed to create thread\n");
	}

	config.number_of_slaves = appParam->totalNoS2PCards;
	config.data_length  = appParam->totalNoEquipments;
	config.cards_per_slave  = MAXIMUM_EQUIPMENTS_ON_CARDS;
	config.total_data_length = appParam->totalBytes;
	config.is_last_equipment_first = false;
	appParam->dataOffset = 0;

	retVal = ioctl(appParam->iDeviceHandle, ARDUINO_CONFIGURE_DATA, &config);
	if(retVal < 0) {
		printf("Failed to configure data\n");
		return -1;
	}

	retVal = ioctl(appParam->iDeviceHandle, ARDUINO_SEND_DATA, 
                       appParam->dataForEquipments);
        if(retVal < 0) {
                printf("Failed to send data\n");
                return -1;
        }
	return retVal;
}
int closeDevice(applicationParameters *appParam)
{
	if (appParam == NULL) {
		printf("Invalid null parameter received:%s\n", __func__);
		return -1;
	}
	close(appParam->iDeviceHandle);
	return 0;
}

void stopStateMachine()
{
	emt_state = EMT_STATE_CLOSE_APP;
	printf("State Machine Stopped\n");
}
