#include <windows.h>
#include <gl/gl.h>
#include <gl/glut.h> // (or others, depending on the system in use)
#include <math.h>
#include <iostream>
#include <vector>
#include <string>
using namespace std;

#define	CIRCLE_EDGE_NUM 30
#define MAIN_BALL_RADIUS 10

#define BOARD_LEFT		0
#define BOARD_RIGHT		600
#define BOARD_BOTTOM	0
#define BOARD_TOP		600
#define BOARD_STARTLINE	50 - MAIN_BALL_RADIUS 

#define BRICK_WIDTH	30
#define BRICK_HEIGHT 8
#define BRICK_ROW 5
#define BRICK_COL 4
#define BRICK_NUMS BRICK_ROW * BRICK_COL

#define	PI		3.1415926
#define GRAVITY_ACC	-0.001

#define RED 0
#define GREEN 1
#define YELLOW 2
#define BLUE 3
#define WHITE 4
#define MAGENTA 5

struct Point {
	GLfloat x = 0;
	GLfloat y = 0;
};
struct CollisionObj {
	Point coor;
	bool collsion;
};
CollisionObj brick[20];

double t1, t2;
bool drag = false;	// 그리기 시작 : true, 그리기 종료 : false
int brick_remain = BRICK_NUMS;
int try_score = 0;
int game_mode = 0; // 0 : 게임 시작 전, 1 : 게임 중, 3: 게임 클리어
int menu_toggle = 0;

Point ball_v = { 0, 0 };	// 초기 공의 속도 정의하기
Point gravity_a = { 0, GRAVITY_ACC }; // 가속도 공식
Point ball[1] = { BOARD_RIGHT / 2 , BOARD_STARTLINE };	// 공의 좌표
Point mouse[1] = { 0,0 };	// 마우스 좌표
Point collision_vec;	// 충돌 벡터

// Point operate funcs
Point operator+(const Point a, const Point b);
Point operator-(const Point a, const Point b);
Point operator/(const Point a, double b);
Point operator*(const Point a, double b);
Point operator*(double b, const Point a);
float operator*(const Point a, const Point b);
std::ostream& operator << (std::ostream& out, const Point& point);
Point multiply(const Point target, float mul_x, float mul_y);
Point seg_contact(Point start, Point end, double t1, double t2);
double norm(Point vec);

//==================func==================//
void Init() {
	gluOrtho2D(0, BOARD_RIGHT, 0, BOARD_TOP);
	for (int row = 0; row < BRICK_ROW; row++) {
		for (int col = 0; col < BRICK_COL; col++) {
			brick[row * BRICK_COL + col].coor = { 200.0f + col * 70, 400.0f - row * 40 };
			brick[row * BRICK_COL + col].collsion = false;
		}
	}
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
	bool line(Point* ball, Point start, Point end);
}

// draw
void poly_circle(double radius, int x, int y); // 동그라미 그리는 함수
void poly_circle_board(double radius, int x, int y);
void poly_brick(CollisionObj);
void axis();
void print_ball_info();
void print_info(Point p);
void print(string txt, int num, int x, int y, void* font);

// callback func
void keyboard_ball_control(int key);
void MySpecial(int key, int x, int y);
void MyMouse(int button, int state, int x, int y);
void MyMotion(int x, int y);
void MyKeyboard(unsigned char key, int x, int y);

// display result
void RenderScene(void);

