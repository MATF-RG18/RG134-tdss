#include<stdio.h>
#include<stdlib.h>
#include<GL/glut.h>
#include<math.h>
#include<time.h>
#include<stdbool.h>


/*MAP PARAMETERS*/
static float window_width = 1600, window_height = 900;
static int matrixSizeX, matrixSizeY;
static int tempX, tempY;
static float gPlaneScaleX = 31, gPlaneScaleY = 31;
static float obstacleChance = 0.08;
static int **M;
static float **M_Obstacle;
static bool fullScreen = false;
// static GLfloat lightPosition[4] = {0, 10, 0, 1};

/*MATRIXES*/
static GLdouble ModelMatrix[16];
static GLdouble ProjectionMatrix[16];
static GLint ViewportMatrix[4];

/*CHARACTER PARAMETERS*/
static float characterDiameter = 0.6;
static float movementVector[3] = {0,0,0};
static float movementSpeed = 0.1;
static float diagonalMovementMultiplier = 0.707107;
static int curWorldX, curWorldY;
static int curMatX, curMatY;
static GLdouble objWinX, objWinY, objWinZ;
static bool keyBuffer[128];     
static int currentRotationX, currentRotationY;
// static float gunBarrelX, gunBarrelY;

static void greska(char* text);
static void DecLevelInit();
static void DecTest();

static void characterMovement();
static void characterShoot();
// static int shoot = 0;

/*MOVEMENT FUNCTIONS*/
static void moveLeft();
static void moveRight();
static void moveUp();
static void moveDown();
static void moveUpLeft();
static void moveUpRight();
static void moveDownLeft();
static void moveDownRight();

static void on_keyPress(unsigned char key, int x, int y);
static void on_keyRelease(unsigned char key, int x, int y);
static void on_reshape(int width, int height);
static void on_display(void);
static void on_mouseMove(int x, int y);
static void on_mouseLeftClick(int button, int state, int x, int y);
//static void on_mouseClick(int button, int state, int x,int y);

void DecPlane(float colorR, float colorG, float colorB);
void DecFloorMatrix(float colorR, float colorG, float colorB, float cubeHeight);

int main(int argc, char **argv)
{
    
    /*Globalno svetlo*/
    GLfloat light_ambient[] = { 0, 0, 0, 1 };
    GLfloat light_diffuse[] = { 0.425, 0.415, 0.4, 1 };
    GLfloat light_specular[] = { 1, 1, 1, 1 };
    GLfloat model_ambient[] = { 0.4, 0.4, 0.4, 1 };
    
    /* Inicijalizuje se GLUT. */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    
    /* Kreira se prozor. */
    glutInitWindowSize(window_width, window_height);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("TDSS - Lvl 1");
 
    /* Registruju se callback funkcije. */
    glutKeyboardFunc(on_keyPress);
    glutKeyboardUpFunc(on_keyRelease);
    glutReshapeFunc(on_reshape);
    glutDisplayFunc(on_display);
    glutPassiveMotionFunc(on_mouseMove);
    glutMotionFunc(on_mouseMove);
    glutMouseFunc(on_mouseLeftClick);
 
    /* Obavlja se OpenGL inicijalizacija. */
    glClearColor(0.75, 0.75, 0.75, 0);
    glEnable(GL_DEPTH_TEST | GL_POLYGON_SMOOTH);
    glLineWidth(2);

    /*Svetlo*/
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, model_ambient);
    
    DecLevelInit();
    //DecTest();
    
    /* Program ulazi u glavnu petlju. */
    glutMainLoop();
 
    return 0;
}

static void on_reshape(int width, int height)
{
    /* Pamte se sirina i visina prozora. */
    window_width = width;
    window_height = height;
}

static void on_display(void)
{
    /* Brise se prethodni sadrzaj prozora. */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
    /* Podesava se viewport. */
    glViewport(0, 0, window_width, window_height);
 
    /* Podesava se projekcija. */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50, window_width/window_height, 1, 50);
 
    /* Podesava se vidna tacka. */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    gluLookAt(0, (matrixSizeX+matrixSizeY)/0.85, (matrixSizeX+matrixSizeY)/3, //Sa pozicijom kamere se jos uvek igram
              0, 0, 1, 
              0, 1, 0);
    
