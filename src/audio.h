#ifndef AUDIO_H
#define AUDIO_H

#include <QWidget>

#include <AL/al.h>
#include <rubberband/RubberBandStretcher.h>

#include <fstream>
#include <string>
#include <vector>

using std::ifstream;
using std::string;
using std::vector;
using namespace RubberBand;

//class RubberBandStretcher;

/* General information about the loaded audio.
 * Some of the fields are only used by WAV format
 */
struct AudioInfo
{
	string riffHeader;
	string waveHeader;
	string fmtHeader;
	string dataHeader;

	unsigned int size = 0;
	unsigned int subChunkSize;
	unsigned short blockAlignment;
	unsigned int dataSize;

	unsigned short audioFormat;
	unsigned short numChannels;
	unsigned int sampleRate;
	unsigned int byteRate;
	unsigned short bitRate;
};

class QProgressDialog;

class Audio : public QWidget
{
    Q_OBJECT
public:
    Audio(const char* filename, QProgressDialog* dialog);
	~Audio();

    QProgressDialog* dialog;

	AudioInfo info;

	void play();
	void pause();
	void update(); // make it multi-threaded?
	void reset();
	void seek(int p);

	char getSample(unsigned long index);
	double getProgress();
	int getTotalTime() const;   // In seconds
	int getCurrentTime(); // In seconds
	unsigned long getCurrentPlayingIndex() const;
	unsigned long getDataSize() const;

	bool isPlaying();
    float getStretchFactor() const;
    int getToneTranspos() const;

private:
	char* filename;
    ifstream file;

	bool playing;
	bool paused;

    RubberBandStretcher* ts = nullptr;
    bool stretcherDirty = false;
    float stretchFactor = 1.0f;
    int toneTranspos = 0;   // 0  = no transposition,
                            // 12 = 1 octave up,
                            //-12 = 1 octave down

	// OpenAL variables
	ALuint buffer[5]; // The queue of buffers to be played
	ALuint source;    // OpenAL sound source (where the sound comes from)
	ALenum format;    // MONO or STEREO, 8 or 16 bit
	ALsizei size;
	ALsizei freq;     // usually 41.1Khz
	ALenum state;

	// Raw PCM data
    vector<char> totalPcmData; // The whole song is kept in memory for
	                                    // fast access outside the currently
	                                    // loaded buffers
	unsigned long totalPcmDataIndex = 0; // Current position in the song
	unsigned long currentProgressIndex = 0;

	// Streaming variables
	int buffersProcessed;
	int bufferIndex;
	int currentBuffer;

	void setup();
	void checkForErrors(const char* prefix);

    void loadWAV();
    void loadMP3();
    void loadOGG();

    vector<char> timeStretch(vector<char>& data);
    vector<char> floatToCharVector(vector<float> input, int bitrate);
    vector<float> charToFloatVector(vector<char> input, int bitrate);

    unsigned int readInt(ifstream& file);
    short readShort(ifstream& file);
    string readChar(ifstream& file, int n);
    unsigned char readByte(ifstream& file);

	bool containsSuffix(string name, string suffix);

public slots:
    void updateStretchFactor(int i);
    void updateToneTransposition(int i);
};

#endif // AUDIO_H