//#####################################################//
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
	glutKeyboardFunc(MyKeyboard);
	glutMouseFunc(MyMouse);
	glutMotionFunc(MyMotion);
	glutMainLoop();
}//#####################################################//

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
Point operator*(const Point a, double b) {
	Point c;
	c.x = a.x * b;
	c.y = a.y * b;
	return c;
}
Point operator*(double b, const Point a) {
	return a * b;
}
float operator*(const Point a, const Point b) {
	float c = 0;
	c += a.x * b.x;
	c += a.y * b.y;
	return c;
}
std::ostream& operator << (std::ostream& out, const Point& point) {
	out << "( " << point.x << ", " << point.y << " )";
	return out;
}
Point multiply(const Point target, float mul_x, float mul_y) {
	Point result;
	result = { target.x * mul_x, target.y * mul_y };
	return result;
}
float dot(const vector<float>& vec1, const vector<float>& vec2) {
	float dot = 0.0f;
	for (size_t i = 0; i < vec1.size(); ++i) {
		dot += vec1[i] * vec2[i];
	}
	return dot;
}
Point seg_contact(Point start, Point end, double t1, double t2) {
	Point sol;
	if (t1 <= 0 || t1 >= 1) {	// t1은 충돌이 아니라면
		sol = start + (t2 * (end - start));
	}
	else if (t2 <= 0 || t2 >= 1) {	// t2은 충돌이 아니라면
		sol = start + (t1 * (end - start));
	}
	else {
		// 충돌시에 공의 원점과 가장 가까운 점을 구하는 함수
		Point sol1 = start + (t1 * (end - start));
		Point sol2 = start + (t2 * (end - start));
		//cout << t1 << " : " << t2 << '\n';
		sol = (sol1 + sol2) / 2;
	}
	return sol;
}
double norm(Point vec) {
	return sqrt(vec.x * vec.x + vec.y * vec.y);
}

void go(Point velocity) {
	gravity_a = { 0,GRAVITY_ACC };
	ball_v = ball_v + velocity;
}
void stop() {
	ball_v = { 0,0 };
	gravity_a = { 0, 0 };
	ball[0].y = BOARD_STARTLINE + MAIN_BALL_RADIUS;
	if (brick_remain == 0) {
		game_mode = 2;
	}
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
	bool line(Point* C, Point A, Point B) {
		Point radius = { MAIN_BALL_RADIUS, MAIN_BALL_RADIUS };
		float a = (B - A) * (B - A);
		float b = 2.0f * ((B - A) * (A - C[0]));
		float c = (A - C[0]) * (A - C[0]) - radius * radius;
		float dsc = (b * b) - (4.0f * a * c);
		//std::cout << dsc << '\n';

		// 직선과 출동하는지 확인
		if (dsc <= 0) {
			return false;
		}
		// 직선과 충돌한다면 선분과 출동하는지 확인
		else {
			t1 = (-b + sqrt(dsc)) / (2.0f * a);	// +-에서 +부분
			t2 = (-b - sqrt(dsc)) / (2.0f * a); // +-에서 -부분

			//std::cout << t1 << " : " << t2 << "\n";
			if (t1 >= 0 && t1 <= 1) {	// 첫 해가 원과 선분에 닿는가?
				return true;
			}
			else if (t2 >= 0 && t2 <= 1) {	// 두 번째 해가 선분과 닿는가?
				return true;
			}
			else {	// t1, t2가 0~1사이에 없다면 선분과 충돌하지 않는다.
				return false;
			}
		}
	}
}

namespace react {
	bool line(Point* ball, Point start, Point end) {
		if (collision::line(ball, start, end)) {
			Point _nearest = seg_contact(start, end, t1, t2);
			collision_vec = ball[0] - _nearest;
			//cout << nearest << '\n';
			collision_vec = collision_vec / norm(collision_vec);	// 정규화된 법선 벡터
			float normal_size = -(collision_vec * ball_v);	// 법선벡터 방향으로 곱해줄 벡터의 크기
			//cout << ball[0] << " : " << collision_vec << " : " << normal_size << "\n";
			Point normal_vec = collision_vec * normal_size * 2;	// 법선벡터 방향으로 더해줄 벡터
			ball_v = ball_v + normal_vec;
			return true;
		}
		else return false;
	}
}

// draw
void poly_circle(double radius, int x, int y) {
	glColor3f(1, 0, 0);

	double r = radius;
	glBegin(GL_POLYGON);
	double delta = 2 * PI / CIRCLE_EDGE_NUM;
	for (int i = 0; i < CIRCLE_EDGE_NUM; ++i) {
		glVertex2f(r * cos(delta * i) + x, r * sin(delta * i) + y);
	}
	glEnd();
}