//     glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    DecPlane(0.1, 0.1, 0.1);
    DecFloorMatrix(0.9, 0.9, 0.9, 2);
    
    /*Funkcija za kretanje*/
    characterMovement();
    
    /*gluProject*/
    glGetDoublev(GL_MODELVIEW_MATRIX, ModelMatrix);
    glGetDoublev(GL_PROJECTION_MATRIX, ProjectionMatrix);
    glGetIntegerv(GL_VIEWPORT, ViewportMatrix);
    
    gluProject(movementVector[0], movementVector[1], movementVector[2],
               ModelMatrix, ProjectionMatrix, ViewportMatrix,
               &objWinX, &objWinY, &objWinZ);
    
    
    /*Racunanje rotacije*/
    currentRotationX = tempX - ((int)objWinX-window_width/2);
    currentRotationY = tempY - ((int)objWinY-window_height/2); 
    
    
    glTranslatef(movementVector[0], 1, movementVector[2]);
    glRotatef(atan2(currentRotationX, currentRotationY)*(-180)/M_PI, 0, 1, 0);
    
    /*Inicijalicazija podloge i nivoa*/
    /*Character main*/
    
    
    
    /*Gornja lopta*/
    glPushMatrix();
        glColor3f(0,0,0);
        glTranslatef(0, 0.1, 0);
        glRotatef(90, 1, 0, 0);
        glutSolidSphere(characterDiameter, 20, 20);
    glPopMatrix();
    /*Cilindar*/
    glPushMatrix();
        glColor3f(0,0,0);
        glTranslatef(0, 1.05, 0);
        glRotatef(90, 1, 0, 0);
        gluCylinder(gluNewQuadric(), characterDiameter, characterDiameter, 1, 20, 1);
    glPopMatrix();
    /*Donja lopta*/
    glPushMatrix();
        glColor3f(0,0,0);
        glTranslatef(0, 1, 0);
        glRotatef(90, 1, 0, 0);
        glutSolidSphere(characterDiameter, 20, 20);
    glPopMatrix();

    /*Oruzije*/
    glPushMatrix();
        glDisable(GL_LIGHTING);
        glColor3f(0,0,0);
        glScalef(0.3,0.2,0.7);
        glTranslatef(1, 4, -1);
        glutSolidCube(1);
        glEnable(GL_LIGHTING);
    glPopMatrix();
    /* Nova slika se salje na ekran. */

    glutSwapBuffers();
}

static void on_keyPress(unsigned char key, int x, int y)
{
    switch (key) {
        
        /*Napred - w*/
        case 119:
            keyBuffer[119] = true;
            break;
            
        /*Levo   - a*/
        case 97:
            keyBuffer[97] = true;
            break;
            
        /*Desno  - d*/
        case 100:
            keyBuffer[100] = true;
            break;
            
        /*Nazad  - s*/
        case 115:
            keyBuffer[115] = true;
            break;
            
        case 102:
            if (!fullScreen){
                fullScreen = true;
                glutFullScreen();
            }
            else{
                fullScreen = false;
                glutReshapeWindow(window_width, window_height);
            }
            break;
            
        /* Zavrsava se program. */    
        case 27:
            for (int i=0; i!=matrixSizeX; i++)
                free(M[i]);
            free(M);
            exit(0);
            break;
    }
}

static void on_keyRelease(unsigned char key, int x, int y){
    switch (key) {
        
        /*Napred - w*/
        case 119:
            keyBuffer[119] = false;
            break;
        /*Levo   - a*/
        case 97:
            keyBuffer[97] = false;
            break;
        /*Nazad  - s*/
        case 115:
            keyBuffer[115] = false;
            break;
        /*Desno  - d*/
        case 100:
            keyBuffer[100] = false;
            break;
    }
}

