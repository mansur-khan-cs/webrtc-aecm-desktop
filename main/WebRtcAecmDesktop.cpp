// WebRtcAecmDesktop.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include<stdio.h>
#include<string>
#include<math.h>
#include<windows.h>
#include "echo_control_mobile.h"

using namespace std;

#define ERROR -1
#define SUCCESS 0
#define DELAY 50
#define AUDIO_SAMPLE_RATE 8000
#define AECM_SAMPLES_IN_FRAME 80

int ReadFileIntoShort(char * FileName, int &iFileSize, short ** p_sInBuf)
{
	FILE *pFile = fopen(FileName, "rb");

	if (!pFile)
	{
		fprintf(stderr, "file open failed\n");
		return ERROR;
	}
	else
	{
		fprintf(stderr, "file open successful\n");
	}

	iFileSize = 0;
	if (pFile != NULL) {
		if (!fseek(pFile, 0, SEEK_END)) {
			iFileSize = ftell(pFile);
			fseek(pFile, 0, SEEK_SET);
		}
	}
	else {
		fprintf(stderr, "file open error\n");
		return ERROR;
	}
	*p_sInBuf = new short[iFileSize];
	int iRead = fread(*p_sInBuf, 2, iFileSize / 2, pFile);
	printf("iRead = %d\n", iRead * 2);
	return SUCCESS;
}

void CancelEcho()
{
	short *pNearBuf, *pFarBuf, *pCanceledBuf;
	int iSize = 0, iEchoSize = 0;
	void* AECM_instance = nullptr;
	int iAECERR = 0;

	ReadFileIntoShort("NE.pcm", iSize, &pNearBuf);
	ReadFileIntoShort("FE.pcm", iEchoSize, &pFarBuf);
	iSize /= 2;
	iEchoSize /= 2;
	iSize = min(iSize, iEchoSize);
	pCanceledBuf = new short[iSize];


	//****************************************
	AECM_instance = WebRtcAecm_Create();
	iAECERR = WebRtcAecm_Init(AECM_instance, AUDIO_SAMPLE_RATE);
	if (iAECERR)
	{
		printf("WebRtcAecm_Init failed\n");
	}
	else
	{
		printf("WebRtcAecm_Init successful\n");
	}

	AecmConfig aecConfig;
	aecConfig.cngMode = AecmFalse;
	aecConfig.echoMode = 4;
	if (WebRtcAecm_set_config(AECM_instance, aecConfig) == -1)
	{
		printf("WebRtcAecm_set_config failed\n");
	}
	else
	{
		printf("WebRtcAecm_set_config successful\n");
	}
	//******************************************

	for (int i = 0; i < iSize - 800; i += 800)
	{

		for (int j = i; j < i + 800; j += AECM_SAMPLES_IN_FRAME)
		{
			WebRtcAecm_BufferFarend(AECM_instance, pFarBuf + j, AECM_SAMPLES_IN_FRAME);
		}
		for (int j = i; j < i + 800; j += AECM_SAMPLES_IN_FRAME)
		{
			WebRtcAecm_Process(AECM_instance, pNearBuf + j, NULL, pCanceledBuf + j, AECM_SAMPLES_IN_FRAME, DELAY);

		}
	}

	FILE *pFout = fopen("canceled.pcm", "wb");
	fwrite(pCanceledBuf, 2, iSize, pFout);
	fclose(pFout);
	printf("Cancellation Done\n");
}

int main()
{
	CancelEcho();
	return 0;
}

