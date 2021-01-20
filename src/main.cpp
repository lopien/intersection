///////////////////////////////////////////////////////////////////////////////
// main.cpp
// ========
// test plane and intersect with a line
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2016-01-20
// UPDATED: 2020-07-22
///////////////////////////////////////////////////////////////////////////////

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "Matrices.h"
#include "Plane.h"
#include "Line.h"
#include "Cylinder.h"



// GLUT CALLBACK functions
void displayCB();
void reshapeCB(int w, int h);
void timerCB(int millisec);
void idleCB();
void keyboardCB(unsigned char key, int x, int y);
void mouseCB(int button, int stat, int x, int y);
void mouseMotionCB(int x, int y);
void specialCB(int key, int x, int y);

// CALLBACK function when exit() called ///////////////////////////////////////
void exitCB();

void initGL();
int  initGLUT(int argc, char **argv);
bool initSharedMem();
void clearSharedMem();
void initLights();
void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ);
void drawString(const char *str, int x, int y, float color[4], void *font);
void drawString3D(const char *str, float pos[3], float color[4], void *font);
void showInfo();
void toOrtho();
void toPerspective();
void drawAxis(float size=10);
void drawRoom(float size=10);
void drawGrid(float size=10.0f, float step=1.0f);
void drawPlane(const Plane& p, const Vector3& color);
void drawLine(const Line& line, const Vector3& color);

// constants
const int   SCREEN_WIDTH    = 500;
const int   SCREEN_HEIGHT   = 500;
const float CAMERA_DISTANCE = 25.0f;
const float CAMERA_ANGLE_X  = 45.0f;
const float CAMERA_ANGLE_Y  = -45.0f;
const int   TEXT_WIDTH      = 8;
const int   TEXT_HEIGHT     = 13;
const float DEG2RAD         = acos(-1);

// global variables
void *font = GLUT_BITMAP_8_BY_13;
int screenWidth;
int screenHeight;
bool mouseLeftDown;
bool mouseRightDown;
float mouseX, mouseY;
float cameraAngleX;
float cameraAngleY;
float cameraX;
float cameraY;
float cameraDistance;
int drawMode = 0;
Matrix4 matrixView;
Plane plane1;
Plane plane2;
Line line;
Vector3 color1;
Vector3 color2;
Vector3 color3;
Cylinder cylinder;  // to draw aline



///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    // init global vars
    initSharedMem();

    // register exit callback
    atexit(exitCB);

    // init GLUT and GL
    initGLUT(argc, argv);
    initGL();

    // the last GLUT call (LOOP)
    // window will be shown and display callback is triggered by events
    // NOTE: this call never return main().
    glutMainLoop(); /* Start GLUT event-processing loop */

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// draw a grid on XZ-plane
///////////////////////////////////////////////////////////////////////////////
void drawGrid(float size, float step)
{
    // disable lighting
    glDisable(GL_LIGHTING);

    // 20x20 grid
    glBegin(GL_LINES);

    glColor3f(0.5f, 0.5f, 0.5f);
    for(float i=step; i <= size; i+= step)
    {
        glVertex3f(-size, 0,  i);   // lines parallel to X-axis
        glVertex3f( size, 0,  i);
        glVertex3f(-size, 0, -i);   // lines parallel to X-axis
        glVertex3f( size, 0, -i);

        glVertex3f( i, 0, -size);   // lines parallel to Z-axis
        glVertex3f( i, 0,  size);
        glVertex3f(-i, 0, -size);   // lines parallel to Z-axis
        glVertex3f(-i, 0,  size);
    }

    // x-axis
    glColor3f(1, 0, 0);
    glVertex3f(-size, 0, 0);
    glVertex3f( size, 0, 0);

    // z-axis
    glColor3f(0,0,1);
    glVertex3f(0, 0, -size);
    glVertex3f(0, 0,  size);

    glEnd();

    // enable lighting back
    glEnable(GL_LIGHTING);
}



