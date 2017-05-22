#include "spectrumwidget.h"
#include "audio.h"
#include "keyboardwidget.h"
#include "util.h"

#include <QTimer>

SpectrumWidget::SpectrumWidget(QApplication* parentApp,
                               KeyboardWidget* k)
    :parentApp(parentApp), keyboard(k)
{
	// Create the window
	for(int i = 0; i < 2048 * 2; i++)
	{
		float x = 0.5f * (1.0f - cos(2.0f * M_PI * i / (2048 * 2 - 1)));
		window.push_back(x);
	}
}

SpectrumWidget::~SpectrumWidget()
{
	delete logicTimer;

	makeCurrent();
	vbo.destroy();
	vao.destroy();
	delete shader;
}

void SpectrumWidget::initializeGL()
{
	logicTimer = new QTimer();
	connect(logicTimer, SIGNAL(timeout()),
	        this, SLOT(logic()));
	logicTimer->start(33);

	initializeOpenGLFunctions();
	glClearColor(67 / (float)255, 64 / (float)255, 60 / (float)255, 1.0f);

	modelMatrix.setToIdentity();

	shader = new QOpenGLShaderProgram();
	shader->addShaderFromSourceFile(QOpenGLShader::Vertex,
	                                ":/data/shaders/spectrum.vert");
	shader->addShaderFromSourceFile(QOpenGLShader::Fragment,
	                                ":/data/shaders/spectrum.frag");
	shader->link();
	shader->bind();
}

void SpectrumWidget::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT);

	if(audio != nullptr)
	{
		render();
	}
}

// On window resize
void SpectrumWidget::resizeGL(int width, int height)
{
	projectionMatrix.setToIdentity();

    int newWidth = width;
    if(width > keyboard->getWidth())
    {
        newWidth = keyboard->getWidth();
    }

	projectionMatrix.ortho(0, newWidth, 0, height, 1, -1);
	dirtyMatrix = true;
}

void SpectrumWidget::logic()
{
	update();
	parentApp->processEvents();
}

void SpectrumWidget::connectToAudio(Audio* a)
{
	audio = a;
}

void SpectrumWidget::render()
{
	shader->bind();

	if(dirtyMatrix)
	{
		shader->setUniformValue("projectionMatrix", projectionMatrix);
		dirtyMatrix = false;
	}

	shader->setUniformValue("modelMatrix",
	                        keyboard->getModelMatrix());

	vao.bind();
	glDrawArrays(GL_TRIANGLES, 0, data.size() / 2);
	vao.release();
}

