/**
Copyright 2017 Rafael Muñoz Salinas.All rights reserved.

    Redistribution and use in source and binary forms,
    with or without modification,
    are permitted provided that the following conditions are met :

    1. Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and
                the following disclaimer in the documentation and /
        or other materials provided with the distribution.

           THIS SOFTWARE IS PROVIDED BY Rafael Muñoz Salinas ''AS IS'' AND
               ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO,
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
        PURPOSE ARE DISCLAIMED.IN NO EVENT SHALL Rafael Muñoz Salinas OR
            CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
    OR CONSEQUENTIAL DAMAGES(
        INCLUDING, BUT NOT LIMITED TO,
        PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
        LOSS OF USE, DATA, OR PROFITS;
        OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
    WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT(INCLUDING NEGLIGENCE OR
                OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
    EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    The views and conclusions contained in the software and documentation are
    those of the authors and should
    not be interpreted as representing official policies,
    either expressed or implied,
    of Rafael Muñoz Salinas.*/

// Standard C++ headers
#include <iostream>
#include <string>

// Windows MUST come BEFORE OpenGL
#include <Windows.h>

// OpenGL headers (after Windows.h)
#include <GL/gl.h>
#include <glfw3.h>

// OpenCV
#include <opencv2/opencv.hpp>

// ArUco
#include "aruco.h"

using namespace cv;
// Note: using namespace aruco; removed to avoid conflict with cv::aruco
using namespace std;

string TheInputVideo;
string TheIntrinsicFile;
bool The3DInfoAvailable = false;
float TheMarkerSize = -1;
::aruco::MarkerDetector PPDetector;
VideoCapture TheVideoCapturer;
vector<::aruco::Marker> TheMarkers;
Mat TheInputImage, TheUndInputImage, TheResizedImage;
::aruco::CameraParameters TheCameraParams;
Size TheGlWindowSize;
GLFWwindow *window2;

bool TheCaptureFlag = true;
bool readIntrinsicFile(string TheIntrinsicFile, Mat &TheIntriscCameraMatrix,
                       Mat &TheDistorsionCameraParams, Size size);

// === MOUVEMENT BROWNIEN ===
#include <cmath>
#include <cstdlib>
#include <ctime>

// Structure pour stocker l'offset de mouvement de chaque cube
struct BrownianOffset {
  float x, y, z;
  float vx, vy, vz; // Vitesses
};
vector<BrownianOffset> cubeOffsets;
bool brownianInitialized = false;
float brownianSpeed = 0.002f; // Vitesse du mouvement
float brownianMax = 0.03f;    // Amplitude max du mouvement

// Initialise les offsets aléatoires
void initBrownian(size_t numMarkers) {
  if (!brownianInitialized) {
    srand((unsigned int)time(NULL));
    brownianInitialized = true;
  }
  // S'assurer qu'on a assez d'offsets pour tous les marqueurs
  while (cubeOffsets.size() < numMarkers) {
    BrownianOffset off = {0, 0, 0, 0, 0, 0};
    cubeOffsets.push_back(off);
  }
}

// Met à jour le mouvement brownien
void updateBrownian(size_t idx) {
  if (idx >= cubeOffsets.size())
    return;

  // Ajouter une force aléatoire (mouvement brownien)
  float randForce = brownianSpeed;
  cubeOffsets[idx].vx += (rand() / (float)RAND_MAX - 0.5f) * randForce;
  cubeOffsets[idx].vy += (rand() / (float)RAND_MAX - 0.5f) * randForce;
  cubeOffsets[idx].vz += (rand() / (float)RAND_MAX - 0.5f) * randForce;

  // Friction
  cubeOffsets[idx].vx *= 0.95f;
  cubeOffsets[idx].vy *= 0.95f;
  cubeOffsets[idx].vz *= 0.95f;

  // Appliquer la vitesse
  cubeOffsets[idx].x += cubeOffsets[idx].vx;
  cubeOffsets[idx].y += cubeOffsets[idx].vy;
  cubeOffsets[idx].z += cubeOffsets[idx].vz;

  // Limiter l'amplitude
  if (cubeOffsets[idx].x > brownianMax) {
    cubeOffsets[idx].x = brownianMax;
    cubeOffsets[idx].vx *= -0.5f;
  }
  if (cubeOffsets[idx].x < -brownianMax) {
    cubeOffsets[idx].x = -brownianMax;
    cubeOffsets[idx].vx *= -0.5f;
  }
  if (cubeOffsets[idx].y > brownianMax) {
    cubeOffsets[idx].y = brownianMax;
    cubeOffsets[idx].vy *= -0.5f;
  }
  if (cubeOffsets[idx].y < -brownianMax) {
    cubeOffsets[idx].y = -brownianMax;
    cubeOffsets[idx].vy *= -0.5f;
  }
  if (cubeOffsets[idx].z > brownianMax) {
    cubeOffsets[idx].z = brownianMax;
    cubeOffsets[idx].vz *= -0.5f;
  }
  if (cubeOffsets[idx].z < -brownianMax) {
    cubeOffsets[idx].z = -brownianMax;
    cubeOffsets[idx].vz *= -0.5f;
  }
}
// Fin Mouvement Brownien
void vDrawScene();
void vIdle();
void vResize(GLFWwindow *window, GLsizei iWidth, GLsizei iHeight);
void vMouse(GLFWwindow *window, double x, double y);

