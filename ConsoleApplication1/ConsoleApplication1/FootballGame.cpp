
#include "stdafx.h"
#include "GLee.h"
#include "freeglut.h"
#include <Windows.h>
#include <iostream>
#include <math.h>
#include "gltools.h"

//declaring and initializing the needed variables
using namespace std;
double ballDis = 4.0;
double ballY = -0.6;
double ballX = 0.00;
double tempBallY = 0.00;
double tempBallX = 0.00;
int points = 0;
double k = 4.00;
int attempts = 10;
static float sphereRotation = 0.0;
double hudHorix1 =70.0 ;
double hudHorix2 =80.0 ;
double hudVer = 780.0;
double  rotangle = 0;
double rotx = 0;
double roty = 0;
double rotz = 0;
double skyrot = 0;
int reset = 0;

//target positions
GLfloat target1[] = { 0.0,0.5,-9.8 };
GLfloat target2[] = { -2.0, 0.0, -9.8 };
GLfloat target3[] = { 3.0, 1.0, -9.8 };
GLfloat target4[] = { -3.2, 1.0, -9.8 };
GLfloat target5[] = { 1.8, 0.0, -9.8 };
GLbyte *pBytes8; // 30 target
GLbyte *pBytes7; //20 target
GLbyte *pBytes6; //10 target


// Lighting values
GLfloat  whiteLight[] = { 10.0f, 10.0f, 10.0f, 1.0f };
GLfloat  whiteLight1[] = { 3.0f, 3.0f, 3.0f, 1.0f };

//lighting positions
GLfloat	 lightPos0[] = { 0.0f, 1.0f, -1.0f, 1.0f };
GLfloat	 lightPos1[] = { 0.0f, 2.0f, 0.0f, 0.0f };

//set up variable for the textures
GLint iWidth, iHeight, iComponents;
GLenum eFormat;

//==TOOL=======================================================================================================
//This section is taken from another library
#pragma pack(1)
typedef struct
{
	GLbyte	identsize;              // Size of ID field that follows header (0)
	GLbyte	colorMapType;           // 0 = None, 1 = paletted
	GLbyte	imageType;              // 0 = none, 1 = indexed, 2 = rgb, 3 = grey, +8=rle
	unsigned short	colorMapStart;  // First colour map entry
	unsigned short	colorMapLength; // Number of colors
	unsigned char 	colorMapBits;   // bits per palette entry
	unsigned short	xstart;         // image x origin
	unsigned short	ystart;         // image y origin
	unsigned short	width;          // width in pixels
	unsigned short	height;         // height in pixels
	GLbyte	bits;                   // bits per pixel (8 16, 24, 32)
	GLbyte	descriptor;             // image descriptor
} TGAHEADER;
#pragma pack(8)

////////////////////////////////////////////////////////////////////
// Allocate memory and load targa bits. Returns pointer to new buffer,
// height, and width of texture, and the OpenGL format of data.
// Call free() on buffer when finished!
// This only works on pretty vanilla targas... 8, 24, or 32 bit color
// only, no palettes, no RLE encoding.