///////////////////////////////////////////////////////////////////////////////
// draw axis
///////////////////////////////////////////////////////////////////////////////
void drawAxis(float size)
{
    //glDepthFunc(GL_ALWAYS);     // to avoid visual artifacts with grid lines
    glDisable(GL_LIGHTING);

    float s = size * 0.5f;

    // draw axis
    glLineWidth(2);
    glBegin(GL_LINES);
        glColor3f(1, 0, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(s, 0, 0);
        //glColor3f(0.5f, 0, 0);
        //glVertex3f(0, 0, 0);
        //glVertex3f(-s, 0, 0);
        glColor3f(0, 1, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(0, s, 0);
        //glColor3f(0, 0.5f, 0);
        //glVertex3f(0, 0, 0);
        //glVertex3f(0, -s, 0);
        glColor3f(0, 0, 1);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 0, s);
        //glColor3f(0, 0, 0.5f);
        //glVertex3f(0, 0, 0);
        //glVertex3f(0, 0, -s);
    glEnd();
    glLineWidth(1);

    // restore default settings
    glEnable(GL_LIGHTING);
    //glDepthFunc(GL_LEQUAL);
}



///////////////////////////////////////////////////////////////////////////////
// draw a square room
///////////////////////////////////////////////////////////////////////////////
void drawRoom(float size)
{
    float d = 0.02f;
    float s = size * 0.5f;

    glColor4f(0.3f, 0.3f, 0.3f, 1.0f);
    glBegin(GL_TRIANGLES);
    for(int i = -(int)s; i <= (int)s; ++i)
    {
        if(i == 0)
            continue;

        // -X
        glNormal3f(1, 0, 0);
        glVertex3f(-s, i-d, -s);    // horizontal
        glVertex3f(-s, i+d, -s);
        glVertex3f(-s, i+d,  s);
        glVertex3f(-s, i+d,  s);
        glVertex3f(-s, i-d,  s);
        glVertex3f(-s, i-d, -s);

        glVertex3f(-s, -s, i-d);   // vertical
        glVertex3f(-s,  s, i-d);
        glVertex3f(-s,  s, i+d);
        glVertex3f(-s,  s, i+d);
        glVertex3f(-s, -s, i+d);
        glVertex3f(-s, -s, i-d);

        // +X
        glNormal3f(-1, 0, 0);
        glVertex3f( s, i-d, -s);
        glVertex3f( s, i-d,  s);
        glVertex3f( s, i+d,  s);
        glVertex3f( s, i+d,  s);
        glVertex3f( s, i+d, -s);
        glVertex3f( s, i-d, -s);

        glVertex3f( s, -s, i-d);
        glVertex3f( s, -s, i+d);
        glVertex3f( s,  s, i+d);
        glVertex3f( s,  s, i+d);
        glVertex3f( s,  s, i-d);
        glVertex3f( s, -s, i-d);

        // -Y
        glNormal3f(0, 1, 0);
        glVertex3f(-s, -s, i-d);
        glVertex3f(-s, -s, i+d);
        glVertex3f( s, -s, i+d);
        glVertex3f( s, -s, i+d);
        glVertex3f( s, -s, i-d);
        glVertex3f(-s, -s, i-d);

        glVertex3f(i-d, -s, -s);
        glVertex3f(i-d, -s,  s);
        glVertex3f(i+d, -s,  s);
        glVertex3f(i+d, -s,  s);
        glVertex3f(i+d, -s, -s);
        glVertex3f(i-d, -s, -s);

        // +Y
        glNormal3f(0, -1, 0);
        glVertex3f(-s,  s, i-d);
        glVertex3f( s,  s, i-d);
        glVertex3f( s,  s, i+d);
        glVertex3f( s,  s, i+d);
        glVertex3f(-s,  s, i+d);
        glVertex3f(-s,  s, i-d);

        glVertex3f(i-d,  s, -s);
        glVertex3f(i+d,  s, -s);
        glVertex3f(i+d,  s,  s);
        glVertex3f(i+d,  s,  s);
        glVertex3f(i-d,  s,  s);
        glVertex3f(i-d,  s, -s);

        // -Z
        glNormal3f(0, 0, 1);
        glVertex3f(-s, i-d, -s);
        glVertex3f( s, i-d, -s);
        glVertex3f( s, i+d, -s);
        glVertex3f( s, i+d, -s);
        glVertex3f(-s, i+d, -s);
        glVertex3f(-s, i-d, -s);

        glVertex3f(i-d, -s, -s);
        glVertex3f(i+d, -s, -s);
        glVertex3f(i+d,  s, -s);
        glVertex3f(i+d,  s, -s);
        glVertex3f(i-d,  s, -s);
        glVertex3f(i-d, -s, -s);

        // +Z
        glNormal3f(0, 0, -1);
        glVertex3f(-s, i-d,  s);
        glVertex3f(-s, i+d,  s);
        glVertex3f( s, i+d,  s);
        glVertex3f( s, i+d,  s);
        glVertex3f( s, i-d,  s);
        glVertex3f(-s, i-d,  s);

        glVertex3f(i-d, -s,  s);
        glVertex3f(i-d,  s,  s);
        glVertex3f(i+d,  s,  s);
        glVertex3f(i+d,  s,  s);
        glVertex3f(i+d, -s,  s);
        glVertex3f(i-d, -s,  s);
    }

    d = 0.04f;
    glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
    glNormal3f(1, 0, 0);
    glVertex3f(-s, -d, -s);
    glVertex3f(-s,  d, -s);
    glVertex3f(-s,  d,  s);
    glVertex3f(-s,  d,  s);
    glVertex3f(-s, -d,  s);
    glVertex3f(-s, -d, -s);

    glVertex3f(-s, -s, -d);
    glVertex3f(-s,  s, -d);
    glVertex3f(-s,  s,  d);
    glVertex3f(-s,  s,  d);
    glVertex3f(-s, -s,  d);
    glVertex3f(-s, -s, -d);

    glNormal3f(-1, 0, 0);
    glVertex3f( s, -d, -s);
    glVertex3f( s, -d,  s);
    glVertex3f( s,  d,  s);
    glVertex3f( s,  d,  s);
    glVertex3f( s,  d, -s);
    glVertex3f( s, -d, -s);

    glVertex3f( s, -s, -d);
    glVertex3f( s, -s,  d);
    glVertex3f( s,  s,  d);
    glVertex3f( s,  s,  d);
    glVertex3f( s,  s, -d);
    glVertex3f( s, -s, -d);

    glNormal3f(0, 1, 0);
    glVertex3f(-s, -s, -d);
    glVertex3f(-s, -s,  d);
    glVertex3f( s, -s,  d);
    glVertex3f( s, -s,  d);
    glVertex3f( s, -s, -d);
    glVertex3f(-s, -s, -d);

    glVertex3f(-d, -s, -s);
    glVertex3f(-d, -s,  s);
    glVertex3f( d, -s,  s);
    glVertex3f( d, -s,  s);
    glVertex3f( d, -s, -s);
    glVertex3f(-d, -s, -s);

    glNormal3f(0, -1, 0);
    glVertex3f(-s,  s, -d);
    glVertex3f( s,  s, -d);
    glVertex3f( s,  s,  d);
    glVertex3f( s,  s,  d);
    glVertex3f(-s,  s,  d);
    glVertex3f(-s,  s, -d);

    glVertex3f(-d,  s, -s);
    glVertex3f( d,  s, -s);
    glVertex3f( d,  s,  s);
    glVertex3f( d,  s,  s);
    glVertex3f(-d,  s,  s);
    glVertex3f(-d,  s, -s);

    glNormal3f(0, 0, 1);
    glVertex3f(-s, -d, -s);
    glVertex3f( s, -d, -s);
    glVertex3f( s,  d, -s);
    glVertex3f( s,  d, -s);
    glVertex3f(-s,  d, -s);
    glVertex3f(-s, -d, -s);

    glVertex3f(-d, -s, -s);
    glVertex3f( d, -s, -s);
    glVertex3f( d,  s, -s);
    glVertex3f( d,  s, -s);
    glVertex3f(-d,  s, -s);
    glVertex3f(-d, -s, -s);

    glNormal3f(0, 0, -1);
    glVertex3f(-s, -d,  s);
    glVertex3f(-s,  d,  s);
    glVertex3f( s,  d,  s);
    glVertex3f( s,  d,  s);
    glVertex3f( s, -d,  s);
    glVertex3f(-s, -d,  s);

    glVertex3f(-d, -s,  s);
    glVertex3f(-d,  s,  s);
    glVertex3f( d,  s,  s);
    glVertex3f( d,  s,  s);
    glVertex3f( d, -s,  s);
    glVertex3f(-d, -s,  s);

    glEnd();
}



///////////////////////////////////////////////////////////////////////////////
// draw a plane
///////////////////////////////////////////////////////////////////////////////
void drawPlane(const Plane& plane, const Vector3& color)
{
    Vector3 n = plane.getNormal();
    float d = plane.getDistance();
    n.normalize();
    Vector3 t = n * d;
    //std::cout << n << std::endl;

    glPushMatrix();

    Matrix4 m1, m2;
    m1.lookAt(n);
    m1.translate(t);

    m2 = matrixView * m1;
    glLoadMatrixf(m2.get());

    glDisable(GL_CULL_FACE);
    glColor3f(color.x, color.y, color.z);
    glNormal3f(0, 0, 1);
    glBegin(GL_TRIANGLES);
    glVertex3f(-10, -10, 0);
    glVertex3f( 10, -10, 0);
    glVertex3f( 10,  10, 0);
    glVertex3f( 10,  10, 0);
    glVertex3f(-10,  10, 0);
    glVertex3f(-10, -10, 0);
    glEnd();
    glEnable(GL_CULL_FACE);

    glPopMatrix();
}



///////////////////////////////////////////////////////////////////////////////
// draw a line using cylinder
///////////////////////////////////////////////////////////////////////////////
void drawLine(const Line& line, const Vector3& color)
{
    //glDepthFunc(GL_ALWAYS);     // to avoid visual artifacts with grid lines
    glDisable(GL_LIGHTING);

    // find the point on the line which closest from origin (0,0,0)
    // since the 3rd plane passes the origin, the shortest distance from
    // the line is actually the point of the line
    Vector3 v = line.getDirection();
    Vector3 p = line.getPoint();
    /*
    Vector3 v = line.getDirection();
    Vector3 p = line.getPoint();
    float d = p.dot(v);     // dot product
    float s = v.dot(v);     // squared
    float t = d / s;        // parameter t
    p += t * v;             // point intersected with perpendicular line
    std::cout << p << std::endl;
    */

    // transform cylinder
    glPushMatrix();
    Matrix4 m, r;
    r.lookAt(v);
    m.scale(0.1f, 0.1f, 40);
    m = r * m;  // apply lookat rotation
    m.translate(p);
    m = matrixView * m;
    glLoadMatrixf(m.get());

    // draw cylinder
    glColor3f(color.x, color.y, color.z);
    cylinder.draw();

    glPopMatrix();

    // restore default settings
    glEnable(GL_LIGHTING);
    //glDepthFunc(GL_LEQUAL);
}



///////////////////////////////////////////////////////////////////////////////
// initialize GLUT for windowing
///////////////////////////////////////////////////////////////////////////////
int initGLUT(int argc, char **argv)
{
    // GLUT stuff for windowing
    // initialization openGL window.
    // it is called before any other GLUT routine
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);   // display mode

    glutInitWindowSize(screenWidth, screenHeight);  // window size

    glutInitWindowPosition(100, 100);           // window location

    // finally, create a window with openGL context
    // Window will not displayed until glutMainLoop() is called
    // it returns a unique ID
    int handle = glutCreateWindow(argv[0]);     // param is the title of window

    // register GLUT callback functions
    glutDisplayFunc(displayCB);
    glutTimerFunc(33, timerCB, 33);             // redraw only every given millisec
    //glutIdleFunc(idleCB);                       // redraw whenever system is idle
    glutReshapeFunc(reshapeCB);
    glutKeyboardFunc(keyboardCB);
    glutMouseFunc(mouseCB);
    glutMotionFunc(mouseMotionCB);
    glutSpecialFunc(specialCB);

    return handle;
}



