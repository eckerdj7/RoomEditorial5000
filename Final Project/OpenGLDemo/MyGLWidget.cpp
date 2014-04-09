#include "MyGLWidget.h"

MyGLWidget::MyGLWidget(QWidget* parent) : QGLWidget(parent) {
	camZoom = 10.0f;
	camRotX = -45.0f;
	camRotY = 0.0f;
	lightPos = vec3(0.0f, 5.5f, 10.0f);
}

MyGLWidget::~MyGLWidget() {

	glDetachShader(shaderProgram, vertexShader);
	glDetachShader(shaderProgram, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteBuffers(1, &vbo);
}

void MyGLWidget::initializeGL() {
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClearDepth(1.0);

	

	//Do something similar to this to set up a buffer for colors
	glGenBuffers(1, &vbo);
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	shaderProgram = glCreateProgram();

	const char* vertexSource = textFileRead("lambert.vert");
	const char* fragmentSource = textFileRead("lambert.frag");
	glShaderSource(vertexShader, 1, &vertexSource, 0);
	glShaderSource(fragmentShader, 1, &fragmentSource, 0);
	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	//also want to get the location of "vs_color"
	vLocation = glGetAttribLocation(shaderProgram, "vs_position");
	vColor = glGetAttribLocation(shaderProgram, "vs_color");
	vNormal = glGetAttribLocation(shaderProgram, "vs_normal");

	//Do something similar to this to get the location of "u_modelMatrix"
	u_projLocation = glGetUniformLocation(shaderProgram, "u_projMatrix");
	u_modelMatrix = glGetUniformLocation(shaderProgram, "u_modelMatrix");
	u_lightPos = glGetUniformLocation(shaderProgram, "u_lightPos");

	glUseProgram(shaderProgram);

	blueCube.initializeCubeCoords(shaderProgram, 0.0f, 0.0f, 1.0f);
	redCube.initializeCubeCoords(shaderProgram, 1.0f, 0.0f, 0.0f);
	greenCube.initializeCubeCoords(shaderProgram, 0.0f, 1.0f, 0.0f);
	orangeCube.initializeCubeCoords(shaderProgram, 1.0f, 0.45f, 0.0f);
	purpleCube.initializeCubeCoords(shaderProgram, 1.0f, 0.0f, 1.0f);
	whiteCube.initializeCubeCoords(shaderProgram, 1.0f, 1.0f, 1.0f);

	cameraTrans = mat4(1.0f);
	cameraPos = vec3(0.0f, 0.0f, 10.0f);
	cameraUp = vec3(0.0f, 1.0f, 0.0f);
	cameraRef = vec3(0.0f);

	readScene("testSceneHW1.txt");
}

void MyGLWidget::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	cameraTrans = mat4(1.0f);
	mat4 rotY = glm::rotate(mat4(1.0f), camRotY, vec3(0.0f, 1.0f, 0.0f));
	mat4 rotX = glm::rotate(mat4(1.0f), camRotX, vec3(1.0f, 0.0f, 0.0f));
	
	vec4 camPos(0, 0, camZoom, 1);
	vec4 camUp(0, 1, 0, 0);
	camUp = rotY * rotX * camUp;
	camPos = rotY * rotX * camPos;
	
	//Set the camera
	cameraTrans = glm::lookAt(vec3(camPos.x, camPos.y, camPos.z), vec3(0, 0, 0), vec3(camUp.x, camUp.y, camUp.z));

	//Lighting Calculations and Representation
	lightMatrix = mat4(1.0f);
	mat4 lightTrans = glm::translate(mat4(1.0f), lightPos);
	mat4 lightScale = glm::scale(mat4(1.0f), vec3(0.5f, 0.5f, 0.5f));
	lightTrans = cameraTrans * lightTrans * lightScale;

	glUniformMatrix4fv(u_lightPos, 1, GL_FALSE, &lightTrans[0][0]);

	whiteCube.draw(lightTrans);

	//Traverse the scene graph
	sg->traverse(cameraTrans);

	

	glFlush();
}

void MyGLWidget::setUniformMatrixAndDraw(mat4 matrix)
{
	//glUniformMatrix4fv(u_modelMatrix, 1, GL_FALSE, &matrix[0][0]);
	//glDrawArrays(GL_QUADS, 0, blueCube.vertexCount);
}



//An example on how to use the buffer to get position
//void MyGLWidget::generateGasket() {
//	glm::vec2* points = new glm::vec2[500000];
//	
//	glm::vec2 vertices[3] = {glm::vec2(-1.0, -1.0), glm::vec2(0.0, 1.0), glm::vec2(1.0, -1.0)};
//	points[0] = glm::vec2(0.25, 0.50);
//
//	for(int i = 1; i < 500000; i++) {
//		int k = rand() % 3;
//
//		points[i] = glm::vec2((points[i-1].x + vertices[k].x) / 2.0, (points[i-1].y + vertices[k].y) / 2.0);
//	}
//	glBindBuffer(GL_ARRAY_BUFFER, vbo);
//	glBufferData(GL_ARRAY_BUFFER, 500000 * sizeof(glm::vec2), points, GL_STATIC_DRAW);
//
//	delete [] points;
//
//	glEnableVertexAttribArray(vLocation);
//	glVertexAttribPointer(vLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
//}

