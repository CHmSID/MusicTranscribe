#include "audio.h"

#include <AL/al.h>
#include <vorbis/vorbisfile.h>
#include <mpg123.h>

#include <QDebug>

#include <fstream>
#include <string>
#include <vector>

using std::vector;
using std::string;
using std::ifstream;

Audio::Audio(const char* filename)
{
	this->filename = (char*)filename;
	setup();
}

Audio::~Audio()
{
	alSourceStop(source);
	alDeleteSources(1, &source);
	alDeleteBuffers(5, &buffer[0]);
}

bool Audio::isPlaying()
{
	return playing;
}

void Audio::setup()
{
	alGetError();
	checkForErrors("audio setup");
	playing = false;
	paused = false;

	/* Use the correct method to load the file
	 * into PCM data
	 */
	if(containsSuffix(filename, "wav"))
		loadWAV();
	else if(containsSuffix(filename, "ogg"))
		loadOGG();
	else if(containsSuffix(filename, "mp3"))
		loadMP3();

	alGenBuffers(5, &buffer[0]);
	alGenSources(1, &source);

	if(info.numChannels == 1)
	{
		if(info.bitRate == 8)
			format = AL_FORMAT_MONO8;
		else if (info.bitRate == 16)
			format = AL_FORMAT_MONO16;
		else
			qDebug() << "Wrong bitrate";
	}
	else if(info.numChannels == 2)
	{
		if(info.bitRate == 8)
			format = AL_FORMAT_STEREO8;
		else if (info.bitRate == 16)
			format = AL_FORMAT_STEREO16;
		else
			qDebug() << "Wrong bitrate";
	}
	else
		qDebug() << "Wrong number of channels";

	size = info.byteRate * 0.1f;
	freq = info.sampleRate;

	reset();
}

void Audio::play()
{
	alSourcePlay(source);
	playing = true;
}

void Audio::pause()
{
	alSourcePause(source);
	playing = false;
}

void Audio::update()
{
	alGetSourcei(source, AL_SOURCE_STATE, &state);
	playing = (state == AL_PLAYING);

	alGetSourcei(source, AL_BUFFERS_PROCESSED, &buffersProcessed);

	// While the music is playing, and there are empty buffers that
	// could be filled with data
	while(buffersProcessed && isPlaying())
	{
		currentBuffer++;

		buffersProcessed--;  // One less empty buffer
		currentProgressIndex += size;

		alSourceUnqueueBuffers(source, 1, &buffer[bufferIndex]);

		// 'trueSize' is usually just 'size', but it will be smaller
		// for the last buffer when there's not enough data to populate it
		unsigned int trueSize = size;

		// If there is less than 'size' of the data left
		if(totalPcmData.size() - trueSize < totalPcmDataIndex)
			trueSize = totalPcmData.size() - totalPcmDataIndex;

		alBufferData(buffer[bufferIndex],
		             format, &totalPcmData[totalPcmDataIndex],
		             trueSize * sizeof(char), freq);

		totalPcmDataIndex += trueSize;

		alSourceQueueBuffers(source, 1, &buffer[bufferIndex]);
		bufferIndex++;

		if(bufferIndex > 4)
			bufferIndex = 0;
	}
}

void Audio::reset()
{
	alSourceStop(source);
	playing = false;
	seek(0);  // Reset the song to the beginning
}

void Audio::seek(int p)
{
	if(p < 0)
		return;

	bool wasPlaying = isPlaying();

	//play();
	//pause();

	alSourceStop(source);
	playing = false;

	totalPcmDataIndex = p * 750 * info.numChannels * info.bitRate / 8;
	currentBuffer = 0;
	bufferIndex = 0;
	buffersProcessed = 0;
	currentProgressIndex = p * 750 * info.numChannels * info.bitRate / 8;

	alSourceUnqueueBuffers(source, 5, &buffer[0]);

	for(int i = 0; i < 5; i++){

		int tmpSize = size;

		//if there is less than 'size' of the data left
		if(totalPcmData.size() - tmpSize < totalPcmDataIndex)
			tmpSize = totalPcmData.size() - totalPcmDataIndex;

		alBufferData(buffer[i], format,
		    &totalPcmData[totalPcmDataIndex],
		    tmpSize * sizeof(char), freq);

		totalPcmDataIndex += tmpSize;
	}

	alSourceQueueBuffers(source, 5, &buffer[0]);

	if(wasPlaying)
		play();
	else
		pause();
}

void Audio::loadMP3()
{
	info.riffHeader = "MP3";
	info.bitRate = 16;  // load it from the file?
	info.dataSize = 0;

	// Use mpg123 decoder to decode compressed mp3 data into PCM
	mpg123_handle* decoder;
	int bufferSize = 0;
	int done = 0;

	int err = 0;
	mpg123_init();
	decoder = mpg123_new(nullptr, &err);
	bufferSize = mpg123_outblock(decoder);
	char array[bufferSize];

	int encoding = 0;
	long int sample = 0;
	int numCh = 0;
	mpg123_open(decoder, filename);
	mpg123_getformat(decoder, &sample, &numCh, &encoding);

	info.sampleRate = (unsigned int)sample;
	info.numChannels = (unsigned short)numCh;

	while(mpg123_read(decoder, (unsigned char*)array, bufferSize, (size_t*)&done) == MPG123_OK){

		totalPcmData.insert(totalPcmData.end(), array, array + done);
		info.dataSize += done;
	}

	info.dataSize /= 2;

	info.byteRate = info.sampleRate * info.numChannels;
	info.size = info.dataSize;
	mpg123_close(decoder);
	mpg123_delete(decoder);
	mpg123_exit();
}

bool Audio::containsSuffix(string name, string suffix)
{
	if(suffix.length() > name.length())
		return false;

	return name.substr(name.length() - suffix.length()) == suffix;
}

void Audio::loadOGG()
{

}

void Audio::loadWAV()
{

}

void Audio::checkForErrors(const char* prefix)
{
	ALenum error = alGetError();
	string errCode;

	while(error != AL_NO_ERROR)
	{
		switch(error)
		{
			case AL_INVALID_ENUM:
				errCode = "invalid enum";
				break;
			case AL_INVALID_NAME:
				errCode = "invalid name";
				break;
			case AL_INVALID_OPERATION:
				errCode = "invalid op";
				break;
			case AL_INVALID_VALUE:
				errCode = "invalid value";
				break;
			default:
				errCode = "unknown error";
				break;
		}

		qDebug() << "ERROR!in" << prefix << ":" << errCode.c_str();
		error = alGetError();
	}
}
