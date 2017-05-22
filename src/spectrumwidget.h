#ifndef SPECTRUMWIDGET_H
#define SPECTRUMWIDGET_H

#include <QApplication>
#include <QMatrix4x4>

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>

#include <vector>
#include <complex>

using std::vector;
using std::complex;

class KeyboardWidget;
class Audio;

/*
 * Takes care of calculating and rendering the spectrum.
 */
class SpectrumWidget: public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	SpectrumWidget(QApplication* parentApp, KeyboardWidget* k);
	~SpectrumWidget();

public slots:
	void calculateSpectrum(int size);

    // Can not be done in a constructor as the audio might not exist
    // on instantiation.
    void connectToAudio(Audio* a);

private slots:
	void logic();

signals:
	void positionChanged(int x);

private:
	QApplication* parentApp = nullptr;
	Audio* audio = nullptr;
	KeyboardWidget* keyboard = nullptr;

	// FFT
	vector<complex<float> > inputData;
	vector<complex<double> > outputData;
	vector<float> frequencies;
	vector<double> window;

	QTimer* logicTimer = nullptr;

	// OpenGL
	vector<float> data;

	QOpenGLBuffer vbo;
	QOpenGLVertexArrayObject vao;
	QOpenGLShaderProgram* shader;

	QMatrix4x4 modelMatrix;
	QMatrix4x4 projectionMatrix;

	bool dirtyMatrix;

	void render();

	vector<complex<float> > fastFourierTransform(vector<complex<float> > v);
	void fft(vector<complex<double> >& x);

	float keyToFreq(int k);

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
};

#endif // SPECTRUMWIDGET_H