///////////////////////////////////////////////////////////////////////////////
// initialize OpenGL
// disable unused features
///////////////////////////////////////////////////////////////////////////////
void initGL()
{
    glShadeModel(GL_SMOOTH);                    // shading mathod: GL_SMOOTH or GL_FLAT
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment

    // enable /disable features
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);

     // track material ambient and diffuse from surface color, call it before glEnable(GL_COLOR_MATERIAL)
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    glClearColor(0, 0, 0, 0);                   // background color
    glClearStencil(0);                          // clear stencil buffer
    glClearDepth(1.0f);                         // 0 is near, 1 is far
    glDepthFunc(GL_LEQUAL);

    initLights();
}



///////////////////////////////////////////////////////////////////////////////
// write 2d text using GLUT
// The projection matrix must be set to orthogonal before call this function.
///////////////////////////////////////////////////////////////////////////////
void drawString(const char *str, int x, int y, float color[4], void *font)
{
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    glDisable(GL_LIGHTING);     // need to disable lighting for proper text color
    glDisable(GL_TEXTURE_2D);

    glColor4fv(color);          // set text color
    glRasterPos2i(x, y);        // place text position

    // loop all characters in the string
    while(*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glPopAttrib();
}



///////////////////////////////////////////////////////////////////////////////
// draw a string in 3D space
///////////////////////////////////////////////////////////////////////////////
void drawString3D(const char *str, float pos[3], float color[4], void *font)
{
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    glDisable(GL_LIGHTING);     // need to disable lighting for proper text color
    glDisable(GL_TEXTURE_2D);

    glColor4fv(color);          // set text color
    glRasterPos3fv(pos);        // place text position

    // loop all characters in the string
    while(*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glPopAttrib();
}



///////////////////////////////////////////////////////////////////////////////
// initialize global variables
///////////////////////////////////////////////////////////////////////////////
bool initSharedMem()
{
    // testing line intersection
    Line l1= Line(Vector2(7+8, 6+4),  Vector2(7,6));    // P1(7,6), P2(-8,-4)
    Line l2= Line(Vector2(6+2, -8-8), Vector2(6,-8));   // P1(6,-8), P2(-2,8)
    std::cout << "===== TEST LINE INTERSECT =====" << std::endl;
    std::cout << "Line1: P1(7,6)  - P2(-8,-4)" << std::endl;
    std::cout << "Line2: P1(6,-8) - P2(-2,8)" << std::endl;
    std::cout << "Is Intersected: " << (l1.isIntersected(l2) ? "TRUE" : "FALSE") << std::endl;
    std::cout << "Intersect Point: " << l1.intersect(l2) << std::endl;
    std::cout << std::endl;

    // testing plane intersection
    plane1.set(2, 3, 1, 3); // init with 4 coeff
    plane2.set(-1, 1, 1, 2); // init with 4 coeff
    std::cout << "===== TEST PLANE INTERSECT =====" << std::endl;
    plane1.printSelf();
    plane2.printSelf();
    //float dist = plane1.getDistance(Vector3(-1, -2, -3));
    //std::cout << "Distance: " << dist << std::endl;

    // intersect 2 planes
    line = plane1.intersect(plane2);
    std::cout << "Is Intersected: " << (plane1.isIntersected(plane2) ? "TRUE":"FALSE") << std::endl;
    std::cout << "Intrsect Line: " << std::endl;
    line.printSelf();

    // plane and line colours
    color1.set(0.8f, 0.9f, 0.8f);   // plane1
    color2.set(0.8f, 0.8f, 0.9f);   // plane2
    color3.set(1.0f, 0.5f, 0.0f);   // line

    screenWidth = SCREEN_WIDTH;
    screenHeight = SCREEN_HEIGHT;

    mouseLeftDown = mouseRightDown = false;
    mouseX = mouseY = 0;

    cameraX = cameraY = 0;
    cameraAngleX = CAMERA_ANGLE_X;
    cameraAngleY = CAMERA_ANGLE_Y;
    cameraDistance = CAMERA_DISTANCE;

    drawMode = 0; // 0:fill, 1: wireframe, 2:points

    return true;
}



///////////////////////////////////////////////////////////////////////////////
// clean up shared memory
///////////////////////////////////////////////////////////////////////////////
void clearSharedMem()
{
}



///////////////////////////////////////////////////////////////////////////////
// initialize lights
///////////////////////////////////////////////////////////////////////////////
void initLights()
{
    // set up light colors (ambient, diffuse, specular)
    GLfloat lightKa[] = {.2f, .2f, .2f, 1.0f};  // ambient light
    GLfloat lightKd[] = {.7f, .7f, .7f, 1.0f};  // diffuse light
    GLfloat lightKs[] = {1, 1, 1, 1};           // specular light
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightKa);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightKd);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightKs);

    // position the light
    float lightPos[4] = {0, 0, 20, 1}; // positional light
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_LIGHT0);                        // MUST enable each light source after configuration
}