GLbyte *gltLoadTGA(const char *szFileName, GLint *iWidth, GLint *iHeight, GLint *iComponents, GLenum *eFormat)
{
	FILE *pFile;			// File pointer
	TGAHEADER tgaHeader;		// TGA file header
	unsigned long lImageSize;		// Size in bytes of image
	short sDepth;			// Pixel depth;
	GLbyte	*pBits = NULL;          // Pointer to bits

									// Default/Failed values
	*iWidth = 0;
	*iHeight = 0;
	*eFormat = GL_BGR_EXT;
	*iComponents = GL_RGB8;

	// Attempt to open the file
	//pFile = fopen(szFileName, "rb");
	fopen_s(&pFile, szFileName, "rb");

	if (pFile == NULL)
		return NULL;

	// Read in header (binary)
	fread(&tgaHeader, 18/* sizeof(TGAHEADER)*/, 1, pFile);

	// Do byte swap for big vs little endian
#ifdef __APPLE__
	LITTLE_ENDIAN_WORD(&tgaHeader.colorMapStart);
	LITTLE_ENDIAN_WORD(&tgaHeader.colorMapLength);
	LITTLE_ENDIAN_WORD(&tgaHeader.xstart);
	LITTLE_ENDIAN_WORD(&tgaHeader.ystart);
	LITTLE_ENDIAN_WORD(&tgaHeader.width);
	LITTLE_ENDIAN_WORD(&tgaHeader.height);
#endif


	// Get width, height, and depth of texture
	*iWidth = tgaHeader.width;
	*iHeight = tgaHeader.height;
	sDepth = tgaHeader.bits / 8;

	// Put some validity checks here. Very simply, I only understand
	// or care about 8, 24, or 32 bit targa's.
	if (tgaHeader.bits != 8 && tgaHeader.bits != 24 && tgaHeader.bits != 32)
		return NULL;

	// Calculate size of image buffer
	lImageSize = tgaHeader.width * tgaHeader.height * sDepth;

	// Allocate memory and check for success
	pBits = (GLbyte*)malloc(lImageSize * sizeof(GLbyte));
	if (pBits == NULL)
		return NULL;

	// Read in the bits
	// Check for read error. This should catch RLE or other 
	// weird formats that I don't want to recognize
	if (fread(pBits, lImageSize, 1, pFile) != 1)
	{
		free(pBits);
		return NULL;
	}

	// Set OpenGL format expected
	switch (sDepth)
	{
	case 3:     // Most likely case
		*eFormat = GL_BGR_EXT;
		*iComponents = GL_RGB8;
		break;
	case 4:
		*eFormat = GL_BGRA_EXT;
		*iComponents = GL_RGBA8;
		break;
	case 1:
		*eFormat = GL_LUMINANCE;
		*iComponents = GL_LUMINANCE8;
		break;
	};


	// Done with File
	fclose(pFile);

	// Return pointer to image data
	return pBits;
}
//==END OF TOOL=================================================================================================

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	// set the current clear color to black and the current 
	glClearColor(0.0, 0.0, 0.0, 1.0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(60.0, 16.0 / 9.0, 1, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 0, 0, 0, -1, 0, 1, 0);
}

//you can use this for HUD and placing text to the screen - use the pixels (0,0) top left 
//note once this is called you can use glBegin etc to draw primitives (2D only) - great for menus/HUD/scores etc. 

void setOrthographicProjection() {
	// switch to projection mode     
	glMatrixMode(GL_PROJECTION);
	// save the previous matrix which contains the     
	//set up for the perspective projection     
	glPushMatrix();
	// reset matrix     
	glLoadIdentity();
	// set a 2D orthographic projection     
	gluOrtho2D(0, 600, 0, 800);
	// invert the y axis, down is positive     
	glScalef(1, -1, 1);
	// mover the origin from the bottom left corner     
	// to the upper left corner     
	glTranslatef(0, -800, 0);
	//set for drawing again     
	glMatrixMode(GL_MODELVIEW);
}

