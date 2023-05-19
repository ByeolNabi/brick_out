#include <windows.h>
#include <gl/gl.h>
#include <gl/glut.h> // (or others, depending on the system in use)
#include <math.h>
#include <iostream>

#define	CIRCLE_EDGE_NUM 10
#define MAIN_BALL_RADIUS 10

#define BOARD_LEFT		0
#define BOARD_RIGHT		400
#define BOARD_BOTTOM	0
#define BOARD_TOP		600
#define BOARD_STARTLINE	50 - MAIN_BALL_RADIUS

#define	PI		3.1415926
#define GRAVITY_ACC	-0.001

#define RED 0
#define GREEN 1
#define YELLOW 2
#define BLUE 3
#define WHITE 4
#define MAGENTA 5


struct Point {
	GLfloat x;
	GLfloat y;
};

// Point operate funcs
Point operator+(const Point a, const Point b);
Point operator-(const Point a, const Point b);
Point operator/(const Point a, double b);
Point operator/(double b, const Point a);
Point operator*(const Point a, double b);
Point operator*(double b, const Point a);
Point multiply(const Point target, float mul_x, float mul_y);

Point ball_v = { 0, 0.01 };	// 초기 공의 속도 정의하기
Point gravity_a = { 0, GRAVITY_ACC }; // 가속도 공식
Point ball[1] = { 100, 400 };	// 공의 좌표
Point mouse[1] = {0,0};	// 마우스 좌표

bool drag = false;	// 그리기 시작 : true, 그리기 종료 : false

// 색 데이터
GLfloat colors[][3] = {
	{ 1.0, 0.0, 0.0 },	//red
	{ 0.0, 1.0, 0.0 },	//green
	{ 1.0, 1.0, 0.0 },	//yellow
	{ 0.0, 0.0, 1.0 },	//blue
	{ 1.0, 1.0, 1.0 },	//white
	{ 1.0, 0.0, 1.0 }	//magenta
};

//==================func==================//
void Init() {
	gluOrtho2D(0, BOARD_RIGHT, 0, BOARD_TOP);
}

// physics
void go(Point velocity);
void stop();
void ball_speed_adder();
// if collision happen, return true
namespace collision {
	bool startline(Point* ball);
	bool top(Point* ball);
	bool left(Point* ball);
	bool right(Point* ball);
}
void ball_collision();

// draw
void poly_circle(double radius, int color);
void axis();

// callback func
void keyboard_ball_control(int key);
void MySpecial(int key, int x, int y);
void MyMouse(int button, int state, int x, int y);
void MyMotion(int x, int y);

// display result
void RenderScene(void);

void main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitWindowPosition(100, 100);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(BOARD_RIGHT, BOARD_TOP);
	glutCreateWindow("Bouncing Yeah");
	Init();
	glutDisplayFunc(RenderScene);
	glutIdleFunc(RenderScene);
	glutSpecialFunc(MySpecial);
	glutMouseFunc(MyMouse);
	glutMotionFunc(MyMotion);
	glutMainLoop();
}

// Point operate funcs
Point operator+(const Point a, const Point b) {
	Point c;
	c.x = a.x + b.x;
	c.y = a.y + b.y;
	return c;
}
Point operator-(const Point a, const Point b) {
	Point c;
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	return c;
}
Point operator/(const Point a, double b) {
	Point c;
	c.x = a.x / b;
	c.y = a.y / b;
	return c;
}
Point operator/(double b, const Point a) {
	return a / b;
}
Point operator*(const Point a, double b) {
	Point c;
	c.x = a.x * b;
	c.y = a.y * b;
	return c;
}
Point operator*(double b, const Point a) {
	return a * b;
}
Point multiply(const Point target, float mul_x, float mul_y) {
	Point result;
	result = { target.x * mul_x, target.y * mul_y };
	return result;
}


void go(Point velocity) {
	gravity_a = { 0,GRAVITY_ACC };
	ball_v = ball_v + velocity;
}
void stop() {
	ball_v = { 0,0 };
	gravity_a = { 0, 0 };
	ball[0].y = BOARD_STARTLINE + MAIN_BALL_RADIUS + 1;
}

