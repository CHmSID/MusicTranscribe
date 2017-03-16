#include "waveformwidget.h"
#include "mainwidget.h"
#include "audio.h"
#include "scrollbar.h"
#include "util.h"

#include <math.h>

QOpenGLShaderProgram* WaveformWidget::Chunk::shader = new QOpenGLShaderProgram();
QOpenGLShaderProgram* WaveformWidget::Chunk::texShader = new QOpenGLShaderProgram();
bool WaveformWidget::Chunk::shadersInitialised = false;

QOpenGLShaderProgram* WaveformWidget::Marker::markerShader = new QOpenGLShaderProgram();
bool WaveformWidget::Marker::markerShaderInitialised = false;

WaveformWidget::WaveformWidget(QApplication* parentApp, MainWidget* wid,
                               ScrollBar* scrollbar)
    :parentApp(parentApp), parentWid(wid), scrollbar(scrollbar)
{

}

WaveformWidget::~WaveformWidget()
{
	clearChunks();
	clearMarkers();

	// Delete all static shaders
	// Correct context is switched to in clearChunks() above
	if(Chunk::shadersInitialised)
	{
		delete Chunk::shader;
		delete Chunk::texShader;
		Chunk::shadersInitialised = false;
	}

	if(Marker::markerShaderInitialised)
	{
		delete Marker::markerShader;
		Marker::markerShaderInitialised = false;
	}
}

void WaveformWidget::initializeGL()
{
	logicTimer = new QTimer();
	connect(logicTimer, SIGNAL(timeout()),
	        this, SLOT(logic()));
	logicTimer->start(33.3f);

	initializeOpenGLFunctions();
	glClearColor(41 / (float)255, 49 / (float)255, 58 / (float)255, 1.0f);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_SCISSOR_TEST);

	WaveformWidget::Chunk::initShaders();
	WaveformWidget::Marker::initShaders();
}

void WaveformWidget::paintGL(){

	glClear(GL_COLOR_BUFFER_BIT);

	//render all chunks
	WaveformWidget::Chunk::shader->bind();
	for(unsigned i = 0; i < chunks.size(); i++){

		chunks[i]->render();
	}
	WaveformWidget::Chunk::shader->release();

	//render all markers
	WaveformWidget::Marker::markerShader->bind();
	for(unsigned i = 0; i < markers.size(); i++){

		markers[i]->setMatrices(projectionMatrix, modelMatrix);
		markers[i]->render();
	}

	if(progressMarker != nullptr){

		QMatrix4x4 m(modelMatrix);
		m.translate(getSize() * audio->getProgress(), 0, 0);

		progressMarker->setMatrices(projectionMatrix, m);
		progressMarker->render();
	}

	WaveformWidget::Marker::markerShader->release();
}

void WaveformWidget::resizeGL(int width, int height)
{
	projectionMatrix.setToIdentity();
	projectionMatrix.ortho(0, width, 0, height, 1, -1);
	glScissor(0, 0, width * 2, height * 2);
	dirtyMatrix = true;
}

void WaveformWidget::toggleFollowMarker(bool s)
{
	followMarker = s;
}

void WaveformWidget::logic()
{
	// Reset the matrix
	modelMatrix.setToIdentity();
	modelMatrix.translate(QVector3D(-xCurrentPosition, 0, 0));
	dirtyMatrix = true;

	if(followMarker && scrollbar != nullptr)
	{
//		if(getRelativePosition() > width() / 2)
//		{
		    scrollbar->addValue(getRelativePosition() - width() / 2.0f);
//		}
	}

	update();
	parentApp->processEvents();
}

void WaveformWidget::clearChunks()
{
	makeCurrent();

	for(unsigned i = 0; i < chunks.size(); i++)
	{
		delete chunks[i];
	}

	chunks.clear();
}

void WaveformWidget::clearMarkers()
{
	makeCurrent();

	for(unsigned i = 0; i < markers.size(); i++)
	{
		delete markers[i];
	}

	markers.clear();

	if(progressMarker != nullptr)
	{
		delete progressMarker;
		progressMarker = nullptr;
	}
}

