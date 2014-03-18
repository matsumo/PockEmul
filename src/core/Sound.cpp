
#include "common.h"

//-- #include "Sound.h"


static CRITICAL_SECTION cs;
static HWAVEOUT dev=NULL;
static HANDLE bufs[2];
static unsigned char* bufPtr[2];
static HANDLE bufHdr[2];
static int curBlock=0;
static int ready;
static int bufPos,bufCycles,bufLVal,bufRVal;
static WAVEHDR* wh;
static int sampleCycles;
int soundEnable=0;


void CALLBACK WaveCallback(HWAVE hWave, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
   WAVEHDR *wh;
   HGLOBAL hg;
//   MMRESULT res;

   if(uMsg == WOM_DONE)
   {
       EnterCriticalSection( &cs );

       wh = (WAVEHDR *)dwParam1;

       waveOutUnprepareHeader(dev, wh, sizeof (WAVEHDR));

       //Deallocate the buffer memory
       hg = GlobalHandle(wh->lpData);
       GlobalUnlock(hg);
       GlobalFree(hg);

       //Deallocate the header memory
       hg = GlobalHandle(wh);
       GlobalUnlock(hg);
       GlobalFree(hg);

       ready++;

       LeaveCriticalSection( &cs );
   }
}

void EnableSound()
{
	if (!waveOutGetNumDevs())
	{
		MessageBox(NULL,"No audio devices present","Error",
			MB_OK|MB_ICONSTOP);
		return;
    }

    WAVEFORMATEX outFormatex;
		outFormatex.wFormatTag=WAVE_FORMAT_PCM;
		outFormatex.wBitsPerSample=8;
		outFormatex.nChannels=1;//2;
		outFormatex.nSamplesPerSec=8000;//11025;
		outFormatex.nAvgBytesPerSec=8000;//22050;
		outFormatex.nBlockAlign=1;//2;


	unsigned int ui_Result = waveOutOpen(
			&dev,
			WAVE_MAPPER,
			&outFormatex,
			(DWORD)WaveCallback,0,CALLBACK_FUNCTION);

	if (ui_Result !=
		MMSYSERR_NOERROR)
    {
		char Error[100];
		sprintf(Error,"Erreur:%i",ui_Result);
        MessageBox(NULL,"Could not open audio device",Error,
            MB_OK|MB_ICONSTOP);
        return;
    }
    
	waveOutReset(dev);
    InitializeCriticalSection(&cs);
    soundEnable=1;
    bufs[curBlock]=GlobalAlloc(GMEM_MOVEABLE,2048);
    bufPtr[curBlock]=(unsigned char *)GlobalLock(bufs[curBlock]);
    bufHdr[curBlock]=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,sizeof(WAVEHDR));
    wh=(WAVEHDR*)GlobalLock(bufHdr[curBlock]);
    wh->dwBufferLength=2048;
    wh->lpData=(char *)bufPtr[curBlock];
    ready=2;
	bufPos=0;
	bufCycles=0;
	bufLVal=0;
	bufRVal=0;
}

void DisableSound()
{
    if (!soundEnable)
        return;
    if(dev)
    {
        while (ready<2)
            Sleep(50);
        waveOutReset(dev);      //reset the device
        waveOutClose(dev);      //close the device
        dev=NULL;
    }

    DeleteCriticalSection(&cs);
    soundEnable=0;
}


void SoundDoOut(int l,int r,int cycles,int freq2)
{
    if (l) bufLVal+=cycles;
    if (r) bufRVal+=cycles;
    bufCycles+=cycles;

    if (bufCycles>=sampleCycles)
    {
        bufPtr[curBlock][bufPos]	= 0xff;//(bufRVal*255)/sampleCycles;
		switch(freq2){
		case 0:	bufPtr[curBlock][bufPos+1]	= 0xff; break;
		case 1:	bufPtr[curBlock][bufPos+1]	= 0xff; break;
		case 2:	bufPtr[curBlock][bufPos+1]	= 0x00; break;
		}
        bufCycles	=0;
        bufPos		+=2;
        bufLVal		=0;
		bufRVal=0;
        if (bufPos>=2048)
        {
            while (!ready)
                Sleep(3);
            ready--;
            waveOutPrepareHeader(dev,wh,sizeof(WAVEHDR));
            waveOutWrite(dev,wh,sizeof(WAVEHDR));
            curBlock=(curBlock+1)&1;
            bufs[curBlock]=GlobalAlloc(GMEM_MOVEABLE,2048);
            bufPtr[curBlock]=(unsigned char *)GlobalLock(bufs[curBlock]);
            bufHdr[curBlock]=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,sizeof(WAVEHDR));
            wh=(WAVEHDR*)GlobalLock(bufHdr[curBlock]);
            wh->dwBufferLength=2048;
            wh->lpData=(char *)bufPtr[curBlock];
            bufPos=0;
        }
    }
