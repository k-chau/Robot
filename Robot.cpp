#include <math.h>			// For math routines (such as sqrt & trig).
#include <stdio.h>
#include <stdlib.h>		    // For the "exit" function
#include <GL/glut.h>		// OpenGL Graphics Utility Library
#include "Robot.h"
#include "RgbImage.h"


// Not available in the header file, but needed anyway.
#define GL_LIGHT_MODEL_COLOR_CONTROL      0x81F8
#define GL_SINGLE_COLOR                   0x81F9
#define GL_SEPARATE_SPECULAR_COLOR        0x81FA
#define PI								  3.14159

GLenum jumping = GL_FALSE;
GLenum falling = GL_FALSE;

GLfloat xPos = 0.0;
GLfloat yPos = 1.0;
GLfloat zPos = 0.0;

GLfloat yEarth = 0.0;

GLfloat xEyes = 0.0;
GLfloat yEyes = 0.0;
GLfloat zEyes = 0.0;

const int NumLoadedTextures = 7;
const GLfloat g = -0.0005f;                      // Gravity of space

static GLuint textureName[NumLoadedTextures];		// Holds OpenGL's internal texture names (not filenames)
static GLfloat movement[] = { 0.0, 0.0, 0.0, 0.0 };

char* filenames[NumLoadedTextures] = {
	"stars.bmp",
	"benderbody.bmp",
	"earth.bmp",
	"benderface.bmp",
	"bendercolor.bmp",
	"bender_arms_and_legs.bmp",
	"galaxy.bmp"
};

// The next global variable controls the animation's state and speed.
float RotateAngle = 0.0f;		// Angle in degrees of rotation around y-axis
float Azimuth = 0.0;			// Rotated up or down by this amount
float AngleStepSize = 3.0f;		// Step three degrees at a time
const float AngleStepMax = 10.0f;
const float AngleStepMin = 0.1f;
float DistanceAway = 20.0f;
float MinDistance = 5.0f;
float MaxDistance = 80.0f;
float DistanceStepSize = 0.1f;
float yVel = 0.0f;


// Some global state variables
int MeshCount = 20;				// The mesh resolution for the mushroom top
int WireFrameOn = 1;
int RoboEyesOn = 0;             // 1 = Robot's view

bool UseMipmapping = true;

// Light controls
float GlobalAmbient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
bool Light0On = true;
bool Light1On = true;
bool Light2On = true;
bool SpotlightOn = false;
float Light0AmbientDiffuse[4] = { 1.0f, 1.0f, 0.1f, 1.0f };
float Light0Specular[4] = { 0.7f, 0.7f, 0.1f, 1.0f };
float Light1AmbientDiffuse[4] = { 1.0, 1.0, 1.0, 1.0 };
float Light1Specular[4] = { 1.0, 1.0, 1.0, 1.0 };
float Light2AmbientDiffuse[4] = { 0.2f, 0.4f, 0.8f, 1.0f };
float Light2Specular[4] = { 0.2f, 0.4f, 0.8f, 1.0f };
float SpotlightAmbientDiffuse[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
float SpotlightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float Light0Pos[4] = { -5.0, 5.0, 0.0, 1.0 };
float Light1Pos[4] = { 0.0, 5.0, 0.0, 1.0 };
float Light2Pos[4] = { 5.0, 5.0, 0.0, 1.0 };
float SpotlightPos[4] = { -2.5f, 7.0f, 0.0f, 1.0f };

float blackColor[4] = { 0.0, 0.0, 0.0, 1.0 };
float whiteColor[4] = { 1.0, 1.0, 1.0, 1.0 };

float dir[3] = { 0.0f, -1.0f, 0.0f };

// Angle of rotation for the camera direction
float angle = 0.0f;

// Actual vector representing the camera's direction
float lx = 0.0f;
float lz = -1.0f;
float fraction = 0.1f;

// Material colors
float SpecularExponent = 90;
float PlaneBaseAmbientDiffuse[4] = { 0.5f, 0.2f, 0.2f, 1.0 };
float UnderTextureAmbientDiffuse[4] = { 1.0, 1.0, 1.0, 1.0 };



/**
 * Read a texture map from a BMP bitmap file.
 */
void loadTextureFromFile(char *filename) {
	RgbImage theTexMap(filename);
	// Pixel alignment: each row is word aligned (aligned to a 4 byte boundary)
	//    Therefore, no need to call glPixelStore( GL_UNPACK_ALIGNMENT, ... );

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Use bilinear interpolation between texture pixels.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	if (!UseMipmapping) {
		// This sets up the texture may without mipmapping
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, theTexMap.GetNumCols(), theTexMap.GetNumRows(),
			0, GL_RGB, GL_UNSIGNED_BYTE, theTexMap.ImageData());
	}
	else {
		// Keep the next line for best mipmap linear and bilinear interpolation.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, theTexMap.GetNumCols(), theTexMap.GetNumRows(),
			GL_RGB, GL_UNSIGNED_BYTE, theTexMap.ImageData());
	}

}