///////////////////////////////////////////////////////////////////////////////
// set camera position and lookat direction
///////////////////////////////////////////////////////////////////////////////
void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(posX, posY, posZ, targetX, targetY, targetZ, 0, 1, 0); // eye(x,y,z), focal(x,y,z), up(x,y,z)
}



///////////////////////////////////////////////////////////////////////////////
// display info messages
///////////////////////////////////////////////////////////////////////////////
void showInfo()
{
    // backup current model-view matrix
    glPushMatrix();                     // save current modelview matrix
    glLoadIdentity();                   // reset modelview matrix

    // set to 2D orthogonal projection
    glMatrixMode(GL_PROJECTION);        // switch to projection matrix
    glPushMatrix();                     // save current projection matrix
    glLoadIdentity();                   // reset projection matrix
    gluOrtho2D(0, screenWidth, 0, screenHeight); // set to orthogonal projection

    float color[4] = {1, 1, 1, 1};

    std::stringstream ss;
    ss << std::fixed << std::setprecision(3);

    // print here

    // unset floating format
    ss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);

    // restore projection matrix
    glPopMatrix();                   // restore to previous projection matrix

    // restore modelview matrix
    glMatrixMode(GL_MODELVIEW);      // switch to modelview matrix
    glPopMatrix();                   // restore to previous modelview matrix
}