void WaveformWidget::pushVertex(vector<float> &v, QVector2D d, QVector3D c)
{
	v.push_back(d.x());
	v.push_back(d.y());

	v.push_back(c.x());
	v.push_back(c.y());
	v.push_back(c.z());
}

int WaveformWidget::getRelativePosition() const
{

	if(audio != nullptr)
	{
		return audio->getProgress() * getSize() - xCurrentPosition;
	}
	else
		return 0;
}

int WaveformWidget::getSize() const
{
	return size;
}

void WaveformWidget::setPosition(int p)
{
	xCurrentPosition = p / 100.0f;
}

void WaveformWidget::updateMatrices()
{
	if(Chunk::shadersInitialised){

		Chunk::shader->setUniformValue("projectionMatrix",
		                        projectionMatrix);
		Chunk::shader->setUniformValue("modelMatrix",
		                        modelMatrix);
	}

	if(Marker::markerShaderInitialised)
	{
		Marker::markerShader->setUniformValue("projectionMatrix",
		                              projectionMatrix);
		Marker::markerShader->setUniformValue("modelMatrix",
		                              modelMatrix);
	}

	dirtyMatrix = false;
}

void WaveformWidget::generateData(Audio *audio)
{
	this->audio = audio;
	makeCurrent();
	long index = 0; //current sample of the audio
	size = (float)audio->info.dataSize / audio->info.byteRate *
	       audio->info.sampleRate / samplesPerPixel;
	int tmpSize = size; //size left to process

	int chunkIndex = 0;
	int chunkWidth = 512;
	int chunkHeight = 100;

	vector<double> data;

	while(tmpSize != 0)
	{
		int length = 0; //pixels to be generated in this pass

		if(tmpSize >= chunkWidth)
			length = chunkWidth;   //there's more than one chunk's
		                            //worth of data left
		else
			length = tmpSize;       //less than one chunk,
		                            //it will not be 512 long

		tmpSize -= length;

		for(int i = 0; i < length; i++){

			double rsmLeft = 0;     //root square mean
			double rsmRight = 0;    //left and right channels

			for(int k = 0; k < samplesPerPixel; k++){

				short sLeft = 0;    //current sample
				short sRight = 0;

				if(audio->info.bitRate == 16){

					if(audio->info.numChannels == 1){

						sLeft = read2Bytes(audio->getSample(index),
						               audio->getSample(index + 1));

						index += 2;
					}
					else if(audio->info.numChannels == 2){

						sLeft = read2Bytes(audio->getSample(index),
						               audio->getSample(index + 1));

						sRight = read2Bytes(audio->getSample(index + 2),
						               audio->getSample(index + 3));

						index += 4;
					}
				}
				else if(audio->info.bitRate == 8){

					if(audio->info.numChannels == 1){

						sLeft = audio->getSample(index);
						index++;
					}
					else if(audio->info.numChannels == 2){

						sLeft = audio->getSample(index);

						sRight = audio->getSample(index + 2);

						index += 2;
					}
				}

				rsmLeft += (sLeft * sLeft);
				rsmRight += (sRight * sRight);
			}

			rsmLeft /= (float)samplesPerPixel;
			rsmLeft = sqrt(rsmLeft);

			rsmRight /= (float)samplesPerPixel;
			rsmRight = sqrt(rsmRight);

			if(audio->info.numChannels == 1){

				data.push_back(rsmLeft);
			}
			else if(audio->info.numChannels == 2){

				data.push_back(rsmLeft);
				data.push_back(rsmRight);
			}
		}
	}

	//normalise data
	double highest = 0;
	for(unsigned i = 0; i < data.size(); i++){

		if(data[i] > highest)
			highest = data[i];
	}

	for(unsigned i = 0; i < data.size(); i++){

		data[i] = data[i] / highest * 50;
	}

	//generateChunks
	vector<float> v;
	tmpSize = size;
	long dataIndex = 0;

	while(tmpSize != 0){

		int length = 0; //pixels to be generated in this pass
		v.clear();

		if(tmpSize >= chunkWidth)
			length = chunkWidth;   //there's more than one chunk's
		                            //worth of data left
		else
			length = tmpSize;       //less than one chunk

		tmpSize -= length;

		for(int i = 0; i < length; i++){

			if(audio->info.numChannels == 1){

				pushVertex(v, QVector2D(i + 1, 50 - data[dataIndex]),
				           QVector3D(152 / (float)255, 169 / (float)255, 216 / (float)255));
				pushVertex(v, QVector2D(i + 1, 50 + data[dataIndex]),
				           QVector3D(152 / (float)255, 169 / (float)255, 216 / (float)255));
				dataIndex++;
			}
			else if(audio->info.numChannels == 2){

				pushVertex(v, QVector2D(i + 1, 50),
				           QVector3D(132 / (float)255, 156 / (float)255, 216 / (float)255));
				pushVertex(v, QVector2D(i + 1, 50 - data[dataIndex + 1]),
				           QVector3D(132 / (float)255, 156 / (float)255, 216 / (float)255));

				pushVertex(v, QVector2D(i + 1, 50 + data[dataIndex]),
				           QVector3D(152 / (float)255, 169 / (float)255, 216 / (float)255));
				pushVertex(v, QVector2D(i + 1, 50),
				           QVector3D(152 / (float)255, 169 / (float)255, 216 / (float)255));
				dataIndex += 2;
			}
		}

		chunks.push_back(new Chunk(this, length, chunkHeight, v, chunkIndex));
		chunkIndex++;
	}

	if(progressMarker == nullptr)
		progressMarker = new Marker(this, 0, QVector3D(1, 0, 0));
	else
		qDebug() << "WARNING: progress marker already exists!";
}

