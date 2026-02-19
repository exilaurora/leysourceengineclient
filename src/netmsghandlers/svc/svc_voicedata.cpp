#include "../../valve/buf.h"

#include "../../leychan.h"
#include "svc_voicedata.h"

#include "../../../deps/osw/ISteamUser017.h"

extern ISteamUser017* temporaryHack;

#if !defined(_WIN32)
// Minimal stubs for Windows multimedia types/functions so code compiles on POSIX
typedef void* HWAVEOUT;

typedef struct {
	unsigned short wFormatTag;
	unsigned short nChannels;
	unsigned long nSamplesPerSec;
	unsigned long nAvgBytesPerSec;
	unsigned short nBlockAlign;
	unsigned short wBitsPerSample;
	unsigned short cbSize;
} WAVEFORMATEX;

typedef struct {
	char* lpData;
	unsigned long dwBufferLength;
	unsigned long dwFlags;
	void* dwReserved;
} WAVEHDR;

static const int CALLBACK_NULL = 0;
static const int MMSYSERR_NOERROR = 0;
static const int WAIT_OBJECT_0 = 0;

#define WAVE_FORMAT_PCM 1

static int waveOutOpen(HWAVEOUT* phwo, int device, WAVEFORMATEX* pwfx, unsigned long, unsigned long, int)
{
	*phwo = (HWAVEOUT)1; // dummy handle
	return MMSYSERR_NOERROR;
}

static int waveOutPrepareHeader(HWAVEOUT, WAVEHDR*, unsigned long)
{
	return MMSYSERR_NOERROR;
}

static int waveOutWrite(HWAVEOUT, WAVEHDR*, unsigned long)
{
	return MMSYSERR_NOERROR;
}

static int waveOutUnprepareHeader(HWAVEOUT, WAVEHDR*, unsigned long)
{
	return MMSYSERR_NOERROR;
}

static int waveOutClose(HWAVEOUT)
{
	return MMSYSERR_NOERROR;
}

static unsigned long WaitForSingleObject(void*, unsigned long)
{
	return WAIT_OBJECT_0;
}
#endif

bool svc_voicedata::Register(leychan* chan)
{
	leychan::netcallbackfn fn = reinterpret_cast<leychan::netcallbackfn>(&svc_voicedata::ParseMessage);

	return chan->RegisterMessageHandler(this->GetMsgType(), this, fn);
}

bool svc_voicedata::ParseMessage(leychan* chan, svc_voicedata* thisptr, bf_read& msg)
{
	int client = msg.ReadByte();
	int proximity = msg.ReadByte();
	int bits = msg.ReadWord();

	if (msg.GetNumBitsLeft() < bits)
		bits = msg.GetNumBitsLeft();

	// printf("Received svc_VoiceData, client: %i | proximity: %i | bits: %i\n", client, proximity, bits);



	char* voicedata = new char[(bits + 8) / 8 + 10];
	msg.ReadBits(voicedata, bits);
	svc_voicedata::PlaybackAudio(voicedata, bits);

	delete[] voicedata;


	return true;
}

// TODO: Improve audio playback
void svc_voicedata::PlaybackAudio(char* voiceData, int lengthInBits)
{
	static char* uncompressedVoiceBuf = new char[1000000];

	unsigned int uncompressedSize = 0;

	int lengthInBytes = lengthInBits / 8 + lengthInBits % 8;

	// gmod uses 44100 sample rate
	uint32 sampleRate = 44100; //temporaryHack->GetVoiceOptimalSampleRate();

	EVoiceResult worked = temporaryHack->DecompressVoice(
		voiceData,
		lengthInBytes,
		uncompressedVoiceBuf,
		100000,
		&uncompressedSize,
		sampleRate
	);

	if (worked != EVoiceResult::k_EVoiceResultOK)
	{
		printf("svc_voicedata: PlayBack failed: %d\n", worked);
		return;
	}

	static WAVEFORMATEX* swfx = 0;

	if (!swfx)
	{
		WAVEFORMATEX* wfx = new WAVEFORMATEX;
		memset(wfx, 0, sizeof(WAVEFORMATEX));

		wfx->wFormatTag = WAVE_FORMAT_PCM;
		wfx->nChannels = 1;
		wfx->wBitsPerSample = 16;
		wfx->nSamplesPerSec = sampleRate;
		wfx->nBlockAlign = wfx->nChannels * wfx->wBitsPerSample / 8;
		wfx->nAvgBytesPerSec = wfx->nSamplesPerSec * wfx->nBlockAlign;
		wfx->cbSize = 0;
		swfx = wfx;
	}


	HWAVEOUT hWaveOut = 0;

	if (waveOutOpen(&hWaveOut, 0, swfx, 0, 0, CALLBACK_NULL))
	{
		printf("svc_voicedata:: Opening Audio out failed\n");
		return;
	}

	unsigned int cVoiceSize = 1000000 + sizeof(WAVEHDR) + 32;
	static char* cVoice = new char[cVoiceSize];
	memcpy(cVoice + sizeof(WAVEHDR), uncompressedVoiceBuf, uncompressedSize);

	WAVEHDR* header = (WAVEHDR*)cVoice;
	memset(header, 0, sizeof(WAVEHDR));
	header->lpData = cVoice + sizeof(WAVEHDR);
	header->dwBufferLength = uncompressedSize + sizeof(WAVEHDR);

	if (!waveOutPrepareHeader(hWaveOut, header, sizeof(WAVEHDR)))
	{
		int writeret = (int)waveOutWrite(hWaveOut, header, sizeof(WAVEHDR));

		if (writeret != MMSYSERR_NOERROR)
		{
			printf("svc_voicedata:: failed writing audio %d\n", writeret);
		}
		else {
			if (WaitForSingleObject(hWaveOut, 3000) != WAIT_OBJECT_0)
			{
				return;
			}
		}
		waveOutUnprepareHeader(hWaveOut, header, sizeof(WAVEHDR));
	}

	waveOutClose(hWaveOut);
}