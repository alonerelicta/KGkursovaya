#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;


	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}


	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist * cos(fi2) * cos(fi1),
			camDist * cos(fi2) * sin(fi1),
			camDist * sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}


	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);


		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale * 0.08;
		s.Show();

		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale * 1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL* ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01 * dx;
		camera.fi2 += -0.01 * dy;
	}


	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k * r.direction.X() + r.origin.X();
		y = k * r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02 * dy);
	}


}

void mouseWheelEvent(OpenGL* ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01 * delta;

}

void keyDownEvent(OpenGL* ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL* ogl, int key)
{

}



GLuint texId;

//выполняется перед первым рендером
void initRender(OpenGL* ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);


	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE* texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char* texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH);


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}
void calculateNormal(double p1[], double p2[], double p3[]) {//украл у добросовестного другалечка
	double u1 = p2[0] - p1[0];
	double u2 = p2[1] - p1[1];
	double u3 = p2[2] - p1[2];

	double v1 = p3[0] - p1[0];
	double v2 = p3[1] - p1[1];
	double v3 = p3[2] - p1[2];

	double normalX = u2 * v3 - u3 * v2;
	double normalY = u3 * v1 - u1 * v3;
	double normalZ = u1 * v2 - u2 * v1;

	double length = sqrt(normalX * normalX + normalY * normalY + normalZ * normalZ);

	normalX /= length;
	normalY /= length;
	normalZ /= length;

	glNormal3d(normalX, normalY, normalZ);
}


void drawQuads() {
	double A[] = { 8, 5, 0 };
	double B[] = { 3, 0, 0 };
	double C[] = { 0, 4, 0 };
	double D[] = { 7, 6, 0 };
	double E[] = { 6, 11, 0 };
	double Q[] = { 8, 7, 0 };
	double G[] = { 16, 8, 0 };
	double W[] = { 13, 2, 0 };


	double AZ[] = { 8, 5, 3 };
	double BZ[] = { 3, 0, 3 };
	double CZ[] = { 0, 4, 3 };
	double DZ[] = { 7, 6, 3 };
	double EZ[] = { 6, 11, 3 };
	double QZ[] = { 8, 7, 3 };
	double GZ[] = { 16, 8, 3 };
	double WZ[] = { 13, 2, 3 };

	glBegin(GL_QUADS);

	glColor3d(0.2, 0.7, 0.7);

	glNormal3d(0, 0, -1);
	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(C);
	glVertex3dv(D);

	glNormal3d(0, 0, -1);
	glVertex3dv(Q);
	glVertex3dv(G);
	glVertex3dv(W);
	glVertex3dv(A);

	glNormal3d(0, 0, 1);
	glVertex3dv(AZ);
	glVertex3dv(BZ);
	glVertex3dv(CZ);
	glVertex3dv(DZ);

	glNormal3d(0, 0, 1);
	glVertex3dv(QZ);
	glVertex3dv(GZ);
	glVertex3dv(WZ);
	glVertex3dv(AZ);


	glColor3d(0, 0, 1);
	calculateNormal(A, AZ, BZ);
	glVertex3dv(A);
	glVertex3dv(AZ);
	glVertex3dv(BZ);
	glVertex3dv(B);

	calculateNormal(B, BZ, CZ);
	glVertex3dv(B);
	glVertex3dv(BZ);
	glVertex3dv(CZ);
	glVertex3dv(C);

	calculateNormal(C, CZ, DZ);
	glVertex3dv(C);
	glVertex3dv(CZ);
	glVertex3dv(DZ);
	glVertex3dv(D);

	calculateNormal(D, DZ, EZ);
	glVertex3dv(D);
	glVertex3dv(DZ);
	glVertex3dv(EZ);
	glVertex3dv(E);

	calculateNormal(E, EZ, QZ);
	glVertex3dv(E);
	glVertex3dv(EZ);
	glVertex3dv(QZ);
	glVertex3dv(Q);

	calculateNormal(Q, QZ, GZ);
	glVertex3dv(Q);
	glVertex3dv(QZ);
	glVertex3dv(GZ);
	glVertex3dv(G);

	calculateNormal(G, GZ, WZ);
	glVertex3dv(G);
	glVertex3dv(GZ);
	glVertex3dv(WZ);
	glVertex3dv(W);

	calculateNormal(W, WZ, AZ);
	glVertex3dv(W);
	glVertex3dv(WZ);
	glVertex3dv(AZ);
	glVertex3dv(A);

	glEnd();

}