///////////////////////////////////////////////////////////////////////////////
// set projection matrix as orthogonal
///////////////////////////////////////////////////////////////////////////////
void toOrtho()
{
    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);

    // set orthographic viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, 0, screenHeight, -1, 1);

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}



///////////////////////////////////////////////////////////////////////////////
// set the projection matrix as perspective
///////////////////////////////////////////////////////////////////////////////
void toPerspective()
{
    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);

    // set perspective viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)(screenWidth)/screenHeight, 1.0f, 1000.0f); // FOV, AspectRatio, NearClip, FarClip

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}












//=============================================================================
// CALLBACKS
//=============================================================================

void displayCB()
{
    // clear buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // save the initial ModelView matrix before modifying ModelView matrix
    glPushMatrix();

    // tramsform camera
    matrixView.identity();
    matrixView.rotateY(cameraAngleY);
    matrixView.rotateX(cameraAngleX);
    matrixView.translate(0, 0, -cameraDistance);
    glLoadMatrixf(matrixView.get());

    drawRoom(20);
    drawAxis(20);
    drawPlane(plane1, color1);
    drawPlane(plane2, color2);
    drawLine(line, color3);

    // draw info messages
    showInfo();

    glPopMatrix();

    glutSwapBuffers();
}



void reshapeCB(int w, int h)
{
    screenWidth = w;
    screenHeight = h;
    toPerspective();
}


