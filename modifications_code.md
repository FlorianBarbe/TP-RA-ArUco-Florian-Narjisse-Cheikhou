# Récapitulatif des modifs du code (Pour le rapport)

Voici les blocs de code que j'ai dû toucher pour que ça marche, avec les explications style "dev".

## 1. Les Headers (Début du fichier)
J'ai dû inverser l'ordre des includes parce que Windows déteste quand on met `GL/gl.h` avant `windows.h`. Et j'ai ajouté `cmath` pour les calculs.

```cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath> // Ajouté pour le sin/cos/sqrt

// Attention : Windows.h DOIT être avant gl.h sinon conflits d'API
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h> // Pour la gestion fenêtre/clavier

// Mes includes ArUco et OpenCV
#include <aruco/aruco.h>
#include <opencv2/opencv.hpp>
```

## 2. Variables Globales pour la Physique
J'ai ajouté ça juste avant les fonctions, pour stocker la position et la vitesse de chaque cube. C'est du "fait main" pour le mouvement brownien.

```cpp
// Structure pour gérer la physique de chaque cube
struct CubeState {
    float x, y, z;    // Position relative (offset)
    float vx, vy, vz; // Vitesse actuelle
};

// Liste qui stockera l'état de chaque marqueur détecté
std::vector<CubeState> cubeOffsets;

// Initialisation des vitesses aléatoires
void initBrownian(size_t count) {
    if (cubeOffsets.size() != count) {
        cubeOffsets.resize(count);
        for (int i = 0; i < count; i++) {
            // Vitesse aléatoire petite entre -0.005 et +0.005
            cubeOffsets[i].vx = ((rand() % 100) / 100.0f - 0.5f) * 0.01f;
            cubeOffsets[i].vy = ((rand() % 100) / 100.0f - 0.5f) * 0.01f;
            cubeOffsets[i].x = 0;
            cubeOffsets[i].y = 0;
            cubeOffsets[i].z = 0;
        }
    }
}
```

## 3. Le Main (Choix Caméra)
Dans le `main()`, j'ai viré le `VideoCapture(0)` codé en dur pour demander à l'utilisateur quelle caméra lancer. Pratique quand on a plusieurs webcams.

```cpp
int main(int argc, char** argv) {
    // ... initialisations GLFW ...

    // Demande ID caméra
    int camId = 0;
    std::cout << "Entrez l'ID de la camera (0, 1, 2...) : ";
    std::cin >> camId;

    TheVideoCapturer.open(camId);
    
    // ... boucle principale ...
}
```

## 4. La boucle de rendu `vDrawScene` (Le gros morceau)
C'est là que tout se passe. J'ai tout réécrit pour gérer le fond vidéo, la 3D, la physique et les couleurs.

```cpp
void vDrawScene() {
    // Sécurité anti-crash
    if (TheResizedImage.rows == 0) return;

    // --- PARTIE 1 : Le Fond Vidéo (2D) ---
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST); // On désactive la profondeur pour dessiner le fond
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, TheGlWindowSize.width, 0, TheGlWindowSize.height, -1.0, 1.0);
    
    // On dessine l'image de la webcam pixel par pixel
    glPixelZoom(1, -1);
    glRasterPos3f(0, TheGlWindowSize.height - 0.5, -1.0);
    glDrawPixels(TheGlWindowSize.width, TheGlWindowSize.height, GL_RGB, GL_UNSIGNED_BYTE, TheResizedImage.ptr(0));

    // --- PARTIE 2 : Les Cubes (3D) ---
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    double proj_matrix[16];
    // On charge la matrice de projection de la caméra calibrée
    TheCameraParams.glGetProjectionMatrix(TheInputImage.size(), TheGlWindowSize, proj_matrix, 0.01, 100);
    glLoadMatrixd(proj_matrix);

    // Mise à jour de la physique
    initBrownian(TheMarkers.size());
    // ... logic de collisions ici ...

    // Boucle pour chaque marqueur
    double modelview_matrix[16];
    for (unsigned int m = 0; m < TheMarkers.size(); m++) {
        TheMarkers[m].glGetModelViewMatrix(modelview_matrix);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glLoadMatrixd(modelview_matrix); // On déplace le monde sur le marqueur

        // Couleur Rainbow (Shader CPU)
        float time = (float)glfwGetTime();
        // Déphasage : m * 30.0f pour qu'ils n'aient pas la même couleur en même temps
        float hue = fmod(time * 50.0f + m * 30.0f, 360.0f);
        
        // ... (Conversion HSV vers RGB) ...
        glColor3f(r, g, b);

        // Dessin du Cube
        glPushMatrix();
        glTranslatef(cubeOffsets[m].x, cubeOffsets[m].y, size/2 + cubeOffsets[m].z); // On applique le mouvement
        
        // On dessine le cube face par face (GL_QUADS)
        glBegin(GL_QUADS);
        // ... vertices ...
        glEnd();
        glPopMatrix();
    }
}
```