/** 
 * glutKeyboardFunc is called below to set this function to handle
 *		all "normal" key presses.
 */
void myKeyboardFunc(unsigned char key, int x, int y) {
	switch (key) {
	case 'a':
		if (!RoboEyesOn)
			movement[0] -= 3.0f;
		break;
	case 's':
		xPos += cosf(90.0f * PI / 180.0f + movement[0] * PI / 180.0f);
		zPos -= sinf(90.0f * PI / 180.0f + movement[0] * PI / 180.0f);
		(xPos > 4.8f) ? xPos = 4.8f : (xPos < -4.8f ? xPos = -4.8f : xPos = xPos); // Set boundaries
		(zPos > 5.0f) ? zPos = 5.0f : (zPos < -4.8f ? zPos = -4.8f : zPos = zPos);
		break;
	case 'd':
		if (!RoboEyesOn)
			movement[0] += 3.0f;
		break;
	case 'w':
		xPos -= cosf(90.0f * PI / 180.0f + movement[0] * PI / 180.0f);
		zPos += sinf(90.0f * PI / 180.0f + movement[0] * PI / 180.0f);
		(xPos > 4.8f) ? xPos = 4.8f : (xPos < -4.8f ? xPos = -4.8f : xPos = xPos); // Set boundaries
		(zPos > 4.8f) ? zPos = 4.8f : (zPos < -4.8f ? zPos = -4.8f : zPos = zPos);
		break;
	case 'e':                                                // Turn on to robot's persepective
		RoboEyesOn = 1 - RoboEyesOn; 
		break;
	case 'q':                                                // Reset
		xPos = zPos = movement[0] = movement[1] = movement[2] = movement[3] = 0.0f;
		xEyes = zEyes = 0.0f;
		yPos = 1.0f;
		break;
	case 'f':
		WireFrameOn = 1 - WireFrameOn;
		if (WireFrameOn) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);		// Just show wireframes
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);		// Show solid polygons
		}
		break;
	case 'H':                                               // Move left arm backward
		movement[2] += 5.0f;
		if (movement[2] > 25.0f)
			movement[2] = 25.0f;
		break;
	case 'h':                                               // Move left arm forward
		movement[2] -= 5.0f;
		if (movement[2] < -40.0f)
			movement[2] = -40.0f;
		break;
	case 'J':                                               // Move right arm backward
		movement[3] += 5.0f;
		if (movement[3] > 25.0f)
			movement[3] = 25.0f;
		break;
	case 'j':                                               // Move right arm forward
		movement[3] -= 5.0f;
		if (movement[3] < -40.0f)
			movement[3] = -40.0f;
		break;
	case 'R':
		AngleStepSize *= 1.5;
		if (AngleStepSize>AngleStepMax) {
			AngleStepSize = AngleStepMax;
		}
		break;
	case 'r':
		AngleStepSize /= 1.5;
		if (AngleStepSize<AngleStepMin) {
			AngleStepSize = AngleStepMin;
		}
		break;	
	case 'T':                                               // Turn the head
		movement[1] += 5.0f;
		if (movement[1] > 60.0f)
			movement[1] = 60.0f;
		break;
	case 't':                                               // Turn the head
		movement[1] -= 5.0f;
		if (movement[1] < -60.0f)
			movement[1] = -60.0f;
		break;
	case 'b':
		UseMipmapping ? UseMipmapping = false : UseMipmapping = true;
		glBindTexture(GL_TEXTURE_2D, textureName[0]);
		loadTextureFromFile(filenames[0]);
		glBindTexture(GL_TEXTURE_2D, textureName[3]);
		loadTextureFromFile(filenames[3]);
		glBindTexture(GL_TEXTURE_2D, textureName[6]);
		loadTextureFromFile(filenames[6]);
		break;
	case '0':
		SpotlightOn = !SpotlightOn;
		if (SpotlightOn) {
			glEnable(GL_LIGHT3);
		}
		else {
			glDisable(GL_LIGHT3);
		}
		break;
	case '1':
		Light0On = !Light0On;
		if (Light0On) {
			glEnable(GL_LIGHT0);
		}
		else {
			glDisable(GL_LIGHT0);
		}
		break;
	case '2':
		Light1On = !Light1On;
		if (Light1On) {
			glEnable(GL_LIGHT1);
		}
		else {
			glDisable(GL_LIGHT1);
		}
		break;
	case '3':
		Light2On = !Light2On;
		if (Light2On) {
			glEnable(GL_LIGHT2);
		}
		else {
			glDisable(GL_LIGHT2);
		}
		break;
	case '+':
		MoveCloser();
		break;
	case '-':
		MoveAway();
		break;
	case 32:    // Spacebar key
		startJump();
		break;
	case 27:	// Escape key
		exit(1);
	}

	glutPostRedisplay();
}

