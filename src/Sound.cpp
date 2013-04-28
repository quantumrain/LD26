#include "Pch.h"
#include "Common.h"

IXAudio2* gXAudio;

// Grabbed from http://msdn.microsoft.com/en-us/library/windows/desktop/ee415781(v=vs.85).aspx

#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD & dwChunkSize, DWORD & dwChunkDataPosition)
{
    HRESULT hr = S_OK;
    if( INVALID_SET_FILE_POINTER == SetFilePointer( hFile, 0, NULL, FILE_BEGIN ) )
        return HRESULT_FROM_WIN32( GetLastError() );

    DWORD dwChunkType;
    DWORD dwChunkDataSize;
    DWORD dwRIFFDataSize = 0;
    DWORD dwFileType;
    DWORD bytesRead = 0;
    DWORD dwOffset = 0;

    while (hr == S_OK)
    {
        DWORD dwRead;
        if( 0 == ReadFile( hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL ) )
            hr = HRESULT_FROM_WIN32( GetLastError() );

        if( 0 == ReadFile( hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL ) )
            hr = HRESULT_FROM_WIN32( GetLastError() );

        switch (dwChunkType)
        {
        case fourccRIFF:
            dwRIFFDataSize = dwChunkDataSize;
            dwChunkDataSize = 4;
            if( 0 == ReadFile( hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL ) )
                hr = HRESULT_FROM_WIN32( GetLastError() );
            break;

        default:
            if( INVALID_SET_FILE_POINTER == SetFilePointer( hFile, dwChunkDataSize, NULL, FILE_CURRENT ) )
            return HRESULT_FROM_WIN32( GetLastError() );            
        }

        dwOffset += sizeof(DWORD) * 2;
        
        if (dwChunkType == fourcc)
        {
            dwChunkSize = dwChunkDataSize;
            dwChunkDataPosition = dwOffset;
            return S_OK;
        }

        dwOffset += dwChunkDataSize;
        
        if (bytesRead >= dwRIFFDataSize) return S_FALSE;

    }

    return S_OK;   
}

HRESULT ReadChunkData(HANDLE hFile, void * buffer, DWORD buffersize, DWORD bufferoffset)
{
    HRESULT hr = S_OK;
    if( INVALID_SET_FILE_POINTER == SetFilePointer( hFile, bufferoffset, NULL, FILE_BEGIN ) )
        return HRESULT_FROM_WIN32( GetLastError() );
    DWORD dwRead;
    if( 0 == ReadFile( hFile, buffer, buffersize, &dwRead, NULL ) )
        hr = HRESULT_FROM_WIN32( GetLastError() );
    return hr;
}

struct Sound
{
	WAVEFORMATEXTENSIBLE wfx;
	XAUDIO2_BUFFER buffer;
	IXAudio2SourceVoice* pSourceVoice;
};

HRESULT LoadSound(IXAudio2* xAudio, const char* path, Sound& sound)
{
	ZeroMemory(&sound, sizeof(sound));

	// Open the file
	HANDLE hFile = CreateFileA(
		path,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL );

	if( INVALID_HANDLE_VALUE == hFile )
		return HRESULT_FROM_WIN32( GetLastError() );

	if( INVALID_SET_FILE_POINTER == SetFilePointer( hFile, 0, NULL, FILE_BEGIN ) )
		return HRESULT_FROM_WIN32( GetLastError() );

	DWORD dwChunkSize;
	DWORD dwChunkPosition;
	//check the file type, should be fourccWAVE or 'XWMA'
	FindChunk(hFile,fourccRIFF,dwChunkSize, dwChunkPosition );
	DWORD filetype;
	ReadChunkData(hFile,&filetype,sizeof(DWORD),dwChunkPosition);
	if (filetype != fourccWAVE)
		return S_FALSE;

	FindChunk(hFile,fourccFMT, dwChunkSize, dwChunkPosition );
	ReadChunkData(hFile, &sound.wfx, dwChunkSize, dwChunkPosition );

	//fill out the audio data buffer with the contents of the fourccDATA chunk
	FindChunk(hFile,fourccDATA,dwChunkSize, dwChunkPosition );
	BYTE * pDataBuffer = new BYTE[dwChunkSize];
	ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

	sound.buffer.AudioBytes = dwChunkSize;  //buffer containing audio data
	sound.buffer.pAudioData = pDataBuffer;  //size of the audio buffer in bytes
	sound.buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

	HRESULT hr;

	if( FAILED(hr = xAudio->CreateSourceVoice( &sound.pSourceVoice, (WAVEFORMATEX*)&sound.wfx ) ) )
		return hr;

	return S_OK;
}

Sound gSound[kSid_Max];

void SoundInit()
{
	ZeroMemory(gSound, sizeof(gSound));

	if (FAILED(XAudio2Create(&gXAudio, 0, XAUDIO2_DEFAULT_PROCESSOR)))
	{
		DebugLn("XAudioCreate failed");
		return;
	}

	IXAudio2MasteringVoice* masterVoice = 0;

	if (FAILED(gXAudio->CreateMasteringVoice(&masterVoice)))
	{
		DebugLn("CreateMasterVoice failed");
		return;
	}

	LoadSound(gXAudio, "data\\dit.wav", gSound[kSid_Dit]);
	LoadSound(gXAudio, "data\\buzz.wav", gSound[kSid_Buzz]);
	LoadSound(gXAudio, "data\\switch.wav", gSound[kSid_Switch]);
	LoadSound(gXAudio, "data\\win.wav", gSound[kSid_Win]);
}

void SoundShutdown()
{
	if (!gXAudio)
		return;

	for(int i = 0; i < kSid_Max; i++)
	{
		Sound& sound = gSound[i];

		if (sound.pSourceVoice)
		{
			sound.pSourceVoice->Stop();
			sound.pSourceVoice->FlushSourceBuffers();
			sound.pSourceVoice->DestroyVoice();
		}
	}

	gXAudio->Release();
}

void SoundPlay(SoundId sid, float freq, float volume)
{
	if (!gXAudio)
		return;

	Sound& sound = gSound[sid];

	if (sound.pSourceVoice)
	{
		sound.pSourceVoice->Stop();
		sound.pSourceVoice->FlushSourceBuffers();
		sound.pSourceVoice->SubmitSourceBuffer(&sound.buffer);
		sound.pSourceVoice->SetFrequencyRatio(freq);
		sound.pSourceVoice->SetVolume(volume);
		sound.pSourceVoice->Start();
	}
}