////////////////////////////////////////////////////////////////
//Chunk
////////////////////////////////////////////////////////////////

void WaveformWidget::Chunk::initShaders(){

	if(!shadersInitialised){

		texShader->addShaderFromSourceFile(QOpenGLShader::Vertex,
		                                   ":/data/shaders/waveform.vert");
		texShader->addShaderFromSourceFile(QOpenGLShader::Fragment,
		                                   ":/data/shaders/waveform.frag");
		texShader->link();
		texShader->bind();

		shader->addShaderFromSourceFile(QOpenGLShader::Vertex,
		                                ":/data/shaders/chunk.vert");
		shader->addShaderFromSourceFile(QOpenGLShader::Fragment,
		                                ":/data/shaders/chunk.frag");
		shader->link();
		shader->bind();

		shadersInitialised = true;
	}
}

WaveformWidget::Chunk::Chunk(WaveformWidget* p, int width, int height,
                             vector<float> v, int index){

	parent = p;
	f = QOpenGLContext::currentContext()->functions();
	f->initializeOpenGLFunctions();

	//RENDER TO FBO

	f->glGenFramebuffers(1, &fbo);
	f->glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	f->glGenTextures(1, &tex);
	f->glBindTexture(GL_TEXTURE_2D, tex);

	f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
	                0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	f->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
	                       GL_TEXTURE_2D, tex, 0);

	if(f->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		qDebug() << "Framebuffer failed!" << f->glCheckFramebufferStatus(GL_FRAMEBUFFER);

	QOpenGLBuffer tmpVbo;
	QOpenGLVertexArrayObject tmpVao;

	tmpVao.create();
	tmpVao.bind();
	tmpVbo.create();
	tmpVbo.bind();
	tmpVbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	tmpVbo.allocate(v.data(), sizeof(float) * v.size());

	texShader->bind();
	texShader->enableAttributeArray(0);
	texShader->setAttributeBuffer(0, GL_FLOAT, 0, 2, 5 * sizeof(float));
	texShader->enableAttributeArray(1);
	texShader->setAttributeBuffer(1, GL_FLOAT, 2 * sizeof(float), 3, 5 * sizeof(float));

	f->glViewport(0, 0, width, height);
	f->glScissor(0, 0, width, height);

	texShader->bind();

	f->glClear(GL_COLOR_BUFFER_BIT);

	QMatrix4x4 projection;
	projection.ortho(0, width, 0, height, 1, -1);
	texShader->setUniformValue("projectionMatrix",
	                           projection);

	f->glDrawArrays(GL_LINES, 0, v.size() / 5);
	texShader->release();

	f->glScissor(0, 0, parent->width(), parent->height());
	parent->resizeGL(parent->width(), parent->height());
	parent->dirtyMatrix = true;

	tmpVao.release();
	tmpVbo.release();

	tmpVao.destroy();
	tmpVbo.destroy();

	f->glBindFramebuffer(GL_FRAMEBUFFER, 0);
	f->glBindTexture(GL_TEXTURE_2D, 0);

	//END RENDER TO FBO

	this->index = index;

	float x = index * 512;
	float y = 0;

	dataPoints = {x,         y,          0.0f, 0.0f,
	              x + width, y + height, 1.0f, 1.0f,
	              x,         y + height, 0.0f, 1.0f,
	              x,         y,          0.0f, 0.0f,
	              x + width, y,          1.0f, 0.0f,
	              x + width, y + height, 1.0f, 1.0f};

	shader->bind();

	vbo.create();
	vbo.bind();
	vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	vbo.allocate(dataPoints.data(), sizeof(float) * dataPoints.size());

	vao.create();
	vao.bind();

	shader->enableAttributeArray(0);
	shader->setAttributeBuffer(0, GL_FLOAT, 0, 2, 4 * sizeof(float));
	shader->enableAttributeArray(1);
	shader->setAttributeBuffer(1, GL_FLOAT, 2 * sizeof(float), 2, 4 * sizeof(float));

	vao.release();
	vbo.release();
}