/** 
 * glutSpecialFunc is called below to set this function to handle
 *		all "special" key presses.  See glut.h for the names of
 *		special keys.
 */
void mySpecialKeyFunc(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		if (!RoboEyesOn) {
			Azimuth += AngleStepSize;
			if (Azimuth > 80.0f) {
				Azimuth = 80.0f;
			}
		}
		break;
	case GLUT_KEY_DOWN:
		if (!RoboEyesOn) {
			Azimuth -= AngleStepSize;
			if (Azimuth < -80.0f) {
				Azimuth = -80.0f;
			}
		}
		break;
	case GLUT_KEY_LEFT:
		if (RoboEyesOn) {
			movement[1] -= 0.1f;
			if (movement[1] < -180.0f)
				movement[1] = -180.0f;
			lx = sin(movement[1]);
			lz = -cos(movement[1]);
		} 
		else {
			RotateAngle += AngleStepSize;
			if (RotateAngle > 180.0f) {
				RotateAngle -= 360.0f;
			}
		}
		break;
	case GLUT_KEY_RIGHT:
		if (RoboEyesOn) {
			movement[1] += 0.1f;
			if (movement[1] > 180.0f)
				movement[1] = 180.0f;
			lx = sin(movement[1]);
			lz = -cos(movement[1]);
		} 
		else {
			RotateAngle -= AngleStepSize;
			if (RotateAngle < -180.0f) {
				RotateAngle += 360.0f;
			}
		}
		break;
	case GLUT_KEY_PAGE_UP:
		MoveCloser();
		break;
	case GLUT_KEY_PAGE_DOWN:
		MoveAway();
		break;
	}

	glutPostRedisplay();
}

/**
 * Set the lighting and positions.
 */
void moodLighting() {
	// Set lights' positions (directions)
	glLightfv(GL_LIGHT0, GL_POSITION, Light0Pos);
	glLightfv(GL_LIGHT1, GL_POSITION, Light1Pos);
	glLightfv(GL_LIGHT2, GL_POSITION, Light2Pos);
	glLightfv(GL_LIGHT3, GL_POSITION, SpotlightPos);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, blackColor);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, blackColor);

	if (Light0Pos[3] != 0.0f && Light0On) {
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, Light0AmbientDiffuse);
	}
	if (Light1Pos[3] != 0.0f && Light1On) {
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, Light1AmbientDiffuse);
	}
	if (Light2Pos[3] != 0.0f && Light2On) {
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, Light2AmbientDiffuse);
	}
	if (SpotlightPos[3] != 0.0f && SpotlightOn) {
		glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, 20.0f);
		glLightf(GL_LIGHT3, GL_SPOT_EXPONENT, 5.0f);
		glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, dir);
	}
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, blackColor);
}

/**
 * Zoom in
 */
void MoveCloser() {
	DistanceAway = DistanceAway - DistanceStepSize;
	if (DistanceAway < MinDistance) {
		DistanceAway = MinDistance;
	}
}

/**
 * Zoom out
 */
void MoveAway() {
	DistanceAway = DistanceAway + DistanceStepSize;
	if (DistanceAway > MaxDistance) {
		DistanceAway = MaxDistance;
	}
}