//------------------------F U N K C I J E   K R E T A N J A-------------------------
static void moveLeft(){
    movementVector[0] -= movementSpeed;
}
static void moveRight(){
    movementVector[0] += movementSpeed;
}
static void moveUp(){
    movementVector[2] -= movementSpeed;
}
static void moveDown(){
    movementVector[2] += movementSpeed;
}
static void moveUpLeft(){
    movementVector[2] -= movementSpeed * diagonalMovementMultiplier;
    movementVector[0] -= movementSpeed * diagonalMovementMultiplier;
}
static void moveUpRight(){
    movementVector[2] -= movementSpeed * diagonalMovementMultiplier;
    movementVector[0] += movementSpeed * diagonalMovementMultiplier;
}
static void moveDownLeft(){
    movementVector[2] += movementSpeed * diagonalMovementMultiplier;
    movementVector[0] -= movementSpeed * diagonalMovementMultiplier;
}
static void moveDownRight(){
    movementVector[2] += movementSpeed * diagonalMovementMultiplier;
    movementVector[0] += movementSpeed * diagonalMovementMultiplier;
}

//-------------------------K O N T R O L A   K R E T A N J A-------------------------
void characterMovement(){
    
    int wa, wd;
    float localX, localY;
    
    /*Racunanje koordinata polja u kojem se nalazimo u prostoru:*/
    /*Za X osu*/
    curWorldX = (int)movementVector[0];
    if (curWorldX < 0){
        if (curWorldX % 2 != 0) curWorldX -= 1;
    } 
    else if (curWorldX % 2 != 0) curWorldX += 1;
    
    /*Za Y osu*/
    curWorldY = (int)movementVector[2];
    if (curWorldY < 0){
        if (curWorldY % 2 != 0) curWorldY -= 1;
    } 
    else if (curWorldY % 2 != 0) curWorldY += 1;

    
    /*Racunanje nase pozicije u matrici*/
    curMatX = curWorldX/2 + matrixSizeX/2;
    curMatY = curWorldY/2 + matrixSizeY/2;
    
    /*Izracunavanje za slucaj kada dijagonalno prilazimo prepreci, pa treba odluciti na koju ce nas stranu skrenuti*/
    localX = fmod(movementVector[0] + 1.0, 2);
    if (localX < 0.0) localX = 2.0 + localX;
    localY = fmod((1.0 - movementVector[2]), 2);
    if (localY < 0.0) localY = 2.0 + localY;
    
    if (localX + localY >= 2.0)
        wa = 1;
    else wa = 0;
    
    if ((2.0 - localX) + localY >= 2.0)
        wd = 1;
    else wd = 0;

    
//------------detekcija kolizije za ivice terena------------------
    if (movementVector[2] < (-1)*(matrixSizeY-1))
        keyBuffer[119] = false;
    if (movementVector[0] < (-1)*(matrixSizeX-1))
        keyBuffer[97] = false;
    if (movementVector[0] > (matrixSizeX-1))
        keyBuffer[100] = false;
    if (movementVector[2] > matrixSizeY-1)
        keyBuffer[115] = false;
                   
//---------------detekcija kolizije za prepreke------------------
    /*w+a*/
    if (keyBuffer[119] && keyBuffer[97]){
        if (curMatX-1 >= 0 && curMatY-1 >= 0){
            if (M[curMatX-1][curMatY] == 1){
                if ((M[curMatX][curMatY-1] == 1 && M[curMatX-1][curMatY-1] == 1) || (M[curMatX][curMatY-1] == 1 && M[curMatX-1][curMatY-1] == 0)){
                    if (movementVector[0] > curWorldX - 0.4 && movementVector[2] > curWorldY - 0.4)
                        moveUpLeft();
                    else if (movementVector[0] <= curWorldX - 0.4 && movementVector[2] > curWorldY - 0.4)
                        moveUp();
                    else if (movementVector[0] > curWorldX - 0.4 && movementVector[2] <= curWorldY - 0.4)
                        moveLeft();
                    }
                    else {
                        if (movementVector[0] > curWorldX - 0.4)
                            moveUpLeft();
                        else moveUp();
                    }
                }
                else if (M[curMatX][curMatY-1] == 1){
                    if (movementVector[2] > curWorldY - 0.4)
                        moveUpLeft();
                    else moveLeft();
                }
                else if (M[curMatX-1][curMatY-1] == 1){
                    if (!(movementVector[0] < curWorldX - 0.4 && movementVector[2] < curWorldY - 0.4))
                        moveUpLeft();
                    else {
                        if (wa == 1) moveUp();
                        else if (wa == 0) moveLeft();
                    }
                }
            else moveUpLeft();
        }
        else {
            if (curMatX-1 < 0){
                if (M[curMatX][curMatY-1] == 1){
                        if (movementVector[2] > curWorldY - 0.4)
                            moveUpLeft();
                        else moveLeft();
                }
                else moveUpLeft();
            }
            else if (curMatY-1 < 0){ 
                if (M[curMatX-1][curMatY] == 1){
                    if (movementVector[0] > curWorldX - 0.4)
                        moveUpLeft();
                    else moveUp();
                }
                else moveUpLeft();
            }
            else
                moveUpLeft();
        }
    }
    /*w+d*/
    else if (keyBuffer[119] && keyBuffer[100]){     
        if (curMatX+1 < matrixSizeX && curMatY-1 >= 0){
            if (M[curMatX+1][curMatY] == 1){
                if ((M[curMatX][curMatY-1] == 1 && M[curMatX+1][curMatY-1] == 1) || (M[curMatX][curMatY-1] == 1 && M[curMatX+1][curMatY-1] == 0)){
                    if (movementVector[0] < curWorldX + 0.4 && movementVector[2] > curWorldY - 0.4)
                        moveUpRight();
                    else if (movementVector[0] >= curWorldX + 0.4 && movementVector[2] > curWorldY - 0.4)
                        moveUp();
                    else if (movementVector[0] < curWorldX + 0.4 && movementVector[2] <= curWorldY - 0.4)
                        moveRight();
                    }
                    else {
                        if (movementVector[0] < curWorldX + 0.4)
                            moveUpRight();
                        else moveUp();
                    }
                }
                else if (M[curMatX][curMatY-1] == 1){
                    if (movementVector[2] > curWorldY - 0.4)
                        moveUpRight();
                    else moveRight();
                }
                else if (M[curMatX+1][curMatY-1] == 1){
                    
                    if (!(movementVector[0] > curWorldX + 0.4 && movementVector[2] < curWorldY - 0.4))
                        moveUpRight();
                    else {
                        if (wd == 1) moveUp();
                        else if (wd == 0) moveRight();
                    }
                }
            else moveUpRight();
        }  
        else {
            if (curMatX + 1 >= matrixSizeX){
                if (M[curMatX][curMatY-1] == 1){
                    if (movementVector[2] > curWorldY - 0.4)
                        moveUpRight();
                    else moveRight();
                }
                else moveUpRight();
            } 
            else if (curMatY - 1 < 0){
                if (M[curMatX+1][curMatY] == 1) {
                    if (movementVector[0] < curWorldX + 0.4)
                        moveUpRight();
                    else moveUp();
                }
                else moveUpRight();
            }
            else moveUpRight();
        }

    }
    /*s+a*/
    else if (keyBuffer[115] && keyBuffer[97]){
        if (curMatX-1 >= 0 && curMatY+1 < matrixSizeY){
            if (M[curMatX-1][curMatY] == 1){
                if ((M[curMatX][curMatY+1] == 1 && M[curMatX-1][curMatY+1] == 1) || (M[curMatX][curMatY+1] == 1 && M[curMatX-1][curMatY+1] == 0)){
                    if (movementVector[0] > curWorldX - 0.4 && movementVector[2] < curWorldY + 0.4)
                        moveDownLeft();
                    else if (movementVector[0] <= curWorldX - 0.4 && movementVector[2] < curWorldY + 0.4)
                        moveDown();
                    else if (movementVector[0] > curWorldX - 0.4 && movementVector[2] <= curWorldY - 0.4)
                        moveLeft();
                    }
                    else {
                        if (movementVector[0] > curWorldX - 0.4)
                            moveDownLeft();
                        else moveDown();
                    }
                }
                else if (M[curMatX][curMatY+1] == 1){
                    if (movementVector[2] < curWorldY + 0.4)
                        moveDownLeft();
                    else moveLeft();
                }
                else if (M[curMatX-1][curMatY+1] == 1){
                    if (!(movementVector[0] < curWorldX - 0.4 && movementVector[2] > curWorldY + 0.4))
                        moveDownLeft();
                    else {
                        if (wd == 0) moveDown();
                        else if (wd == 1) moveLeft();
                    }
                }
            else moveDownLeft();
        }
        else {
            if (curMatY+1 >= matrixSizeX && curMatX-1 < 0){
                moveDownLeft();
            }
            else if (curMatY + 1 >= matrixSizeY) {
                if (M[curMatX-1][curMatY] == 1){
                        if (movementVector[0] > curWorldX - 0.4)
                            moveDownLeft();
                        else moveDown();
                }
                else moveDownLeft();
            }
            else if (curMatX - 1 < 0){
                if (M[curMatX][curMatY+1] == 1){
                    if (movementVector[2] < curWorldY + 0.4)
                        moveDownLeft();
                    else moveLeft();
                }
                else moveDownLeft();
            }
            else
                moveDownLeft();
        }
    }
    /*s+d*/
    else if (keyBuffer[115] && keyBuffer[100]){
        if (curMatX+1 < matrixSizeX && curMatY+1 < matrixSizeY){
            if (M[curMatX+1][curMatY] == 1){
                if ((M[curMatX][curMatY+1] == 1 && M[curMatX+1][curMatY+1] == 1) || (M[curMatX][curMatY+1] == 1 && M[curMatX+1][curMatY+1] == 0)){
                    if (movementVector[0] < curWorldX + 0.4 && movementVector[2] < curWorldY + 0.4)
                        moveDownRight();
                    else if (movementVector[0] >= curWorldX + 0.4 && movementVector[2] < curWorldY + 0.4)
                        moveDown();
                    else if (movementVector[0] < curWorldX + 0.4 && movementVector[2] <= curWorldY - 0.4)
                        moveRight();
                    }
                    else {
                        if (movementVector[0] < curWorldX + 0.4)
                            moveDownRight();
                        else moveDown();
                    }
                }
                else if (M[curMatX][curMatY+1] == 1){
                    if (movementVector[2] < curWorldY + 0.4)
                        moveDownRight();
                    else moveRight();
                }
                else if (M[curMatX+1][curMatY+1] == 1){
                    
                    if (!(movementVector[0] > curWorldX + 0.4 && movementVector[2] > curWorldY + 0.4))
                        moveDownRight();
                    else {
                        if (wa == 0) moveDown();
                        else if (wa == 1) moveRight();
                    }
                }
            else moveDownRight();
        }  
        else {
            if (curMatY+1 >= matrixSizeY && curMatX+1 >= matrixSizeX){
                moveDownRight();
            }
            else if (curMatY + 1 >= matrixSizeY) {
                if (M[curMatX+1][curMatY] == 1){
                        if (movementVector[0] < curWorldX + 0.4)
                            moveDownRight();
                        else moveDown();
                }
                else moveDownRight();
            }
            else if (curMatX + 1 >= matrixSizeX){
                if (M[curMatX][curMatY+1] == 1){
                    if (movementVector[2] < curWorldY + 0.4)
                        moveDownRight();
                    else moveRight();
                }
                else moveDownRight();
            }
            else
                moveDownRight();
        }
    }
    /*w,a,s,d*/
    else {
        /*w*/
        if (keyBuffer[119]){
            if (curMatY-1 >= 0){
                if (curMatX-1 >= 0 && curMatX+1 < matrixSizeX){
                    if (M[curMatX][curMatY-1] == 1){
                        if (movementVector[2] > curWorldY - 0.4)
                            moveUp();
                    }
                    else if (M[curMatX-1][curMatY-1] == 1 && M[curMatX+1][curMatY-1] == 1){
                        if (movementVector[2] > curWorldY - 0.4)
                            moveUp();
                        else if (movementVector[0] > curWorldX - 0.55 && movementVector[0] < curWorldX + 0.55)
                            moveUp();
                    }
                    else if (M[curMatX-1][curMatY-1] == 1){
                        if (movementVector[2] > curWorldY - 0.4)
                            moveUp();
                        else if (movementVector[0] > curWorldX - 0.55)
                            moveUp();
                    }
                    else if (M[curMatX+1][curMatY-1] == 1){
                        if (movementVector[2] > curWorldY - 0.4)
                            moveUp();
                        else if (movementVector[0] < curWorldX + 0.55)
                            moveUp();
                    }
                    else if (M[curMatX-1][curMatY] == 1){
                        if (movementVector[2] > curWorldY - 0.4)
                            moveUp();
                        else if (movementVector[0] > curWorldX - 0.55)
                            moveUp();
                    }
                    else if (M[curMatX+1][curMatY] == 1){
                        if (movementVector[2] > curWorldY - 0.4)
                            moveUp();
                        else if (movementVector[0] < curWorldX + 0.55)
                            moveUp();
                    }
                    else moveUp();
                } else if (M[curMatX][curMatY-1] == 1){
                        if (movementVector[2] > curWorldY - 0.4)
                            moveUp();
                }
                else moveUp();
            }
            else moveUp();
        }
        /*a*/
        if (keyBuffer[97]){
            if (curMatX-1 >= 0){
                if (curMatY-1 >= 0 && curMatY+1 < matrixSizeY){
                    if (M[curMatX-1][curMatY] == 1){
                        if (movementVector[0] > curWorldX - 0.4)
                            moveLeft();
                    }
                    else if (M[curMatX-1][curMatY-1] == 1 && M[curMatX-1][curMatY+1] == 1){
                        if (movementVector[0] > curWorldX - 0.4)
                            moveLeft();
                        else if (movementVector[2] > curWorldY - 0.55 && movementVector[2] < curWorldY + 0.55)
                            moveLeft();
                    }
                    else if (M[curMatX-1][curMatY-1] == 1){
                        if (movementVector[0] > curWorldX - 0.4)
                            moveLeft();
                        else if (movementVector[2] > curWorldY - 0.55)
                            moveLeft();
                    }
                    else if (M[curMatX-1][curMatY+1] == 1){
                        if (movementVector[0] > curWorldX - 0.4)
                            moveLeft();
                        else if (movementVector[2] < curWorldY + 0.55)
                            moveLeft();
                    }
                    else if (M[curMatX][curMatY-1] == 1){
                        if (movementVector[0] > curWorldX - 0.4)
                            moveLeft();
                        else if (movementVector[2] > curWorldY - 0.55)
                            moveLeft();
                    }
                    else if (M[curMatX][curMatY+1] == 1){
                        if (movementVector[0] > curWorldX - 0.4)
                            moveLeft();
                        else if (movementVector[2] < curWorldY + 0.55)
                            moveLeft();
                    }
                    else moveLeft();
                } else if (M[curMatX-1][curMatY] == 1){
                        if (movementVector[0] > curWorldX - 0.4)
                            moveLeft();
                }
                else moveLeft();
            }
            else moveLeft();
        }        
        /*s*/
        if (keyBuffer[115]){
            if (curMatY+1 < matrixSizeY){
                if (curMatX-1 >= 0 && curMatX+1 < matrixSizeX){
                    if (M[curMatX][curMatY+1] == 1){
                        if (movementVector[2] < curWorldY + 0.4)
                            moveDown();
                    }
                    else if (M[curMatX-1][curMatY+1] == 1 && M[curMatX+1][curMatY+1] == 1){
                        if (movementVector[2] < curWorldY + 0.4)
                            moveDown();
                        else if (movementVector[0] > curWorldX - 0.55 && movementVector[0] < curWorldX + 0.55)
                            moveDown();
                    }
                    else if (M[curMatX-1][curMatY+1] == 1){
                        if (movementVector[2] < curWorldY + 0.4)
                            moveDown();
                        else if (movementVector[0] > curWorldX - 0.55)
                            moveDown();
                    }
                    else if (M[curMatX+1][curMatY+1] == 1){
                        if (movementVector[2] < curWorldY + 0.4)
                            moveDown();
                        else if (movementVector[0] < curWorldX + 0.55)
                            moveDown();
                    }
                    else if (M[curMatX-1][curMatY] == 1){
                        if (movementVector[2] < curWorldY + 0.4)
                            moveDown();
                        else if (movementVector[0] > curWorldX - 0.55)
                            moveDown();
                    }
                    else if (M[curMatX+1][curMatY] == 1){
                        if (movementVector[2] < curWorldY + 0.4)
                            moveDown();
                        else if (movementVector[0] < curWorldX + 0.55)
                            moveDown();
                    }
                    else moveDown();
                } else if (M[curMatX][curMatY+1] == 1){
                        if (movementVector[2] < curWorldY + 0.4)
                            moveDown();
                }
                else moveDown();
            }
            else moveDown();
        }
        /*d*/
        if (keyBuffer[100]){
            if (curMatX+1 < matrixSizeX){
                if (curMatY-1 >= 0 && curMatY+1 < matrixSizeY){
                    if (M[curMatX+1][curMatY] == 1){
                        if (movementVector[0] < curWorldX + 0.4)
                            moveRight();
                    }
                    else if (M[curMatX+1][curMatY-1] == 1 && M[curMatX+1][curMatY+1] == 1){
                        if (movementVector[0] < curWorldX + 0.4)
                            moveRight();
                        else if (movementVector[2] > curWorldY - 0.55 && movementVector[2] < curWorldY + 0.55)
                            moveRight();
                    }
                    else if (M[curMatX+1][curMatY-1] == 1){
                        if (movementVector[0] < curWorldX + 0.4)
                            moveRight();
                        else if (movementVector[2] > curWorldY - 0.55)
                            moveRight();
                    }
                    else if (M[curMatX+1][curMatY+1] == 1){
                        if (movementVector[0] < curWorldX + 0.4)
                            moveRight();
                        else if (movementVector[2] < curWorldY + 0.55)
                            moveRight();
                    }
                    else if (M[curMatX][curMatY-1] == 1){
                        if (movementVector[0] < curWorldX + 0.4)
                            moveRight();
                        else if (movementVector[0] > curWorldX - 0.55)
                            moveRight();
                    }
                    else if (M[curMatX][curMatY+1] == 1){
                        if (movementVector[0] < curWorldX + 0.4)
                            moveRight();
                        else if (movementVector[2] < curWorldY + 0.55)
                            moveRight();
                    }
                    else moveRight();
                } else if (M[curMatX+1][curMatY] == 1){
                        if (movementVector[0] < curWorldX + 0.4)
                            moveRight();
                }
                else moveRight();
            }
            else moveRight();
        }
    }
    glutPostRedisplay();
}

