// OpenGL.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <gl/glut.h>
#include <algorithm>
#include <iostream>
#include "structdata.h"

static const int screenwidth = 800;			 //自定义屏幕宽度
static const int screenheith = 800;			//自定义屏幕高度
vector<point> points;						//点集
vector<polygon> polygons;					//多边形集

int move_x, move_y;							//鼠标坐标值（一直变化）
bool select = false;						// 判断鼠标左键是不是被按下 

//重写sort排序规则
int cmp(edge e1, edge e2) {
	return e1.node_x < e2.node_x;
}

//初始化显示窗口
void init() {
	glClearColor(1.0, 1.0, 1.0, 0.0);				//设置背景色是白色

	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0.0, screenwidth, 0.0, screenheith);//设置屏幕初始化位置
}

//获得有序边表
vector<edge_table> getSET(polygon pg, int max_y, int min_y) {
	vector<edge_table> setable(max_y - min_y + 1);								//定义边表

	edge e;
	for (int i = 0; i < pg.p.size(); i++) {
		point p1 = pg.p[i];														//获取处理当前点
		point p2 = pg.p[(i + 1) % pg.p.size()];									//当前点的下一个点
		point p0 = pg.p[(i - 1+pg.p.size()) % pg.p.size()];						//当前点的上一个点，防止数组越界
		point p3 = pg.p[(i + 2) % pg.p.size()];									//当前点的后两个点

		if (p1.y != p2.y) {														//排除平行线
			e.slope = (float)(p1.x - p2.x) / (float)(p1.y - p2.y);				//计算斜率的倒数
			if (p1.y < p2.y) {													//如果是增大
				e.node_x = p1.x;
				if (p3.y >= p2.y) {												//连续增
					e.max_y = p2.y-1;											//最高的y值减少一个像素点
				}
				else {
					e.max_y = p2.y;												//普通处理
				}
				//cout << "test" << e.node_x << " " << e.slope << endl;
				setable[p1.y - min_y].e.push_back(e);							//压入队列
			}
			else{																//如果是减少
				e.node_x = p2.x;												
				if (p0.y >= p1.y) {												//如果是连续递减
					e.max_y = p1.y - 1;											//第二条减少的边最高y值减少一个像素点
				}
				else {
					e.max_y = p1.y;								
				}
				//cout << "test" << e.node_x << " " << e.slope << endl;
				setable[p2.y - min_y].e.push_back(e);							//压入队列
			}
		}
	}
	//cout << "test" << endl;
	for (int i = 0; i < max_y-min_y+1; i++) {									//对边表的x值进行从小到大排序
		if (setable[i].e.size()>0) {
			//cout << "setable.size:" << setable[i].e.size() << endl;
			sort(setable[i].e.begin(),setable[i].e.end(),cmp);
		}
	}

	return setable;																//返回边表
}

/*画点函数*/
void setPixel(int x, int y) {
	glBegin(GL_POINTS);
	glVertex2i(x, y);
	glEnd();
}

//填充函数
void paint(polygon pg) {
	int max_y = -1;															//初始化最大值
	int min_y = 1000;														//初始化最小值
	/*求出坐标的最大值和最小值*/
	for (int i = 0; i < pg.p.size(); i++) {
		if (pg.p[i].y > max_y) {
			max_y = pg.p[i].y;
		}
		if (pg.p[i].y < min_y) {
			min_y = pg.p[i].y;
		}
	}

	//get the active edge table and tht sort edge table
	vector<edge_table> sort_edge_table(max_y - min_y + 1);					//定义有序边表
	sort_edge_table = getSET(pg, max_y, min_y);								//创建有序边表

	vector<edge> active_edge_table;											//创建活化边表
	//cout << "test" << sort_edge_table[min_y].e[0].max_y << endl;
	for (int i = min_y; i <= max_y; i++) {
		//插入排序插入新边表
		if (sort_edge_table[i - min_y].e.size() > 0) {
			int n = sort_edge_table[i - min_y].e.size();
			//cout << n << endl;
			for (int k = 0; k < n; k++) {
				active_edge_table.push_back(sort_edge_table[i - min_y].e[k]);
			}
			sort(active_edge_table.begin(), active_edge_table.end(), cmp);
		}
		//填充
		int maxaet = active_edge_table.size();
		for (int k = 1; k < maxaet; k+=2) {
			for (int j = (int)active_edge_table[k - 1].node_x; j <= (int)active_edge_table[k].node_x; j++) {
				//cout << active_edge_table[k - 1].node_x << " " << active_edge_table[k].node_x << endl;
				setPixel(j, i);
			}
			//k++;
		}

		//删除旧边
		for (int j = 0; j < maxaet; j++) {
			if ((int)active_edge_table[j].max_y <= i) {
				vector<edge>::iterator iter = active_edge_table.begin() + j;
				active_edge_table.erase(iter);
				maxaet = active_edge_table.size();							//更新活化边表最大值
				j--;														//这是一个我找了一周的bug，否则的话就会出现条纹
			}
		}

		maxaet = active_edge_table.size();
		//更新x值
		for (int j = 0; j < maxaet; j++) {
			active_edge_table[j].node_x += active_edge_table[j].slope;		//x增
		}
		sort(active_edge_table.begin(), active_edge_table.end(), cmp);		//重新排序
	}
}