/************************************
 *
 *
 *
 *
 ************************************/

bool readArguments(int argc, char **argv) {
  if (argc != 4) {
    cerr << "Invalid number of arguments" << endl;
    cerr << "Usage: (in.avi|live)  intrinsics.yml   size " << endl;
    return false;
  }
  TheInputVideo = argv[1];
  TheIntrinsicFile = argv[2];
  TheMarkerSize = atof(argv[3]);
  return true;
}

/************************************
 *
 *
 *
 *
 ************************************/

void error2(int error, const char *description) {
  cout << "GLFW error code: " << error << ", description: " << description
       << endl;
}

int main(int argc, char **argv) {
  try { // parse arguments
        // if (readArguments(argc, argv) == false)
    //    return 0;
    TheInputVideo = "live";
    TheIntrinsicFile = "camera.yml";
    TheMarkerSize = 0.1;

    // Sélection de la caméra
    int cameraId = 0;
    cout << "======================================" << endl;
    cout << "  TP Realite Augmentee - ArUco" << endl;
    cout << "======================================" << endl;
    cout << "Entrez l'ID de la camera (0 = principale, 1 = secondaire) : ";
    cin >> cameraId;
    cout << "Tentative d'ouverture de la camera " << cameraId << "..." << endl;

    // read from camera
    if (TheInputVideo == "live")
      TheVideoCapturer.open(cameraId);
    else
      TheVideoCapturer.open(TheInputVideo);
    if (!TheVideoCapturer.isOpened()) {
      cerr << "Could not open video" << endl;
      return -1;
    }

    // read first image
    TheVideoCapturer >> TheInputImage;
    // read camera paramters if passed
    TheCameraParams.readFromXMLFile(TheIntrinsicFile);
    TheCameraParams.resize(TheInputImage.size());

    glfwSetErrorCallback(error2);
    if (!glfwInit())
      exit(1);
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
    // glfwWindowHint(GLFW_DEPTH_BITS, 24);
    window2 =
        glfwCreateWindow(TheInputImage.size().width,
                         TheInputImage.size().height, "ArUco", NULL, NULL);
    cout << "Width: " << TheInputImage.size().width
         << " Height: " << TheInputImage.size().height << endl;
    if (!window2) {
      glfwTerminate();
      exit(1);
    }

    // Setting window's initial position
    glfwSetWindowPos(window2, 200, 200);

    glfwSetCursorPosCallback(window2, vMouse);
    glfwSetFramebufferSizeCallback(window2, vResize);

    glfwMakeContextCurrent(window2);

    // glutInit(&argc, argv);
    // glutInitWindowPosition(0, 0);
    // glutInitWindowSize(TheInputImage.size().width,
    // TheInputImage.size().height); glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH |
    // GLUT_DOUBLE); glutCreateWindow("AruCo"); glutDisplayFunc(vDrawScene);
    // glutIdleFunc(vIdle);
    // glutReshapeFunc(vResize);
    // glutMouseFunc(vMouse);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClearDepth(1.0);
    TheGlWindowSize = TheInputImage.size();
    vResize(window2, TheGlWindowSize.width, TheGlWindowSize.height);
    // glutMainLoop();

    // render loop
    while (!glfwWindowShouldClose(window2)) {
      // Clearing color and depth buffers
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // Getting current frame from the camera
      vIdle();

      // Showing images
      vDrawScene();
      // check and call events and swap the buffers
      glfwPollEvents();
      glfwSwapBuffers(window2);
    }
  } catch (std::exception &ex)

  {
    cout << "Exception :" << ex.what() << endl;
  }
}
/************************************
 *
 *
 *
 *
 ************************************/

void vMouse(GLFWwindow *window, double x, double y) {}

