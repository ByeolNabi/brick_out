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
#define GRAVITY_ACC	-0.0005

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

// ÃÊ±â °øÀÇ ¼Óµµ Á¤ÀÇÇÏ±â
Point ball_v = { 0, 0.01 };
// °¡¼Óµµ °ø½Ä
Point gravity_a = { 0, GRAVITY_ACC };
// °øÀÇ ÁÂÇ¥
Point ball[1] = { 100, 400 };
// ¸¶¿ì½º ÁÂÇ¥
Point mouse[1] = {};

// »ö µ¥ÀÌÅÍ
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
void go();
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
	glutMainLoop();
}

void go() {

}
void stop() {
	ball_v = { 0,0 };
	gravity_a = { 0, 0 };
	ball[0].y = BOARD_STARTLINE + MAIN_BALL_RADIUS + 1;
}

void ball_speed_adder() {
	// x°ü·Ã
	ball_v.x += gravity_a.x;	// °¡¼Óµµ¸¸Å­ ¼Óµµ Áõ°¡
	ball[0].x += ball_v.x;	// ¼Óµµ¸¸Å­ À§Ä¡ ÀÌµ¿
	// y°ü·Ã
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
	// »óÇÏ
	if (collision::startline(ball)) {
		stop();
	}
	if (collision::top(ball)) {
		ball_v.y *= -1;
	}
	// ÁÂ¿ì
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
		// ÁÂ, ¿ì
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
		// À§, ¾Æ·¡
	case GLUT_KEY_DOWN:  break;
	case GLUT_KEY_UP:  break;
	default: break;
	}
}

void MySpecial(int key, int x, int y) {
	keyboard_ball_control(key);
}

<<<<<<< Updated upstream
=======
void MyMouse(int button, int state, int x, int y) {
	// í´ë¦­ í•˜ë©´ drag ì‹œìž‘
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		drag = true;
		mouse[0] = { (float)x, BOARD_TOP - (float)y };
	}
	// í´ë¦­ ë•Œë©´ drag ëë‚´ê¸°
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		float threshold = 0.2;
		drag = false;
		Point mouse_velo = ball[0] - mouse[0];
		mouse_velo = multiply(mouse_velo,0.01,0.015);
		mouse_velo.x = mouse_velo.x > threshold ? threshold : 
			-threshold < mouse_velo.x? mouse_velo.x : -threshold;	// xì†ë„ ì œì•ˆí•˜ê¸°
		go(mouse_velo);
	}
}

void MyMotion(int x, int y) {
	if (drag) {
		mouse[0] = { (float)x, BOARD_TOP - (float)y };
		//printf("mouse[0] %.0lf : %.0lf\n", mouse[0].x, mouse[0].y);
	}
}

>>>>>>> Stashed changes
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