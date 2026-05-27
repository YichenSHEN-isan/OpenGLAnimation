#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cmath>
#include <GL/freeglut.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

// Define global variables
#define M_PI 3.14159
GLuint groundTexture;
GLfloat cameraZ = 12.0f;  // Initial camera Z position
double lastFrameTime = 0.0;

using namespace std;

/*Sakura Falling Effect*/

struct SakuraPetal {
    GLfloat x, y, z;
    GLfloat speed;
};
const int numPetals = 1000;
SakuraPetal petals[numPetals];
// Random position (constrained by a cube) & falling speed
void initPetals() {
    for (int i = 0; i < numPetals; i++) {
        petals[i].x = (rand() / (float)RAND_MAX) * 20.0f - 10.0f;
        petals[i].y = (rand() / (float)RAND_MAX) * 20.0f;
        petals[i].z = (rand() / (float)RAND_MAX) * 20.0f - 10.0f;
        petals[i].speed = (rand() / (float)RAND_MAX) * 0.01f + 0.003f;
    }
}

void drawPetals() {
    GLfloat mat_ambient_sakura[] = { 1.0f, 0.725f, 0.756f, 1.0f };
    GLfloat mat_diffuse_sakura[] = { 1.0f, 0.725f, 0.756f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_sakura);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_sakura);
    glBegin(GL_QUADS);
    for (int i = 0; i < numPetals; i++) {
        glVertex3f(petals[i].x, petals[i].y, petals[i].z);
        glVertex3f(petals[i].x + 0.1f, petals[i].y, petals[i].z);
        glVertex3f(petals[i].x + 0.1f, petals[i].y - 0.1f, petals[i].z + 0.1f);
        glVertex3f(petals[i].x, petals[i].y - 0.1f, petals[i].z + 0.1f);
    }
    glEnd();
}

/*Textured Ground*/