/************************************
 *
 *
 *
 *
 ************************************/
void axis(float size) {
  glColor3f(1, 0, 0);
  glBegin(GL_LINES);
  glVertex3f(0.0f, 0.0f, 0.0f); // origin of the line
  glVertex3f(size, 0.0f, 0.0f); // ending point of the line
  glEnd();

  glColor3f(0, 1, 0);
  glBegin(GL_LINES);
  glVertex3f(0.0f, 0.0f, 0.0f); // origin of the line
  glVertex3f(0.0f, size, 0.0f); // ending point of the line
  glEnd();

  glColor3f(0, 0, 1);
  glBegin(GL_LINES);
  glVertex3f(0.0f, 0.0f, 0.0f); // origin of the line
  glVertex3f(0.0f, 0.0f, size); // ending point of the line
  glEnd();
}

// Fonction qui dessine un cube de différentes manières (type)
void drawBox(GLfloat size, GLenum type) {
  static const GLfloat n[6][3] = {{-1.0, 0.0, 0.0}, {0.0, 1.0, 0.0},
                                  {1.0, 0.0, 0.0},  {0.0, -1.0, 0.0},
                                  {0.0, 0.0, 1.0},  {0.0, 0.0, -1.0}};
  static const GLint faces[6][4] = {{0, 1, 2, 3}, {3, 2, 6, 7}, {7, 6, 5, 4},
                                    {4, 5, 1, 0}, {5, 6, 2, 1}, {7, 4, 0, 3}};
  GLfloat v[8][3];
  GLint i;

  v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 2;
  v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 2;
  v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 2;
  v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2;
  v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 2;
  v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 2;

  for (i = 5; i >= 0; i--) {
    glBegin(type);
    glNormal3fv(&n[i][0]);
    glVertex3fv(&v[faces[i][0]][0]);
    glVertex3fv(&v[faces[i][1]][0]);
    glVertex3fv(&v[faces[i][2]][0]);
    glVertex3fv(&v[faces[i][3]][0]);
    glEnd();
  }
}

void drawWireCube(GLdouble size) { drawBox(size, GL_LINE_LOOP); }

// Fonction qui dessine un cube solide avec des couleurs
// (Supprimée pour nettoyage)

