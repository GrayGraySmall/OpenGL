// OpenGL.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <gl/glut.h>
#include <algorithm>
#include <iostream>
#include "structdata.h"

static const int screenwidth = 800;			 //�Զ�����Ļ���
static const int screenheith = 800;			//�Զ�����Ļ�߶�
vector<point> points;						//�㼯
vector<polygon> polygons;					//����μ�

int move_x, move_y;							//�������ֵ��һֱ�仯��
bool select = false;						// �ж��������ǲ��Ǳ����� 

//��дsort�������
int cmp(edge e1, edge e2) {
	return e1.node_x < e2.node_x;
}

//��ʼ����ʾ����
void init() {
	glClearColor(1.0, 1.0, 1.0, 0.0);				//���ñ���ɫ�ǰ�ɫ

	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0.0, screenwidth, 0.0, screenheith);//������Ļ��ʼ��λ��
}

//�������߱�
vector<edge_table> getSET(polygon pg, int max_y, int min_y) {
	vector<edge_table> setable(max_y - min_y + 1);								//����߱�

	edge e;
	for (int i = 0; i < pg.p.size(); i++) {
		point p1 = pg.p[i];														//��ȡ����ǰ��
		point p2 = pg.p[(i + 1) % pg.p.size()];									//��ǰ�����һ����
		point p0 = pg.p[(i - 1+pg.p.size()) % pg.p.size()];						//��ǰ�����һ���㣬��ֹ����Խ��
		point p3 = pg.p[(i + 2) % pg.p.size()];									//��ǰ��ĺ�������

		if (p1.y != p2.y) {														//�ų�ƽ����
			e.slope = (float)(p1.x - p2.x) / (float)(p1.y - p2.y);				//����б�ʵĵ���
			if (p1.y < p2.y) {													//���������
				e.node_x = p1.x;
				if (p3.y >= p2.y) {												//������
					e.max_y = p2.y-1;											//��ߵ�yֵ����һ�����ص�
				}
				else {
					e.max_y = p2.y;												//��ͨ����
				}
				//cout << "test" << e.node_x << " " << e.slope << endl;
				setable[p1.y - min_y].e.push_back(e);							//ѹ�����
			}
			else{																//����Ǽ���
				e.node_x = p2.x;												
				if (p0.y >= p1.y) {												//����������ݼ�
					e.max_y = p1.y - 1;											//�ڶ������ٵı����yֵ����һ�����ص�
				}
				else {
					e.max_y = p1.y;								
				}
				//cout << "test" << e.node_x << " " << e.slope << endl;
				setable[p2.y - min_y].e.push_back(e);							//ѹ�����
			}
		}
	}
	//cout << "test" << endl;
	for (int i = 0; i < max_y-min_y+1; i++) {									//�Ա߱��xֵ���д�С��������
		if (setable[i].e.size()>0) {
			//cout << "setable.size:" << setable[i].e.size() << endl;
			sort(setable[i].e.begin(),setable[i].e.end(),cmp);
		}
	}

	return setable;																//���ر߱�
}

/*���㺯��*/
void setPixel(int x, int y) {
	glBegin(GL_POINTS);
	glVertex2i(x, y);
	glEnd();
}

//��亯��
void paint(polygon pg) {
	int max_y = -1;															//��ʼ�����ֵ
	int min_y = 1000;														//��ʼ����Сֵ
	/*�����������ֵ����Сֵ*/
	for (int i = 0; i < pg.p.size(); i++) {
		if (pg.p[i].y > max_y) {
			max_y = pg.p[i].y;
		}
		if (pg.p[i].y < min_y) {
			min_y = pg.p[i].y;
		}
	}

	//get the active edge table and tht sort edge table
	vector<edge_table> sort_edge_table(max_y - min_y + 1);					//��������߱�
	sort_edge_table = getSET(pg, max_y, min_y);								//��������߱�

	vector<edge> active_edge_table;											//������߱�
	//cout << "test" << sort_edge_table[min_y].e[0].max_y << endl;
	for (int i = min_y; i <= max_y; i++) {
		//������������±߱�
		if (sort_edge_table[i - min_y].e.size() > 0) {
			int n = sort_edge_table[i - min_y].e.size();
			//cout << n << endl;
			for (int k = 0; k < n; k++) {
				active_edge_table.push_back(sort_edge_table[i - min_y].e[k]);
			}
			sort(active_edge_table.begin(), active_edge_table.end(), cmp);
		}
		//���
		int maxaet = active_edge_table.size();
		for (int k = 1; k < maxaet; k+=2) {
			for (int j = (int)active_edge_table[k - 1].node_x; j <= (int)active_edge_table[k].node_x; j++) {
				//cout << active_edge_table[k - 1].node_x << " " << active_edge_table[k].node_x << endl;
				setPixel(j, i);
			}
			//k++;
		}

		//ɾ���ɱ�
		for (int j = 0; j < maxaet; j++) {
			if ((int)active_edge_table[j].max_y <= i) {
				vector<edge>::iterator iter = active_edge_table.begin() + j;
				active_edge_table.erase(iter);
				maxaet = active_edge_table.size();							//���»�߱����ֵ
				j--;														//����һ��������һ�ܵ�bug������Ļ��ͻ��������
			}
		}

		maxaet = active_edge_table.size();
		//����xֵ
		for (int j = 0; j < maxaet; j++) {
			active_edge_table[j].node_x += active_edge_table[j].slope;		//x��
		}
		sort(active_edge_table.begin(), active_edge_table.end(), cmp);		//��������
	}
}