//-------------------------K O O R D I N A T E   M I S A-------------------------
static void on_mouseMove(int x, int y){
    tempX = x-window_width/2;
    tempY = window_height - y - window_height/2;
    glutPostRedisplay();
}

static void on_mouseLeftClick(int button, int state, int x, int y){
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
        characterShoot();
    }
}
//-------------------------F U N K C I J E   L I K O V A-------------------------------------------

static void characterShoot(){
//     DecTest();
//     printf("(%f, %f) - (%d, %d) - (%d, %d)\n", movementVector[0], movementVector[2], curWorldX, curWorldY, curMatX, curMatY);
}


//-------------------------I N I C I J A L I Z A C I J A   T E R E N A-----------------------------
static void DecLevelInit(){
    tempX = window_width/2; 
    tempY = window_height/2;
    
    /*Odredjivanje dimenzija matrice na osnovu velicine terena*/
    matrixSizeX = (gPlaneScaleX/2);
    matrixSizeX = (matrixSizeX % 2 == 0) ? matrixSizeX-1 : matrixSizeX;
    matrixSizeY = (gPlaneScaleY/2);
    matrixSizeY = (matrixSizeY % 2 == 0) ? matrixSizeY-1 : matrixSizeY;
    
    M = malloc(sizeof(int*)*matrixSizeX);
    if (M == NULL) 
        greska("Greska u alokaciji");
    
    for (int i=0; i!=matrixSizeX; i++){
        M[i] = malloc(sizeof(int)*matrixSizeY);
        if (M[i] == NULL) 
            greska("Greska u alokaciji");
    }
    
    M_Obstacle = malloc(sizeof(float*)*matrixSizeX);
    if (M_Obstacle == NULL) 
        greska("Greska u alokaciji");
    
    for (int i=0; i!=matrixSizeX; i++){
        M_Obstacle[i] = malloc(sizeof(float)*matrixSizeY);
        if (M_Obstacle[i] == NULL) 
            greska("Greska u alokaciji");
    }
    
    srand(time(NULL));
    float r;
    
    for (int i=0; i!=matrixSizeX; i++){
        for (int j=0; j!=matrixSizeY; j++){
            /*Nasumicno generisanje prepreka*/
            r = (float)rand() / (float)RAND_MAX;
            if (r <= obstacleChance){
                M[i][j] = 1;
                M_Obstacle[i][j] = r*r*300;
            }
            else { 
                M[i][j] = 0;
                M_Obstacle[i][j] = 0;
            }
        }
    }
    /*Okolina kvadratica u kojem se spawnujemo ne sme sadrzati prepreke*/
    M[matrixSizeX/2-1][matrixSizeY/2-1] = 0;
    M[matrixSizeX/2-1][matrixSizeY/2] = 0;
    M[matrixSizeX/2][matrixSizeY/2-1] = 0;
    M[matrixSizeX/2-1][matrixSizeY/2+1] = 0;
    M[matrixSizeX/2+1][matrixSizeY/2-1] = 0;
    M[matrixSizeX/2+1][matrixSizeY/2] = 0;
    M[matrixSizeX/2][matrixSizeY/2+1] = 0;
    M[matrixSizeX/2+1][matrixSizeY/2+1] = 0;
    M[matrixSizeX/2][matrixSizeY/2] = 0;
    
    /*Podesavanje buffera za kretanje na false*/
    for (int i=0; i!=128; i++)
        keyBuffer[i] = false;
}