void poly_circle_board(double radius, int x, int y) {
	Point bottom_start;
	Point bottom_end;
	// 동그라미
	glColor3f(0, 0, 0);
	double r = radius;
	glBegin(GL_LINES);
	double delta = 2 * PI / CIRCLE_EDGE_NUM;
	for (int i = 0; i < CIRCLE_EDGE_NUM; ++i) {
		Point start = { r * cos(delta * i) + x, r * sin(delta * i) + y };
		Point end = { r * cos(delta * (i + 1)) + x, r * sin(delta * (i + 1)) + y };
		if (i >= 19 && i <= 25) {
			if (i == 19) bottom_start = start;
			if (i == 25) bottom_end = end;
			continue;
		}
		/*if (i == 0) glColor3f(1, 1, 1);	//원 그리는 방향 확인을 위한 코드
		else if (i == 2) glColor3f(1, 1, 1);
		else glColor3f(0, 0, 0);*/
		glVertex2f(start.x, start.y);
		glVertex2f(end.x, end.y);
		react::line(ball, start, end);
	}
	glEnd();
	// 바닥 네모
	glBegin(GL_LINES);
	glVertex2f(bottom_start.x, bottom_start.y);
	glVertex2f(bottom_start.x, BOARD_STARTLINE);
	react::line(ball, bottom_start, { bottom_start.x, BOARD_STARTLINE });
	glVertex2f(bottom_start.x, BOARD_STARTLINE);
	glVertex2f(bottom_end.x, BOARD_STARTLINE);
	if (collision::line(ball, { bottom_start.x, BOARD_STARTLINE - 10 }, { bottom_end.x, BOARD_STARTLINE - 10 })) {
		stop();
	}
	glVertex2f(bottom_end.x, BOARD_STARTLINE);
	glVertex2f(bottom_end.x, bottom_end.y);
	react::line(ball, { bottom_end.x, BOARD_STARTLINE }, bottom_end);
	glEnd();


}

void poly_brick() {
	glColor3f(0.5, 0.3, 0.1);
	for (int i = 0; i < BRICK_COL * BRICK_ROW; ++i) {
		if (brick[i].collsion == false) {
			glBegin(GL_POLYGON);
			int a1 = brick[i].coor.x - BRICK_WIDTH, a2 = brick[i].coor.y + BRICK_HEIGHT;
			int b1 = brick[i].coor.x - BRICK_WIDTH, b2 = brick[i].coor.y - BRICK_HEIGHT;
			int c1 = brick[i].coor.x + BRICK_WIDTH, c2 = brick[i].coor.y - BRICK_HEIGHT;
			int d1 = brick[i].coor.x + BRICK_WIDTH, d2 = brick[i].coor.y + BRICK_HEIGHT;
			glVertex2f(a1, a2);
			glVertex2f(b1, b2);
			glVertex2f(c1, c2);
			glVertex2f(d1, d2);
			if (react::line(ball, { float(a1), float(a2) }, { float(b1), float(b2) })) {
				brick[i].collsion = true;
				brick_remain--;
			}
			else if (react::line(ball, { float(b1), float(b2) }, { float(c1), float(c2) })) {
				brick[i].collsion = true;
				brick_remain--;
			}
			else if (react::line(ball, { float(c1), float(c2) }, { float(d1), float(d2) })) {
				brick[i].collsion = true;
				brick_remain--;
			}
			else if (react::line(ball, { float(d1), float(d2) }, { float(a1), float(a2) })) {
				brick[i].collsion = true;
				brick_remain--;
			}
			glEnd();
		}
	}
}