/**
 * Set the robot's eye camera.
 */
void roboEyes() {
	xEyes = xPos;
	yEyes = yPos + 0.5f;
	zEyes = zPos;
	
	(xEyes > 4.8f) ? xEyes = 4.8f : (xEyes < -4.8f ? xEyes = -4.8f : xEyes = xEyes); 
	(zEyes > 4.8f) ? zEyes = 4.8f : (zEyes < -4.8f ? zEyes = -4.8f : zEyes = zEyes);
}

/**
 * Checks if Robot can jump.
 */
void startJump() {
	if (!jumping && !falling) {
		jumping = GL_TRUE;
		yVel = 0.05f;
	}
}

/**
 * Detects and animates Robot to jump (ascend and descend).
 */
void jump() {
	if (jumping) {                 // Entered jump phase
		if (yVel > 0.0f && yPos <= 5.0f && !detectCollision()) {
			yVel += g;
			yPos += yVel;
		} else {
			jumping = GL_FALSE;
			falling = GL_TRUE;
			yVel = 0.0f;
		}
	}
	
	if (falling) {                // Enters falling phase
		if (yPos >= 1.0f) {
			yVel -= g;
			yPos -= yVel;
		} else {			
			yVel = 0.0f;
			yPos = 1.0f;
			jumping = GL_FALSE;
			falling = GL_FALSE;		
		}
	}
}

/**
 * Render and animate the star wall
 */
void starWall() {
	// Cause backfaces to not be drawn
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);	

	// Draw the back wall (textured galaxy wall).
	glBindTexture(GL_TEXTURE_2D, textureName[6]);	// Texture 6 is active now
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);		// Same normal for the next four vertices.
	glTexCoord2f(0.0, 0.0);			// Texture coordinates for the back wall vertex
	glVertex3f(-4.0, 0.0, -4.0);
	glTexCoord2f(1.0, 0.0);			// Texture coordinates for the back wall vertex
	glVertex3f(4.0, 0.0, -4.0);
	glTexCoord2f(1.0, 1.0);			// Texture coordinates for the back wall vertex
	glVertex3f(4.0, 4.0, -4.0);
	glTexCoord2f(0.0, 1.0);			// Texture coordinates for the back wall vertex
	glVertex3f(-4.0, 4.0, -4.0);
	glEnd();

	// Draw the left side wall
	glBindTexture(GL_TEXTURE_2D, textureName[0]);	// Texture 0 is active now
	glBegin(GL_QUADS);
	glNormal3f(1.0, 0.0, 0.0);		// Same normal for the next four vertices.
	glTexCoord2f(0.0, 0.0);			// Texture coordinates for the side wall vertex
	glVertex3f(-4.0, 0.0, -4.0);
	glTexCoord2f(1.0, 0.0);			// Texture coordinates for the side wall vertex
	glVertex3f(-4.0, 4.0, -4.0);
	glTexCoord2f(1.0, 1.0);			// Texture coordinates for the side wall vertex
	glVertex3f(-4.0, 4.0, 4.0);
	glTexCoord2f(0.0, 1.0);			// Texture coordinates for the side wall vertex
	glVertex3f(-4.0, 0.0, 4.0);
	glEnd();

	// Draw the right side wall (textured star wall)
	glBegin(GL_QUADS);
	glNormal3f(-1.0, 0.0, 0.0);		// Same normal for the next four vertices.
	glTexCoord2f(0.0, 0.0);			// Texture coordinates for the side wall vertex
	glVertex3f(4.0, 0.0, -4.0);
	glTexCoord2f(1.0, 0.0);			// Texture coordinates for the side wall vertex
	glVertex3f(4.0, 0.0, 4.0);
	glTexCoord2f(1.0, 1.0);			// Texture coordinates for the side wall vertex
	glVertex3f(4.0, 4.0, 4.0);
	glTexCoord2f(0.0, 1.0);			// Texture coordinates for the side wall vertex
	glVertex3f(4.0, 4.0, -4.0);
	glEnd();

	//Draw the front wall
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, -1.0);		// Same normal for the next four vertices.
	glTexCoord2f(0.0, 0.0);			// Texture coordinates for the front wall vertex
	glVertex3f(4.0, 0.0, 4.0);
	glTexCoord2f(1.0, 0.0);			// Texture coordinates for the front wall vertex
	glVertex3f(-4.0, 0.0, 4.0);
	glTexCoord2f(1.0, 1.0);			// Texture coordinates for the front wall vertex
	glVertex3f(-4.0, 4.0, 4.0);
	glTexCoord2f(0.0, 1.0);			// Texture coordinates for the front wall vertex
	glVertex3f(4.0, 4.0, 4.0);
	glEnd();

	glDisable(GL_CULL_FACE);
}

