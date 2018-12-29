#include<stdio.h>
#include<stdlib.h>
#include<GL/glut.h>
#include<math.h>
#include<time.h>
#include<stdbool.h>
#include<string.h>
#include <unistd.h>

/*Ako stavimo GAME_MODE na false, mozemo da fullscreenujemo program na 'f', umesto da ga glutGameMode automatski fullscreenuje.
  Meni ponekad glutGameMode pravi dropove u fpsu, pa ako se desi i tebi samo cepi ovo na false i fullsceenuj.*/
#define GAME_MODE true

#define STR_MAX_LEN 256

#define MAX_BULLETS_ON_SCREEN 20

#define TIMER_INITIAL 1000
#define TIMER_INITIAL_ID_1 10
#define TIMER_INITIAL_ID_2 20
#define TIMER_INITIAL_ID_3 30

#define TIMER_SPAWN_INTERVAL 800
#define TIMER_SPAWN_ID 1

#define TIMER_DEATH 2000
#define TIMER_DEATH_ID 2

#define COLISION_TERRAIN 0
#define COLISION_ENEMY 1


/*MAP PARAMETERS*/
/*
 * M i M_obstacle su matrice preko kojih je realizovan teren. U matrici M, ako je M[i][j] == 0, na tom mestu nema prepreke, a ako je 1 onda ima.
 * Za 'i' i 'j' za koje vazi da je M[i][j] == 1, u matrici M_obstacle[i][j] se nalazi visina prepreke. Raspodela pozicija prepreka i njihove visine se 
 * generisu nasumicno u DecLevelInit(). 
 * gPlaneScaleX i gPlaneScaleY odredjuju duzinu stranica terena u worldspace koordinatnom sistemu, a matrixSizeX i matrixSizeY na osnovu toga
 * racunaju dimenzije samih matrica. 
 * Kamera i prikaz score-a nisu najbolje podeseni za slucajeve kad su gPlaneScale parametri manji od 31 ili veci od 41.
 */
/*Stavio sam samo 3 rezolucije kao podrzane: '1600x900', '1920x1080' i '1366x768'. Ako ne odgovara ni jedna, dodaj u main jos jednu glutGameModeString() proveru 
 *za neku koju moze kod tebe.*/
static float window_width, window_height;
static int matrixSizeX, matrixSizeY;
static int tempX, tempY;
static float gPlaneScaleX = 31, gPlaneScaleY = 31; // ne sme biti manje od 5;
static const float obstacleChance = 0.08;
static int **M;
static float **M_Obstacle;
static const int maxEnemyNumber = 100;
static int currentEnemyNumber = 0;
static int enemiesKilledCounter = 0;
static char enemiesKilledStr[STR_MAX_LEN];
static char playerLives[STR_MAX_LEN];

/*MATRIXES*/
static GLdouble ModelMatrix[16];
static GLdouble ProjectionMatrix[16];
static GLint ViewportMatrix[4];
static bool notSetP = true;

/*CHARACTER PARAMETERS*/
static int characterHealth = 20;
static int characterHit = 0;
static const float characterDiameter = 0.6;
static float movementVector[3] = {0,0,0};
static float movementSpeed = 0.1;
static float diagonalMovementMultiplier = 0.707107;
static int curWorldX, curWorldY; // Trenutna pozicija lika u worldspace-u.
static int curMatX, curMatY;  // Trenutna pozicija lika u matrici.
static GLdouble objWinX, objWinY, objWinZ; // Trenutna pozicija lika na ekranu.
static int currentRotationX, currentRotationY;

/*DISPLAY FUNCTIONS*/
static bool gameMode = GAME_MODE;
static bool fullScr = false;
static void displayInitialCountdown();
static void displayScore();
static void displayText(GLfloat x, GLfloat y, GLfloat z, char* text, GLfloat scale);
static void output(GLfloat x, GLfloat y, char *string);
static void greska(char* text);
static void DecLevelInit();
static void DecTest();

static bool shootingEnabled = false;
static bool enemiesEnabled = false;
static void characterShoot();


/*MOVEMENT FUNCTIONS*/
/*
 * Za kretanje se koristi keyBuffer[i], gde i predstavlja ascii vrednost dugmeta na koja su kretanje bindovana.
 * Dok je dugme pritisnuto, keyBuffer[i] je true, false u suprotnom.
 */
static bool keyBuffer[128];   
static void moveLeft();
static void moveRight();
static void moveUp();
static void moveDown();
static void moveUpLeft();
static void moveUpRight();
static void moveDownLeft();
static void moveDownRight();
static void characterMovement();
static bool movementEnabled = false;
static float colisionRange; // Ova promenljiva sluzi uzima u obzir precnik lika i odredjuje distancu lika od prepreke na kojoj se javlja kolizija.


static void on_keyPress(unsigned char key, int x, int y);
static void on_keyRelease(unsigned char key, int x, int y);
static void on_reshape(int width, int height);
static void on_display(void);
static void on_mouseMove(int x, int y);
static void on_mouseLeftClick(int button, int state, int x, int y);
static void on_timerInitial(int value);
static int timerInitialTick_1 = 0;
static int timerInitialTick_2 = 0;
static int timerInitialTick_3 = 0;
static void on_timerSpawnInterval(int value);
static void on_timerDeath(int value);


void DecPlane(float colorR, float colorG, float colorB);
void DecFloorMatrix(float colorR, float colorG, float colorB, float cubeHeight);