void resetPerspectiveProjection() {
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

//drawing and texturing the skybox
void drawSkyBox() {
	GLbyte*pBytes14;
	pBytes14 = gltLoadTGA("Textures/Clouds.tga", &iWidth, &iHeight, &iComponents, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes14);
	free(pBytes14);
	glEnable(GL_TEXTURE_2D);

	glPushMatrix();
	glTranslated(0.0,0.0,-6.0);
	glRotated(skyrot,0.0,1.0,0.0);

	//front face
	glBegin(GL_QUADS);
	glTexCoord2d(0.25, 0.33);
	glNormal3d(0.0,0.0,1.0);
	glVertex3d(-8.0,-8.0,-8.0);

	glTexCoord2d(0.25, 0.66);
	glNormal3d(0.0, 0.0, 1.0);
	glVertex3d(-8.0, 8.0, -8.0);

	glTexCoord2d(0.5, 0.66);
	glNormal3d(0.0, 0.0, 1.0);
	glVertex3d( 8.0, 8.0, -8.0);

	glTexCoord2d(0.5, 0.33);
	glNormal3d(0.0, 0.0, 1.0);
	glVertex3d( 8.0, -8.0, -8.0);
	glEnd();


	//top face
	glBegin(GL_QUADS);
	glTexCoord2d(0.25, 0.66);
	glNormal3d(0.0, -1.0, 0.0);
	glVertex3d(-8.0, 8.0, -8.0);

	glTexCoord2d(0.25, 1.0);
	glNormal3d(0.0, -1.0, 0.0);
	glVertex3d(-8.0, 8.0, 8.0);

	glTexCoord2d(0.5, 1.0);
	glNormal3d(0.0, -1.0, 0.0);
	glVertex3d(8.0, 8.0, 8.0);

	glTexCoord2d(0.5, 0.66);
	glNormal3d(0.0, -1.0, 0.0);
	glVertex3d(8.0, 8.0, -8.0);
	glEnd();


	//left face
	glBegin(GL_QUADS);
	glTexCoord2d(0.25, 0.33);
	glNormal3d(1.0, 0.0, 0.0);
	glVertex3d(-8.0, -8.0, -8.0);

	glTexCoord2d(0.25, 0.66);
	glNormal3d(1.0, 0.0, 0.0);
	glVertex3d(-8.0, 8.0, -8.0);

	glTexCoord2d(0.0, 0.66);
	glNormal3d(1.0, 0.0, 0.0);
	glVertex3d(-8.0, 8.0, 8.0);

	glTexCoord2d(0.0, 0.33);
	glNormal3d(1.0, 0.0, 0.0);
	glVertex3d(-8.0, -8.0, 8.0);
	glEnd();

	//right face
	glBegin(GL_QUADS);
	glTexCoord2d(0.5, 0.33);
	glNormal3d(-1.0, 0.0, 0.0);
	glVertex3d(8.0, -8.0, -8.0);

	glTexCoord2d(0.5, 0.66);
	glNormal3d(-1.0, 0.0, 0.0);
	glVertex3d(8.0, 8.0, -8.0);

	glTexCoord2d(0.75, 0.66);
	glNormal3d(-1.0, 0.0, 0.0);
	glVertex3d(8.0, 8.0, 8.0);

	glTexCoord2d(0.75, 0.33);
	glNormal3d(-1.0, 0.0, 0.0);
	glVertex3d(8.0, -8.0, 8.0);
	glEnd();

	//behind face
	glBegin(GL_QUADS);
	glTexCoord2d(0.75, 0.33);
	glNormal3d(0.0, 0.0, -1.0);
	glVertex3d(8.0, -8.0, 8.0);

	glTexCoord2d(0.75, 0.66);
	glNormal3d(0.0, 0.0, -1.0);
	glVertex3d(8.0, 8.0, 8.0);

	glTexCoord2d(1.0, 0.66);
	glNormal3d(0.0, 0.0, -1.0);
	glVertex3d(-8.0, 8.0, 8.0);

	glTexCoord2d(1.0, 0.33);
	glNormal3d(0.0, 0.0, -1.0);
	glVertex3d(-8.0, -8.0, 8.0);
	glEnd();


	//bottom face
	glBegin(GL_QUADS);
	glTexCoord2d(0.25, 0.33);
	glNormal3d(0.0, 1.0, 0.0);
	glVertex3d(-8.0, -8.0, -8.0);

	glTexCoord2d(0.5, 0.33);
	glNormal3d(0.0, 1.0, 0.0);
	glVertex3d(8.0, -8.0, -8.0);

	glTexCoord2d(0.5, 0.00);
	glNormal3d(0.0, 1.0, 0.0);
	glVertex3d(8.0, -8.0, 8.0);

	glTexCoord2d(0.25, 0.0);
	glNormal3d(0.0, 1.0, 0.0);
	glVertex3d(-8.0, -8.0, 8.0);
	glEnd();

	glPopMatrix();
}

//setting up the required parameters for the textures and lights
void SetupRC()
{

	//setting up texture parameters
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);



	// Hidden surface removal
	glEnable(GL_DEPTH_TEST);

	// Counter clock-wise polygons face out
	glFrontFace(GL_CCW);



	// Setup and enable light 0
	glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteLight);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
	glEnable(GL_LIGHT0);

	// Setup and enable light 1
	glLightfv(GL_LIGHT1, GL_DIFFUSE, whiteLight1);
	glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);
	glEnable(GL_LIGHT1);

	// Enable color tracking
	glEnable(GL_COLOR_MATERIAL);

}