/**
 * Render and animate the star floor
 */
void starFloor() {
	// Draw the base plane 
	glBindTexture(GL_TEXTURE_2D, textureName[0]);	// Texture 0 is active now
	glBegin(GL_QUADS);
	glNormal3f(0.0, 1.0, 0.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-4.0, 0.0, 4.0);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(4.0, 0.0, 4.0);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(4.0, 0.0, -4.0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-4.0, 0.0, -4.0);
	glEnd();
}

/**
 * Checks if robot is colliding with the Earth.
 */
bool detectCollision() {
	bool xCollide = false;
	bool yCollide = false;
	bool zCollide = false;
	
	for (int i = -1; i < MeshCount - 1; i++) {
		float theta = (((float)((i + 1) % MeshCount)) / (float)MeshCount)*2.0f*PI;
		for (int j = 0; j < MeshCount; j++) {
			float phi = ((((float)j) / (float)MeshCount) - 0.5f)*PI;
			float x = -sinf(theta)*cosf(phi);
			float y = sinf(phi);
			float z = -cosf(theta)*cosf(phi);

			if (!xCollide) {
				if (xPos >= -3.0f)
					xCollide = xPos - 0.378f <= (x - 2.5f);
				else { xCollide = xPos + 0.378f >= (x - 3.5f); }
			}
			if (!yCollide)
				yCollide = yPos + 0.5f >= (y + 3.00f);
			if (!zCollide) {
				if (zPos >= 0.0f)
					zCollide = (zPos - 0.378f <= z);
				else { zCollide = zPos + 0.378f >= z; }
			}
		}
	}

	if (xCollide && yCollide && zCollide)                // Check if coordinates collided
		return GL_TRUE;

	return GL_FALSE;
}

/**
 * drawScene() handles the animation and the redrawing of the
 *		graphics window contents.
 */
void drawScene(void) {
	// Clear the rendering window
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Rotate the image
	glMatrixMode(GL_MODELVIEW);			// Current matrix affects objects positions
	glLoadIdentity();						// Initialize to the identity

	// Set perspective
	if (RoboEyesOn) {
		roboEyes();
		gluLookAt(xEyes, yEyes, zEyes + sinf(movement[0]), xEyes + lx, yEyes, zEyes + lz, 0.0, 1.0, 0.0);
	}
	else {
		glTranslatef(0.0, -1.0, -DistanceAway);	    // Translate from origin (in front of viewer)
		glRotatef(Azimuth, 1.0, 0.0, 0.0);			// Set Azimuth angle
		glRotatef(RotateAngle, 0.0, 1.0, 0.0);		// Rotate around y-axi
	}

	// Set lights
	moodLighting();
	
	// Material and texture mode
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, UnderTextureAmbientDiffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, whiteColor);

	// Draw the wall 
	starWall();

	// Draw the first sphere
	glBindTexture(GL_TEXTURE_2D, textureName[2]);	// Texture 1 is active now
	glPushMatrix();
	glTranslatef( -2.5, 3.0, 0.0 );		
	myDrawSphere( true );				
	glPopMatrix();

	// Render the Robot.
	glPushMatrix();
	frankenstein();
	glPopMatrix(); 

	// Material and texture mode
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Draw the floor
	starFloor();

	jump();

	glDisable(GL_TEXTURE_2D);

	// Flush the pipeline, swap the buffers
	glFlush();
	glutSwapBuffers();

	glutPostRedisplay();
}

/**
 * Cover the Robot's holes.
 */