//-------------------------G E N E R A C I J A   P O D L O G E-------------------------
void DecPlane(float colorR, float colorG, float colorB){
    glPushMatrix();
        glColor3f(colorR, colorG, colorB);
        glScalef(gPlaneScaleX, 1, gPlaneScaleY);
        glutSolidCube(1);
    glPopMatrix();
}

//-------------------------G E N E R A C I J A   T E R E N A---------------------------
void DecFloorMatrix(float colorR, float colorG, float colorB, float cubeHeight){
    float color, localCubeHeight;
    int xPos, yPos;
    
    // 1 ako ima prepreke
    // 0 ako nema
    for (int i=0; i!=matrixSizeX; i++){
        xPos = i-(matrixSizeX/2);
        for (int j=0; j!=matrixSizeY; j++){
            yPos = j-(matrixSizeY/2);
            color = 0.525-j*0.03;
            if (M[i][j] == 0){
                glPushMatrix();
                    glTranslatef(xPos*2, 0.5, yPos*2);
                    glColor3f(0.88, 0.88, 0.9);
                    glScalef(1.8, 0.1, 1.8);
                    glutSolidCube(1);
                glPopMatrix();
            }
            else if (M[i][j] == 1){
                localCubeHeight = cubeHeight + M_Obstacle[i][j];
                glPushMatrix();
                    glTranslatef(xPos*2, localCubeHeight/2, yPos*2);
                    glScalef(1.8, localCubeHeight, 1.8);
                    glColor3f(color, color, color);
                    glutSolidCube(1);
                glPopMatrix();
            }
        }
    }
}

//-------------------------T E S T   F U N K C I J A-------------------------
static void DecTest(){
    printf("Matrica:\n");
    for (int i=0; i!=matrixSizeX; i++) {
        for (int j=0; j!=matrixSizeY; j++) {
            printf("%d ", M[i][j]);
        }
        printf("\n");
    }
    printf("X i Y skalirani: %d, %d\n", matrixSizeX, matrixSizeY);
}

static void greska(char* text){
    fprintf(stderr, "%s\n", text);
    exit(1);
}