//drawing the football and texturing it
void Football() {

	GLbyte *pBytes5;
	pBytes5 = gltLoadTGA("Textures/FootballCompleteMap.tga", &iWidth, &iHeight, &iComponents, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes5);
	free(pBytes5);
	glEnable(GL_TEXTURE_2D);


	glTranslated(ballX, ballY, -ballDis);

	GLUquadricObj *quadric;
	quadric = gluNewQuadric();

	gluQuadricDrawStyle(quadric, GLU_FILL);
	gluQuadricNormals(quadric, GLU_SMOOTH);
	gluQuadricOrientation(quadric, GLU_OUTSIDE);
	//enable the texturing
	gluQuadricTexture(quadric, GL_TRUE);

	glPushMatrix();
	glRotatef(rotangle,rotx,roty,rotz);
	gluSphere(quadric, 0.35f, 50, 35);
	glPopMatrix();
	

}

//drawing and texturing the 10 point target
void target10() {

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	pBytes6 = gltLoadTGA("Textures/targetGreen.tga", &iWidth, &iHeight, &iComponents, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes6);
	free(pBytes6);


	GLUquadricObj *quadric;
	quadric = gluNewQuadric();

	gluQuadricDrawStyle(quadric, GLU_FILL);
	gluQuadricNormals(quadric, GLU_SMOOTH);
	gluQuadricOrientation(quadric, GLU_OUTSIDE);

	//enable the texturing

	gluQuadricTexture(quadric, GL_TRUE);
	gluDisk(quadric, 0.0, 0.6, 32, 32);
	glDisable(GL_BLEND);
}

//drawing and texuring the 20 point target
void target20() {

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	pBytes7 = gltLoadTGA("Textures/targetBlue.tga", &iWidth, &iHeight, &iComponents, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes7);
	free(pBytes7);

	GLUquadricObj *quadric;
	quadric = gluNewQuadric();

	gluQuadricDrawStyle(quadric, GLU_FILL);
	gluQuadricNormals(quadric, GLU_SMOOTH);
	gluQuadricOrientation(quadric, GLU_OUTSIDE);

	//enable the texturing

	gluQuadricTexture(quadric, GL_TRUE);
	gluDisk(quadric, 0.0, 0.5, 32, 32);
	glDisable(GL_BLEND);
}

//drawing and texuring the 30 point target
void target30() {

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	pBytes8 = gltLoadTGA("Textures/targetRed.tga", &iWidth, &iHeight, &iComponents, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes8);
	free(pBytes8);

	GLUquadricObj *quadric;
	quadric = gluNewQuadric();

	gluQuadricDrawStyle(quadric, GLU_FILL);
	gluQuadricNormals(quadric, GLU_SMOOTH);
	gluQuadricOrientation(quadric, GLU_OUTSIDE);

	//enable the texturing
	gluQuadricTexture(quadric, GL_TRUE);
	gluDisk(quadric, 0.0, 0.4, 32, 32);
	glDisable(GL_BLEND);
}

