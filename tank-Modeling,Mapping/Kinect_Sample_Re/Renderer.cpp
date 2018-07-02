#include "Renderer.h"

void draw_center(void)
{
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f); /* R */
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.2f, 0.0f, 0.0f);
	glEnd();
	glRasterPos3f(0.2f, 0.0f, 0.0f);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'x');

	glBegin(GL_LINES);
	glColor3f(0.0f, 1.0f, 0.0f); /* G */
	glVertex3f(0.0f, 0.2f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glEnd();
	glRasterPos3f(0.0f, 0.2f, 0.0f);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'y');

	glBegin(GL_LINES);
	glColor3f(0.0f, 0.0f, 1.0f); /* B */
	glVertex3f(0.0f, 0.0f, -0.2f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glEnd();
	glRasterPos3f(0.0f, 0.0f, -0.2f);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'z');
}

void idle() {
	static GLuint previousClock = glutGet(GLUT_ELAPSED_TIME);
	static GLuint currentClock = glutGet(GLUT_ELAPSED_TIME);
	static GLfloat deltaT;

	currentClock = glutGet(GLUT_ELAPSED_TIME);
	deltaT = currentClock - previousClock;
	if (deltaT < 1000.0 / 20.0) { return; }
	else { previousClock = currentClock; }

	//char buff[256];
	//sprintf_s(buff, "Frame Rate = %f", 1000.0 / deltaT);
	//frameRate = buff;

	glutPostRedisplay();
}

void close()
{
	glDeleteTextures(1, &dispBindIndex);
	glutLeaveMainLoop();
	CloseHandle(hMutex);
}

void add_quats(float q1[4], float q2[4], float dest[4])
{
	static int count = 0;
	float t1[4], t2[4], t3[4];
	float tf[4];

	vcopy(q1, t1);
	vscale(t1, q2[3]);

	vcopy(q2, t2);
	vscale(t2, q1[3]);

	vcross(q2, q1, t3);
	vadd(t1, t2, tf);
	vadd(t3, tf, tf);
	tf[3] = q1[3] * q2[3] - vdot(q1, q2);

	dest[0] = tf[0];
	dest[1] = tf[1];
	dest[2] = tf[2];
	dest[3] = tf[3];

	if (++count > RENORMCOUNT) {
		count = 0;
		normalize_quat(dest);
	}
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(58, (double)width / height, 0.1, 100);
	glMatrixMode(GL_MODELVIEW);
}

void motion(int x, int y)
{
	GLfloat spin_quat[4];
	float gain;
	gain = 2.0; /* trackball gain */

	if (drag_state == GLUT_DOWN)
	{
		if (button_state == GLUT_LEFT_BUTTON)
		{
			trackball(spin_quat,
				(gain * rot_x - 500) / 500,
				(500 - gain * rot_y) / 500,
				(gain * x - 500) / 500,
				(500 - gain * y) / 500);
			add_quats(spin_quat, quat, quat);
		}
		else if (button_state == GLUT_RIGHT_BUTTON)
		{
			t[0] -= (((float)trans_x - x) / 500);
			t[1] += (((float)trans_y - y) / 500);
		}
		else if (button_state == GLUT_MIDDLE_BUTTON)
			t[2] -= (((float)trans_z - y) / 500 * 4);
		else if (button_state == 3 || button_state == 4) // scroll
		{

		}
		//glutPostRedisplay();
	}

	rot_x = x;
	rot_y = y;

	trans_x = x;
	trans_y = y;
	trans_z = y;
}

void mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		if (button == GLUT_LEFT_BUTTON)
		{
			rot_x = x;
			rot_y = y;

			//t[0] = t[0] + 1;


		}
		else if (button == GLUT_RIGHT_BUTTON)
		{
			trans_x = x;
			trans_y = y;
		}
		else if (button == GLUT_MIDDLE_BUTTON)
		{
			trans_z = y;
		}
		else if (button == 3 || button == 4)
		{
			const float sign = (static_cast<float>(button) - 3.5f) * 2.0f;
			t[2] -= sign * 500 * 0.00015f;
		}
	}

	drag_state = state;
	button_state = button;
}