void vDrawScene() {
  if (TheResizedImage.rows ==
      0) // prevent from going on until the image is initialized
    return;

  /// clear
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Désactiver le depth test pour dessiner l'image de fond
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);

  /// draw image in the buffer
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, TheGlWindowSize.width, 0, TheGlWindowSize.height, -1.0, 1.0);
  glViewport(0, 0, TheGlWindowSize.width, TheGlWindowSize.height);
  glDisable(GL_TEXTURE_2D);
  glPixelZoom(1, -1);
  glRasterPos3f(0, TheGlWindowSize.height - 0.5, -1.0);
  glDrawPixels(TheGlWindowSize.width, TheGlWindowSize.height, GL_RGB,
               GL_UNSIGNED_BYTE, TheResizedImage.ptr(0));

  // Effacer le depth buffer APRÈS l'image de fond
  glClear(GL_DEPTH_BUFFER_BIT);

  /// Set the appropriate projection matrix
  glMatrixMode(GL_PROJECTION);
  double proj_matrix[16];
  TheCameraParams.glGetProjectionMatrix(TheInputImage.size(), TheGlWindowSize,
                                        proj_matrix, 0.01, 100);
  glLoadIdentity();
  glLoadMatrixd(proj_matrix);

  // Activer le depth test pour le rendu 3D
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  // Désactiver le culling
  glDisable(GL_CULL_FACE);

  // now, for each marker,
  double modelview_matrix[16];
  for (unsigned int m = 0; m < TheMarkers.size(); m++) {
    TheMarkers[m].glGetModelViewMatrix(modelview_matrix);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLoadMatrixd(modelview_matrix);

    // Dessiner les axes
    axis(TheMarkerSize);

    // === MOUVEMENT BROWNIEN ===
    initBrownian(TheMarkers.size());
    updateBrownian(m);

    // === GESTION DES COLLISIONS ===
    // On vérifie la collision avec tous les autres marqueurs
    // Seuil de collision = Distance minimale (taille cube)
    double collisionThreshold = TheMarkerSize * 0.8;

    // Position absolue du marqueur m
    double m_pos[3] = {modelview_matrix[12], modelview_matrix[13],
                       modelview_matrix[14]};

    for (unsigned int other = m + 1; other < TheMarkers.size(); other++) {
      double other_modelview[16];
      TheMarkers[other].glGetModelViewMatrix(other_modelview);
      double o_pos[3] = {other_modelview[12], other_modelview[13],
                         other_modelview[14]};

      // Distance euclidienne
      double dx = m_pos[0] - o_pos[0];
      double dy = m_pos[1] - o_pos[1];
      double dz = m_pos[2] - o_pos[2];
      double dist = sqrt(dx * dx + dy * dy + dz * dz);

      if (dist < collisionThreshold) {
        // COLLISION DETECTEE !
        // On repousse les cubes l'un de l'autre
        // dx, dy sont la direction de répulsion

        if (m < cubeOffsets.size() && other < cubeOffsets.size()) {
          // REBOND : On inverse la vitesse pour "repartir" dans l'autre sens
          // Mais on amortit le rebond (x0.8) pour éviter l'effet "ressort
          // infini" qui fait clignoter
          cubeOffsets[m].vx = -cubeOffsets[m].vx * 0.8f;
          cubeOffsets[m].vy = -cubeOffsets[m].vy * 0.8f;

          cubeOffsets[other].vx = -cubeOffsets[other].vx * 0.8f;
          cubeOffsets[other].vy = -cubeOffsets[other].vy * 0.8f;

          // SEPARATION FORCEE : On décale un peu les cubes pour qu'ils ne
          // soient plus en collision Cela évite que la collision soit
          // redétectée à la frame suivante (ce qui causait le clignotement)
          float pushX = (dx > 0 ? 1.0f : -1.0f) * 0.005f;
          float pushY = (dy > 0 ? 1.0f : -1.0f) * 0.005f;

          cubeOffsets[m].x += pushX;
          cubeOffsets[m].y += pushY;

          cubeOffsets[other].x -= pushX;
          cubeOffsets[other].y -= pushY;
        }
      }
    }

    // Dessiner un cube 3D solide au-dessus du marqueur
    glPushMatrix();
    GLfloat cubeSize = TheMarkerSize * 0.8f;

    // Positionner le cube avec offset brownien (mouvement aléatoire)
    float offX = 0, offY = 0, offZ = 0;
    if (m < cubeOffsets.size()) {
      offX = cubeOffsets[m].x;
      offY = cubeOffsets[m].y;
      offZ = cubeOffsets[m].z;
    }

    // Position normale (Z=0, plus de levage artificiel ni de masque)
    glTranslatef(offX, offY, cubeSize / 2.0f + offZ);

    // Forcer le mode remplissage
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    GLfloat s = cubeSize / 2.0f; // Demi-taille

    // === COULEURS DYNAMIQUES (RGB RAINBOW) ===
    // Calcul de la teinte (Hue) en fonction du temps et de l'index du marqueur
    // Vitesse du cycle : 1.0 (1 tour par seconde environ)
    // Déphasage : m * 0.5 (chaque cube est décalé)
    float time = (float)glfwGetTime();
    float hue = fmod(time * 50.0f + m * 30.0f, 360.0f);

    // Conversion HSV vers RGB (S=1, V=1 pour couleur vive)
    float r, g, b;
    float h = hue / 60.0f;
    int i = (int)floor(h);
    float f = h - i;
    float q = 1.0f - f;

    switch (i % 6) {
    case 0:
      r = 1;
      g = f;
      b = 0;
      break;
    case 1:
      r = q;
      g = 1;
      b = 0;
      break;
    case 2:
      r = 0;
      g = 1;
      b = f;
      break;
    case 3:
      r = 0;
      g = q;
      b = 1;
      break;
    case 4:
      r = f;
      g = 0;
      b = 1;
      break;
    case 5:
      r = 1;
      g = 0;
      b = q;
      break;
    default:
      r = 1;
      g = 1;
      b = 1;
      break; // Should not happen
    }

    // Application de la couleur unique à tout le cube
    glColor3f(r, g, b);

    // Dessin du cube (toutes faces même couleur)
    glBegin(GL_QUADS);
    // Front
    glNormal3f(0, 0, 1);
    glVertex3f(-s, -s, s);
    glVertex3f(s, -s, s);
    glVertex3f(s, s, s);
    glVertex3f(-s, s, s);
    // Back
    glNormal3f(0, 0, -1);
    glVertex3f(s, -s, -s);
    glVertex3f(-s, -s, -s);
    glVertex3f(-s, s, -s);
    glVertex3f(s, s, -s);
    // Right
    glNormal3f(1, 0, 0);
    glVertex3f(s, -s, s);
    glVertex3f(s, -s, -s);
    glVertex3f(s, s, -s);
    glVertex3f(s, s, s);
    // Left
    glNormal3f(-1, 0, 0);
    glVertex3f(-s, -s, -s);
    glVertex3f(-s, -s, s);
    glVertex3f(-s, s, s);
    glVertex3f(-s, s, -s);
    // Top
    glNormal3f(0, 1, 0);
    glVertex3f(-s, s, s);
    glVertex3f(s, s, s);
    glVertex3f(s, s, -s);
    glVertex3f(-s, s, -s);
    // Bottom
    glNormal3f(0, -1, 0);
    glVertex3f(-s, -s, -s);
    glVertex3f(s, -s, -s);
    glVertex3f(s, -s, s);
    glVertex3f(-s, -s, s);
    glEnd(); // Right

    glPopMatrix();
  }
  glDisable(GL_DEPTH_TEST);
}