void timerCB(int millisec)
{
    glutTimerFunc(millisec, timerCB, millisec);
    glutPostRedisplay();
}


void idleCB()
{
    glutPostRedisplay();
}


void keyboardCB(unsigned char key, int x, int y)
{
    switch(key)
    {
    case 27: // ESCAPE
        exit(0);
        break;

    case ' ':
        break;

    case 'd': // switch rendering modes (fill -> wire -> point)
    case 'D':
        drawMode = ++drawMode % 3;
        if(drawMode == 0)        // fill mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
        }
        else if(drawMode == 1)  // wireframe mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        else                    // point mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        break;

    default:
        ;
    }
}


void mouseCB(int button, int state, int x, int y)
{
    mouseX = x;
    mouseY = y;

    if(button == GLUT_LEFT_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseLeftDown = true;
        }
        else if(state == GLUT_UP)
            mouseLeftDown = false;
    }

    else if(button == GLUT_RIGHT_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseRightDown = true;
        }
        else if(state == GLUT_UP)
            mouseRightDown = false;
    }
}


void mouseMotionCB(int x, int y)
{
    if(mouseLeftDown)
    {
        cameraAngleY += (x - mouseX);
        cameraAngleX += (y - mouseY);
        mouseX = x;
        mouseY = y;
    }
    if(mouseRightDown)
    {
        cameraDistance -= (y - mouseY) * 0.2f;
        mouseY = y;
    }
}


void specialCB(int key, int x, int y)
{
    switch(key)
    {
    case GLUT_KEY_LEFT:
        cameraX -= 1;
        break;

    case GLUT_KEY_RIGHT:
        cameraX += 1;
        break;

    case GLUT_KEY_UP:
        cameraY += 1;
        break;

    case GLUT_KEY_DOWN:
        cameraY -= 1;
        break;

    default:
        ;
    }
}


void exitCB()
{
    clearSharedMem();
}