/*BULLET PARAMETERS*/
typedef struct{
    float bulletStartingPoint[3];
    float bulletTraj[2];
    float bulletTrajLength;
    float bulletTrajNorm[2];
    float currentX;
    float currentY;
    float currentZ;
    bool bulletSet;
    bool getMovementVector;
    float bulletVelocity;
} bullet;
static bullet bullets[MAX_BULLETS_ON_SCREEN];
static const int bulletDmg = 5;

static int bulletTracker = 0;
static const float maxBulletVelocity = 30.0;
static const float bulletSpeed = 0.25;

static void bulletInit();

/*Ova funkcija proverava koliziju na sledeci nacin:
  - Ako je prosledjen flag COLISION_TERRAIN, onda se kao x i y prosledjuje trenutna pozicija metka u prostoru
    i provera se kolizija metka sa terenom.
  - Ako je prosledjen flag COLISION_ENEMY, onda se kao x i y prosledjuje trenutna pozicija protivnika u prostoru
    i proverava se kolizicija protivnika sa svim metkovima.*/
static bool checkBulletColision(float x, float y, int colisionFlag);


/*ENEMY PARAMETERS*/
typedef struct{
    float x;
    float y;
    int health;
    bool alive;
} enemy;
static enemy *enemies;
static const float enemyDiameter = 0.6;
static float enemySpeed = 0.102;
static void enemyInit();
static void enemySpawn();
static bool enemyNearPlayer(float i, float j);
static void drawEnemy();
static void enemyMovement();

int main(int argc, char **argv)
{
    /*Globalno svetlo*/
    GLfloat light_ambient[] = { 0, 0, 0, 1 };
    GLfloat light_diffuse[] = { 0.425, 0.415, 0.4, 1 };
    GLfloat light_specular[] = { 0.9, 0.9, 0.9, 1 };
    GLfloat model_ambient[] = { 0.4, 0.4, 0.4, 1 };
    
    
    /* Inicijalizuje se GLUT. */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    
    /* Kreira se prozor. */
    glutInitWindowSize(window_width, window_height);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("TDSS - Lvl 1");
    

    glutGameModeString("1600x900:32@60");
    if (glutGameModeGet(GLUT_GAME_MODE_POSSIBLE)){
        window_width = 1600;
        window_height = 900;
        if (gameMode)
            glutEnterGameMode();
    }
    else
    {
        glutGameModeString("1920x1080:32@60");
        if (glutGameModeGet(GLUT_GAME_MODE_POSSIBLE)){
            window_width = 1920;
            window_height = 1080;
            if (gameMode)
                glutEnterGameMode();
        }
        else
        {
            glutGameModeString("1366x768:32@60");
            if (glutGameModeGet(GLUT_GAME_MODE_POSSIBLE)){
                window_width = 1366;
                window_height = 768;
                if (gameMode)
                    glutEnterGameMode();
            }
        }
    }
    /*Kursor se menja u ikonicu nisana*/
    glutSetCursor(GLUT_CURSOR_CROSSHAIR);
 
    /* Registruju se callback funkcije. */
    glutKeyboardFunc(on_keyPress);
    glutKeyboardUpFunc(on_keyRelease);
    glutReshapeFunc(on_reshape);
    glutDisplayFunc(on_display);
    glutPassiveMotionFunc(on_mouseMove);
    glutMotionFunc(on_mouseMove);
    glutMouseFunc(on_mouseLeftClick);
 
    /* Obavlja se OpenGL inicijalizacija. */
    glClearColor(0.7, 0.7, 0.7, 0);
    glEnable(GL_DEPTH_TEST | GL_POLYGON_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(2);

    /*Svetlo*/
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, model_ambient);
    
    /*Inicijalizacija nekih parametara*/
    glutReshapeWindow(window_width, window_height);
    DecLevelInit();
    bulletInit();
    enemyInit();
    
    //DecTest();
    
    
    /* Program ulazi u glavnu petlju. */
    glutMainLoop();
 
    return 0;
}

static void on_reshape(int width, int height){
    /* Pamte se sirina i visina prozora. */
    window_width = width;
    window_height = height;
}