//drawing the plants seen on the sides of the walls
void plants() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLbyte *pBytes10;
	pBytes10 = gltLoadTGA("Textures/palmBranchA.tga", &iWidth, &iHeight, &iComponents, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes10);
	free(pBytes10);
	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(-3.9f, -1.0f, -2.0f);

	glTexCoord2f(0.0, 1.0);
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(-3.9f, 1.0f, -2.0f);

	glTexCoord2f(1.0, 1.0);
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(-3.9f, 1.0f, -3.0f);

	glTexCoord2f(1.0, 0.0);
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(-4.0f, -1.0f, -3.0f);
	glEnd();


	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 0.0);
	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3f(3.9f, -1.0f, -2.0f);

	glTexCoord2f(1.0, 1.0);
	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3f(3.9f, 1.0f, -2.0f);

	glTexCoord2f(0.0, 1.0);
	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3f(3.9f, 1.0f, -3.0f);

	glTexCoord2f(0.0, 0.0);
	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3f(3.9f, -1.0f, -3.0f);
	glEnd();
	glDisable(GL_BLEND);
}

//drawing and texuring the walls and the ground
//placing the tagrets in the proper locations
//placing the decorations
void Draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawSkyBox();
	GLbyte *pBytes2;
	pBytes2 = gltLoadTGA("Textures/brick_texture_Flowers.tga", &iWidth, &iHeight, &iComponents, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes2);
	free(pBytes2);
	glEnable(GL_TEXTURE_2D);


	glBegin(GL_QUADS);

	//front wall plane
	glTexCoord2f(0.0, 0.0);
	glNormal3f(0.0,0.0,0.5);
	glVertex3f(-4.0f, -1.0f, -10.0f);
	
	glTexCoord2f(0.0f, 1.0f);
	glNormal3f(0.0, 0.0, 0.5);
	glVertex3f(-4.0f, 1.5f, -10.0f);

	glTexCoord2f(1.0f, 1.0f);
	glNormal3f(0.0,0.0, 0.5);
	glVertex3f(4.0f, 1.5f, -10.0f);

	glTexCoord2f(1.0f, 0.0f);
	glNormal3f(0.0, 0.0, 0.5);
	glVertex3f(4.0f, -1.0f, -10.0f);
	glEnd();

	//////////////////////////////////////////////////////
	GLbyte *pBytes;
	pBytes = gltLoadTGA("Textures/brick_texture_lo_res.tga", &iWidth, &iHeight, &iComponents, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);
	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);
	//left wall plane
	glTexCoord2f(0.0, 0.0);
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(-4.0f, -1.0f, 0.0f);

	glTexCoord2f(0.0, 1.0);
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(-4.0f, 1.5f, 0.0f);

	glTexCoord2f(1.0, 1.0);
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(-4.0f, 1.5f, -10.0f);

	glTexCoord2f(1.0, 0.0);
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(-4.0f, -1.0f, -10.0f);


	//right wall plane
	glTexCoord2f(1.0, 0.0);
	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3f(4.0f, -1.0f, 0.0f);

	glTexCoord2f(1.0, 1.0);
	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3f(4.0f, 1.5f, 0.0f);

	glTexCoord2f(0.0, 1.0);
	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3f(4.0f, 1.5f, -10.0f);

	glTexCoord2f(0.0, 0.0);
	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3f(4.0f, -1.0f, -10.0f);
	glEnd();

	/////////////////////////////////////////////////////////

	GLbyte *pBytes1;
	pBytes1 = gltLoadTGA("Textures/ground_grass_.tga", &iWidth, &iHeight, &iComponents, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes1);
	free(pBytes1);
	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);

	//ground plane
	glTexCoord2f(0.0, 0.0);
	glNormal3f(0.0, 0.0, 1.0);
	glVertex3f(-4.0f, -1.0f, 0.0f);

	glTexCoord2f(1.0f, 0.0f);
	glNormal3f(0.0, 0.0, 1.0);
	glVertex3f(4.0f, -1.0f, 0.0f);

	glTexCoord2f(1.0f, 1.0f);
	glNormal3f(0.0, 0.0, 1.0);
	glVertex3f(4.0f, -1.0f, -10.0f);

	glTexCoord2f(0.0f, 1.0f);
	glNormal3f(0.0, 0.0, 1.0);
	glVertex3f(-4.0f, -1.0f, -10.0f);

	glEnd();

	///////////////////////////////////////////////////////////

	//placing the targets in the proper locations

	//20 point target -01
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glTranslatef(target1[0], target1[1], target1[2]);
	target20();
	glPopMatrix();

	//10 point target -02
	glPushMatrix();
	glTranslatef(target2[0], target2[1], target2[2]);
	target10();
	glPopMatrix();

	//20 point target -03
	glPushMatrix();
	glTranslatef(target3[0], target3[1], target3[2]);
	target20();
	glPopMatrix();

	//30 point target -04
	glPushMatrix();
	glTranslatef(target4[0], target4[1], target4[2]);
	target30();
	glPopMatrix();

	//10 point target -05
	glPushMatrix();
	glTranslatef(target5[0], target5[1], target5[2]);
	target10();
	glPopMatrix();
	

	//placing decorations - plants
	plants();
	glPushMatrix();
	glTranslated(0.0,0.0,-6.0);
	plants();
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.0, 0.0, -5.0);
	plants();
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.0, 0.0, -4.0);
	plants();
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.0, 0.0, -1.0);
	plants();
	glPopMatrix();


	glPushMatrix();
	glTranslated(0.0, 0.0, -2.0);
	plants();
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.0, 0.0, -3.0);
	plants();
	glPopMatrix();

	//calling the football functions for it to be drawn
	Football();

	glTranslated(-ballX, -ballY, ballDis);
}