//2khz
#if 1
    if ((bufCycles>=sampleCycles) && (freq2==1))
    {
        bufPtr[curBlock][bufPos]	= 0x00;//(bufRVal*255)/sampleCycles;
        bufPtr[curBlock][bufPos+1]	= 0x00;//(bufLVal*255)/sampleCycles;
        bufCycles	=0;
        bufPos		+=2;
        bufLVal		=0;
		bufRVal=0;
        if (bufPos>=2048)
        {
            while (!ready)
                Sleep(3);
            ready--;
            waveOutPrepareHeader(dev,wh,sizeof(WAVEHDR));
            waveOutWrite(dev,wh,sizeof(WAVEHDR));
            curBlock=(curBlock+1)&1;
            bufs[curBlock]=GlobalAlloc(GMEM_MOVEABLE,2048);
            bufPtr[curBlock]=(unsigned char *)GlobalLock(bufs[curBlock]);
            bufHdr[curBlock]=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,sizeof(WAVEHDR));
            wh=(WAVEHDR*)GlobalLock(bufHdr[curBlock]);
            wh->dwBufferLength=2048;
            wh->lpData=(char *)bufPtr[curBlock];
            bufPos=0;
        }
    }
#endif
}

void SoundOutBits(int l,int r,int cycles,int freq2)
{
    if (!soundEnable) return;

    while ((bufCycles+cycles)>sampleCycles)
    {
        int left=sampleCycles-bufCycles;
        SoundDoOut(l,r,left,freq2);
        cycles-=left;
    }
    SoundDoOut(l,r,cycles,freq2);
}

void SoundSetCycles(int n)
{
    sampleCycles=n;
}

//---------------------------------------------------------------------------


#if 0
    /*
    * first we need to set up the WAVEFORMATEX structure. 
    * the structure describes the format of the audio.
    */
    wfx.nSamplesPerSec = 44100; /* sample rate */
    wfx.wBitsPerSample = 16; /* sample size */
    wfx.nChannels = 2; /* channels*/
    /*
    * WAVEFORMATEX also has other fields which need filling.
    * as long as the three fields above are filled this should
    * work for any PCM (pulse code modulation) format.
    */
    wfx.cbSize = 0; /* size of _extra_ info */
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nBlockAlign = (wfx.wBitsPerSample >> 3) * wfx.nChannels;
    wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
#endif
    
HWAVEOUT hWaveOut; /* device handle */

int CloseSound(HWAVEOUT hWaveOut)
{
    waveOutClose(hWaveOut);
	return 0;

}
int OpenSound(WAVEFORMATEX* pwfx)
{
//    MMRESULT result;/* for waveOut return values */
    /*
    * try to open the default wave device. WAVE_MAPPER is
    * a constant defined in mmsystem.h, it always points to the
    * default wave device on the system (some people have 2 or
    * more sound cards).
    */
    if(waveOutOpen(	&hWaveOut, 
					WAVE_MAPPER, 
					pwfx, 
					0, 
					0, 
					CALLBACK_NULL
					) != MMSYSERR_NOERROR) {
        fprintf(stderr, "unable to open WAVE_MAPPER device\n");
        ExitProcess(1);
    }
    return 0;
}


void writeAudioBlock(HWAVEOUT hWaveOut, LPSTR block, DWORD size)
{
static	WAVEHDR header1,header2;
static  BOOL flip=TRUE;

if (flip)
{
	while(waveOutUnprepareHeader(	hWaveOut, 
									&header1, 
									sizeof(WAVEHDR)) == WAVERR_STILLPLAYING );
	/*
	* initialise the block header with the size
	* and pointer.
	*/
	ZeroMemory(&header1, sizeof(WAVEHDR));
	header1.dwBufferLength = size;
	header1.lpData = block;
	/*
	* prepare the block for playback
	*/
	waveOutPrepareHeader(hWaveOut, &header1, sizeof(WAVEHDR));
	/*
	* write the block to the device. waveOutWrite returns immediately
	* unless a synchronous driver is used (not often).
	*/
	waveOutWrite(hWaveOut, &header1, sizeof(WAVEHDR));
	/*
	* wait a while for the block to play then start trying
	* to unprepare the header. this will fail until the block has
	* played.
	*/
}
else
{
	while(waveOutUnprepareHeader(	hWaveOut, 
									&header2, 
									sizeof(WAVEHDR)) == WAVERR_STILLPLAYING );
	/*
	* initialise the block header with the size
	* and pointer.
	*/
	ZeroMemory(&header2, sizeof(WAVEHDR));
	header2.dwBufferLength = size;
	header2.lpData = block;
	/*
	* prepare the block for playback
	*/
	waveOutPrepareHeader(hWaveOut, &header2, sizeof(WAVEHDR));
	/*
	* write the block to the device. waveOutWrite returns immediately
	* unless a synchronous driver is used (not often).
	*/
	waveOutWrite(hWaveOut, &header2, sizeof(WAVEHDR));
	/*
	* wait a while for the block to play then start trying
	* to unprepare the header. this will fail until the block has
	* played.
	*/
}
flip= !flip;

}
