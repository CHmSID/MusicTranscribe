#include "audio.h"

#include <AL/al.h>
#include <vorbis/vorbisfile.h>
#include <mpg123.h>
#include <rubberband/RubberBandStretcher.h>

#include <QDebug>
#include <QProgressDialog>
#include <QFileInfo>

#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>

using std::vector;
using std::string;
using std::ifstream;
using namespace RubberBand;

Audio::Audio(const char* filename, QProgressDialog* dialog)
{
	this->filename = (char*)filename;
    this->dialog = dialog;
	setup();
    dialog->close();
}

Audio::~Audio()
{
	alSourceStop(source);
	alDeleteSources(1, &source);
	alDeleteBuffers(5, &buffer[0]);

    delete ts;
}

bool Audio::isPlaying()
{
	return playing;
}

unsigned long Audio::getCurrentPlayingIndex() const
{
	return currentProgressIndex;
}

unsigned long Audio::getDataSize() const
{
	return totalPcmData.size();
}

char Audio::getSample(unsigned long index)
{
	if(index >= totalPcmData.size())
		return totalPcmData[totalPcmData.size() - 1];

	return totalPcmData[index];
}

// Returns a value between 0 and 1
double Audio::getProgress()
{
	if(currentProgressIndex == 0)
		return 0;

	if(currentProgressIndex >= totalPcmData.size() - size * 5)
		return 1;

	double progress = (double)currentProgressIndex / totalPcmData.size();

	if(progress > 1)
		progress = 1;

	return progress;
}

int Audio::getTotalTime() const
{
	return info.dataSize / info.byteRate;
}

int Audio::getCurrentTime()
{
	return getTotalTime() * getProgress();
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
        if(info.bitRate == 8){
			format = AL_FORMAT_MONO8;
            qDebug() << "HERE";
        }
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

//	size = info.byteRate * 0.1f;
    size = 8192;
    freq = info.sampleRate;

    ts = new RubberBandStretcher(info.sampleRate, info.numChannels,
                                 RubberBandStretcher::OptionProcessRealTime | RubberBandStretcher::OptionPitchHighConsistency,
                                 1.0f, 1.0f);

	reset();
}