//printing 2d text to the screen
void printScore(int x, int y, char *string) {
	//set the position of the text in the window using the x and y coordinates
	glRasterPos2f(x, y);

	//get the length of the string for displaying
	int len = (int)strlen(string);

	//loop to display character by character
	for (int i = 0; i < len; i++)
	{

		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
	}
};

//combining the 2d and 3d assests and rendering the whole scene together
void RenderScene(void) {

	//draw all you 3D stuff

	Draw();

	glPushMatrix();
	glLoadIdentity();
	setOrthographicProjection();
	glDisable(GL_LIGHTING);

	//verticle Bar HUD
	GLbyte *pBytes3;
	pBytes3 = gltLoadTGA("Textures/fillBarHorizontal.tga", &iWidth, &iHeight, &iComponents, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes3);
	free(pBytes3);
	glEnable(GL_TEXTURE_2D);

	glDisable(GL_DEPTH_TEST);
	glBegin(GL_QUADS);

	glTexCoord2f(0.0, 1.0);
	glVertex3d(30.0, 40.0,-1.0);

	glTexCoord2f(1.0, 1.0);
	glVertex3d(120.0, 40.0,-1.0);

	glTexCoord2f(1.0, 0.0);
	glVertex3d(120.0, 80.0,-1.0);

	glTexCoord2f(0.0, 0.0);
	glVertex3d(30.0, 80.0,-1.0);

	glEnd();


	glBegin(GL_QUADS);
	glVertex3d(hudHorix1, 40.0, 0.0);
	glVertex3d(hudHorix2, 40.0, 0.0);
	glVertex3d(hudHorix2, 80.0, 0.0);
	glVertex3d(hudHorix1, 80.0, 0.0);
	glEnd();

	//////////////////////////////////////////////////////////


	//horizontal bar HUD
	GLbyte *pBytes4;
	pBytes4 = gltLoadTGA("Textures/fillBarVerticalR.tga", &iWidth, &iHeight, &iComponents, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes4);
	free(pBytes4);

	glBegin(GL_QUADS);

	glTexCoord2f(0.0, 1.0);
	glVertex3d(550, 600,-1.0);

	glTexCoord2f(0.0, 0.0);
	glVertex3d(550, 780,-1.0);

	glTexCoord2f(1.0, 0.0);
	glVertex3d(570, 780,-1.0);

	glTexCoord2f(1.0, 1.0);
	glVertex3d(570, 600,-1.0);
	glEnd();



	glBegin(GL_QUADS);
	glVertex3d(550, hudVer, 0.0);
	glVertex3d(550, 780, 0.0);
	glVertex3d(570, 780, 0.0);
	glVertex3d(570, hudVer, 0.0);
	glEnd();

	////////////////////////////////////////////////////////


	//printing the score on to the screen	
	char str[10] = { 0 };
	_itoa_s(points, str, 10);
	printScore(510, 40, "Score: ");
	printScore(560, 40, str);

	//printing the number of attempts left to the screen
	char str2[10] = { 0 };
	_itoa_s(attempts, str2, 10);
	printScore(500, 80, "Attempts:");
	printScore(560, 80, str2);


	if (attempts <1) {
		printScore(220, 250, "Press R to reset");
	}

	// draw/update all your 2D stuff positioned in pixels NOT world coordinates     
	glPopMatrix();
	resetPerspectiveProjection();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glutSwapBuffers();
}