void drawCaps(float radius, float height) {
	// Draw the two end faces of the cylinder
	glBegin( GL_TRIANGLE_FAN );
	glNormal3f( 0.0, 1.0, 0.0 );
	glVertex3f( 0, height*0.5f, 0 );
	for ( int i=0; i<=MeshCount; i++ ) {
		float theta = 2.0f*3.14159f*((float)(i%MeshCount))/(float)MeshCount;
		glVertex3f( -radius*sinf(theta), height*0.5f, radius*cosf(theta) );
	}
	glEnd();
	glBegin( GL_TRIANGLE_FAN );
	glNormal3f( 0.0, -1.0, 0.0 );
	glVertex3f( 0, -height*0.5f, 0 );
	for ( int i=0; i<=MeshCount; i++ ) {
		float theta = 2.0f*3.14159f*((float)(i%MeshCount))/(float)MeshCount;
		glVertex3f( radius*sinf(theta),-height*0.5f , radius*cosf(theta) );

	}
	glEnd();
}

/**
 * drawCylinder() handles the creation of the Robot's cylindrical parts
 */
void drawCylinder(float radius, float height) {
	for (int i = -1; i<MeshCount-1; i++) {
		float theta1 = 2.0f*PI*((float)i) / (float)MeshCount;
		float theta2 = 2.0f*PI*((float)((i + 1) % MeshCount)) / (float)MeshCount;
		float z1 = radius*cosf(theta1);
		float z2 = radius*cosf(theta2);
		float x1 = radius*sinf(theta1);
		float x2 = radius*sinf(theta2);
		int ii = i;

		glBegin(GL_QUAD_STRIP);
		for (int j = MeshCount; j >= 0; j--) {
			float s2 = (theta2) / (2.0f * PI) + cosf(200.0f);
			float s1 = (theta1) / (2.0f * PI) + cosf(200.0f) ;
			float y = -0.5f*height + ((float)j) / (float)MeshCount*height;
			float t = ((y / 2.0f) + (height / 2.0f)) / height;
			glNormal3f(x2, 0, z2);
			glTexCoord2f(s2, t);		// The texture coordinates are already here.
			glVertex3f(x2, y, z2);
			glNormal3f(x1, 0, z1);
			glTexCoord2f(s1, t);		// The texture coordinates are already here.
			glVertex3f(x1, y, z1);
		}
	glEnd();
	}
}

/**
 * frankenstein() handles putting together the animation to render a robot.
 */
void frankenstein() {
	glScalef(0.75f, 0.75f, 0.75f);
	glTranslatef(xPos, yPos, zPos);
	glRotatef(movement[0], 0.0f, 1.0f, 0.0f);	
	glBindTexture(GL_TEXTURE_2D, textureName[1]);	// Texture 1 is active now
	glPushMatrix();
	glTranslatef(0.0f, 0.2f, 0.0f);
	drawCylinder(0.405f, 1.25f);                    // Draw Robo's body
	drawCaps(0.405f, 1.25f);
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, textureName[3]);   // Texture 3 is active now
	glPushMatrix();
	glTranslatef(0.0f, 1.15f, 0.0f);
	glRotatef(movement[1], 0.0f, 1.0f, 0.0f);
	drawCylinder(0.25f, 0.65f);                     // Draw head
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, textureName[4]);   // Texture 4 is active now
	glPushMatrix();
	glScalef(0.25f, 0.25f, 0.25f);
	glTranslatef(0.0f, 5.7f, 0.0f);
	glRotatef(movement[1], 0.0f, 1.0f, 0.0f);
	myDrawSphere(true);
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, textureName[5]);    // Texture 5 is active now
	glPushMatrix();
	glTranslatef(-0.2f, -0.675f, 0.0f);
	drawCylinder(0.099f, 0.5f);                      // Draw right leg
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, textureName[4]);    // Texture 4 is active now
	glPushMatrix();
	glTranslatef(-0.2f, -0.95f, 0.0f);
	drawCylinder(0.135f, 0.0875f);                   // Draw right foot
	drawCaps(0.135f, 0.0875f);
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, textureName[5]);    // Texture 5 is active now
	glPushMatrix();
	glTranslatef(0.2f, -0.675f, 0.0f);
	drawCylinder(0.099f, 0.5f);                      // Draw left leg
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, textureName[4]);    // Texture 4 is active now
	glPushMatrix();
	glTranslatef(0.2f, -0.95f, 0.0f);
	drawCylinder(0.135f, 0.0875f);                   // Draw left foot
	drawCaps(0.135f, 0.0875f);
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, textureName[5]);    // Texture 5 is active now
	glPushMatrix();
	glTranslatef(0.475f, 0.35f, 0.05f);
	glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
	glRotatef(270.0f, 0.0f, 1.0f, 0.0f);
	glRotatef(movement[2], 0.0f, 0.0f, 1.0f);
	drawCylinder(0.099f, 0.75f);                     // Draw left arm
	drawCaps(0.099f, 0.75f);
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, textureName[5]);    // Texture 5 is active now
	glPushMatrix();
	glTranslatef(-0.475f, 0.35f, 0.05f);
	glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
	glRotatef(270.0f, 0.0f, 1.0f, 0.0f);
	glRotatef(movement[3], 0.0f, 0.0f, 1.0f);
	drawCylinder(0.099f, 0.75f);                     // Draw right arm
	drawCaps(0.099f, 0.75f);
	glPopMatrix();
}