void ball_speed_adder() {
	// x관련
	ball_v.x += gravity_a.x;	// 가속도만큼 속도 증가
	ball[0].x += ball_v.x;	// 속도만큼 위치 이동
	// y관련
	ball_v.y += gravity_a.y;
	ball[0].y += ball_v.y;
}

// if collision happen, return true
namespace collision {
	bool startline(Point* ball_coor) {
		return BOARD_STARTLINE > ball_coor[0].y - MAIN_BALL_RADIUS;
	}
	bool top(Point* ball_coor) {
		return BOARD_TOP < ball_coor[0].y + MAIN_BALL_RADIUS;
	}
	bool left(Point* ball_coor) {
		return BOARD_LEFT > ball_coor[0].x - MAIN_BALL_RADIUS;
	}
	bool right(Point* ball_coor) {
		return BOARD_RIGHT < ball_coor[0].x + MAIN_BALL_RADIUS;
	}
}

void ball_collision() {
	// 상하
	if (collision::startline(ball)) {
		stop();
	}
	if (collision::top(ball)) {
		ball_v.y *= -1;
	}
	// 좌우
	if (collision::left(ball) || collision::right(ball)) {
		ball_v.x *= -1;
		if (ball_v.y > 0) {
			ball_v.y = ball_v.y + 0.3 > 0.6 ? 0.6 : ball_v.y + 0.3;
		}
	}
}

// draw
void poly_circle(double radius, int color) {
	glColor3fv(colors[color]);

	double r = radius;
	glBegin(GL_POLYGON);
	double delta = 2 * PI / CIRCLE_EDGE_NUM;
	for (int i = 0; i < CIRCLE_EDGE_NUM; ++i) {
		glVertex2f(r * cos(delta * i) + ball[0].x, r * sin(delta * i) + ball[0].y);
	}
	glEnd();
}

void axis() {
	glColor3f(0, 0, 1);
	glBegin(GL_LINES);	//x
	glVertex2f(0, 0);
	glVertex2f(BOARD_RIGHT, 0);
	glEnd();

	glColor3f(1, 0, 0);	//y
	glBegin(GL_LINES);
	glVertex2f(1, 0);
	glVertex2f(1, BOARD_TOP);

	for (int i = 50; i < BOARD_TOP; ) {
		glVertex2f(-10, i);
		glVertex2f(10, i);
		i += 50;
	}
	glEnd();
}

void keyboard_ball_control(int key) {
	switch (key) {
		// 좌, 우
	case GLUT_KEY_LEFT:
		//gravity_a.x -= 0.0001;
		ball_v.x -= 0.01;
		//ball[0].x -= 10;
		//std::cout << "left\n";
		break;
	case GLUT_KEY_RIGHT:
		//gravity_a.x += 0.0001;
		ball_v.x += 0.01;
		//ball[0].x += 10;
		//std::cout << "right\n";
		break;
		// 위, 아래
	case GLUT_KEY_DOWN:  break;
	case GLUT_KEY_UP:  break;
	default: break;
	}
}

void MySpecial(int key, int x, int y) {
	keyboard_ball_control(key);
}


void MyMouse(int button, int state, int x, int y) {
	// 클릭 하면 drag 시작
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		drag = true;
		mouse[0] = { (float)x, BOARD_TOP - (float)y };
	}
	// 클릭 때면 drag 끝내기
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		float threshold = 0.2;
		drag = false;
		Point mouse_velo = ball[0] - mouse[0];
		mouse_velo = multiply(mouse_velo,0.01,0.015);
		mouse_velo.x = mouse_velo.x > threshold ? threshold : 
			-threshold < mouse_velo.x? mouse_velo.x : -threshold;	// x속도 제한하기
		drag = false;
		Point mouse_velo = multiply(ball[0] - mouse[0],0.01,0.025);
		go(mouse_velo);
	}
}

void MyMotion(int x, int y) {
	if (drag) {
		mouse[0] = { (float)x, BOARD_TOP - (float)y };
		//printf("mouse[0] %.0lf : %.0lf\n", mouse[0].x, mouse[0].y);
	}
}

void RenderScene(void) {
	glClearColor(0.8, 0.8, 0.8, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	// value edit
	ball_speed_adder();
	ball_collision();

	// draw
	poly_circle(MAIN_BALL_RADIUS, RED);
	axis();

	std::cout << ball_v.x << " : " << ball_v.y << "\n";
	glFlush();
}