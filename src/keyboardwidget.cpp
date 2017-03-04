#include "keyboardwidget.h"

#include <QOpenGLFunctions>
#include <QTimer>

#include <vector>

#include "mainwidget.h"

using std::vector;

// Initialise static variables
QOpenGLShaderProgram* KeyboardWidget::Key::keyShader = new QOpenGLShaderProgram();
bool KeyboardWidget::Key::keyShaderInitialised = false;
QVector3D KeyboardWidget::Key::whiteColor = QVector3D(0.9f, 0.9f, 0.9f);
QVector3D KeyboardWidget::Key::blackColor = QVector3D(0.1f, 0.1f, 0.1f);

KeyboardWidget::KeyboardWidget(QApplication* parentApp,
                               MainWidget* parentWid)
    :parentApp(parentApp), parentWid(parentWid)
{

}

KeyboardWidget::~KeyboardWidget()
{
	delete logicTimer;
}

void KeyboardWidget::initializeGL()
{
	initializeOpenGLFunctions();
	glClearColor(1.0f, 1.0f, 0.0f, 1.0f);

	logicTimer = new QTimer();
	connect(logicTimer, SIGNAL(timeout()),
	        this, SLOT(logic()));
	logicTimer->start(32);

	Key::initShaders();

	int x = 0;
	int width;

	int whiteWidth = 24;
	int blackWidth = 14;

	for(int i = 0; i < 98; i++){

		int k = (i + 11) % 12;
		bool isWhite = false;
		int height = 60;
		width = blackWidth;

		if((k <= 4 && (k % 2) == 0) || (k > 4 && (k % 2) != 0)){

			isWhite = true;
			height = 100;

			if(k == 0 || k == 5){

				width = whiteWidth - 4;
			}
			else if(k == 2 || k == 7 || k == 9){

				width = whiteWidth + 3;
			}
			else if(k == 4 || k == 11){

				width = whiteWidth - 4;
			}
			else{

				width = whiteWidth;
			}

			if(i == 0 || i == 97)
				width = 13;
		}
		else{

			x -= blackWidth / 2;
		}

		keys.push_back(new Key(this, isWhite, i, QVector4D(x, 100, width, height)));

		if(isWhite)
			x += width + 1;
		else
			x += width / 2;
	}
}

void KeyboardWidget::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT);

	Key::keyShader->bind();
	for(unsigned i = 0; i < keys.size(); i++){

		if(keys[i]->isWhite())
			keys[i]->render();
	}
	for(unsigned i = 0; i < keys.size(); i++){

		if(!keys[i]->isWhite())
			keys[i]->render();
	}
	Key::keyShader->release();
}

void KeyboardWidget::resizeGL(int width, int height)
{
	projectionMatrix.setToIdentity();
	projectionMatrix.ortho(0, width, 0, height, 1, -1);
	dirtyMatrix = true;
}

void KeyboardWidget::logic()
{
	update();
}

/*************************************
 * Key
 * **********************************/

void KeyboardWidget::Key::initShaders()
{
	if(!keyShaderInitialised)
	{
		keyShader->addShaderFromSourceFile(QOpenGLShader::Vertex,
		                                   ":/data/shaders/keyboardkey.vert");
		keyShader->addShaderFromSourceFile(QOpenGLShader::Fragment,
		                                   ":/data/shaders/keyboardkey.frag");

		keyShader->link();
		keyShader->bind();
		qDebug() << "Linked";
		keyShaderInitialised = true;
	}
}

KeyboardWidget::Key::Key(KeyboardWidget *p, bool isWhite, int index, QVector4D geom)
    :parent(p), index(index), white(isWhite)
{
	data = {geom.x(), geom.y(),
	       geom.x() + geom.z(), geom.y(),
	       geom.x() + geom.z(), geom.y() - geom.w(),
	       geom.x(), geom.y() - geom.w()};

	if(white)
		color = whiteColor;
	else
		color = blackColor;

	keyShader->bind();
	vbo.create();
	vbo.bind();
	vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	vbo.allocate(data.data(), sizeof(float) * data.size());

	vao.create();
	vao.bind();

	keyShader->enableAttributeArray(0);
	keyShader->setAttributeBuffer(0, GL_FLOAT, 0, 2, 0);

	vao.release();
	vbo.release();
}

KeyboardWidget::Key::~Key()
{
	// Make sure we're in the right OpenGL context
	parent->makeCurrent();
	vao.destroy();
	vbo.destroy();
}

bool KeyboardWidget::Key::isWhite() const
{
	return white;
}

void KeyboardWidget::Key::render()
{
	if(parent->dirtyMatrix)
	{
		keyShader->setUniformValue("projectionMatrix",
		                           parent->projectionMatrix);
		keyShader->setUniformValue("modelMatrix",
		                           parent->modelMatrix);
	}

	keyShader->setUniformValue("currentColor", color);

	vao.bind();
	parent->glDrawArrays(GL_TRIANGLE_FAN, 0, data.size() / 2);
	vao.release();
}