//check if the football collides with any of the targets on the wall
void collision() {

	//taget 01
	if (ballDis > 9.00) {
		if ((ballX <= 0.4) && (ballX >= -0.4)) {
			if (ballY >= 0.1 && ballY <= 0.9) {
				points = points + 20;
			}
		}
	}

	//taget 02
	if (ballDis > 9.00) {
		if ((ballX <= -1.4) && (ballX >= -2.)) {
			if (ballY >= -0.5 && ballY <= 0.5) {
				points = points + 10;
			}
		}
	}

	//taget 03
	if (ballDis > 9.00) {
		if ((ballX <= 3.4) && (ballX >= 2.4)) {
			if (ballY >= 0.5 && ballY <= 1.5) {
				points = points + 20;
			}
		}
	}

	//taget 04
	if (ballDis > 9.00) {
		if ((ballX <= -2.7) && (ballX >= -3.5)) {
			if (ballY >= 0.5 && ballY <= 1.3) {
				points = points + 30;
			}
		}
	}

	//taget 05
	if (ballDis > 9.00) {
		if ((ballX <= 2.3) && (ballX >= 1.1)) {
			if (ballY >= -0.5 && ballY <= 0.5) {
				points = points + 10;
			}
		}
	}
}

//check the location of the football on the Z axis to determine distance
void checkDistance() {

	//checking if ball has hit the wall and resetting the ball 
	if (ballDis == 10) {
		ballDis = 4.00;
		ballY = -0.6;
		ballX = 0.00;
		tempBallY = 0.0;
		tempBallX = 0.0;
		hudVer = 780;
		hudHorix1 = 70;
		hudHorix2 = 80;
	}

	if (ballDis > 4.00) {
		ballY = ballY + (tempBallY / 6);
		if ((ballY) <= -0.6) {
			ballY = -0.6;
		}
	}

	if (ballDis > 4.00) {
		ballX = ballX + (tempBallX / 10);
	}
}

//to animate the ballmoving forward
void kick(int xy) {

	ballDis = ballDis + 0.5;
	rotangle = rotangle + 25;
	rotx = rotx + 10;
	roty = roty - 15;
	rotz = rotz + 18;
	checkDistance();
	collision();
	glutPostRedisplay();

	if (k < 10.00) {
		k = k + 0.5;
		glutTimerFunc(25, kick, 1);
	}
	else {
		k = 4.0;
		ballDis = 4.0;
	}
}

//adding rotation to the skybox
void skyRotation(int xy) {
	if (reset == 0) {
		if (skyrot < 360) {
			skyrot = skyrot + 0.02;
		}
		else {
			skyrot = 0.0;
		}

		glutPostRedisplay();
		glutTimerFunc(35, skyRotation, 1);
	}
}