GLuint loadTexture(const char* filepath) { // Accroding to path
    int width = 0;
    int height = 0;
    int sourceChannels = 0;
    unsigned char* data = stbi_load(filepath, &width, &height, &sourceChannels, STBI_rgb);

    if (data == nullptr) {
        std::fprintf(stderr, "Failed to load texture '%s': %s\n", filepath, stbi_failure_reason());
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
    return textureID;
}

void drawTexturedGround() {
    float groundSize = 50.0f;
    float textureRepeat = 100.0f;  // Times the texture repeat across the ground

    glBegin(GL_QUADS);
    glNormal3f(0.0, 1.0, 0.0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-groundSize, 0.0f, -groundSize);
    glTexCoord2f(0.0f, textureRepeat); glVertex3f(-groundSize, 0.0f, groundSize);
    glTexCoord2f(textureRepeat, textureRepeat); glVertex3f(groundSize, 0.0f, groundSize);
    glTexCoord2f(textureRepeat, 0.0f); glVertex3f(groundSize, 0.0f, -groundSize);
    glEnd();
}

/*Basic parts for the objects*/

void drawCylinder(GLfloat radius, GLfloat height, GLint slices, GLint stacks) {
    GLUquadricObj* quadric = gluNewQuadric();
    gluCylinder(quadric, radius, radius, height, slices, stacks);
    gluDeleteQuadric(quadric);
}

void drawTruncatedPyramid(GLfloat bottomWidth, GLfloat bottomHeight, GLfloat topWidth, GLfloat topHeight, GLfloat height) {
    glBegin(GL_QUADS);

    // Draw bottom face
    glVertex3f(-bottomWidth / 2, -bottomHeight / 2, 0);
    glVertex3f(bottomWidth / 2, -bottomHeight / 2, 0);
    glVertex3f(bottomWidth / 2, bottomHeight / 2, 0);
    glVertex3f(-bottomWidth / 2, bottomHeight / 2, 0);

    // Draw top face
    glVertex3f(-topWidth / 2, -topHeight / 2, height);
    glVertex3f(topWidth / 2, -topHeight / 2, height);
    glVertex3f(topWidth / 2, topHeight / 2, height);
    glVertex3f(-topWidth / 2, topHeight / 2, height);

    // Draw front face
    glVertex3f(-bottomWidth / 2, -bottomHeight / 2, 0);
    glVertex3f(bottomWidth / 2, -bottomHeight / 2, 0);
    glVertex3f(topWidth / 2, -topHeight / 2, height);
    glVertex3f(-topWidth / 2, -topHeight / 2, height);

    // Draw back face
    glVertex3f(-bottomWidth / 2, bottomHeight / 2, 0);
    glVertex3f(bottomWidth / 2, bottomHeight / 2, 0);
    glVertex3f(topWidth / 2, topHeight / 2, height);
    glVertex3f(-topWidth / 2, topHeight / 2, height);

    // Draw left face
    glVertex3f(-bottomWidth / 2, -bottomHeight / 2, 0);
    glVertex3f(-bottomWidth / 2, bottomHeight / 2, 0);
    glVertex3f(-topWidth / 2, topHeight / 2, height);
    glVertex3f(-topWidth / 2, -topHeight / 2, height);

    // Draw right face
    glVertex3f(bottomWidth / 2, -bottomHeight / 2, 0);
    glVertex3f(bottomWidth / 2, bottomHeight / 2, 0);
    glVertex3f(topWidth / 2, topHeight / 2, height);
    glVertex3f(topWidth / 2, -topHeight / 2, height);

    glEnd();
}
// Stone lanterns along the way
void drawToro() {
    // Draw the supporting part (hierarchical)
    GLfloat mat_ambient_sup[] = { 0.26f, 0.25f, 0.20f, 1.0f };
    GLfloat mat_diffuse_sup[] = { 0.026f, 0.025f, 0.02f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_sup);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_sup);

    glPushMatrix();
    glScalef(0.2f, 0.05f, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();
    glTranslatef(0.0f, 0.2f, 0.0f);
    glPushMatrix();
    glScalef(0.05f, 0.4f, 0.05f);
    glutSolidCube(1.0f);
    glPopMatrix();
    glTranslatef(0.0f, 0.2f, 0.0f);
    glPushMatrix();
    glScalef(0.2f, 0.05f, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Draw the fire box
    GLfloat mat_ambient_fire[] = { 1.0f, 0.8f, 0.16f, 1.0f };
    GLfloat mat_diffuse_fire[] = { 1.0f, 0.8f, 0.16f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_fire);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_fire);

    glPushMatrix();
    glTranslatef(0.0f, 0.15f, 0.0f);
    glutSolidSphere(0.12f, 32, 32);
    glPopMatrix();

    // Draw the roof
    GLfloat mat_ambient_roof[] = { 0.26f, 0.25f, 0.20f, 1.0f };
    GLfloat mat_diffuse_roof[] = { 0.26f, 0.25f, 0.2f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_roof);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_roof);

    glPushMatrix();
    glTranslatef(0.0f, 0.25f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    drawTruncatedPyramid(0.4f, 0.4f, 0.2f, 0.2f, 0.1f);
    glPopMatrix();

    // Draw the top piece
    glPushMatrix();
    glTranslatef(0.0f, 0.4f, 0.0f);
    glutSolidSphere(0.05f, 32, 32);
    glPopMatrix();
}
// The big door at the end of the road
void drawTorii() { // Does not use hierarchical because different parts have different colors

    // Draw the two cylinder base
    GLfloat mat_ambient_base[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat mat_diffuse_base[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_base);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_base);

    glPushMatrix();
    glTranslatef(-1.0f, 0.0f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    drawCylinder(0.15f, 0.4f, 32, 32);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1.0f, 0.0f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    drawCylinder(0.15f, 0.4f, 32, 32);
    glPopMatrix();

    // Draw the two cyliner posts
    GLfloat mat_ambient_post[] = { 1.0f, 0.1f, 0.1f, 1.0f };
    GLfloat mat_diffuse_post[] = { 1.0f, 0.1f, 0.1f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_post);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_post);

    glPushMatrix();
    glTranslatef(-1.0f, 0.4f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    drawCylinder(0.1f, 2.7f, 32, 32);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1.0f, 0.4f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    drawCylinder(0.1f, 2.7f, 32, 32);
    glPopMatrix();

    // Draw the top beam
    GLfloat mat_ambient_tbeam[] = { 1.0f, 0.1f, 0.1f, 1.0f };
    GLfloat mat_diffuse_tbeam[] = { 1.0f, 0.1f, 0.1f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_tbeam);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_tbeam);
    glPushMatrix();
    glTranslatef(0.0f, 3.0f, 0.0f);
    glScalef(3.2f, 0.2f, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Draw the slice piece on the top beam
    GLfloat mat_ambient_tslice[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat mat_diffuse_tslice[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_tslice);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_tslice);
    glPushMatrix();
    glTranslatef(0.0f, 3.1f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    drawTruncatedPyramid(3.2f, 0.2f, 3.6f, 0.2f, 0.15f);
    glPopMatrix();

    // Draw the middle beam
    GLfloat mat_ambient_mbeam[] = { 1.0f, 0.1f, 0.1f, 1.0f };
    GLfloat mat_diffuse_mbeam[] = { 1.0f, 0.1f, 0.1f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_mbeam);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_mbeam);
    glPushMatrix();
    glTranslatef(0.0f, 2.6f, 0.0f);
    glScalef(2.7f, 0.2f, 0.15f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Draw the name board between the two beams
    GLfloat mat_ambient_board[] = { 0.5f, 0.4f, 0.08f, 1.0f };
    GLfloat mat_diffuse_board[] = { 0.5f, 0.4f, 0.08f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_board);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_board);
    glPushMatrix();
    glTranslatef(0.0f, 2.8f, 0.0f);
    glScalef(0.2f, 0.2f, 0.15f);
    glutSolidCube(1.0f);
    glPopMatrix();
}

/*Shadow for objects*/

void shadowMatrix(GLfloat shadowMat[16], GLfloat groundplane[4], GLfloat lightpos[4]) {
    GLfloat dot = groundplane[0] * lightpos[0] +
        groundplane[1] * lightpos[1] +
        groundplane[2] * lightpos[2] +
        groundplane[3] * lightpos[3];

    shadowMat[0] = dot - lightpos[0] * groundplane[0];
    shadowMat[4] = 0.0f - lightpos[0] * groundplane[1];
    shadowMat[8] = 0.0f - lightpos[0] * groundplane[2];
    shadowMat[12] = 0.0f - lightpos[0] * groundplane[3];

    shadowMat[1] = 0.0f - lightpos[1] * groundplane[0];
    shadowMat[5] = dot - lightpos[1] * groundplane[1];
    shadowMat[9] = 0.0f - lightpos[1] * groundplane[2];
    shadowMat[13] = 0.0f - lightpos[1] * groundplane[3];

    shadowMat[2] = 0.0f - lightpos[2] * groundplane[0];
    shadowMat[6] = 0.0f - lightpos[2] * groundplane[1];
    shadowMat[10] = dot - lightpos[2] * groundplane[2];
    shadowMat[14] = 0.0f - lightpos[2] * groundplane[3];

    shadowMat[3] = 0.0f - lightpos[3] * groundplane[0];
    shadowMat[7] = 0.0f - lightpos[3] * groundplane[1];
    shadowMat[11] = 0.0f - lightpos[3] * groundplane[2];
    shadowMat[15] = dot - lightpos[3] * groundplane[3];
}

void drawShadows() {
    GLfloat lightPosition[4] = { 10.0f, 10.0f, 10.0f, 0.0f };  // Position of the light
    GLfloat groundPlane[4] = { 0.0f, 1.0f, 0.0f, 0.0f };  // Ground plane (y = 0)

    GLfloat shadowMat[16];
    shadowMatrix(shadowMat, groundPlane, lightPosition);

    glDisable(GL_LIGHTING);
    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);  // Shadow color
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPushMatrix();
    glMultMatrixf(shadowMat);

    // Draw all objects that have shadow
    drawTorii();
    for (float i = 1; i <= 10; i++) {
        glPushMatrix();
        glTranslatef(-1.0f, 0.0f, 1.0f * i);
        drawToro();
        glPopMatrix();

        glPushMatrix();
        glTranslatef(1.0f, 0.0f, 1.0f * i);
        drawToro();
        glPopMatrix();
    }

    glPopMatrix();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

/*Objects to be displayed and animated*/

void render(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up the camera
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, 1.0, 1.0, 20.0);
    gluLookAt(0.0, 0.5, cameraZ, 0.0, 1.5, 0.0, 0.0, 1.0, 0.0);
    glMatrixMode(GL_MODELVIEW);

    // Set up the lighting
    GLfloat light_ambient[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat light_diffuse[] = { 0.6f, 0.6f, 0.6f, 1.0f };
    GLfloat light_position[] = { 10.0f, 10.0f, 10.0f, 0.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, groundTexture);
    // Draw textured ground
    drawTexturedGround();
    // Continue with other rendering
    glDisable(GL_TEXTURE_2D);

    drawShadows();

    // Draw objects
    glPushMatrix();
    drawTorii();
    glPopMatrix();

        // All Toros
    for (float i = 1; i <= 10; i++) {
        glPushMatrix();
        glTranslatef(-1.0f, 0.0f, 1.0f * i);
        drawToro();
        glPopMatrix();

        glPushMatrix();
        glTranslatef(1.0f, 0.0f, 1.0f * i);
        drawToro();
        glPopMatrix();
    }

    drawPetals();
    
    glutSwapBuffers();
}
// Update positions accroding to speed
void updatePetals() {
    double currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    double deltaTime = currentTime - lastFrameTime;
    for (int i = 0; i < numPetals; i++) {
        petals[i].y -= petals[i].speed;
        petals[i].x += static_cast<GLfloat>(petals[i].speed * 80.0f * deltaTime);
        if (petals[i].y < -10.0f) {  // Reset the x position when the petal falls under the limit
            petals[i].y += 20.0f;
            petals[i].x = (rand() / (float)RAND_MAX) * 20.0f - 10.0f;  
        }
    }
}
// Overall update, accroding to actual time
void update(void) {
    double currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0; // Convert milliseconds to seconds
    double deltaTime = currentTime - lastFrameTime;
    cameraZ -= static_cast<GLfloat>(0.5f * deltaTime); // Move the camera along the road
    if (cameraZ < 0.0f) cameraZ =12.0f;
    updatePetals(); // Sakura
    glutPostRedisplay();
    lastFrameTime = currentTime;
}







int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(800, 600);
    glutCreateWindow("3D Scene");
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // Set the background color to black

    groundTexture = loadTexture("assets/stone_texture.jpg"); // Load texture for ground (stb_image)

    initPetals(); // Initialize the Sakura

    glutDisplayFunc(render);

    glutIdleFunc(update);

    glutMainLoop();
    return 0;
}