WaveformWidget::Chunk::~Chunk(){

	parent->makeCurrent();
	parent->glDeleteFramebuffers(1, &fbo);
	parent->glDeleteTextures(1, &tex);
	vao.destroy();
	vbo.destroy();
}

void WaveformWidget::Chunk::render(){

	if(parent->dirtyMatrix){

		shader->setUniformValue("projectionMatrix",
		                        parent->projectionMatrix);
		shader->setUniformValue("modelMatrix",
		                        parent->modelMatrix);
//        parent->dirtyMatrix = false;
	}

	vao.bind();
	f->glBindTexture(GL_TEXTURE_2D, tex);
	f->glDrawArrays(GL_TRIANGLES, 0, dataPoints.size() / 4);
	f->glBindTexture(GL_TEXTURE_2D, 0);
	vao.release();
}

///////////////////////////////////////////////////////////////
//Marker
///////////////////////////////////////////////////////////////

void WaveformWidget::Marker::initShaders(){

	if(!markerShaderInitialised){

		markerShader->addShaderFromSourceFile(QOpenGLShader::Vertex,
		                                      ":/data/shaders/marker.vert");
		markerShader->addShaderFromSourceFile(QOpenGLShader::Fragment,
		                                      ":/data/shaders/marker.frag");

		markerShader->link();
		markerShader->bind();

		markerShaderInitialised = true;
	}
}

WaveformWidget::Marker::Marker(WaveformWidget* p, int pos, QVector3D col){

	parent = p;

	data = {(float)pos, 0.0f,   col.x(), col.y(), col.z(),
	        (float)pos, 100.0f, col.x(), col.y(), col.z()};

	markerShader->bind();

	vbo.create();
	vbo.bind();
	vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	vbo.allocate(data.data(), sizeof(float) * data.size());

	vao.create();
	vao.bind();

	markerShader->enableAttributeArray(0);
	markerShader->enableAttributeArray(1);
	markerShader->setAttributeBuffer(0, GL_FLOAT, 0, 2, 5 * sizeof(float));
	markerShader->setAttributeBuffer(1, GL_FLOAT, 2 * sizeof(float), 3, 5 * sizeof(float));

	vao.release();
	vbo.release();
}

WaveformWidget::Marker::~Marker(){

	parent->makeCurrent();
	vao.destroy();
	vbo.destroy();
}

void WaveformWidget::Marker::setMatrices(const QMatrix4x4& proj, const QMatrix4x4& mod){

	if(parent->dirtyMatrix){

		markerShader->setUniformValue("projectionMatrix",
		                              proj);
		markerShader->setUniformValue("modelMatrix",
		                              mod);

//        parent->dirtyMatrix = false;
	}
}

void WaveformWidget::Marker::render(){

	vao.bind();
	parent->glDrawArrays(GL_LINES, 0, data.size() / 5);
	vao.release();
}