/************************************
 *
 *
 *
 *
 ************************************/
void vIdle() {
  if (TheCaptureFlag) {
    // capture image
    TheVideoCapturer.grab();
    TheVideoCapturer.retrieve(TheInputImage);

    // SAFETY CHECK: If frame is empty, stop here
    if (TheInputImage.empty())
      return;

    TheUndInputImage.create(TheInputImage.size(), CV_8UC3);
    // transform color that by default is BGR to RGB because windows systems do
    // not allow reading BGR images with opengl properly
    cv::cvtColor(TheInputImage, TheInputImage, cv::COLOR_BGR2RGB);
    // remove distorion in image

    // cv::undistort(TheInputImage, TheUndInputImage,
    // TheCameraParams.CameraMatrix, TheCameraParams.Distorsion);
    TheUndInputImage = TheInputImage.clone();
    // detect markers
    // PPDetector.detect(TheUndInputImage, TheMarkers,
    // TheCameraParams.CameraMatrix, Mat(), TheMarkerSize, false);
    PPDetector.detect(TheUndInputImage, TheMarkers, TheCameraParams,
                      TheMarkerSize, false);
    // resize the image to the size of the GL window
    cv::resize(TheUndInputImage, TheResizedImage, TheGlWindowSize);
  }
}

/************************************
 *
 *
 *
 *
 ************************************/
void vResize(GLFWwindow *window, GLsizei iWidth, GLsizei iHeight) {
  TheGlWindowSize = Size(iWidth, iHeight);
  // not all sizes are allowed. OpenCv images have padding at the end of each
  // line in these that are not aligned to 4 bytes
  if (iWidth * 3 % 4 != 0) {
    iWidth += iWidth * 3 % 4; // resize to avoid padding
    vResize(window, iWidth, TheGlWindowSize.height);
  } else {
    // resize the image to the size of the GL window
    if (TheUndInputImage.rows != 0)
      cv::resize(TheUndInputImage, TheResizedImage, TheGlWindowSize);
  }
  // glfwSetWindowSize(window, iWidth, iHeight);
}

/*
void ArUco::drawScene2() {
    // Si l'image est vide alors on ne fait rien
    if (m_ResizedImage.rows == 0) {
        return;
    }

    // Clearing color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /// draw image in the buffer
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, m_GlWindowSize.width, 0, m_GlWindowSize.height, -1.0, 1.0);
    glViewport(0, 0, m_GlWindowSize.width, m_GlWindowSize.height);
    glDisable(GL_TEXTURE_2D);
    glPixelZoom(1, -1);
    glRasterPos3f(0, m_GlWindowSize.height, -1.0);
    glDrawPixels(m_GlWindowSize.width, m_GlWindowSize.height, GL_RGB,
GL_UNSIGNED_BYTE, m_ResizedImage.ptr(0));
    /// Set the appropriate projection matrix so that rendering is done in a
enrvironment
    // like the real camera (without distorsion)
    glMatrixMode(GL_PROJECTION);
    double proj_matrix[16];
    m_CameraParams.glGetProjectionMatrix(m_InputImage.size(), m_GlWindowSize,
proj_matrix, 0.05, 10); glLoadIdentity(); glLoadMatrixd(proj_matrix);

    // now, for each marker,
    double modelview_matrix[16];
    for (unsigned int m = 0; m < m_Markers.size(); m++)
    {
        m_Markers[m].glGetModelViewMatrix(modelview_matrix);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glLoadMatrixd(modelview_matrix);

        drawAxis(m_MarkerSize);

        glColor3f(1, 0.4, 0.4);
        glTranslatef(0, 0, m_MarkerSize / 2);
        glPushMatrix();
        drawWireCube(m_MarkerSize);

        glPopMatrix();
    }
}
*/