static void on_display(void){
    /* Brise se prethodni sadrzaj prozora. */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
    /* Podesava se viewport. */
    glViewport(0, 0, window_width, window_height);
 
    /* Podesava se projekcija. */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50, window_width/window_height, 1, 1000);
 
    /* Podesava se vidna tacka. */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(0, (matrixSizeX+matrixSizeY)/0.8, (matrixSizeX+matrixSizeY)/2.8, 
              0, 0, 0, 
              0, 1, 0);
    
    GLfloat light_position[4] = {0, 30, 0, 1};
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    
    /*Iscrtavanje terena*/
    DecPlane(0.1, 0.1, 0.1);
    DecFloorMatrix(0.9, 0.9, 0.9, 2);

    /*Ispis trenutnog rezultata i zivota*/
    displayScore();
    
    /*Ako igrac ostane bez helta, scena se pauzira i program se gasi posle sekunde*/
    if (characterHealth <= 0){
        glutTimerFunc(TIMER_DEATH, on_timerDeath, TIMER_DEATH_ID);
        enemiesEnabled = false;
        displayText(-5.2, 0, 5.6, "GAME OVER", 2);
    }
    /*Inace sve ide normalno*/
    
    /*Cekamo 3 sekunde pre nego sto krenemo da spawnujemo protivnike*/
    glutTimerFunc(TIMER_INITIAL, on_timerInitial, TIMER_INITIAL_ID_1);
    displayInitialCountdown();
    
    if (characterHit + enemiesKilledCounter == maxEnemyNumber){
        displayText(-4.5, 0, 5.6, "YOU WON!!!", 2);
    }
    
    if (enemiesEnabled) {
        
        /*Za svakom zivog protivnika, proveravamo da li se desila kolizija sa nekim metkom.
        Ako jeste, smanjujemo mu health-e. Kad ostane bez health-a, nestaje.*/
        
        for (int i=0; i!=currentEnemyNumber; i++){
            if (enemies[i].alive){
                if (checkBulletColision(enemies[i].x, enemies[i].y, COLISION_ENEMY)){
                    enemies[i].health -= bulletDmg;
                    if (enemies[i].health <= 0){
                        enemies[i].alive = false;
                        enemiesKilledCounter++;
                    }
                }
            }
            /*Racunanje kretanja i kolizije*/
            enemyMovement(i);
            if (enemies[i].alive){
                glPushMatrix();
                    glTranslatef(enemies[i].x, 0.75, enemies[i].y);
                    drawEnemy();
                glPopMatrix();
            }
        }
    }
        
    /*Funkcija za kretanje*/
    if (movementEnabled)
        characterMovement();
    
    /*gluProject*/
    glGetDoublev(GL_MODELVIEW_MATRIX, ModelMatrix);
    if (notSetP){
        glGetDoublev(GL_PROJECTION_MATRIX, ProjectionMatrix);
        notSetP = false;
    }
    glGetIntegerv(GL_VIEWPORT, ViewportMatrix);
    
    /*Nalazimo koordiante naseg lika u koordinatama ekrana*/
    gluProject(movementVector[0], movementVector[1], movementVector[2],
            ModelMatrix, ProjectionMatrix, ViewportMatrix,
            &objWinX, &objWinY, &objWinZ);
    
    /*Racunanje rotacije*/
    currentRotationX = tempX - ((int)objWinX-window_width/2);
    currentRotationY = tempY - ((int)objWinY-window_height/2); 

    /*Metkovi*/
    if (shootingEnabled){
        for (int i=0; i!=MAX_BULLETS_ON_SCREEN; i++){
            
            /*Za svaki metak, ako je setovan, tj ako je "instanciran trenutno", treba azurirati predjeni put 
            i poziciju, inicijalizovati mu startingPoint i normiran vektor kretanja ako vec nisu inicijalizovani.
            Na kraju proveravamo koliziju sa terenom.*/
            
            if (bullets[i].bulletSet){
                bullets[i].bulletVelocity += bulletSpeed;
                
                if (bullets[i].getMovementVector){
                    bullets[i].bulletStartingPoint[0] = movementVector[0];
                    bullets[i].bulletStartingPoint[1] = 1.0;
                    bullets[i].bulletStartingPoint[2] = movementVector[2];
                    
                    bullets[i].bulletTrajLength = sqrt(currentRotationX*currentRotationX + currentRotationY*currentRotationY);
                    bullets[i].bulletTrajNorm[0] = currentRotationX / bullets[i].bulletTrajLength;
                    bullets[i].bulletTrajNorm[1] = -currentRotationY / bullets[i].bulletTrajLength;
                    bullets[i].getMovementVector = false;
                }
                
                bullets[i].currentX = bullets[i].bulletStartingPoint[0] + bullets[i].bulletVelocity*bullets[i].bulletTrajNorm[0];
                bullets[i].currentY = bullets[i].bulletStartingPoint[1];
                bullets[i].currentZ = bullets[i].bulletStartingPoint[2] + bullets[i].bulletVelocity*bullets[i].bulletTrajNorm[1];
                
                glPushMatrix();
                    glTranslatef(bullets[i].currentX, bullets[i].currentY, bullets[i].currentZ);
                    glDisable(GL_LIGHTING);
                    glColor3f(0,0,0);
                    glutSolidSphere(0.2, 5, 5);
                    glEnable(GL_LIGHTING);
                glPopMatrix();
                if (bullets[i].bulletVelocity >= maxBulletVelocity || checkBulletColision(bullets[i].currentX, bullets[i].currentZ, COLISION_TERRAIN)){
                    bullets[i].bulletSet = false;
                    bullets[i].bulletVelocity = 1.0;
                }
            }
        }
    }
    
    
    /*Pomeranje i rotacija igraca*/
    glTranslatef(movementVector[0], 1, movementVector[2]);
    glRotatef(atan2(currentRotationX, currentRotationY)*(-180)/M_PI, 0, 1, 0);

    /*Character main*/
    GLfloat low_shininess[] = { 100 };
    GLfloat material_ambient[] = {.3, .3, .3, 1};
    GLfloat material_diffuse[] = {.8, .8, .7, 1};
    glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, low_shininess);
    
    /*Gornja lopta*/
    glPushMatrix();
        glTranslatef(0, 0.1, 0);
        glRotatef(90, 1, 0, 0);
        glutSolidSphere(characterDiameter, 20, 20);
    glPopMatrix();
    /*Cilindar*/
    glPushMatrix();
        glTranslatef(0, 1.05, 0);
        glRotatef(90, 1, 0, 0);
        gluCylinder(gluNewQuadric(), characterDiameter, characterDiameter, 1, 20, 1);
    glPopMatrix();
    /*Donja lopta*/
    glPushMatrix();
        glTranslatef(0, 1, 0);
        glRotatef(90, 1, 0, 0);
        glutSolidSphere(characterDiameter, 20, 20);
    glPopMatrix();

    /*Oruzije*/
    glPushMatrix();
        glDisable(GL_LIGHTING);
        glColor3f(0,0,0);
        glScalef(0.3,0.2,0.7);
        glTranslatef(0, 4, -1);
        glutSolidCube(1);
        glEnable(GL_LIGHTING);
    glPopMatrix();
    /* Nova slika se salje na ekran. */

    glutSwapBuffers();
}