/**
 * Parameter texCoordSpherical controls whether spherical or cylindrical texture coordinates are used.
 */
void myDrawSphere ( bool texCoordSpherical ) {
	// The sphere is drawn with the center of the texture map at the front of the sphere.
	for ( int i=-1; i<MeshCount-1; i++ ) {
		glBegin(GL_TRIANGLE_STRIP);
		// Draw i-th triangle strip of sphere

		// Draw south pole vertex
		glNormal3f(0.0, -1.0, 0.0 );
		glTexCoord2f( (i+0.5f)/(float)MeshCount, 0.0 );	// Texture coord's at poles to be discussed in class
		glVertex3f( 0.0, -1.0, 0.0 );

		// PROJECT 5: MUST ADD TEXTURE COORDINATES FOR VERTICES BELOW
		float thetaLeft = (((float)i)/(float)MeshCount)*2.0f*PI;
		float thetaRight = (((float)((i+1)%MeshCount))/(float)MeshCount)*2.0f*PI;
		for ( int j=0; j<MeshCount; j++ ) {
			float phi = ((((float)j)/(float)MeshCount)-0.5f)*PI;
			float x = -sinf(thetaRight)*cosf(phi);
			float y = sinf(phi);
			float z = -cosf(thetaRight)*cosf(phi);
			float s = thetaRight / (2.0f * PI);
			float spherical = (phi / PI) + 0.5f;
			float cylindrical = (y / 2) + 0.5f;
			glNormal3f( x, y, z);
			texCoordSpherical ? glTexCoord2f(s, spherical) : glTexCoord2f(s, cylindrical);
			glVertex3f( x, y, z);
			x = -sinf(thetaLeft)*cosf(phi);
			z = -cosf(thetaLeft)*cosf(phi);
			s = thetaLeft / (2.0f * PI);
			glNormal3f( x, y, z);
			texCoordSpherical ? glTexCoord2f(s, spherical) : glTexCoord2f(s, cylindrical);
			glVertex3f( x, y, z);
		}
		
		// Draw north pole vertex
		glNormal3f(0.0, 1.0, 0.0 );
		glTexCoord2f( (i+0.5f)/(float)MeshCount, 1.0 );	// Texture coord's at poles to be discussed in class
		glVertex3f( 0.0, 1.0, 0.0 );
		
		glEnd();		// End of one triangle strip going up the sphere.
	}
}

/**
 * Initialize OpenGL's rendering modes
 */
void initRendering() {
	glEnable(GL_DEPTH_TEST);	// Depth testing must be turned on

	// Possibly turn on wireframe mode.
	if (WireFrameOn) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);		// Just show wireframes
	}

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, GlobalAmbient);
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, SpecularExponent);
	if (Light0On) {
		glEnable(GL_LIGHT0);
	}
	else {
		glDisable(GL_LIGHT0);
	}
	glLightfv(GL_LIGHT0, GL_AMBIENT, Light0AmbientDiffuse);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, Light0AmbientDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, Light0Specular);

	if (Light1On) {
		glEnable(GL_LIGHT1);
	}
	else {
		glDisable(GL_LIGHT1);
	}
	glLightfv(GL_LIGHT1, GL_AMBIENT, Light1AmbientDiffuse);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, Light1AmbientDiffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, Light1Specular);

	if (Light2On) {
		glEnable(GL_LIGHT2);
	}
	else {
		glDisable(GL_LIGHT2);
	}
	glLightfv(GL_LIGHT2, GL_AMBIENT, Light2AmbientDiffuse);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, Light2AmbientDiffuse);
	glLightfv(GL_LIGHT2, GL_SPECULAR, Light2Specular);

	if (SpotlightOn) {
		glEnable(GL_LIGHT3);
	}
	else {
		glDisable(GL_LIGHT3);
	}
	glLightfv(GL_LIGHT3, GL_AMBIENT, SpotlightAmbientDiffuse);
	glLightfv(GL_LIGHT3, GL_DIFFUSE, SpotlightAmbientDiffuse);
	glLightfv(GL_LIGHT3, GL_SPECULAR, SpotlightSpecular);

	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
	glShadeModel(GL_SMOOTH);		// Use smooth shading

}