void SpectrumWidget::calculateSpectrum(int binSize)
{
	makeCurrent();

	long index = audio->getCurrentPlayingIndex();
	long size = audio->getDataSize();

	inputData.clear();
	outputData.clear();
	frequencies.clear();

	// Calculate how much data needs to be padded
	// Padding will increase the resolution of the bin
    // without taking additional samples (less samples = more accuracy in higher ranges)

    int genuineData = 8192; // How many actual samples we'll take from audio
	if(genuineData > binSize)
		genuineData = binSize;

	//How many 0s to pad from each side
	int sidePadding = (binSize - genuineData) / 2;

	//pad from left
	for(int i = 0; i < sidePadding; i++)
	{
		outputData.push_back(complex<double>(0, 0));
	}

	//fill actual data
	for(long i = index; i < index + (genuineData *
	                                 audio->info.numChannels *
	                                 audio->info.bitRate / 8);)
	{
		double result = 0;

		if(i >= 0 && i < size - 1)
		{
			if(audio->info.numChannels == 1)
			{
				if(audio->info.bitRate == 8)
				{
					result = audio->getSample(i);
					i++;
				}
				else if(audio->info.bitRate == 16)
				{
					result = read2Bytes(audio->getSample(i),
					                    audio->getSample(i + 1));
					i += 2;
				}
			}
			else if(audio->info.numChannels == 2)
			{
				//average
				if(audio->info.bitRate == 8)
				{
					result = (audio->getSample(i) +
					          audio->getSample(i + 1)) / 2;
					i += 2;
				}
				else if(audio->info.bitRate == 16)
				{
					result = (read2Bytes(audio->getSample(i),
					                     audio->getSample(i + 1)) +
					          read2Bytes(audio->getSample(i + 2),
					                     audio->getSample(i + 3))) / 2;

					i += 4;
				}
			}
		}
		else
		{
			//we're not where we're supposed to be, move along
			i++;
		}

		result /= 65535.0f; //convert from char/short to float

		outputData.push_back(complex<double>(result * 2.5f, 0));
	}

	// Windowing
//	for(int i = sidePadding; i < sidePadding + genuineData; i++)
//	{
//		outputData.at(i) = outputData.at(i) * window.at(i - sidePadding);
//	}

	//pad from right
	for(int i = 0; i < sidePadding; i++)
	{
		outputData.push_back(complex<double>(0, 0));
	}

	fft(outputData);

	float highest = 0;
	for(unsigned i = 0; i < frequencies.size(); i++)
	{
		if(frequencies[i] > frequencies[highest])
			highest = i;
	}

	for(unsigned i = 0; i < outputData.size() / 2; i++)
	{
		double re = outputData[i].real();
		double im = outputData[i].imag();

		double power = sqrt(re * re + im * im);

		frequencies.push_back(power);
	}

	data.clear();

	for(int i = 0; i < 98; i++)
	{
		int key = i + 3;
		float freq = keyToFreq(key);

        float startX = 0;
        float endX = 0;
		int height = 0;

		float isWhite = keyboard->isKeyWhite(i);

		if(keyboard->isKeyWhite(i))
		{
			if(i == 0)
				startX = keyboard->getKeyPosition(i);
			else
				startX = keyboard->getKeyPosition(i - 1) +
				         keyboard->getKeyWidth(i - 1);

			if(i == 97)
				endX = keyboard->getKeyPosition(i) +
				       keyboard->getKeyWidth(i);
			else
				endX = keyboard->getKeyPosition(i + 1);
		}
		else
		{
			startX = keyboard->getKeyPosition(i);
			endX = keyboard->getKeyPosition(i) + keyboard->getKeyWidth(i);
		}

        float width = endX - startX;
        startX += audio->getToneTranspos() * width;
        endX += audio->getToneTranspos() * width;

		height = freq * 2.0f;

		data.push_back(startX);
		data.push_back(0);
		data.push_back(isWhite);

		data.push_back(startX);
		data.push_back(height);
		data.push_back(isWhite);

		data.push_back(endX);
		data.push_back(0);
		data.push_back(isWhite);

		data.push_back(endX);
		data.push_back(0);
		data.push_back(isWhite);

		data.push_back(startX);
		data.push_back(height);
		data.push_back(isWhite);

		data.push_back(endX);
		data.push_back(height);
		data.push_back(isWhite);
	}

	if(!vbo.isCreated())
		vbo.create();

	if(!vao.isCreated())
		vao.create();

	shader->bind();

	vbo.bind();
	vbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
	vbo.allocate(data.data(), sizeof(float) * data.size());

	vao.bind();

	shader->enableAttributeArray(0);
	shader->setAttributeBuffer(0, GL_FLOAT, 0, 2, 3 * sizeof(float));

	shader->enableAttributeArray(1);
	shader->setAttributeBuffer(1, GL_FLOAT, 2 * sizeof(float), 1, 3 * sizeof(float));

	vao.release();
	vbo.release();
}

void SpectrumWidget::fft(vector<complex<double> >& x) // taken from wikipedia
{
	const size_t N = x.size();
	if (N <= 1) return;

	// divide
	vector<complex<double> > even;
	even.resize(N / 2);

	vector<complex<double> > odd;
	odd.resize(N / 2);

	for(unsigned i = 0; i < N / 2; i++)
	{
		even[i] = x[i * 2];
		odd[i] = x[i * 2 + 1];
	}

	// conquer
	fft(even);
	fft(odd);

	// combine
	for (size_t k = 0; k < N/2; ++k)
	{
		complex<double> t = std::polar(1.0, -2 * M_PI * k / N) * odd[k];
		x[k    ] = even[k] + t;
		x[k+N/2] = even[k] - t;
	}
}

// Maps a key to the closes frequency bin we possess
float SpectrumWidget::keyToFreq(int key)
{
	float f = pow(2.0f, (key - 49) / 12.0f) * 440.0f;

	int closest = 0;

	for(unsigned i = 0; i < frequencies.size(); i++)
	{
		if(qAbs(i * audio->info.sampleRate / frequencies.size() - f) <
		    qAbs(closest * audio->info.sampleRate / frequencies.size() - f))
			closest = i;
	}

	return frequencies[closest];
}