void vzero(float* v)
{
	v[0] = 0.0f;
	v[1] = 0.0f;
	v[2] = 0.0f;
}

void vset(float* v, float x, float y, float z)
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

void vsub(const float *src1, const float *src2, float *dst)
{
	dst[0] = src1[0] - src2[0];
	dst[1] = src1[1] - src2[1];
	dst[2] = src1[2] - src2[2];
}

void vcopy(const float *v1, float *v2)
{
	register int i;
	for (i = 0; i < 3; i++)
		v2[i] = v1[i];
}

void vcross(const float *v1, const float *v2, float *cross)
{
	float temp[3];

	temp[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
	temp[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
	temp[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
	vcopy(temp, cross);
}

float vlength(const float *v)
{
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

void vscale(float *v, float div)
{
	v[0] *= div;
	v[1] *= div;
	v[2] *= div;
}

void vnormal(float *v)
{
	vscale(v, 1.0f / vlength(v));
}

float vdot(const float *v1, const float *v2)
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

void vadd(const float *src1, const float *src2, float *dst)
{
	dst[0] = src1[0] + src2[0];
	dst[1] = src1[1] + src2[1];
	dst[2] = src1[2] + src2[2];
}

void trackball(float q[4], float p1x, float p1y, float p2x, float p2y)
{
	float a[3]; /* Axis of rotation */
	float phi;  /* how much to rotate about axis */
	float p1[3], p2[3], d[3];
	float t;

	if (p1x == p2x && p1y == p2y) {
		/* Zero rotation */
		vzero(q);
		q[3] = 1.0;
		return;
	}

	/*
	 * First, figure out z-coordinates for projection of P1 and P2 to
	 * deformed sphere
	 */
	vset(p1, p1x, p1y, tb_project_to_sphere(TRACKBALLSIZE, p1x, p1y));
	vset(p2, p2x, p2y, tb_project_to_sphere(TRACKBALLSIZE, p2x, p2y));

	/*
	 *  Now, we want the cross product of P1 and P2
	 */
	vcross(p2, p1, a);

	/*
	 *  Figure out how much to rotate around that axis.
	 */
	vsub(p1, p2, d);
	t = vlength(d) / (2.0f*TRACKBALLSIZE);

	/*
	 * Avoid problems with out-of-control values...
	 */
	if (t > 1.0) t = 1.0;
	if (t < -1.0) t = -1.0;
	phi = 2.0f * asin(t);

	axis_to_quat(a, phi, q);
}

void axis_to_quat(float a[3], float phi, float q[4])
{
	vnormal(a);
	vcopy(a, q);
	vscale(q, sin(phi / 2.0f));
	q[3] = cos(phi / 2.0f);
}

float tb_project_to_sphere(float r, float x, float y)
{
	float d, t, z;

	d = sqrt(x*x + y * y);
	if (d < r * 0.70710678118654752440f) {    /* Inside sphere */
		z = sqrt(r*r - d * d);
	}
	else {           /* On hyperbola */
		t = r / 1.41421356237309504880f;
		z = t * t / d;
	}
	return z;
}

void normalize_quat(float q[4])
{
	int i;
	float mag;

	mag = (q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
	for (i = 0; i < 4; i++) q[i] /= mag;
}

void build_rotmatrix(float m[4][4], float q[4])
{
	m[0][0] = 1.0f - 2.0f * (q[1] * q[1] + q[2] * q[2]);
	m[0][1] = 2.0f * (q[0] * q[1] - q[2] * q[3]);
	m[0][2] = 2.0f * (q[2] * q[0] + q[1] * q[3]);
	m[0][3] = 0.0f;

	m[1][0] = 2.0f * (q[0] * q[1] + q[2] * q[3]);
	m[1][1] = 1.0f - 2.0f * (q[2] * q[2] + q[0] * q[0]);
	m[1][2] = 2.0f * (q[1] * q[2] - q[0] * q[3]);
	m[1][3] = 0.0f;

	m[2][0] = 2.0f * (q[2] * q[0] - q[1] * q[3]);
	m[2][1] = 2.0f * (q[1] * q[2] + q[0] * q[3]);
	m[2][2] = 1.0f - 2.0f * (q[1] * q[1] + q[0] * q[0]);
	m[2][3] = 0.0f;

	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;
}

void InitializeWindow(int argc, char* argv[])
{
	// initialize glut settings
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowSize(1000 / 2, 1000 / 2);

	glutInitWindowPosition(0, 0);

	dispWindowIndex = glutCreateWindow("3D Model");

	trackball(quat, 2.0, 0.0, 0.0, 0.0);

	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutMotionFunc(motion);
	glutMouseFunc(mouse);
	glutCloseFunc(close);

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	// bind textures
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	reshape(1000, 1000);

	/*glGenTextures(1, &dispBindIndex);
	glBindTexture(GL_TEXTURE_2D, dispBindIndex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/
}


void display()
{
	// clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// set matrix
	glMatrixMode(GL_PROJECTION);
	//glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluPerspective(90, 1, 1, 200); //(60->10으로 줄이면 보이는 각도를 줄인거-> 각도만큼 보이는만큼만 맞춰서 보여주기때문에 확대된거처럼
	glTranslatef(t[0] - 1.5, t[1], t[2] - 5);
	glScalef(1, 1, 1);			//이거는 물체 사이즈가 커진거
	//gluLookAt(2.0, 6.0, 0.0, 0, 0, 0, 0, 1.0, 0);

	GLfloat m[4][4], m1[4][4];
	build_rotmatrix(m, quat);
	
	m1[0][0] = 1.0f;
	m1[0][1] = 0.0f;
	m1[0][2] = 0.0f;	//x축 shearing 원래는0.0
	m1[0][3] = 0.0f;

	m1[1][0] = 0.0f;
	m1[1][1] = 1.0f;
	m1[1][2] = 0.0f;	//y축 shearing ->모델자체를 바꿨다. 평범한거에서 프로젝션하면서 찌그러트린게아니라 이미 찌그러진상태를 프로젝션한것
	m1[1][3] = 0.0f;

	m1[2][0] = 0.0f;
	m1[2][1] = 0.0f;
	m1[2][2] = 1.0f;
	m1[2][3] = 0.0f;

	m1[3][0] = 0.0f;
	m1[3][1] = 0.0f;
	m1[3][2] = 0.0f;
	m1[3][3] = 1.0f;

	glMultMatrixf(&m[0][0]);

	glMatrixMode(GL_PROJECTION);
	//texture mapping 출력

	glTexImage2D(GL_TEXTURE_2D, 0, 3, 225, 225, 0, GL_RGB, GL_UNSIGNED_BYTE, mytexels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	//Texture Mapping된 탱크 출력
	//f_arr의 크기만큼
	for (int i = 1; i <= 45424; i++)
	{
		face f = f_arr[i];
		//정육면체 했던거처럼 1나를 4각형이루게
		int v_0 = f.p[0].v_index;
		int n_0 = f.p[0].vn_index;
		int vt_0 = f.p[0].vt_index;
		glTexCoord3f(vt_xyz[vt_0].x, vt_xyz[vt_0].y, vt_xyz[vt_0].z);
		glNormal3f(vn_xyz[n_0].x, vn_xyz[n_0].y, vn_xyz[n_0].z);
		glVertex3f(v_xyz[v_0].x, v_xyz[v_0].y, v_xyz[v_0].z);

		int v_1 = f.p[1].v_index;
		int n_1 = f.p[1].vn_index;
		int vt_1 = f.p[1].vt_index;
		glTexCoord3f(vt_xyz[vt_1].x, vt_xyz[vt_1].y, vt_xyz[vt_1].z);
		glNormal3f(vn_xyz[n_1].x, vn_xyz[n_1].y, vn_xyz[n_1].z);
		glVertex3f(v_xyz[v_1].x, v_xyz[v_1].y, v_xyz[v_1].z);

		int v_2 = f.p[2].v_index;
		int n_2 = f.p[2].vn_index;
		int vt_2 = f.p[2].vt_index;
		glTexCoord3f(vt_xyz[vt_2].x, vt_xyz[vt_2].y, vt_xyz[vt_2].z);
		glNormal3f(vn_xyz[n_2].x, vn_xyz[n_2].y, vn_xyz[n_2].z);
		glVertex3f(v_xyz[v_2].x, v_xyz[v_2].y, v_xyz[v_2].z);

		int v_3 = f.p[3].v_index;
		int n_3 = f.p[3].vn_index;
		int vt_3 = f.p[3].vt_index;
		glTexCoord3f(vt_xyz[vt_3].x, vt_xyz[vt_3].y, vt_xyz[vt_3].z);
		glNormal3f(vn_xyz[n_3].x, vn_xyz[n_3].y, vn_xyz[n_3].z);
		glVertex3f(v_xyz[v_3].x, v_xyz[v_3].y, v_xyz[v_3].z);
	}

	//Shading 모델
	//최대위치와 최소 위치를 구한다.
	float max_value = 0;
	float min_value = 10;
	for (register int j = 1; j <= 47731; j = j + 1)
	{
		if (max_value < v_xyz[j].z)
		{
			max_value = v_xyz[j].z;
		}
		if (min_value > v_xyz[j].z)
		{
			min_value = v_xyz[j].z;
		}
	}
	float dist = max_value - min_value;
	float plus = abs(min_value);
	glBegin(GL_QUADS);
	//단순 탱크 모델 출력 -> 옆에만 두게 화면 되기때문에 translation 해준다.
	for (int i = 1; i <= 45424; i++)
	{
		face f = f_arr[i];
		//정육면체 했던거처럼 1나를 4각형이루게
		int v_0 = f.p[0].v_index;
		float z = v_xyz[v_0].z + plus;
		float value = z / dist;
		glVertex3f(v_xyz[v_0].x - 3, v_xyz[v_0].y, v_xyz[v_0].z);
		glColor3f(1 - value, 1 - value, 1 - value);

		int v_1 = f.p[1].v_index;
		z = v_xyz[v_1].z + plus;
		value = z / dist;
		glVertex3f(v_xyz[v_1].x - 3, v_xyz[v_1].y, v_xyz[v_1].z);
		glColor3f(1 - value, 1 - value, 1 - value);

		int v_2 = f.p[2].v_index;
		z = v_xyz[v_2].z + plus;
		value = z / dist;
		glVertex3f(v_xyz[v_2].x - 3, v_xyz[v_2].y, v_xyz[v_2].z);
		glColor3f(1 - value, 1 - value, 1 - value);

		int v_3 = f.p[3].v_index;
		z = v_xyz[v_3].z + plus;
		value = z / dist;
		glVertex3f(v_xyz[v_3].x - 3, v_xyz[v_3].y, v_xyz[v_3].z);
		glColor3f(1 - value, 1 - value, 1 - value);
	}
	glEnd();
	glutSwapBuffers();
}

int main(int argc, char* argv[])
{
	//vertex : 3차원공간상의 점
	vertex = new Vertex[READ_SIZE];
	vertex_color = new Vertex[READ_SIZE];
	//mymesh 

	int i, j, k = 0;

	FILE* f = fopen("image.bmp", "rb");
	unsigned char info[54];
	fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header
										   // extract image height and width from header
	int width = *(int*)&info[18];
	int height = *(int*)&info[22];
	int size = 3 * width * height;
	unsigned char* data = new unsigned char[size];
	fread(data, sizeof(unsigned char), size, f);
	fclose(f);
	for (i = 0; i < width; i++)
	{
		for (j = 0; j < height; j++)
		{
			mytexels[j][i][0] = data[k * 3 + 2];
			mytexels[j][i][1] = data[k * 3 + 1];
			mytexels[j][i][2] = data[k * 3];
			k++;
		}
	}
	//tank의 v, vn, f읽어서 배열에 저장하기
	ifstream ifs("tank.obj");
	string  ch;
	if (ifs.is_open())
	{
		while (!ifs.eof())
		{
			ifs >> ch;
			//vertex인경우 x,y,z 읽기
			if (ch == "v")
			{
				double x, y, z;
				ifs >> x >> y >> z;
				v_xyz[v_index] = { x,y,z };
				v_index++;
			}
			//vt인경우 uv 읽기
			else if (ch == "vt")
			{
				double u, v;
				ifs >> u >> v;
				vt_xyz[vt_index] = { u,v };
				vt_index++;
			}
			//vn인경우 x,y,z,읽기
			else if (ch == "vn")
			{
				double x, y, z;
				ifs >> x >> y >> z;
				vn_xyz[vn_index] = { x,y,z };
				vn_index++;
			}
			//s가들어오는경우가 있다 이거 예외처리 해주자
			else if (ch == "s")
			{
				int _temp;
				ifs >> _temp;
			}
			//f인경우 네개의 point
			else if(ch == "f")
			{
				string point_0, point_1, point_2, point_3;	//string으로 읽은다음 parsing해주기
				ifs >> point_0 >> point_1 >> point_2 >> point_3;
				//face[f_index]에 p[0]의 v_index, vt_index, vn_index 넣기
				int flag = 0;
				string num = "";
				for (int i = 0; i < point_0.size(); i++)
				{
					if (point_0[i] == '/')
					{
						if (flag == 0)
						{
							f_arr[f_index].p[0].v_index = atoi(num.c_str());
							flag++;
							num = "";
						}
						else if (flag == 1)
						{
							f_arr[f_index].p[0].vt_index = atoi(num.c_str());
							flag++;
							num = "";
						}
					}
					else
					{
						num += point_0[i];
					}
				}
				f_arr[f_index].p[0].vn_index = atoi(num.c_str());

				//face[f_index]에 p[1]의 v_index, vt_index, vn_index 넣기
				flag = 0;
				num = "";
				for (int i = 0; i < point_1.size(); i++)
				{
					if (point_1[i] == '/')
					{
						if (flag == 0)
						{
							f_arr[f_index].p[1].v_index = atoi(num.c_str());
							flag++;
							num = "";
						}
						else if (flag == 1)
						{
							f_arr[f_index].p[1].vt_index = atoi(num.c_str());
							flag++;
							num = "";
						}
					}
					else
					{
						num += point_1[i];
					}
				}
				f_arr[f_index].p[1].vn_index = atoi(num.c_str());

				//face[f_index]에 p[2]의 v_index, vt_index, vn_index 넣기
				flag = 0;
				num = "";
				for (int i = 0; i < point_2.size(); i++)
				{
					if (point_2[i] == '/')
					{
						if (flag == 0)
						{
							f_arr[f_index].p[2].v_index = atoi(num.c_str());
							flag++;
							num = "";
						}
						else if (flag == 1)
						{
							f_arr[f_index].p[2].vt_index = atoi(num.c_str());
							flag++;
							num = "";
						}
					}
					else
					{
						num += point_2[i];
					}
				}
				f_arr[f_index].p[2].vn_index = atoi(num.c_str());

				//face[f_index]에 p[3]의 v_index, vt_index, vn_index 넣기
				flag = 0;
				num = "";
				for (int i = 0; i < point_3.size(); i++)
				{
					if (point_3[i] == '/')
					{
						if (flag == 0)
						{
							f_arr[f_index].p[3].v_index = atoi(num.c_str());
							flag++;
							num = "";
						}
						else if (flag == 1)
						{
							f_arr[f_index].p[3].vt_index = atoi(num.c_str());
							flag++;
							num = "";
						}
					}
					else
					{
						num += point_3[i];
					}
				}
				f_arr[f_index].p[3].vn_index = atoi(num.c_str());
				f_index++;
			}
		}
		//         47731 /          64387 /            47729 /           45872        --> v : 47730개  vt : 64386개   vn : 47728개   f : 45424개 
		cout << v_index << " " << vt_index << " " << vn_index << " " << f_index << "\n";
	}
	else
	{
		cout << "파일을 못읽었습니다.\n";
	}
	fclose(f);

	InitializeWindow(argc, argv);

	display();

	glutMainLoop();
	delete[] vertex;
	delete[] vertex_color;
	return 0;
}