void drawTriangles()
{
	double A[] = { 8, 5, 0 };
	double B[] = { 3, 0, 0 };
	double C[] = { 0, 4, 0 };
	double D[] = { 7, 6, 0 };
	double E[] = { 6, 11, 0 };
	double Q[] = { 8, 7, 0 };
	double G[] = { 16, 8, 0 };
	double W[] = { 13, 2, 0 };


	double AZ[] = { 8, 5, 3 };
	double BZ[] = { 3, 0, 3 };
	double CZ[] = { 0, 4, 3 };
	double DZ[] = { 7, 6, 3 };
	double EZ[] = { 6, 11, 3 };
	double QZ[] = { 8, 7, 3 };
	double GZ[] = { 16, 8, 3 };
	double WZ[] = { 13, 2, 3 };


	glBegin(GL_TRIANGLES);
	glColor3d(0.2, 0.7, 0.7);

	glNormal3d(0,0,-1);
	glVertex3dv(A);
	glVertex3dv(D);
	glVertex3dv(Q);

	glNormal3d(0, 0, 1);
	glVertex3dv(AZ);
	glVertex3dv(DZ);
	glVertex3dv(QZ);

	glNormal3d(0, 0, -1);
	glVertex3dv(D);
	glVertex3dv(E);
	glVertex3dv(Q);

	glNormal3d(0, 0, 1);
	glVertex3dv(DZ);
	glVertex3dv(EZ);
	glVertex3dv(QZ);

	glEnd();
}
void drawSemiCircle1() {
	int numSegments = 100;
	double radius = 2.5;
	double centerX = 1.5;
	double centerY = 2;
	double startAngle = M_PI - (M_PI / 3.4);
	double endAngle = 2 * M_PI - (M_PI / 3.4);

	glBegin(GL_TRIANGLE_FAN);
	glColor3d(0, 0, 1);
	glNormal3d(0, 0, -1);

	glVertex3d(centerX, centerY,0);

	for (int i = 0; i <= numSegments; i++) {
		double angle = startAngle + (i / static_cast<double>(numSegments)) * (endAngle - startAngle);
		double x = centerX + radius * std::cos(angle);
		double y = centerY + radius * std::sin(angle);
		glVertex3d(x, y,0);
		glNormal3d(0, 0, -1);
	}
	
	glEnd();
}

void drawSemiCircle2() {
	int numSegments = 100;
	double radius = 2.5;
	double centerX = 1.5;
	double centerY = 2;
	double height = 3;

	double startAngle = M_PI - (M_PI / 3.4);
	double endAngle = 2 * M_PI - (M_PI / 3.4);

	glBegin(GL_TRIANGLE_STRIP);
	glColor3d(0, 0, 1);

	for (int i = 0; i <= numSegments; i++) {
		double angle = startAngle + (i / static_cast<double>(numSegments)) * (endAngle - startAngle);
		double x = centerX + radius * std::cos(angle);
		double y = centerY + radius * std::sin(angle);


		glNormal3d(x-centerX,y-centerY,0);
		glVertex3d(x, y, 0.0);
		glVertex3d(x, y, height);
		
	}

	glEnd();
}

void drawSemiCircle3() {
	int numSegments = 100;
	double radius = 2.5;
	double centerX = 1.5;
	double centerY = 2;
	double height = 3.0;
	glNormal3d(0, 0, 1);
	double startAngle = M_PI - (M_PI / 3.4);
	double endAngle = 2 * M_PI - (M_PI / 3.4);

	glBegin(GL_TRIANGLE_FAN);
	glColor3d(0, 0, 1);

	for (int i = 0; i <= numSegments; i++) {
		double angle = startAngle + (i / static_cast<double>(numSegments)) * (endAngle - startAngle);
		double x = centerX + radius * std::cos(angle);
		double y = centerY + radius * std::sin(angle);
		glVertex3d(x, y, height);
		glNormal3d(0, 0, 1);
	}
	
	glEnd();
}

void Render(OpenGL* ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
	//размер блика
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);

	//===================================
	//Прогать тут  


//Начало рисования квадратика станкина
	
	drawTriangles();
	drawQuads();
	drawSemiCircle1();
	drawSemiCircle2();
	drawSemiCircle3();
	//конец рисования квадратика станкина


   //Сообщение вверху экрана


	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	//(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R=" << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;

	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}