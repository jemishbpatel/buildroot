#define DEBUG_ENABLED				(0)
#define MAXIMUM_SUPPORTED_S2P_CARDS	(10) /* HW limitation on S2P Card */
#define ACTUAL_NO_OF_PORT_OR_LINES	(5)  /* HW limitation on S2P card */
#define NO_OF_EQUIPMENTS_IN_A_LINE	(16)	 /* This may goes 16/24/32 */
#define MAXIMUM_EQUIPMENTS_ON_CARDS	(ACTUAL_NO_OF_PORT_OR_LINES*NO_OF_EQUIPMENTS_IN_A_LINE)
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "bmpInfo.h"

typedef struct _applicationParameters {
	uint8_t **pixArray;
	uint8_t **bitArray;
	uint8_t **s2pcData;
	uint8_t *dataForEquipments;
	uint16_t totalNoEquipments;
	uint8_t totalNoS2PCards;
	uint32_t designLength;
	uint32_t totalBytes;
	int  iDeviceHandle;
	bool dataProcessed;
	uint32_t dataOffset;
	unsigned int dataLines;
} __attribute__((packed)) applicationParameters;

typedef enum _emt_state_t { 
	EMT_STATE_NONE,
	EMT_STATE_SIGNAL_RECEIVED,
	EMT_STATE_DATA_SENT,
	EMT_STATE_OFFSET_SAVED,
	EMT_STATE_UPDATE_GUI,
	EMT_STATE_CLOSE_APP
} emt_state_t;

int loadActualImage(char *filename, cBmpFile *inFile, applicationParameters *appParam);
int processImage(cBmpFile *inFile, applicationParameters *appParam);
int initializeDevice(applicationParameters *appParam);
int configureData(applicationParameters *appParam);
int closeDevice(applicationParameters *appParam);
int setDataLines(applicationParameters *appParam, unsigned int value);
void stopStateMachine();
