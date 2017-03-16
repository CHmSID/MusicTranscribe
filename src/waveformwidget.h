#ifndef WAVEFORMWIDGET_H
#define WAVEFORMWIDGET_H

#include <QApplication>
#include <QVector3D>
#include <QVector2D>
#include <QMatrix4x4>
#include <QtGlobal>

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <vector>

class MainWidget;
class Audio;
class ScrollBar;

using std::vector;

class WaveformWidget: public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	WaveformWidget(QApplication* parentApp, MainWidget* parentWid,
	               ScrollBar* scrollbar);
	~WaveformWidget();

	void generateData(Audio* music);
	void clearChunks();
	void clearMarkers();

	int getSize() const;
	int getRelativePosition() const;

public slots:
	void logic();
	void setPosition(int);
	void toggleFollowMarker(bool s);

signals:
	void positionChanged(int w);
	void areaScrolled(bool c);

private:
	void pushVertex(vector<float>& v, QVector2D pos, QVector3D color);
	void updateMatrices();

	QApplication* parentApp = nullptr;
	MainWidget* parentWid = nullptr;

	QTimer* logicTimer = nullptr;
	ScrollBar* scrollbar = nullptr;
	Audio* audio = nullptr;

	bool followMarker = true;

	int size = 0;

	// Determines the resolution of the waveform
	int samplesPerPixel = 750;

	double xCurrentPosition = 0;
	double xFinalPosition = 0;

	int mousePressX = 0;
	int mouseReleaseX = 0;

	class Chunk
	{
	public:
		Chunk(WaveformWidget* p, int width, int height,
		      vector<float> v, int index);
		~Chunk();

		static void initShaders();

		static QOpenGLShaderProgram* shader;
		static QOpenGLShaderProgram* texShader;
		static bool shadersInitialised;

		void render();

	private:
		QOpenGLBuffer vbo;
		QOpenGLVertexArrayObject vao;

		GLuint fbo, tex;

		WaveformWidget* parent;
		vector<float> dataPoints;

		QOpenGLFunctions* f;

		// Which chunk is this?
		int index;
	};

	class Marker
	{
	public:
		Marker(WaveformWidget* p, int pos, QVector3D color);
		~Marker();

		static void initShaders();

		static QOpenGLShaderProgram* markerShader;
		static bool markerShaderInitialised;

		void render();
		void setMatrices(const QMatrix4x4& proj, const QMatrix4x4& mod);

	private:
		QOpenGLBuffer vbo;
		QOpenGLVertexArrayObject vao;

		WaveformWidget* parent;
		vector<float> data;
	};

	vector<Chunk*> chunks;
	vector<Marker*> markers;
	Marker* progressMarker = nullptr;

protected:
	void paintGL();
	void initializeGL();
	void resizeGL(int width, int height);

	QMatrix4x4 projectionMatrix;
	QMatrix4x4 modelMatrix;
	bool dirtyMatrix;
};

#endif // WAVEFORMWIDGET_H