void MyGLWidget::resizeGL(int width, int height) {
	glViewport(0, 0, width, height);

	//Here's how to make matrices for transformations, check the documentation of GLM for rotation, scaling, and translation
	glm::mat4 projection = glm::perspective(90.0f, static_cast<float>(width) / static_cast<float>(height), 0.1f, 30.0f);
	//glm::mat4 camera = glm::lookAt(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	//Can multiply matrices together, careful about ordering!
	//projection = projection;// * camera;
	
	//Do something similar for u_modelMatrix before rendering things
	glUniformMatrix4fv(u_projLocation, 1, GL_FALSE, &projection[0][0]);
}

//from swiftless.com
char* MyGLWidget::textFileRead(const char* fileName) {
    char* text;
    
    if (fileName != NULL) {
        FILE *file = fopen(fileName, "rt");
        
        if (file != NULL) {
            fseek(file, 0, SEEK_END);
            int count = ftell(file);
            rewind(file);
            
            if (count > 0) {
                text = (char*)malloc(sizeof(char) * (count + 1));
                count = fread(text, sizeof(char), count, file);
                text[count] = '\0';	//cap off the string with a terminal symbol, fixed by Cory
            }
            fclose(file);
        }
    }
    return text;
}

void MyGLWidget::rotateX(mat4& m, float deg)
{
	m = glm::rotate(m, deg, vec3(1.0f, 0.0f, 0.0f));
}

void MyGLWidget::rotateY(mat4& m, float deg)
{
	m = glm::rotate(m, deg, vec3(0.0f, 1.0f, 0.0f));
}

void MyGLWidget::rotateZ(mat4& m, float deg)
{
	m = glm::rotate(m, deg, vec3(0.0f, 0.0f, 1.0f));
}

void MyGLWidget::scale(mat4& m, vec3 scaleBy)
{
	m = glm::scale(m, scaleBy);
}

void MyGLWidget::translateToFloorPos(mat4& m, int x, int z, Floor& f)
{
	m = glm::translate(m, f.floorIndexToVec3(x, z));
}

void MyGLWidget::makeWorld(mat4& m, vec3 scaleBy, float rotate, int x, int z, Floor& f)
{
	m = mat4(1.0f);
	translateToFloorPos(m, x, z, f);
	rotateY(m, rotate);
	scale(m, scaleBy);
}

void MyGLWidget::readScene(string filename)
{
	ifstream fin;
	fin.open(filename);
	if (fin.fail())
	{
		exit(1);
	}
	int x, z, itemCount;
	fin >> x >> z >> itemCount;
	floor = new Floor(x, z);
	table = new Table();
	chair = new Chair();
	box = new Box();
	floor->setCube(&purpleCube);
	table->setCube(&orangeCube);
	chair->setCube(&blueCube);
	box->setCube(&redCube);
	sg = new SceneGraph(x, z);
	sg->setFloorSize(x, z);
	sg->linkGeometry(floor);
	


	for (int i=0; i<itemCount; ++i)
	{
		SceneGraph* s = new SceneGraph();
		string type;
		int xx, zz, theta;
		float sx, sy, sz;
		fin >> type >> xx >> zz >> theta >> sx >> sy >> sz;
		if (type == "table")
			s->linkGeometry(table);
		if (type == "chair")
			s->linkGeometry(chair);
		if (type == "box")
			s->linkGeometry(box);
		s->setFloorSize(x,z);
		s->setRotY(theta);
		s->setScaleX(sx);
		s->setScaleY(sy);
		s->setScaleZ(sz);
		
		sg->addChild(s, xx, zz);
	}

}

void MyGLWidget::rotateCameraY(int num)
{
	camRotY = (float)num;
	paintGL();
	update();
}

void MyGLWidget::rotateCameraX(int num)
{
	camRotX = (float)num;
	paintGL();
	update();
}
void MyGLWidget::lightX(int num)
{
	lightPos.x = num / 10.0f;
	paintGL();
	update();
}
void MyGLWidget::lightY(int num)
{
	lightPos.y = num / 10.0f;
	paintGL();
	update();
}
void MyGLWidget::lightZ(int num)
{
	lightPos.z = num / 10.0f;
	paintGL();
	update();
}

void MyGLWidget::rotateCameraLeft()
{
	camRotY -= 0.1f;
	paintGL();
}

void MyGLWidget::rotateCameraRight()
{
	camRotY += 0.1f;
	paintGL();
}

void MyGLWidget::zoomCamera(int num)
{
	float zoom = ((float)num) / 10.0f;
	camZoom = zoom;
	paintGL();
	update();
}

void MyGLWidget::loadNewScene(string fileName)
{
	readScene(fileName);
	paintGL();
	update();
}