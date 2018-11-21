// Function prototypes for WireFrameScene.cpp

void myKeyboardFunc(unsigned char key, int x, int y);
void mySpecialKeyFunc(int key, int x, int y);

void moodLighting();

void drawScene(void);
void myDrawSphere(bool texCoordSpherical);

void starWall();
void starFloor();

void drawCylinder(float radius, float height);
void drawCaps(float radius, float height);
void frankenstein(void);

void initRendering();
void resizeWindow(int w, int h);

void MoveCloser();
void MoveAway();

void startJump();
void jump();

void roboEyes();

bool detectCollision();