//cheking if any special key is pressed
void SpecialKeys(int key, int x, int y) {
	if (attempts > 0) {
		//code to aim the ball up
		if (key == GLUT_KEY_UP) {
			reset = 0;
			if (tempBallY >= 1.5) {
				tempBallY = tempBallY;
			}
			else {
				tempBallY = tempBallY + 0.1;

			}

			if (tempBallY <= 1.5 && tempBallY > 0.0) {
				hudVer = hudVer - 12;
			}
		}

		//code to lower the aim
		if (key == GLUT_KEY_DOWN) {
			reset = 0;
			if (tempBallY <= -0.6) {
				tempBallY = tempBallY;

			}
			else {
				tempBallY = tempBallY - 0.1;
				
			}

			if (tempBallY > 0.1 && tempBallY < 1.6) {
				hudVer = hudVer + 12;
			}	
		}

		//code to aim the ball left
		if (key == GLUT_KEY_LEFT) {
			reset = 0;
			if (tempBallX <= -3.1) {
				tempBallX = -3.1;
			}
			else {
				tempBallX = tempBallX - 0.1;
				hudHorix1 = hudHorix1 - 1.3;
				hudHorix2 = hudHorix2 - 1.3;
			}
		}

		//code to aim the ball right
		if (key == GLUT_KEY_RIGHT) {
			reset = 0;
			if (tempBallX >= 3.1) {
				tempBallX = 3.1;
				
			}
			else {
				tempBallX = tempBallX + 0.1;
				hudHorix1 = hudHorix1 + 1.3;
				hudHorix2 = hudHorix2 + 1.3;
			}
		}

		checkDistance();
		glutPostRedisplay();
	}
}

//checking if normal key is pressed
void keyInput(unsigned char key, int x, int y) {
	if (attempts > 0) {
		if (key == 32 || key == 'z') {
			k = 4.0;
			glutTimerFunc(25, kick, 1);
			attempts--;
			glutPostRedisplay();
		}
	}
	if (attempts > 0) {
		if (key == 'r' || key == 'R') {
			
			reset = 1;
			glLoadIdentity();
			glDisable(GL_LIGHTING);
			setOrthographicProjection();

			printScore(250, 250, "Confirm Reset: press Y");

			
			resetPerspectiveProjection();
			glEnable(GL_LIGHTING);
			glutSwapBuffers();
		}
	}

	if (attempts > 0) {
		if (key == 'y' || key == 'Y') {
			reset = 0;
			glutTimerFunc(35,skyRotation,1);
			attempts = 10;
			points = 0;
			glutPostRedisplay();
		}
	}

	if (attempts < 1) {
		if (key == 'r' || key == 'R') {
			attempts = 10;
			points = 0;
			glutPostRedisplay();
		}
	}
}

//printing instructions on how to play to the console
void instructions() {
	std::cout << "INSTRUCTIONS TO PLAY ";
	std::cout << "\n";
	std::cout << "____________________________________ ";
	std::cout << "\n";
	std::cout << "\n";
	std::cout << " - Use arrow keys to aim ";
	std::cout << "\n";
	std::cout << " - Press 'Z' or 'Space' to shoot "; 
	std::cout << "\n";
	std::cout << " - Press 'R' to reset the game";
}

int main(int argc, char* argv[]) {

	// Initialize GLUT
	glutInit(&argc, argv);
	// Set up some memory buffers for our display
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	// Set the window size
	glutInitWindowSize(1280, 720);
	// Create the window with the title "Football Game"
	glutCreateWindow("Football Game");
	// Bind the two functions (above) to respond when necessary
	glutReshapeFunc(reshape);
	SetupRC();
	instructions();
	glutDisplayFunc(RenderScene);
	
	glutSpecialFunc(SpecialKeys);
	glutKeyboardFunc(keyInput);
	
	glutTimerFunc(35, skyRotation, 1);

	
	glutMainLoop();



	return 0;
}