//------------------------C A L L B A C K   T A S T A T U R E-----------------------------
static void on_keyPress(unsigned char key, int x, int y){
    switch (key) {
        
        /*Napred - w*/
        case 'w':
        case 'W':
            keyBuffer[119] = true;
            break;
            
        /*Levo   - a*/
        case 'a':
        case 'A':
            keyBuffer[97] = true;
            break;
            
        /*Desno  - d*/
        case 'd':
        case 'D':
            keyBuffer[100] = true;
            break;
            
        /*Nazad  - s*/
        case 's':
        case 'S':
            keyBuffer[115] = true;
            break;

        case 70:
        case 102:
            if (!(gameMode)){
                if (fullScr == false){
                    glutFullScreen();
                    fullScr = true;
                }
                else {
                    glutReshapeWindow(window_width, window_height);
                    fullScr = false;
                }
            }
            break;
        /* Zavrsava se program. */    
        case 27:
            for (int i=0; i!=matrixSizeX; i++){
                free(M[i]);
                free(M_Obstacle[i]);
            }
            free(M);
            free(M_Obstacle);
            free(enemies);

            exit(0);
            break;
    }
}
static void on_keyRelease(unsigned char key, int x, int y){
    switch (key) {
        
        /*Napred - w*/
        case 'w':
        case 'W':
            keyBuffer[119] = false;
            break;
        /*Levo   - a*/
        case 'a':
        case 'A':
            keyBuffer[97] = false;
            break;
        /*Nazad  - s*/
        case 's':
        case 'S':
            keyBuffer[115] = false;
            break;
        /*Desno  - d*/
        case 'd':
        case 'D':
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
/*Ovde ima puno koda, pokriva svaki slucaj kolizije glavnog lika i terena, radi bez obzira na dimenzije terena.
  Uzima u obzir precnik lika i ima posebne provere za odnos pozicija lika sa preprekama za dijagonalno i pravolinijsko kretanje. Kako je najveci deo koda 
  samo razmatranje slucajeva, ne bih se udubljivao u objasnjavanje previse. */
void characterMovement(){
    
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
    
    /*Izracunavanje za slucaj kada dijagonalno prilazimo prepreci, pa treba odluciti na koju ce nas stranu skrenuti. Gleda se da li se nalazimo u gornjem ili donjem trouglu polja u kojem smo, i skrece se na osnovu toga.*/
    int wa, wd; 
    float localX, localY;
    
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
                    if (movementVector[0] > curWorldX - colisionRange && movementVector[2] > curWorldY - colisionRange)
                        moveUpLeft();
                    else if (movementVector[0] <= curWorldX - colisionRange && movementVector[2] > curWorldY - colisionRange)
                        moveUp();
                    else if (movementVector[0] > curWorldX - colisionRange && movementVector[2] <= curWorldY - colisionRange)
                        moveLeft();
                    }
                    else {
                        if (movementVector[0] > curWorldX - colisionRange)
                            moveUpLeft();
                        else moveUp();
                    }
                }
                else if (M[curMatX][curMatY-1] == 1){
                    if (movementVector[2] > curWorldY - colisionRange)
                        moveUpLeft();
                    else moveLeft();
                }
                else if (M[curMatX-1][curMatY-1] == 1){
                    if (!(movementVector[0] < curWorldX - colisionRange && movementVector[2] < curWorldY - colisionRange))
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
                        if (movementVector[2] > curWorldY - colisionRange)
                            moveUpLeft();
                        else moveLeft();
                }
                else moveUpLeft();
            }
            else if (curMatY-1 < 0){ 
                if (M[curMatX-1][curMatY] == 1){
                    if (movementVector[0] > curWorldX - colisionRange)
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
                    if (movementVector[0] < curWorldX + colisionRange && movementVector[2] > curWorldY - colisionRange)
                        moveUpRight();
                    else if (movementVector[0] >= curWorldX + colisionRange && movementVector[2] > curWorldY - colisionRange)
                        moveUp();
                    else if (movementVector[0] < curWorldX + colisionRange && movementVector[2] <= curWorldY - colisionRange)
                        moveRight();
                    }
                    else {
                        if (movementVector[0] < curWorldX + colisionRange)
                            moveUpRight();
                        else moveUp();
                    }
                }
                else if (M[curMatX][curMatY-1] == 1){
                    if (movementVector[2] > curWorldY - colisionRange)
                        moveUpRight();
                    else moveRight();
                }
                else if (M[curMatX+1][curMatY-1] == 1){
                    
                    if (!(movementVector[0] > curWorldX + colisionRange && movementVector[2] < curWorldY - colisionRange))
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
                    if (movementVector[2] > curWorldY - colisionRange)
                        moveUpRight();
                    else moveRight();
                }
                else moveUpRight();
            } 
            else if (curMatY - 1 < 0){
                if (M[curMatX+1][curMatY] == 1) {
                    if (movementVector[0] < curWorldX + colisionRange)
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
                    if (movementVector[0] > curWorldX - colisionRange && movementVector[2] < curWorldY + colisionRange)
                        moveDownLeft();
                    else if (movementVector[0] <= curWorldX - colisionRange && movementVector[2] < curWorldY + colisionRange)
                        moveDown();
                    else if (movementVector[0] > curWorldX - colisionRange && movementVector[2] >= curWorldY - colisionRange)
                        moveLeft();
                    }
                    else {
                        if (movementVector[0] > curWorldX - colisionRange)
                            moveDownLeft();
                        else moveDown();
                    }
                }
                else if (M[curMatX][curMatY+1] == 1){
                    if (movementVector[2] < curWorldY + colisionRange)
                        moveDownLeft();
                    else moveLeft();
                }
                else if (M[curMatX-1][curMatY+1] == 1){
                    if (!(movementVector[0] < curWorldX - colisionRange && movementVector[2] > curWorldY + colisionRange))
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
                        if (movementVector[0] > curWorldX - colisionRange)
                            moveDownLeft();
                        else moveDown();
                }
                else moveDownLeft();
            }
            else if (curMatX - 1 < 0){
                if (M[curMatX][curMatY+1] == 1){
                    if (movementVector[2] < curWorldY + colisionRange)
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
                    if (movementVector[0] < curWorldX + colisionRange && movementVector[2] < curWorldY + colisionRange)
                        moveDownRight();
                    else if (movementVector[0] >= curWorldX + colisionRange && movementVector[2] < curWorldY + colisionRange)
                        moveDown();
                    else if (movementVector[0] < curWorldX + colisionRange && movementVector[2] >= curWorldY - colisionRange)
                        moveRight();
                    }
                    else {
                        if (movementVector[0] < curWorldX + colisionRange)
                            moveDownRight();
                        else moveDown();
                    }
                }
                else if (M[curMatX][curMatY+1] == 1){
                    if (movementVector[2] < curWorldY + colisionRange)
                        moveDownRight();
                    else moveRight();
                }
                else if (M[curMatX+1][curMatY+1] == 1){
                    
                    if (!(movementVector[0] > curWorldX + colisionRange && movementVector[2] > curWorldY + colisionRange))
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
                        if (movementVector[0] < curWorldX + colisionRange)
                            moveDownRight();
                        else moveDown();
                }
                else moveDownRight();
            }
            else if (curMatX + 1 >= matrixSizeX){
                if (M[curMatX][curMatY+1] == 1){
                    if (movementVector[2] < curWorldY + colisionRange)
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
                        if (movementVector[2] > curWorldY - colisionRange)
                            moveUp();
                    }
                    else if (M[curMatX-1][curMatY-1] == 1 && M[curMatX+1][curMatY-1] == 1){
                        if (movementVector[2] > curWorldY - colisionRange)
                            moveUp();
                        else if (movementVector[0] > curWorldX - colisionRange+0.15 && movementVector[0] < curWorldX + colisionRange+0.15)
                            moveUp();
                    }
                    else if (M[curMatX-1][curMatY-1] == 1){
                        if (movementVector[2] > curWorldY - colisionRange)
                            moveUp();
                        else if (movementVector[0] > curWorldX - colisionRange+0.15)
                            moveUp();
                    }
                    else if (M[curMatX+1][curMatY-1] == 1){
                        if (movementVector[2] > curWorldY - colisionRange)
                            moveUp();
                        else if (movementVector[0] < curWorldX + colisionRange+0.15)
                            moveUp();
                    }
                    else if (M[curMatX-1][curMatY] == 1){
                        if (movementVector[2] > curWorldY - colisionRange)
                            moveUp();
                        else if (movementVector[0] > curWorldX - colisionRange+0.15)
                            moveUp();
                    }
                    else if (M[curMatX+1][curMatY] == 1){
                        if (movementVector[2] > curWorldY - colisionRange)
                            moveUp();
                        else if (movementVector[0] < curWorldX + colisionRange+0.15)
                            moveUp();
                    }
                    else moveUp();
                } else if (M[curMatX][curMatY-1] == 1){
                        if (movementVector[2] > curWorldY - colisionRange)
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
                        if (movementVector[0] > curWorldX - colisionRange)
                            moveLeft();
                    }
                    else if (M[curMatX-1][curMatY-1] == 1 && M[curMatX-1][curMatY+1] == 1){
                        if (movementVector[0] > curWorldX - colisionRange)
                            moveLeft();
                        else if (movementVector[2] > curWorldY - colisionRange+0.15 && movementVector[2] < curWorldY + colisionRange+0.15)
                            moveLeft();
                    }
                    else if (M[curMatX-1][curMatY-1] == 1){
                        if (movementVector[0] > curWorldX - colisionRange)
                            moveLeft();
                        else if (movementVector[2] > curWorldY - colisionRange+0.15)
                            moveLeft();
                    }
                    else if (M[curMatX-1][curMatY+1] == 1){
                        if (movementVector[0] > curWorldX - colisionRange)
                            moveLeft();
                        else if (movementVector[2] < curWorldY + colisionRange+0.15)
                            moveLeft();
                    }
                    else if (M[curMatX][curMatY-1] == 1){
                        if (movementVector[0] > curWorldX - colisionRange)
                            moveLeft();
                        else if (movementVector[2] > curWorldY - colisionRange+0.15)
                            moveLeft();
                    }
                    else if (M[curMatX][curMatY+1] == 1){
                        if (movementVector[0] > curWorldX - colisionRange)
                            moveLeft();
                        else if (movementVector[2] < curWorldY + colisionRange+0.15)
                            moveLeft();
                    }
                    else moveLeft();
                } else if (M[curMatX-1][curMatY] == 1){
                        if (movementVector[0] > curWorldX - colisionRange)
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
                        if (movementVector[2] < curWorldY + colisionRange)
                            moveDown();
                    }
                    else if (M[curMatX-1][curMatY+1] == 1 && M[curMatX+1][curMatY+1] == 1){
                        if (movementVector[2] < curWorldY + colisionRange)
                            moveDown();
                        else if (movementVector[0] > curWorldX - colisionRange+0.15 && movementVector[0] < curWorldX + colisionRange+0.15)
                            moveDown();
                    }
                    else if (M[curMatX-1][curMatY+1] == 1){
                        if (movementVector[2] < curWorldY + colisionRange)
                            moveDown();
                        else if (movementVector[0] > curWorldX - colisionRange+0.15)
                            moveDown();
                    }
                    else if (M[curMatX+1][curMatY+1] == 1){
                        if (movementVector[2] < curWorldY + colisionRange)
                            moveDown();
                        else if (movementVector[0] < curWorldX + colisionRange+0.15)
                            moveDown();
                    }
                    else if (M[curMatX-1][curMatY] == 1){
                        if (movementVector[2] < curWorldY + colisionRange)
                            moveDown();
                        else if (movementVector[0] > curWorldX - colisionRange+0.15)
                            moveDown();
                    }
                    else if (M[curMatX+1][curMatY] == 1){
                        if (movementVector[2] < curWorldY + colisionRange)
                            moveDown();
                        else if (movementVector[0] < curWorldX + colisionRange+0.15)
                            moveDown();
                    }
                    else moveDown();
                } else if (M[curMatX][curMatY+1] == 1){
                        if (movementVector[2] < curWorldY + colisionRange)
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
                        if (movementVector[0] < curWorldX + colisionRange)
                            moveRight();
                    }
                    else if (M[curMatX+1][curMatY-1] == 1 && M[curMatX+1][curMatY+1] == 1){
                        if (movementVector[0] < curWorldX + colisionRange)
                            moveRight();
                        else if (movementVector[2] > curWorldY - colisionRange+0.15 && movementVector[2] < curWorldY + colisionRange+0.15)
                            moveRight();
                    }
                    else if (M[curMatX+1][curMatY-1] == 1){
                        if (movementVector[0] < curWorldX + colisionRange)
                            moveRight();
                        else if (movementVector[2] > curWorldY - colisionRange+0.15)
                            moveRight();
                    }
                    else if (M[curMatX+1][curMatY+1] == 1){
                        if (movementVector[0] < curWorldX + colisionRange)
                            moveRight();
                        else if (movementVector[2] < curWorldY + colisionRange+0.15)
                            moveRight();
                    }
                    else if (M[curMatX][curMatY-1] == 1){
                        if (movementVector[0] < curWorldX + colisionRange)
                            moveRight();
                        else if (movementVector[0] > curWorldX - colisionRange+0.15)
                            moveRight();
                    }
                    else if (M[curMatX][curMatY+1] == 1){
                        if (movementVector[0] < curWorldX + colisionRange)
                            moveRight();
                        else if (movementVector[2] < curWorldY + colisionRange+0.15)
                            moveRight();
                    }
                    else moveRight();
                } else if (M[curMatX+1][curMatY] == 1){
                        if (movementVector[0] < curWorldX + colisionRange)
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
        if (shootingEnabled)
            characterShoot();
    }
}