void axis() {
	glColor3f(0, 0, 1);	//x
	glBegin(GL_LINES);
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

// 우하단에 x,y속도와 공의 좌표 출력
void print_ball_info() {
	glColor3f(0, 0, 0);
	glRasterPos2f(10, 10);
	string variableStr = "speed : " + to_string(ball_v.x) + ", " + to_string(ball_v.y) + "  coor : " + to_string(ball[0].x) + ", " + to_string(ball[0].y) + "\n";
	for (char c : variableStr) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
	}
}
void print_info(Point p) {
	glColor3f(0, 0, 0);
	glRasterPos2f(10, 30);
	string variableStr = "vec : " + to_string(p.x) + ", " + to_string(p.y);
	for (char c : variableStr) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
	}
}
void print(string txt, int num, int x, int y, void* font = GLUT_BITMAP_HELVETICA_12) {
	glColor3f(0, 0, 0);
	glRasterPos2f(x, y);
	string variableStr = txt + to_string(num);
	for (char c : variableStr) {
		glutBitmapCharacter(font, c);
	}
}
void print_string(string txt, int x, int y, void* font) {
	glRasterPos2f(x, y);
	string variableStr = txt;
	for (char c : variableStr) {
		glutBitmapCharacter(font, c);
	}
}

// event
void keyboard_ball_control(int key) {
	switch (key) {
	case GLUT_KEY_UP:
		menu_toggle++;
		break;
	case GLUT_KEY_DOWN:
		menu_toggle++;
		break;
	default: break;
	}
}
void MyKeyboard(unsigned char key, int x, int y) {
	if (key == 13) {
		if (game_mode == 0) {
			if (menu_toggle%2 == 0) {
				game_mode = 1;
				menu_toggle = 0;
			}
			else if (menu_toggle%2 == 1) {
				exit(0);
			}
		}
		if (game_mode == 2) {
			exit(0);
		}
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
		mouse_velo = multiply(mouse_velo, 0.01, 0.02);
		mouse_velo.x = mouse_velo.x > threshold ? threshold :
			-threshold < mouse_velo.x ? mouse_velo.x : -threshold; // x속도 제한하기
		drag = false;
		try_score++;
		go(mouse_velo);
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
		ball[0].x = (float)x;
		ball[0].y = BOARD_TOP - (float)y;
	}
}

void MyMotion(int x, int y) {
	if (drag) {
		mouse[0] = { (float)x, BOARD_TOP - (float)y };
		//printf("mouse[0] %.0lf : %.0lf\n", mouse[0].x, mouse[0].y);
	}
}

Point nearest;
//======================================================//
void RenderScene(void) {
	if (game_mode == 0) {
		glClearColor(0.8, 0.8, 0.8, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		glColor3f(0, 0, 0);
		print_string("Break Out", 230, 500, GLUT_BITMAP_TIMES_ROMAN_24);
		if (menu_toggle%2 == 0) glColor3f(1, 0, 0);
		else glColor3f(0, 0, 0);
		print_string("start", 260, 230, GLUT_BITMAP_TIMES_ROMAN_24);
		if (menu_toggle%2 == 1) glColor3f(1, 0, 0);
		else glColor3f(0, 0, 0);
		print_string("exit", 262, 200, GLUT_BITMAP_TIMES_ROMAN_24);
		glFlush();
	}
	else if (game_mode == 1) {
		glClearColor(0.8, 0.8, 0.8, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		// value edit
		ball_speed_adder();

		// draw
		poly_circle(MAIN_BALL_RADIUS, ball[0].x, ball[0].y);
		poly_circle_board(BOARD_TOP < BOARD_RIGHT ? BOARD_TOP / 2 - 30 : BOARD_RIGHT / 2 - 30, BOARD_RIGHT / 2, BOARD_TOP / 2);
		poly_brick();
		//print_ball_info();
		print("bricks : ", brick_remain, 10, 50);
		print("try score : ", try_score, 10, 60);
		axis();

		glFlush();
	}
	// 게임 클리어
	else if (game_mode == 2) {
		glClearColor(0.8, 0.8, 0.8, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		print_string("GAME CLEAR", 220, 500, GLUT_BITMAP_TIMES_ROMAN_24);
		print("click score : ", try_score, 220, 400, GLUT_BITMAP_TIMES_ROMAN_24);
		print_string("press enter to exit", 200, 200, GLUT_BITMAP_TIMES_ROMAN_24);
		glFlush();
	}
} 