//����
void lineSegment(){
	glClear(GL_COLOR_BUFFER_BIT);											//����
	glColor3f(0.0, 0.4, 0.2);												//����������ɫ

	int j = points.size() - 1;

	if (!polygons.empty()) {
		int i = 0, j = 0;
		for (i = 0; i < polygons.size(); i++) {
			for (j = 0; j < polygons[i].p.size()-1; j++) {
				glBegin(GL_LINES);
					glVertex2i(polygons[i].p[j].x, polygons[i].p[j].y);		//����
					glVertex2i(polygons[i].p[j+1].x, polygons[i].p[j+1].y);
				glEnd();
			}
			/*��ֹԽ��*/
			glBegin(GL_LINES);
				glVertex2i(polygons[i].p[j].x, polygons[i].p[j].y);			//����
				glVertex2i(polygons[i].p[0].x, polygons[i].p[0].y);
			glEnd();
			paint(polygons[i]);
		}
	}

	//����ǰ��δ��ɵĶ����
	for (int i = 0; i < j; i++) {
		glBegin(GL_LINES);
			glVertex2i(points[i].x, points[i].y);
			glVertex2i(points[i+1].x, points[i+1].y);
		glEnd();
	}

	//��̬����
	if (!points.empty()) {
		glBegin(GL_LINES);
			glVertex2i(points[j].x, points[j].y); 
			glVertex2i(move_x, move_y);
		glEnd();
	}

	/*���û�����������*/
	if (select) {
		select = false;
		if (!points.empty()) {											//����㼯�ǿ�����
			glBegin(GL_LINES);
				glVertex2i(points[j].x, points[j].y); 
				glVertex2i(points[0].x, points[0].y);
			glEnd();

			polygon pg;													//��ն���β��������ζ���
			for (int i = 0; i < points.size(); i++) {
				pg.p.push_back(points[i]);
			}
			polygons.push_back(pg);
		}
		if (points.size() <= 1) {										//���ֻ��һ����Ͱ����������������ͼ��
			glClear(GL_COLOR_BUFFER_BIT);
			polygons.clear();
		}
		points.clear();													//��յ㼯
	}

	glFlush();//������еĻ���
}

//�����갴��
void myMouse(int button, int state, int x, int y) {
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {
		point p;
		p.x = x;
		p.y = screenheith - y;
		points.push_back(p);								//����������
		glutPostRedisplay();								//���ô��� 
	}
	else if (state == GLUT_DOWN && button == GLUT_RIGHT_BUTTON) {
		select = true;
		glutPostRedisplay();								//��ն���� 
	}
}

// ��̬��ȡ�������ֵ
void myPassiveMotion(int x, int y) {
	move_x = x;
	move_y = screenheith - y;
	glutPostRedisplay();
}

//������
int main(int argc, char** argv){
	glutInit(&argc, argv);							 //��ʼ�� GLUT
	glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB);		//������ʾģʽ
	glutInitWindowPosition(50, 100);				//���ô��������Ͻǵ���һ��λ����ʾ
	glutInitWindowSize(screenwidth, screenheith);	//���ô��ڵĴ�С
	glutCreateWindow("��������");

	init();
	glutMouseFunc(myMouse);
	glutDisplayFunc(lineSegment);					//������ʾ����
	glutPassiveMotionFunc(myPassiveMotion);
	glutMainLoop();									//ѭ����ʾ
    return 0;
}