//-------------------------F U N K C I J E   L I K O V A-------------------------------------------
/*Inicijalizacija strukture metkova*/
static void bulletInit(){
    for (int i=0; i!=MAX_BULLETS_ON_SCREEN; i++){
        bullets[i].bulletSet = false;
        bullets[i].getMovementVector = false;
        bullets[i].bulletVelocity = 1.0;
    }
}
/*Pucanje*/
static void characterShoot(){
//     DecTest();
//     printf("(%f, %f) - (%d, %d) - (%d, %d)\n", movementVector[0], movementVector[2], curWorldX, curWorldY, curMatX, curMatY);
    if (bulletTracker == MAX_BULLETS_ON_SCREEN)
        bulletTracker = 0;
    
    bullets[bulletTracker].bulletSet = true;
    bullets[bulletTracker].getMovementVector = true;
    bulletTracker++;
}
/*Kolizija metkova sa terenom i protivnicima - u zavisnosti od prosledjenog Flag-a*/
static bool checkBulletColision(float x, float y, int colisionFlag){
    
    /*Kolizija sa terenom*/
    if (colisionFlag == COLISION_TERRAIN){
        int bulletCurWorldX;
        int bulletCurWorldY;
        int bulletCurMatX;
        int bulletCurMatY;
        
        bulletCurWorldX = (int)x;
        if (bulletCurWorldX < 0){
            if (bulletCurWorldX % 2 != 0) bulletCurWorldX -= 1;
        } 
        else if (bulletCurWorldX % 2 != 0) bulletCurWorldX += 1;
        
        /*Za Y osu*/
        bulletCurWorldY = (int)y;
        if (bulletCurWorldY < 0){
            if (bulletCurWorldY % 2 != 0) bulletCurWorldY -= 1;
        } 
        else if (bulletCurWorldY % 2 != 0) bulletCurWorldY += 1;
        
        /*Racunanje nase pozicije u matrici*/
        bulletCurMatX = bulletCurWorldX/2 + matrixSizeX/2;
        bulletCurMatY = bulletCurWorldY/2 + matrixSizeY/2;
        
        if ((bulletCurMatX >= 0 && bulletCurMatX < matrixSizeX) && (bulletCurMatY >= 0 && bulletCurMatY < matrixSizeY))
            if (M[bulletCurMatX][bulletCurMatY] == 1)
                return true;
        return false;
    } 
    /*Kolizija sa protivnicima*/
    else if (colisionFlag == COLISION_ENEMY){
        for (int i=0; i!=MAX_BULLETS_ON_SCREEN; i++){
            if (bullets[i].bulletSet){
                if (bullets[i].currentX <= x+0.5 && bullets[i].currentX >= x-0.5){
                    if (bullets[i].currentZ <= y+0.5 && bullets[i].currentZ >= y-0.5){
                        /*Ako je doslo do kolizije, metku se resetuje pocetna pozicija i brise se instanca*/
                        bullets[i].bulletVelocity = 1.0;
                        bullets[i].bulletSet = false;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}
/*Posle gubljenja svih zivota, igra se gasi nakon 1.5 sekunde*/
static void on_timerDeath(int value){
    if (value == TIMER_DEATH_ID){
        for (int i=0; i!=matrixSizeX; i++){
                free(M[i]);
                free(M_Obstacle[i]);
        }
        free(M);
        free(M_Obstacle);
        free(enemies);
        exit(0);
    }
}


//-------------------------------P R O T I V N I C I-----------------------------
/*Inicijalni tajmer od 3 sekunde.
 Ovde omogucavamo pucanje, kretanje i protivnike*/
static void on_timerInitial(int value){
    if (value == TIMER_INITIAL_ID_1 + timerInitialTick_1){
        timerInitialTick_1++;
        glutTimerFunc(TIMER_INITIAL, on_timerInitial, TIMER_INITIAL_ID_2);
    }
    else if (value == TIMER_INITIAL_ID_2 + timerInitialTick_2){
        timerInitialTick_2++;
        glutTimerFunc(TIMER_INITIAL, on_timerInitial, TIMER_INITIAL_ID_3);
    }
    else if (value == TIMER_INITIAL_ID_3 + timerInitialTick_3){
        timerInitialTick_3++;
        movementEnabled = true;
        shootingEnabled = true;
        enemiesEnabled = true;
        glutTimerFunc(TIMER_SPAWN_INTERVAL, on_timerSpawnInterval, TIMER_SPAWN_ID);
    }
}
/*Tajmer za interval spawnovanja*/
static void on_timerSpawnInterval(int value){
    
    if (value == TIMER_SPAWN_ID){
        if (currentEnemyNumber < maxEnemyNumber){
//             printf("Spawning...\n");
            enemySpawn();
            currentEnemyNumber++;
            glutTimerFunc(TIMER_SPAWN_INTERVAL, on_timerSpawnInterval, TIMER_SPAWN_ID);
        }
    }
}
static void enemyInit(){
    enemies = malloc(sizeof(enemy)*maxEnemyNumber);
    if (!enemies) 
        greska("Malloc failed\n");
    
    for (int i=0; i!=maxEnemyNumber; i++){
        enemies[i].health = 10;
        enemies[i].alive = true;
    }
}
static void enemySpawn(){
    float i, j;
    do {
        i = (float)(rand() % matrixSizeX);
        j = (float)(rand() % matrixSizeY);        
    } while (M[(int)i][(int)j] == 1 || enemyNearPlayer(i, j));
    
    enemies[currentEnemyNumber].x = (i*2)-matrixSizeX+1;
    enemies[currentEnemyNumber].y = (j*2)-matrixSizeY+1;
//     printf("(%f, %f) - %d - (%f, %f)\n", i, j, M[(int)i][(int)j], movementVector[0], movementVector[2]);
}
static bool enemyNearPlayer(float i, float j){
    int tmpX = (int)i, tmpY = (int)j;
    for (int l=tmpX-2; l!=tmpX+3; l++)
        for (int k=tmpY-2; k!=tmpY+3; k++)
            if (l == curMatX && k == curMatY)
                return true;
    return false;
    
}
static void drawEnemy(){
    GLfloat low_shininess[] = { 100 };
    GLfloat material_ambient[] = {0.8, .5, .5, 1};
    GLfloat material_diffuse[] = {1, 0, 0, 1};
    glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, low_shininess);
    
    glPushMatrix();
        glTranslatef(0, 0.1, 0);
        glRotatef(90, 1, 0, 0);
        glutSolidSphere(enemyDiameter, 20, 20);
    glPopMatrix();
    /*Cilindar*/
    glPushMatrix();
        glTranslatef(0, 1.05, 0);
        glRotatef(90, 1, 0, 0);
        gluCylinder(gluNewQuadric(), enemyDiameter, enemyDiameter, 1, 20, 1);
    glPopMatrix();
    /*Donja lopta*/
    glPushMatrix();
        glTranslatef(0, 1, 0);
        glRotatef(90, 1, 0, 0);
        glutSolidSphere(enemyDiameter, 20, 20);
    glPopMatrix();
}
static void enemyMovement(int enemy){
    float enemyMovementVector[2];
    float enemyMovementNormVector[2];
    float enemyMovementVectorLength;
    
    float enemyPlayerDistance;
    
    if (enemies[enemy].alive){
        enemyMovementVector[0] = movementVector[0] - enemies[enemy].x;
        enemyMovementVector[1] = movementVector[2] - enemies[enemy].y;
        
        enemyMovementVectorLength = sqrt(enemyMovementVector[0]*enemyMovementVector[0] + enemyMovementVector[1]*enemyMovementVector[1]);
        enemyMovementNormVector[0] = enemyMovementVector[0]/enemyMovementVectorLength;
        enemyMovementNormVector[1] = enemyMovementVector[1]/enemyMovementVectorLength;
        
        enemies[enemy].x += enemyMovementNormVector[0] * enemySpeed;
        enemies[enemy].y += enemyMovementNormVector[1] * enemySpeed;
        
        enemyPlayerDistance = sqrt(pow(enemies[enemy].x-movementVector[0],2) + pow(enemies[enemy].y-movementVector[2],2));
        
        if (enemyPlayerDistance <= enemyDiameter/2 + characterDiameter/2 + 0.25){
            characterHealth -= 5;
            characterHit++;
            enemies[enemy].alive = false;
        }
    }
}


//------------------------I S P I S   N A   E K R A N U---------------------------------------------
static void output(GLfloat x, GLfloat y, char *string){
    
    char *p;

    glPushMatrix();
        glTranslatef(x, y, 0);
        glScalef(1/152.38, 1/152.38, 1/152.38);
        for (p = string; *p; p++)
            glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
    glPopMatrix();
}
static void displayInitialCountdown(){
    int timerSum = timerInitialTick_1 + timerInitialTick_2 + timerInitialTick_3;
    if (timerSum == 0)
        displayText(-0.8, -3, 5, "3", 4);
    else if (timerSum == 1)
        displayText(-0.8, -3, 5, "2", 4);
    else if (timerSum == 2)
        displayText(-0.8, -3, 5, "1", 4);
}
static void displayScore(){
    sprintf(enemiesKilledStr, "Killed: %d", enemiesKilledCounter);
    switch(characterHealth){
        case 20:
            sprintf(playerLives, "Lives: ****");
            break;
        case 15:
            sprintf(playerLives, "Lives: ***");
            break;
        case 10:
            sprintf(playerLives, "Lives: **");
            break;
        case 5:
            sprintf(playerLives, "Lives: *");
            break;
        default:
            sprintf(playerLives, "Lives: ");
            break;
    }
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glDisable(GL_LIGHTING);
    glColor3f(0.4, 0, 0);
    output(1.5, gPlaneScaleY/1.32, enemiesKilledStr);
    output(-5, gPlaneScaleY/1.32, playerLives);
    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
}
static void displayText(GLfloat x, GLfloat y, GLfloat z, char* text, GLfloat scale){
    glPushMatrix();
            glDisable(GL_LIGHTING);
            glEnable(GL_BLEND);
            glEnable(GL_LINE_SMOOTH);
            glColor3f(0.6, 0, 0);
            glRotatef(-45, 1, 0, 0);
            glTranslatef(x, y, z);
            glScalef(scale, scale, scale);
            glLineWidth(3.0);
            output(0, 3, text);
            glDisable(GL_LINE_SMOOTH);
            glDisable(GL_BLEND);
            glEnable(GL_LIGHTING);
        glPopMatrix();
}


//-------------------------I N I C I J A L I Z A C I J A   T E R E N A-----------------------------
static void DecLevelInit(){
    tempX = window_width/2; 
    tempY = window_height/2;
    
    colisionRange = 1 - characterDiameter;
    
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


//-------------------------G E N E R A C I J A   T E R E N A---------------------------
void DecPlane(float colorR, float colorG, float colorB){
    
    GLfloat material_ambient[] = { 0.1, 0.1, 0.1, 1 };
    GLfloat material_diffuse[] = {colorR, colorG, colorB, 1};
    GLfloat material_specular[] = { 0.3, 0.3, 0.3, 1 };
    GLfloat shininess[] = { 5 };
    
    glPushMatrix();
        glScalef(gPlaneScaleX, 1, gPlaneScaleY);
        glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
        glutSolidCube(1);
    glPopMatrix();
}
void DecFloorMatrix(float colorR, float colorG, float colorB, float cubeHeight){
    float color, localCubeHeight;
    int xPos, yPos;
    
    GLfloat material_diffuse_and_ambient_clear[] = {0.415, 415, 415, 1};
    GLfloat material_diffuse_and_ambient_obstacle[4];
    GLfloat material_specular[] = { 0.5, 0.5, 0.5, 0.5 };
    GLfloat low_shininess[] = { 15 };
    GLfloat medium_shininess[] = { 100 };
    
    // 1 ako ima prepreke
    // 0 ako nema
    material_diffuse_and_ambient_clear[0] = 1;
    material_diffuse_and_ambient_clear[1] = 0.1;
    material_diffuse_and_ambient_clear[2] = 0.15;
    material_diffuse_and_ambient_clear[3] = 1;
    
    for (int i=0; i!=matrixSizeX; i++){
        glMaterialfv(GL_FRONT, GL_AMBIENT, material_diffuse_and_ambient_clear);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse_and_ambient_clear);
        glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, medium_shininess);
        xPos = i-(matrixSizeX/2);
        for (int j=0; j!=matrixSizeY; j++){
            yPos = j-(matrixSizeY/2);
            color = 0.55-j*0.03;
            if (M[i][j] == 0){
                
                glPushMatrix();
                    glTranslatef(xPos*2, 0.5, yPos*2);
//                     glColor3f(0.88, 0.88, 0.9);
                    glScalef(1.8, 0.1, 1.8);
                    glutSolidCube(1);
                glPopMatrix();
            }
            else if (M[i][j] == 1){
                localCubeHeight = cubeHeight + M_Obstacle[i][j];
                
                material_diffuse_and_ambient_obstacle[0] = color;
                material_diffuse_and_ambient_obstacle[1] = color;
                material_diffuse_and_ambient_obstacle[2] = color;
                material_diffuse_and_ambient_obstacle[3] = 1;
                
                glPushMatrix();
                    glTranslatef(xPos*2, localCubeHeight/2, yPos*2);
                    glScalef(1.8, localCubeHeight, 1.8);
                    glMaterialfv(GL_FRONT, GL_AMBIENT, material_diffuse_and_ambient_obstacle);
                    glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse_and_ambient_obstacle);
                    glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
                    glMaterialfv(GL_FRONT, GL_SHININESS, low_shininess);
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