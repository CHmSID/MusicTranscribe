#ifndef WAVEFORMWIDGET_H
#define WAVEFORMWIDGET_H

#include <QApplication>
#include <QVector3D>
#include <QVector2D>
#include <QMatrix4x4>
#include <QtGlobal>
#include <QWheelEvent>

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

/*
 * WaveformWidget handles the rendering of the waveform and audio seeking
 * through clicking and scrolling.
 */
class WaveformWidget: public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	WaveformWidget(QApplication* parentApp, MainWidget* parentWid,
	               ScrollBar* scrollbar);
	~WaveformWidget();

    void generateData(Audio* music);    // Generates vertices and colors for
                                        // OpenGL
    void clearChunks();                 // Delete the chunks we have and
                                        // Prepare for new audio to be loaded
	void clearMarkers();

	int getSize() const;
    int getRelativePosition() const;    // The position of the playing head
                                        // relative to the visible area

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

    // Determines the precision of the waveform
	int samplesPerPixel = 750;

	double xCurrentPosition = 0;
	double xFinalPosition = 0;

	int mousePressX = 0;
	int mouseReleaseX = 0;

    // A 512px wide sprite textured with audio data for optimisation reasons
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

    // A marking on the waveform specifying some position. One of those us used
    // to mark the playing head.
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

    void wheelEvent(QWheelEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

	QMatrix4x4 projectionMatrix;
	QMatrix4x4 modelMatrix;
	bool dirtyMatrix;
};

#endif // WAVEFORMWIDGET_H
