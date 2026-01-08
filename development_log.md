# Journal de Développement ArUco 3D

## 2026-01-08 - Session de Développement

---

## 📸 Évolution du Projet

### Première Détection - Axes Uniquement
Détection des 9 marqueurs ArUco avec affichage des axes XYZ (pas encore de cubes visibles).

![Détection des marqueurs avec axes](screenshot_detection_axes.png)

### Deuxième Test - Même Résultat
Avant corrections du code, seuls les axes étaient visibles.

![Axes sur marqueurs](screenshot_detection_axes2.png)

### Build Réussi - Cubes Visibles !
Après toutes les corrections, les cubes 3D colorés apparaissent (problème de taille initial).

![Cubes 3D colorés - première version](screenshot_cubes_v1.png)

### ✅ Version Finale - Cubes Corrigés !
Cubes réduits à 80% de la taille du marqueur, bien positionnés.

![Cubes 3D finaux - 9 marqueurs détectés](screenshot_cubes_v2_final.png)

### ✅ V3 Stable (23:08) - Sans Collision
Après désactivation du code de collision qui causait le crash.

![Version stable sans collision](screenshot_stable_v3.png)

### ✅ V4 Mouvement Brownien (23:13)
Les cubes bougent de manière aléatoire ! Animation vivante.

![Mouvement brownien des cubes](screenshot_brownian_motion.png)

### ⚠️ **RESET COMPLET (Version de Secours - V3 Safe)**
### 🔄 **Réactivation Prudente (V4 Safe)**
**Version Actuelle (V7 RGB Rainbow - Finale) :**
- ❌ **Masques :** Supprimés.
- ✅ **Collision :** ACTIVE (Amortie + Séparation).
- ✅ **Mouvement Brownien :** ACTIVE.
- ✅ **Couleurs Dynamiques :** Effet Arc-en-ciel (RGB) fluide et indépendant pour chaque cube.
- ✅ **Stabilité :** Optimale.

---

## 🐛 Bugs Identifiés et Corrigés

| ID | Description | Sévérité | Statut |
|----|-------------|----------|--------|
| BUG-001 | Caméra hardcodée à index 1 | Moyenne | ✅ CORRIGÉ |
| BUG-002 | `main2` au lieu de `main` | Critique | ✅ CORRIGÉ |
| BUG-003 | Ordre des headers (Windows.h/GL) | Critique | ✅ CORRIGÉ |
| BUG-004 | `aruco` symbole ambigu (conflit cv::aruco) | Moyenne | ✅ CORRIGÉ |
| BUG-005 | Cubes trop grands, faces entremêlées | Moyenne | ✅ CORRIGÉ |
| BUG-006 | Fichiers inutiles compilés (main.cpp, ArUco-OpenGL.cpp) | Moyenne | ✅ CORRIGÉ |
| BUG-007 | **Crash à la détection** - Accès Tvec vide | Critique | ✅ CORRIGÉ |

---

## 📝 Problèmes Observés

### V1 - Faces Entremêlées
- Les faces rouge et cyan alternaient de façon incorrecte
- Cause : Cubes trop grands (100% du marqueur)
- Solution : Réduction à 80% de la taille du marqueur

### V2 - Crash à la Détection (23:03)
- **Symptôme** : L'application crash dès qu'un marqueur est détecté
- **Cause** : Accès à `Tvec.at<double>()` sur un Mat vide (pose non calculée)
- **Solution** : Vérification `if (Tvec.empty())` avant accès

---

## ✅ Fonctionnalités Implémentées

1. **Sélection de caméra** - L'utilisateur choisit l'ID au démarrage
2. **Détection ArUco** - Bibliothèque ArUco 3.1.12
3. **Cubes 3D colorés** - 6 faces avec 6 couleurs différentes
4. **Axes XYZ** - Rouge (X), Vert (Y), Bleu (Z)
5. **Multi-marqueurs** - Support simultané de plusieurs marqueurs
6. **🆕 Collision** - Cubes passent en orange/jaune quand < 15cm

---

## 📋 Exigences TP

### Section 1 - Premier Programme
- [x] Détecter les marqueurs ArUco
- [x] Afficher info sur les marqueurs

### Section 3 - Première Augmentation
- [x] Cube 3D sur chaque marqueur
- [x] Axes de coordonnées

### Section 4 - Application RA
- [x] Plusieurs marqueurs
- [x] Interaction au mouvement (collision)

---

## 📜 Historique

| Heure | Action | Détail |
|-------|--------|--------|
| 22:21 | Analyse | Exploration du projet |
| 22:24 | Fix | main2 → main |
| 22:24 | Feature | Sélection caméra |
| 22:28 | Fix | Taille cube |
| 22:37 | Fix | Ordre headers Windows/GL |
| 22:41 | Fix | Namespace ::aruco:: |
| 22:41 | Fix | Exclusion fichiers inutiles |
| 22:44 | ✅ | BUILD RÉUSSI |
| 22:46 | Fix | Taille cubes 80% |
| 22:58 | Feature | Détection collision (15cm) |
| 23:03 | 🐛 | Bug crash détection signalé |
| 23:04 | Fix | Vérification Tvec.empty() | |