vector<char> Audio::timeStretch(vector<char>& inData)
{
//    if(ratio == 1)
//        return data;  // Do not waste time on processing for no reason

    vector<float> data =  charToFloatVector(inData, info.bitRate);

    int inFrames = data.size() / info.numChannels;
    ts->setExpectedInputDuration(inFrames);

    vector<float> output;

    int ibs = 1024;
    float *fbuf = new float[info.numChannels * ibs];
    float **ibuf = new float *[info.numChannels];

    for (size_t i = 0; i < info.numChannels; ++i)
        ibuf[i] = new float[ibs];

    int frame = 0;

    while (frame < inFrames) {

        int count = ibs;
        int frameIdx = frame * info.numChannels;
        fbuf = &data[frameIdx];

        for (size_t c = 0; c < info.numChannels; ++c) {

            for (int i = 0; i < count; ++i) {

                float value = fbuf[i * info.numChannels + c];
                ibuf[c][i] = value;
            }
        }

        ts->process(ibuf, count, false);

        int avail = ts->available();

        if (avail > 0) {

            float **obf = new float *[info.numChannels];

            for (size_t i = 0; i < info.numChannels; ++i) {

                obf[i] = new float[avail];
            }

            ts->retrieve(obf, avail);
            float *fobf = new float[info.numChannels * avail];

            for (size_t c = 0; c < info.numChannels; ++c) {

                for (int i = 0; i < avail; ++i) {

                    float value = obf[c][i];

                    if (value > 1.f)
                        value = 1.f;

                    if (value < -1.f)
                        value = -1.f;

                    fobf[i * info.numChannels + c] = value;
                }
            }

            for(int i = 0; i < info.numChannels * avail; ++i){
                output.push_back(fobf[i]);
            }

            delete[] fobf;

            for (size_t i = 0; i < info.numChannels; ++i) {

                delete[] obf[i];
            }

            delete[] obf;
        }

        frame += ibs;
    }

    return floatToCharVector(output, info.bitRate);
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

    // Update the stretch factor if we need to
    if(stretcherDirty)
    {
        ts->setTimeRatio(stretchFactor);
        float newPitchScale = 1.0f + toneTranspos / 12.0f;
        if(newPitchScale == 0)
            newPitchScale = 0.05f;
        ts->setPitchScale(newPitchScale);
        stretcherDirty = false;
    }

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

        vector<char> dataToProcess(totalPcmData.begin() + totalPcmDataIndex,
                                   totalPcmData.begin() + totalPcmDataIndex + trueSize);
        vector<char> processedData = timeStretch(dataToProcess);

		alBufferData(buffer[bufferIndex],
                     format, &processedData[0],
                     processedData.size() * sizeof(char), freq);

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

    for(int i = 0; i < 5; i++)
    {
        int trueSize = size;

		//if there is less than 'size' of the data left
        if(totalPcmData.size() - trueSize < totalPcmDataIndex)
            trueSize = totalPcmData.size() - totalPcmDataIndex;

        vector<char> dataToProcess(totalPcmData.begin() + totalPcmDataIndex,
                                   totalPcmData.begin() + totalPcmDataIndex + trueSize);
        vector<char> processedData = timeStretch(dataToProcess);

		alBufferData(buffer[i], format,
            &processedData[0],
            processedData.size() * sizeof(char), freq);

        totalPcmDataIndex += trueSize;
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

    QFileInfo file(QString::fromStdString(filename));

    qint64 fileSize = file.size() - 4; // Minus the header

    while(mpg123_read(decoder, (unsigned char*)array, bufferSize, (size_t*)&done) == MPG123_OK)
    {
		totalPcmData.insert(totalPcmData.end(), array, array + done);
		info.dataSize += done;
        dialog->setValue((float)info.dataSize / fileSize * 10);

        if(dialog->wasCanceled())
            break;
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
    int bufferSize = 32768; //32KB
    totalPcmData.clear();

    FILE* f;
    f = fopen(filename, "rb");

    vorbis_info* pInfo;
    OggVorbis_File oggFile;

    ov_open(f, &oggFile, nullptr, 0);

    pInfo = ov_info(&oggFile, -1);

    info.riffHeader = "OGG";
    info.size = 0;
    info.dataSize = 0;
    info.numChannels = pInfo->channels;
    info.sampleRate = pInfo->rate;
    info.bitRate = 16;
    info.byteRate = info.sampleRate * info.numChannels;

    long bytes = 0;
    int bitStream = 0;
    char array[bufferSize];

    do
    {
        bytes = ov_read(&oggFile, array, bufferSize, 0, 2, 1, &bitStream);
        totalPcmData.insert(totalPcmData.end(), array, array + bytes);
        info.dataSize += bytes;

    } while(bytes > 0);

    info.dataSize /= 2;
    info.size = info.dataSize;

    ov_clear(&oggFile);
}

void Audio::loadWAV()
{
    file.open(filename, std::ios::binary);

    info.riffHeader = readChar(file, 4);
    info.size = readInt(file);
    info.waveHeader = readChar(file, 4);
    info.fmtHeader = readChar(file, 4);
    info.subChunkSize = readInt(file);
    info.audioFormat = readShort(file);
    info.numChannels = readShort(file);
    info.sampleRate = readInt(file);
    info.byteRate = readInt(file);
    info.blockAlignment = readShort(file);
    info.bitRate = readShort(file);
    info.dataHeader = readChar(file, 4);
    info.dataSize = readInt(file);

    totalPcmData.resize(info.dataSize, 0);  //vector<char> totalPcmData;
    file.read((char*)&totalPcmData[0], info.dataSize);

    file.close();
}

float Audio::getStretchFactor() const
{
    return stretchFactor;
}

int Audio::getToneTranspos() const
{
    return toneTranspos;
}

void Audio::updateStretchFactor(int i)
{
    stretcherDirty = true;
    stretchFactor = (float)i;
}

void Audio::updateToneTransposition(int i)
{
    stretcherDirty = true;
    toneTranspos = i;
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

unsigned int Audio::readInt(ifstream& file)
{
    int size = 4;
    char buffer[size];
    file.read(buffer, size);
    unsigned int x = 0;
    x |= buffer[3] << 24;
    x |= (buffer[2] << 16) & 0x00FF0000;
    x |= (buffer[1] << 8) & 0x0000FF00;
    x |= buffer[0] & 0x000000FF;

    return x;
}

short Audio::readShort(ifstream& file)
{
    int size = 2;
    char buffer[size];
    file.read(buffer, size);
    short x = (buffer[1] << 8) | buffer[0];

    return x;
}

string Audio::readChar(ifstream& file, int n)
{
    char buffer[n];
    file.read(buffer, n);
    buffer[n] = '\0';
    return string(buffer);
}

unsigned char Audio::readByte(ifstream& file)
{
    unsigned char n[1];
    file.read((char*)n, 1);
    return (unsigned)n[0];
}

vector<char> Audio::floatToCharVector(vector<float> input, int bitrate)
{
    vector<char> data;

    for(int i = 0; i < input.size(); ++i)
    {
        if(bitrate == 8)
            data.push_back((char)((input[i] + 1) / 2 * 256.0f));
        else
        {
            short d = (short)((input[i]) / 2 * 65536.0f);
            char x1 = (d & 0xFF00) >> 8;
            char x2 = d & 0x00FF;

            data.push_back(x2);
            data.push_back(x1);
        }
    }

    return data;
}

vector<float> Audio::charToFloatVector(vector<char> input, int bitrate)
{
    vector<float> data;

    for(int i = 0; i < input.size() / (bitrate / 8); ++i)
    {
        if(bitrate == 8){
            data.push_back((unsigned char)input[i] / 255.0f * 2.0f - 1.0f);
        }
        else
        {
            short d = ((unsigned char)input[i * 2 + 1] << 8) | (unsigned char)input[i * 2];
            data.push_back((float)d / 32768.0f);
        }
    }

    return data;
}
