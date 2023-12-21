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

//����� ��� ��������� ������
class CustomCamera : public Camera
{
public:
	//��������� ������
	double camDist;
	//���� �������� ������
	double fi1, fi2;


	//������� ������ �� ���������
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}


	//������� ������� ������, ������ �� ����� ��������, ���������� �������
	void SetUpCamera()
	{
		//�������� �� ������� ������ ������
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
		//������� ��������� ������
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //������� ������ ������


//����� ��� ��������� �����
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//��������� ������� �����
		pos = Vector3(1, 1, 3);
	}


	//������ ����� � ����� ��� ���������� �����, ���������� �������
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
			//����� �� ��������� ����� �� ����������
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//������ ���������
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

		// ��������� ��������� �����
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// �������������� ����������� �����
		// ������� ��������� (���������� ����)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// ��������� ������������ �����
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// ��������� ���������� ������������ �����
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //������� �������� �����




//������ ���������� ����
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL* ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//������ ���� ������ ��� ������� ����� ������ ����
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01 * dx;
		camera.fi2 += -0.01 * dy;
	}


	//������� ���� �� ���������, � ����� ��� ����
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

//����������� ����� ������ ��������
void initRender(OpenGL* ogl)
{
	//��������� �������

	//4 ����� �� �������� �������
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//��������� ������ ��������� �������
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//�������� ��������
	glEnable(GL_TEXTURE_2D);


	//������ ����������� ���������  (R G B)
	RGBTRIPLE* texarray;

	//������ ��������, (������*������*4      4, ���������   ����, �� ������� ������������ �� 4 ����� �� ������� �������� - R G B A)
	char* texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);



	//���������� �� ��� ��������
	glGenTextures(1, &texId);
	//������ ��������, ��� ��� ����� ����������� � ���������, ����� ����������� �� ����� ��
	glBindTexture(GL_TEXTURE_2D, texId);

	//��������� �������� � �����������, � ���������� ��� ������  ��� �� �����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//�������� ������
	free(texCharArray);
	free(texarray);

	//������� ����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//������ � ���� ����������� � "������"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// ������������ �������� : �� ����� ����� ����� 1
	glEnable(GL_NORMALIZE);

	// ���������� ������������� ��� �����
	glEnable(GL_LINE_SMOOTH);


	//   ������ ��������� ���������
	//  �������� GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  ������� � ���������� �������� ���������(�� ���������), 
	//                1 - ������� � ���������� �������������� ������� ��������       
	//                �������������� ������� � ���������� ��������� ����������.    
	//  �������� GL_LIGHT_MODEL_AMBIENT - ������ ������� ���������, 
	//                �� ��������� �� ���������
	// �� ��������� (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}
void calculateNormal(double p1[], double p2[], double p3[]) {//����� � ��������������� ����������
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


	//��������������
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//��������� ���������
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//�������
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//��������
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//����������
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
	//������ �����
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//���� ���� �������, ��� ����������� (����������� ���������)
	glShadeModel(GL_SMOOTH);

	//===================================
	//������� ���  


//������ ��������� ���������� ��������
	
	drawTriangles();
	drawQuads();
	drawSemiCircle1();
	drawSemiCircle2();
	drawSemiCircle3();
	//����� ��������� ���������� ��������


   //��������� ������ ������


	glMatrixMode(GL_PROJECTION);	//������ �������� ������� ��������. 
	//(���� ��������� ��������, ����� �� ������������.)
	glPushMatrix();   //��������� ������� ������� ������������� (������� ��������� ������������� ��������) � ���� 				    
	glLoadIdentity();	  //��������� ��������� �������
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //������� ����� ������������� ��������

	glMatrixMode(GL_MODELVIEW);		//������������� �� �����-��� �������
	glPushMatrix();			  //��������� ������� ������� � ���� (��������� ������, ����������)
	glLoadIdentity();		  //���������� �� � ������

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //������� ����� ��������� ��� ������� ������ � �������� ������.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - ���/���� �������" << std::endl;
	ss << "L - ���/���� ���������" << std::endl;
	ss << "F - ���� �� ������" << std::endl;
	ss << "G - ������� ���� �� �����������" << std::endl;
	ss << "G+��� ������� ���� �� ���������" << std::endl;
	ss << "�����. �����: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "�����. ������: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "��������� ������: R=" << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;

	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //��������������� ������� �������� � �����-��� �������� �� �����.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}