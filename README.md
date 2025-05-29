# Gravity Simulation 3D / 3D-Gravitationssimulation

![OpenGL](https://img.shields.io/badge/OpenGL-3.3-blue)
![C++](https://img.shields.io/badge/C++-17-green)
![GLFW](https://img.shields.io/badge/GLFW-3-orange)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey)

*🌍 English | [🇩🇪 Deutsch](#deutsch)*

---

## English

### 📖 Overview

This is a **forked repository** featuring a 3D gravitational physics simulation built with OpenGL and C++. The project demonstrates real-time N-body gravitational interactions between celestial objects, complete with spacetime curvature visualization and interactive object creation.

> **Note**: This is a fork of an existing project. All credit for the original implementation goes to the original author(s).

### ✨ Features

- **Real-time Physics**: Implements Newton's law of universal gravitation with accurate physics constants
- **3D Visualization**: Modern OpenGL 3.3 Core Profile rendering with custom shaders
- **Interactive Controls**: 
  - FPS-style camera movement (WASD + mouse)
  - Dynamic object creation and mass adjustment
  - Real-time simulation pause/resume
- **Spacetime Grid**: Visual representation of gravitational field distortion
- **Lighting Effects**: Dynamic lighting and glow effects for celestial bodies
- **Collision Detection**: Basic sphere-sphere collision with velocity damping

### 🎮 Controls

| Control | Action |
|---------|--------|
| `WASD` | Move camera horizontally |
| `Space/Shift` | Move camera up/down |
| `Mouse` | Look around (FPS-style) |
| `Mouse Wheel` | Zoom in/out |
| `Left Click` | Create new object |
| `Right Click (Hold)` | Increase object mass |
| `Arrow Keys` | Position object during creation |
| `K` | Pause/Resume simulation |
| `Q` | Quit application |

### 🛠️ Requirements

#### Software Requirements
- **Windows OS** (MSYS2 MinGW-w64 environment)
- **G++ Compiler** (C++17 support)
- **VS Code** (recommended IDE)

#### Dependencies
- **GLFW3** - Window management and input handling
- **GLEW** - OpenGL extension loading
- **GLM** - Mathematics library for graphics
- **OpenGL 3.3+** - Graphics API

### 🚀 Installation & Setup

#### 1. Prerequisites
```bash
# Install MSYS2 from https://www.msys2.org/
# Then install required packages:
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-glfw
pacman -S mingw-w64-x86_64-glew
pacman -S mingw-w64-x86_64-glm
```

#### 2. Clone Repository
```bash
git clone <repository-url>
cd gravity-simulation
```

#### 3. VS Code Setup
- Open the project folder in VS Code
- The `.vscode` folder contains pre-configured settings:
  - `tasks.json` - Build configuration
  - `launch.json` - Debug configuration
  - `c_cpp_properties.json` - IntelliSense settings
  - `settings.json` - File associations

#### 4. Build Project
```bash
# Using VS Code (recommended)
Ctrl+Shift+B  # Run build task

# Or manually
g++ gravity_sim.cpp -o gravity_sim.exe -lglfw3 -lopengl32 -lgdi32 -I C:/msys64/mingw64/include -L C:/msys64/mingw64/lib
```

### 🎯 Usage

1. **Launch the simulation**:
   ```bash
   ./src/gravity_sim.exe
   ```

2. **Explore the pre-loaded system**: 
   - Two Earth-like objects orbiting a central star
   - Observe gravitational interactions and spacetime distortion

3. **Create new objects**:
   - Left-click to start creating an object
   - Use arrow keys to position it
   - Right-click and hold to increase mass
   - Release left-click to launch into simulation

4. **Navigate the 3D space**:
   - Use WASD for movement
   - Mouse for camera rotation
   - Mouse wheel for zooming

### 📁 Project Structure

```
gravity-simulation/
├── .vscode/                    # VS Code configuration
│   ├── c_cpp_properties.json  # IntelliSense settings
│   ├── launch.json            # Debug configuration
│   ├── settings.json          # File associations
│   └── tasks.json             # Build tasks
├── src/
│   ├── 3D_test.cpp            # Basic OpenGL triangle demo
│   ├── gravity_sim.cpp        # Main gravity simulation
│   └── gravity_sim_3Dgrid.cpp # Alternative version with enhanced grid
└── README.md                  # This file
```

### 🔧 Development

#### Debugging
- Press `F5` in VS Code to start debugging session
- Breakpoints and variable inspection fully supported
- GDB integration with pretty-printing enabled

#### Code Documentation
The codebase is extensively documented with bilingual comments (English/German) explaining:
- Function purposes and parameters
- OpenGL concepts and operations
- Physics calculations and algorithms
- Rendering pipeline details

### 🤝 Contributing

Since this is a forked repository, please:
1. Fork this repository
2. Create a feature branch
3. Make your changes with proper documentation
4. Submit a pull request

### 📄 License

Please refer to the original repository for licensing information.

### 🙏 Credits

This project is a fork of an existing gravity simulation. All credit for the original implementation, architecture, and physics calculations goes to the original author(s).

---

## Deutsch

### 📖 Überblick

Dies ist ein **geforktes Repository** mit einer 3D-Gravitationsphysik-Simulation, die mit OpenGL und C++ erstellt wurde. Das Projekt demonstriert Echtzeit-N-Körper-Gravitationswechselwirkungen zwischen Himmelsobjekten, komplett mit Raumzeit-Krümmungsvisualisierung und interaktiver Objekterstellung.

> **Hinweis**: Dies ist ein Fork eines bestehenden Projekts. Alle Anerkennung für die ursprüngliche Implementierung gebührt dem/den ursprünglichen Autor(en).

### ✨ Funktionen

- **Echtzeit-Physik**: Implementiert Newtons Gravitationsgesetz mit genauen Physikkonstanten
- **3D-Visualisierung**: Modernes OpenGL 3.3 Core Profile Rendering mit benutzerdefinierten Shadern
- **Interaktive Steuerung**:
  - FPS-Stil Kamerabewegung (WASD + Maus)
  - Dynamische Objekterstellung und Massenanpassung
  - Echtzeit Simulation pausieren/fortsetzen
- **Raumzeit-Gitter**: Visuelle Darstellung der Gravitationsfeldverzerrung
- **Lichteffekte**: Dynamische Beleuchtung und Leuchteffekte für Himmelskörper
- **Kollisionserkennung**: Basis Kugel-Kugel-Kollision mit Geschwindigkeitsdämpfung

### 🎮 Steuerung

| Steuerung | Aktion |
|-----------|--------|
| `WASD` | Kamera horizontal bewegen |
| `Leertaste/Shift` | Kamera hoch/runter bewegen |
| `Maus` | Umsehen (FPS-Stil) |
| `Mausrad` | Ein-/Auszoomen |
| `Linksklick` | Neues Objekt erstellen |
| `Rechtsklick (Halten)` | Objektmasse erhöhen |
| `Pfeiltasten` | Objekt während Erstellung positionieren |
| `K` | Simulation pausieren/fortsetzen |
| `Q` | Anwendung beenden |

### 🛠️ Anforderungen

#### Software-Anforderungen
- **Windows OS** (MSYS2 MinGW-w64 Umgebung)
- **G++ Compiler** (C++17 Unterstützung)
- **VS Code** (empfohlene IDE)

#### Abhängigkeiten
- **GLFW3** - Fensterverwaltung und Eingabebehandlung
- **GLEW** - OpenGL-Erweiterungsladung
- **GLM** - Mathematikbibliothek für Grafiken
- **OpenGL 3.3+** - Grafik-API

### 🚀 Installation & Einrichtung

#### 1. Voraussetzungen
```bash
# MSYS2 von https://www.msys2.org/ installieren
# Dann erforderliche Pakete installieren:
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-glfw
pacman -S mingw-w64-x86_64-glew
pacman -S mingw-w64-x86_64-glm
```

#### 2. Repository Klonen
```bash
git clone <repository-url>
cd gravity-simulation
```

#### 3. VS Code Einrichtung
- Öffnen Sie den Projektordner in VS Code
- Der `.vscode` Ordner enthält vorkonfigurierte Einstellungen:
  - `tasks.json` - Build-Konfiguration
  - `launch.json` - Debug-Konfiguration
  - `c_cpp_properties.json` - IntelliSense-Einstellungen
  - `settings.json` - Dateizuordnungen

#### 4. Projekt Erstellen
```bash
# Mit VS Code (empfohlen)
Strg+Shift+B  # Build-Task ausführen

# Oder manuell
g++ gravity_sim.cpp -o gravity_sim.exe -lglfw3 -lopengl32 -lgdi32 -I C:/msys64/mingw64/include -L C:/msys64/mingw64/lib
```

### 🎯 Verwendung

1. **Simulation starten**:
   ```bash
   ./src/gravity_sim.exe
   ```

2. **Vorgeladenes System erkunden**:
   - Zwei erdähnliche Objekte, die einen zentralen Stern umkreisen
   - Gravitationswechselwirkungen und Raumzeitverzerrung beobachten

3. **Neue Objekte erstellen**:
   - Linksklick zum Erstellen eines Objekts
   - Pfeiltasten zur Positionierung verwenden
   - Rechtsklick halten zum Erhöhen der Masse
   - Linksklick loslassen zum Starten in die Simulation

4. **Im 3D-Raum navigieren**:
   - WASD für Bewegung verwenden
   - Maus für Kamerarotation
   - Mausrad zum Zoomen

### 📁 Projektstruktur

```
gravity-simulation/
├── .vscode/                    # VS Code Konfiguration
│   ├── c_cpp_properties.json  # IntelliSense Einstellungen
│   ├── launch.json            # Debug-Konfiguration
│   ├── settings.json          # Dateizuordnungen
│   └── tasks.json             # Build-Tasks
├── src/
│   ├── 3D_test.cpp            # Basis OpenGL Dreieck Demo
│   ├── gravity_sim.cpp        # Haupt-Gravitationssimulation
│   └── gravity_sim_3Dgrid.cpp # Alternative Version mit verbessertem Gitter
└── README.md                  # Diese Datei
```

### 🔧 Entwicklung

#### Debugging
- `F5` in VS Code drücken zum Starten der Debug-Sitzung
- Haltepunkte und Variableninspektion vollständig unterstützt
- GDB-Integration mit aktiviertem Pretty-Printing

#### Code-Dokumentation
Die Codebasis ist umfangreich mit zweisprachigen Kommentaren (Englisch/Deutsch) dokumentiert, die erklären:
- Funktionszwecke und Parameter
- OpenGL-Konzepte und -Operationen
- Physikberechnungen und Algorithmen
- Rendering-Pipeline-Details

### 🤝 Beitragen

Da dies ein geforktes Repository ist, bitte:
1. Forken Sie dieses Repository
2. Erstellen Sie einen Feature-Branch
3. Machen Sie Ihre Änderungen mit ordnungsgemäßer Dokumentation
4. Reichen Sie einen Pull Request ein

### 📄 Lizenz

Bitte beziehen Sie sich auf das ursprüngliche Repository für Lizenzinformationen.

### 🙏 Danksagungen

Dieses Projekt ist ein Fork einer bestehenden Gravitationssimulation. Alle Anerkennung für die ursprüngliche Implementierung, Architektur und Physikberechnungen gebührt dem/den ursprünglichen Autor(en).

---

*Made with ❤️ and physics*