//画线
void lineSegment(){
	glClear(GL_COLOR_BUFFER_BIT);											//清屏
	glColor3f(0.0, 0.4, 0.2);												//画线设置绿色

	int j = points.size() - 1;

	if (!polygons.empty()) {
		int i = 0, j = 0;
		for (i = 0; i < polygons.size(); i++) {
			for (j = 0; j < polygons[i].p.size()-1; j++) {
				glBegin(GL_LINES);
					glVertex2i(polygons[i].p[j].x, polygons[i].p[j].y);		//画线
					glVertex2i(polygons[i].p[j+1].x, polygons[i].p[j+1].y);
				glEnd();
			}
			/*防止越界*/
			glBegin(GL_LINES);
				glVertex2i(polygons[i].p[j].x, polygons[i].p[j].y);			//画线
				glVertex2i(polygons[i].p[0].x, polygons[i].p[0].y);
			glEnd();
			paint(polygons[i]);
		}
	}

	//画当前还未完成的多边形
	for (int i = 0; i < j; i++) {
		glBegin(GL_LINES);
			glVertex2i(points[i].x, points[i].y);
			glVertex2i(points[i+1].x, points[i+1].y);
		glEnd();
	}

	//动态画线
	if (!points.empty()) {
		glBegin(GL_LINES);
			glVertex2i(points[j].x, points[j].y); 
			glVertex2i(move_x, move_y);
		glEnd();
	}

	/*当用户按下鼠标左键*/
	if (select) {
		select = false;
		if (!points.empty()) {											//如果点集非空则画线
			glBegin(GL_LINES);
				glVertex2i(points[j].x, points[j].y); 
				glVertex2i(points[0].x, points[0].y);
			glEnd();

			polygon pg;													//封闭多边形并加入多边形队列
			for (int i = 0; i < points.size(); i++) {
				pg.p.push_back(points[i]);
			}
			polygons.push_back(pg);
		}
		if (points.size() <= 1) {										//如果只有一个点就按了鼠标左键，则清空图像
			glClear(GL_COLOR_BUFFER_BIT);
			polygons.clear();
		}
		points.clear();													//清空点集
	}

	glFlush();//清除所有的缓存
}

//监控鼠标按键
void myMouse(int button, int state, int x, int y) {
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {
		point p;
		p.x = x;
		p.y = screenheith - y;
		points.push_back(p);								//将点加入队列
		glutPostRedisplay();								//重置窗口 
	}
	else if (state == GLUT_DOWN && button == GLUT_RIGHT_BUTTON) {
		select = true;
		glutPostRedisplay();								//封闭多边形 
	}
}

// 动态获取鼠标座标值
void myPassiveMotion(int x, int y) {
	move_x = x;
	move_y = screenheith - y;
	glutPostRedisplay();
}

//主函数
int main(int argc, char** argv){
	glutInit(&argc, argv);							 //初始化 GLUT
	glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB);		//设置显示模式
	glutInitWindowPosition(50, 100);				//设置窗口在左上角的哪一个位置显示
	glutInitWindowSize(screenwidth, screenheith);	//设置窗口的大小
	glutCreateWindow("啦啦啦啦");

	init();
	glutMouseFunc(myMouse);
	glutDisplayFunc(lineSegment);					//设置显示窗口
	glutPassiveMotionFunc(myPassiveMotion);
	glutMainLoop();									//循环显示
    return 0;
}
