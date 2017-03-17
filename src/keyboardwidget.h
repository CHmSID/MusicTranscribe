#ifndef KEYBOARDWIDGET_H
#define KEYBOARDWIDGET_H

#include <QApplication>
#include <QOpenGLWidget>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include <vector>

using std::vector;

class MainWidget;

class KeyboardWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	KeyboardWidget(QApplication* parentApp = 0, MainWidget* parentWid = 0);
	~KeyboardWidget();

	int getWidth() const; //Total width of all keys in pixels

	int getKeyPosition(int key) const;
	int getKeyWidth(int key) const;
	bool isKeyWhite(int key) const;

	QMatrix4x4 getModelMatrix() const;

private:
	QTimer* logicTimer;

	QApplication* parentApp = nullptr;
	MainWidget* parentWid = nullptr;

	QMatrix4x4 modelMatrix;
	QMatrix4x4 projectionMatrix;

	bool dirtyMatrix = false;

	int totalWidth = 0;

	class Key
	{
	public:
		Key(KeyboardWidget* p, bool isWhite, int index, QVector4D geom);
		~Key();

		void render();

		int getX();
		int getWidth();
		bool isWhite() const;

		static void initShaders();
		static QOpenGLShaderProgram* keyShader;
		static bool keyShaderInitialised;
		static QVector3D whiteColor;
		static QVector3D blackColor;

	private:
		QOpenGLBuffer vbo;
		QOpenGLVertexArrayObject vao;

		KeyboardWidget* parent;
		vector<float> data;

		int index;
		bool white;
		QVector3D color;
	};

	vector<Key*> keys;

signals:

public slots:
	void logic();

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
};

#endif // KEYBOARDWIDGET_H