/**
 * Called when the window is resized
 *		w, h - width and height of the window in pixels.
 */
void resizeWindow(int w, int h) {
	double aspectRatio;

	// Define the portion of the window used for OpenGL rendering.
	glViewport(0, 0, w, h);	// View port uses whole window

							// Set up the projection view matrix: perspective projection
							// Determine the min and max values for x and y that should appear in the window.
							// The complication is that the aspect ratio of the window may not match the
							//		aspect ratio of the scene we want to view.
	w = (w == 0) ? 1 : w;
	h = (h == 0) ? 1 : h;
	aspectRatio = (double)w / (double)h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(25.0, aspectRatio, 5.0, MaxDistance + 10.0);

}

/**
 * Main routine
 * Set up OpenGL, define the callbacks and start the main loop.
 */
int main(int argc, char** argv) {
	glutInit(&argc, argv);

	// We're going to animate it, so double buffer 
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	// Window position (from top corner), and size (width% and hieght)
	glutInitWindowPosition(10, 60);
	glutInitWindowSize(640, 360);
	glutCreateWindow("Robot");

	// Initialize OpenGL as we like it..
	initRendering();

	// Set up callback functions for key presses
	glutKeyboardFunc(myKeyboardFunc);			// Handles "normal" ascii symbols
	glutSpecialFunc(mySpecialKeyFunc);		// Handles "special" keyboard keys

	// Set up the callback function for resizing windows
	glutReshapeFunc(resizeWindow);

	// Call this for background processing
	// glutIdleFunc( myIdleFunction );

	// call this whenever window needs redrawing
	glutDisplayFunc(drawScene);

	fprintf(stdout, "Arrow keys control viewpoint.\n");
	fprintf(stdout, "Press Page-Up/Page-Down or +/- to move closer to or away from the scene.\n");
	fprintf(stdout, "Press \"w\" to move the Robot forward.\n");
	fprintf(stdout, "Press \"s\" to move the Robot backward.\n");
	fprintf(stdout, "Press \"a\" to turn the Robot left.\n");
	fprintf(stdout, "Press \"d\" to turn the Robot right.\n");
	fprintf(stdout, "Press \"e\" to toggle perspective (default: viewer).\n");
	fprintf(stdout, "Press \"b\" to toggle mip-mapping on the stars.\n");
	fprintf(stdout, "Press \"f\" to toggle wireframe mode.\n");
	fprintf(stdout, "Press \"q\" to return to the origin.\n");
	fprintf(stdout, "Press \"H\" or \"h\" to move the left arm backward or forward (respectively).\n");
	fprintf(stdout, "Press \"J\" or \"j\" to move the right arm backward or forward (respectively).\n");
	fprintf(stdout, "Press \"R\" or \"r\" to increase or decrease rate of movement (respectively).\n");
	fprintf(stdout, "Press \"T\" or \"t\" to turn the Robot's head right or left (respectively).\n");
	fprintf(stdout, "Press \"1\", \"2\", \"3\" to toggle the three lights.\n");
	fprintf(stdout, "Press \"0\" to toggle the spotlight.\n");
	fprintf(stdout, "Press \"SPACEBAR\" to jump.\n");
	fprintf(stdout, "Press \"ESCAPE\" to exit.\n");

	// Load the three texture maps.
	glGenTextures(NumLoadedTextures, textureName);	// Load three (internal) texture names into array
	for (int i = 0; i<NumLoadedTextures; i++) {
		glBindTexture(GL_TEXTURE_2D, textureName[i]);	// Texture #i is active now
		loadTextureFromFile(filenames[i]);			// Load texture #i
	}

	// Start the main loop.  glutMainLoop never returns.
	glutMainLoop();

	return(0);	// This line is never reached.
}
