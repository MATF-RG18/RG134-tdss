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
static float gPlaneScaleX = 26, gPlaneScaleY = 26;
static float obstacleChance = 0.08;
static int **M;

/*CHARACTER PARAMETERS*/
static float movementVector[3] = {0,0,0};
static float movementSpeed = 0.13;
static float diagonalMovementMultiplier = 0.707107;
static bool keyBuffer[128];

static void greska(char* text);
static void DecLevelInit();
static void DecTest();

static void characterMovement();

static void on_keyPress(unsigned char key, int x, int y);
static void on_keyRelease(unsigned char key, int x, int y);
static void on_reshape(int width, int height);
static void on_display(void);
static void on_mouseMove(int x, int y);
//static void on_mouseClick(int button, int state, int x,int y);

void DecPlane(float colorR, float colorG, float colorB);
void DecFloorMatrix(float colorR, float colorG, float colorB);

int main(int argc, char **argv)
{
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
 
    /* Obavlja se OpenGL inicijalizacija. */
    glClearColor(0.75, 0.75, 0.75, 0);
    glEnable(GL_DEPTH_TEST | GL_POLYGON_SMOOTH);
    glLineWidth(2);
    
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
    gluPerspective(100, window_width/window_height, 1, 20);
 
    /* Podesava se vidna tacka. */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, (matrixSizeX+matrixSizeY)/2, 4, //Sa pozicijom kamere se jos uvek igram
              0, 0, 1.5, 
              0, 1, 0);
    
    /*Inicijalicazija podloge i nivoa*/
    DecPlane(0.1, 0.1, 0.1);
    DecFloorMatrix(0.9, 0.9, 0.9);
    
    /*Funkcija za kretanje*/
    characterMovement();
    
    glTranslatef(movementVector[0], 1, movementVector[2]);
    
    /*Character main*/
    glPushMatrix();
        glColor3f(0,0,0);
        glTranslatef(0, 0.4, 0);
        glRotatef(90, 1, 0, 0);
        glScalef(0.9, 0.9, 0.9);
        glutSolidSphere(0.7, 20, 20);
    glPopMatrix();
    
    glPushMatrix();
        glScalef(0.3,.2,0.6);
        glTranslatef(1, 3, -1);
        glutSolidCube(1);
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
        /*Nazad  - s*/
        case 115:
            keyBuffer[115] = true;
            break;
        /*Desno  - d*/
        case 100:
            keyBuffer[100] = true;
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

void characterMovement(){
    /*w+a*/
    if (keyBuffer[119] && keyBuffer[97]){
        movementVector[2] -= movementSpeed * diagonalMovementMultiplier;
        movementVector[0] -= movementSpeed * diagonalMovementMultiplier;
    }
    /*w+d*/
    else if (keyBuffer[119] && keyBuffer[100]){
        movementVector[2] -= movementSpeed * diagonalMovementMultiplier;
        movementVector[0] += movementSpeed * diagonalMovementMultiplier;
    }
    /*s+a*/
    else if (keyBuffer[115] && keyBuffer[97]){
        movementVector[2] += movementSpeed * diagonalMovementMultiplier;
        movementVector[0] -= movementSpeed * diagonalMovementMultiplier;
    }
    /*s+d*/
    else if (keyBuffer[115] && keyBuffer[100]){
        movementVector[2] += movementSpeed * diagonalMovementMultiplier;
        movementVector[0] += movementSpeed * diagonalMovementMultiplier;
    }
    else {
        /*w*/
        if (keyBuffer[119]){
            movementVector[2] -= movementSpeed;
        }
        /*a*/
        if (keyBuffer[97]){
            movementVector[0] -= movementSpeed;
        }
        /*s*/
        if (keyBuffer[115]){
            movementVector[2] += movementSpeed;
        }
        /*d*/
        if (keyBuffer[100]){
            movementVector[0] += movementSpeed;
        }
    }
    glutPostRedisplay();
}

static void on_mouseMove(int x, int y){
    tempX = x;
    tempY = window_height - y;
    
    glutPostRedisplay();
}

void DecPlane(float colorR, float colorG, float colorB){
    glPushMatrix();
        glColor3f(colorR, colorG, colorB);
        glScalef(gPlaneScaleX, 1, gPlaneScaleY);
        glutSolidCube(1);
    glPopMatrix();
}


void DecFloorMatrix(float colorR, float colorG, float colorB){
    float r = 1.5 /*min = 1, max = 2*/, color;
    int xPos, yPos;

    // 1 ako ima prepreke
    // 0 ako nema
    for (int i=0; i!=matrixSizeX; i++){
        xPos = i-(matrixSizeX/2);
        for (int j=0; j!=matrixSizeY; j++){
            yPos = j-(matrixSizeY/2);
            color = 0.6-j*0.03;
            if (M[i][j] == 0){
                glPushMatrix();
                    glTranslatef(xPos*2, 0.5, yPos*2);
                    glColor3f(1, 1, 1);
                    glScalef(1.8, 0.1, 1.8);
                    glutSolidCube(1);
                glPopMatrix();
            }
            else if (M[i][j] == 1){
                //r = (fmod(rand(),(min-max)))+max;
                glPushMatrix();
                    glTranslatef(xPos*2, r/2, yPos*2);
                    glColor3f(color, color, color);
                    glScalef(1.8, r, 1.8);
                    glutSolidCube(1);
                glPopMatrix();
            }
        }
    }
}

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
    
    srand(time(NULL));
    float r;
    
    for (int i=0; i!=matrixSizeX; i++){
        for (int j=0; j!=matrixSizeY; j++){
            /*Nasumicno generisanje prepreka*/
            r = (float)rand() / (float)RAND_MAX;
            if (r <= obstacleChance)
                M[i][j] = 1;
            else 
                M[i][j] = 0;
        }
    }
    /*Kockica u kojoj se spawnujemo ne sme biti prepreka*/
    M[matrixSizeX/2][matrixSizeY/2] = 0;
    
    /*Podesavanje buffera za kretanje na false*/
    for (int i=0; i!=128; i++)
        keyBuffer[i] = false;
}

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