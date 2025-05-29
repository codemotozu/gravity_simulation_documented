/// Gravity Simulation 3D
/// 
/// A 3D gravitational N-body simulation using OpenGL for visualization. // Eine 3D-Gravitations-N-Körper-Simulation mit OpenGL zur Visualisierung.
/// Simulates gravitational interactions between celestial bodies with real physics constants. // Simuliert Gravitationswechselwirkungen zwischen Himmelskörpern mit realen Physikkonstanten.
/// Features interactive object creation, mass adjustment, and space-time grid visualization. // Bietet interaktive Objekterstellung, Massenanpassung und Raum-Zeit-Gitter-Visualisierung.
/// 
/// Controls: // Steuerung:
/// - WASD: Move camera horizontally // Kamera horizontal bewegen
/// - Space/Shift: Move camera up/down // Kamera hoch/runter bewegen
/// - Mouse: Look around // Umsehen
/// - Left Click: Create new object // Neues Objekt erstellen
/// - Right Click (hold): Increase object mass // Objektmasse erhöhen
/// - Arrow Keys: Position object during creation // Objekt während der Erstellung positionieren
/// - K: Pause/Resume simulation // Simulation pausieren/fortsetzen
/// - Q: Quit application // Anwendung beenden
/// 
/// EN: Uses Newton's law of universal gravitation to calculate forces between objects
/// DE: Verwendet Newtons Gravitationsgesetz zur Berechnung der Kräfte zwischen Objekten

#include <GL/glew.h> // OpenGL Extension Wrangler Library for modern OpenGL functions // OpenGL Extension Wrangler Library für moderne OpenGL-Funktionen
#include <GLFW/glfw3.h> // Window management and input handling // Fensterverwaltung und Eingabebehandlung
#include <glm/glm.hpp> // OpenGL Mathematics library for vector/matrix operations // OpenGL Mathematics-Bibliothek für Vektor-/Matrixoperationen
#include <glm/gtc/matrix_transform.hpp> // Matrix transformation functions (translate, rotate, scale) // Matrix-Transformationsfunktionen (Verschieben, Drehen, Skalieren)
#include <glm/gtc/type_ptr.hpp> // Type pointer functions for passing matrices to OpenGL // Typ-Zeiger-Funktionen zum Übergeben von Matrizen an OpenGL
#include <vector> // Dynamic array container for storing objects // Dynamischer Array-Container zum Speichern von Objekten
#include <iostream> // Input/output stream for console messages // Ein-/Ausgabe-Stream für Konsolennachrichten

/// Vertex shader source code
/// Transforms 3D positions to screen coordinates using MVP matrices // Transformiert 3D-Positionen in Bildschirmkoordinaten mit MVP-Matrizen
const char* vertexShaderSource = R"glsl(#version 330 core
layout(location=0)in vec3 aPos;uniform mat4 model;uniform mat4 view;uniform mat4 projection;
void main(){gl_Position=projection*view*model*vec4(aPos,1.0);})glsl";

/// Fragment shader source code
/// Outputs the color for each pixel using uniform color // Gibt die Farbe für jeden Pixel mit einheitlicher Farbe aus
const char* fragmentShaderSource = R"glsl(
#version 330 core
out vec4 FragColor;
uniform vec4 objectColor; // Add this uniform
void main() {
    FragColor = objectColor; // Use the uniform color
}
)glsl";

// Global state variables // Globale Zustandsvariablen
bool running = true; // Main loop control flag // Hauptschleifen-Kontrollflag
bool pause = false; // Simulation pause state // Simulationspause-Zustand
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  1.0f); // Camera position in world space // Kameraposition im Weltraum
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // Camera forward direction // Kamera-Vorwärtsrichtung
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f); // Camera up vector // Kamera-Aufwärtsvektor
float lastX = 400.0, lastY = 300.0; // Last mouse position for camera rotation // Letzte Mausposition für Kameradrehung
float yaw = -90; // Camera yaw angle in degrees // Kamera-Gierwinkel in Grad
float pitch = 0.0; // Camera pitch angle in degrees // Kamera-Nickwinkel in Grad
float deltaTime = 0.0; // Time between current and last frame // Zeit zwischen aktuellem und letztem Frame
float lastFrame = 0.0; // Time of last frame // Zeit des letzten Frames

// Physical constants // Physikalische Konstanten
const double G = 6.6743e-11; // Gravitational constant in m^3 kg^-1 s^-2 // Gravitationskonstante in m^3 kg^-1 s^-2
const float c = 299792458.0; // Speed of light in m/s (for relativistic effects) // Lichtgeschwindigkeit in m/s (für relativistische Effekte)
float initMass = 5.0f * pow(10, 20) / 5; // Default mass for new objects in kg // Standardmasse für neue Objekte in kg

// Function declarations // Funktionsdeklarationen
GLFWwindow* StartGLU(); // Initialize GLFW and create window // GLFW initialisieren und Fenster erstellen
GLuint CreateShaderProgram(const char* vertexSource, const char* fragmentSource); // Compile and link shaders // Shader kompilieren und verknüpfen
void CreateVBOVAO(GLuint& VAO, GLuint& VBO, const float* vertices, size_t vertexCount); // Create vertex buffer objects // Vertex-Buffer-Objekte erstellen
void UpdateCam(GLuint shaderProgram, glm::vec3 cameraPos); // Update camera view matrix // Kamera-Ansichtsmatrix aktualisieren
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods); // Handle keyboard input // Tastatureingabe behandeln
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods); // Handle mouse button input // Maustasten-Eingabe behandeln
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset); // Handle mouse scroll // Maus-Scrollen behandeln
void mouse_callback(GLFWwindow* window, double xpos, double ypos); // Handle mouse movement // Mausbewegung behandeln
glm::vec3 sphericalToCartesian(float r, float theta, float phi); // Convert spherical to Cartesian coordinates // Kugelkoordinaten in kartesische Koordinaten umwandeln
void DrawGrid(GLuint shaderProgram, GLuint gridVAO, size_t vertexCount); // Render the grid // Gitter rendern

/// Object class
/// Represents a celestial body with mass, position, velocity, and visual properties // Repräsentiert einen Himmelskörper mit Masse, Position, Geschwindigkeit und visuellen Eigenschaften
class Object {
    public:
        GLuint VAO, VBO; // Vertex Array Object and Vertex Buffer Object for rendering // Vertex Array Object und Vertex Buffer Object zum Rendern
        glm::vec3 position = glm::vec3(400, 300, 0); // Position in 3D space // Position im 3D-Raum
        glm::vec3 velocity = glm::vec3(0, 0, 0); // Velocity vector // Geschwindigkeitsvektor
        size_t vertexCount; // Number of vertices in the mesh // Anzahl der Vertices im Mesh
        glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // RGBA color // RGBA-Farbe

        bool Initalizing = false; // Object is being created/positioned // Objekt wird erstellt/positioniert
        bool Launched = false; // Object has been launched into simulation // Objekt wurde in die Simulation gestartet
        bool target = false; // Object is a target (unused in this version) // Objekt ist ein Ziel (in dieser Version ungenutzt)

        float mass; // Mass in kilograms // Masse in Kilogramm
        float density; // Density in kg/m^3 // Dichte in kg/m^3
        float radius; // Visual radius (scaled for display) // Visueller Radius (für Anzeige skaliert)

        glm::vec3 LastPos = position; // Previous position (for trails/history) // Vorherige Position (für Spuren/Historie)

        /// Constructor
        /// Creates a new celestial object with given properties // Erstellt ein neues Himmelsobjekt mit gegebenen Eigenschaften
        /// @param initPosition Initial position in 3D space // Anfangsposition im 3D-Raum
        /// @param initVelocity Initial velocity vector // Anfangsgeschwindigkeitsvektor
        /// @param mass Object mass in kg // Objektmasse in kg
        /// @param density Material density in kg/m^3 (default: 3344 - rocky) // Materialdichte in kg/m^3 (Standard: 3344 - felsig)
        Object(glm::vec3 initPosition, glm::vec3 initVelocity, float mass, float density = 3344) {   
            this->position = initPosition;
            this->velocity = initVelocity;
            this->mass = mass;
            this->density = density;
            // Calculate radius from mass and density using sphere volume formula // Radius aus Masse und Dichte mit Kugelvolumenformel berechnen
            // V = (4/3)πr³, m = ρV, therefore r = ∛(3m/4πρ)
            this->radius = pow(((3 * this->mass/this->density)/(4 * 3.14159265359)), (1.0f/3.0f)) / 100000; // Scale down for visualization // Für Visualisierung verkleinern
            
            // Generate vertices (centered at origin) // Vertices generieren (am Ursprung zentriert)
            std::vector<float> vertices = Draw();
            vertexCount = vertices.size();

            CreateVBOVAO(VAO, VBO, vertices.data(), vertexCount);
        }

        /// Generate sphere mesh vertices
        /// Creates a UV sphere mesh with specified resolution // Erstellt ein UV-Kugel-Mesh mit angegebener Auflösung
        std::vector<float> Draw() {
            std::vector<float> vertices;
            int stacks = 10; // Vertical divisions // Vertikale Unterteilungen
            int sectors = 10; // Horizontal divisions // Horizontale Unterteilungen

            // Generate sphere vertices using spherical coordinates // Kugelvertices mit Kugelkoordinaten generieren
            for(float i = 0.0f; i <= stacks; ++i){
                float theta1 = (i / stacks) * glm::pi<float>(); // Current stack angle // Aktueller Stapelwinkel
                float theta2 = (i+1) / stacks * glm::pi<float>(); // Next stack angle // Nächster Stapelwinkel
                for (float j = 0.0f; j < sectors; ++j){
                    float phi1 = j / sectors * 2 * glm::pi<float>(); // Current sector angle // Aktueller Sektorwinkel
                    float phi2 = (j+1) / sectors * 2 * glm::pi<float>(); // Next sector angle // Nächster Sektorwinkel
                    
                    // Convert spherical to Cartesian coordinates // Kugelkoordinaten in kartesische umwandeln
                    glm::vec3 v1 = sphericalToCartesian(radius, theta1, phi1);
                    glm::vec3 v2 = sphericalToCartesian(radius, theta1, phi2);
                    glm::vec3 v3 = sphericalToCartesian(radius, theta2, phi1);
                    glm::vec3 v4 = sphericalToCartesian(radius, theta2, phi2);

                    // Triangle 1: v1-v2-v3 // Dreieck 1: v1-v2-v3
                    vertices.insert(vertices.end(), {v1.x, v1.y, v1.z}); //      /|
                    vertices.insert(vertices.end(), {v2.x, v2.y, v2.z}); //     / |
                    vertices.insert(vertices.end(), {v3.x, v3.y, v3.z}); //    /__|
                    
                    // Triangle 2: v2-v4-v3 // Dreieck 2: v2-v4-v3
                    vertices.insert(vertices.end(), {v2.x, v2.y, v2.z});
                    vertices.insert(vertices.end(), {v4.x, v4.y, v4.z});
                    vertices.insert(vertices.end(), {v3.x, v3.y, v3.z});
                }   
            }
            return vertices;
        }
        
        /// Update object position based on velocity
        /// Uses Euler integration with fixed timestep // Verwendet Euler-Integration mit festem Zeitschritt
        void UpdatePos(){
            this->position[0] += this->velocity[0] / 94; // X position update with timestep // X-Position mit Zeitschritt aktualisieren
            this->position[1] += this->velocity[1] / 94; // Y position update with timestep // Y-Position mit Zeitschritt aktualisieren
            this->position[2] += this->velocity[2] / 94; // Z position update with timestep // Z-Position mit Zeitschritt aktualisieren
            // Recalculate radius (in case mass changed) // Radius neu berechnen (falls Masse geändert)
            this->radius = pow(((3 * this->mass/this->density)/(4 * 3.14159265359)), (1.0f/3.0f)) / 100000;
        }
        
        /// Update vertex buffer with new mesh data
        /// Called when object size changes // Wird aufgerufen, wenn sich die Objektgröße ändert
        void UpdateVertices() {
            // Generate new vertices with current radius // Neue Vertices mit aktuellem Radius generieren
            std::vector<float> vertices = Draw();
            
            // Update the VBO with new vertex data // VBO mit neuen Vertex-Daten aktualisieren
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        }
        
        /// Get current position
        glm::vec3 GetPos() const {
            return this->position;
        }
        
        /// Apply acceleration to velocity
        /// @param x,y,z Acceleration components // Beschleunigungskomponenten
        void accelerate(float x, float y, float z){
            this->velocity[0] += x / 96; // Apply acceleration with timestep // Beschleunigung mit Zeitschritt anwenden
            this->velocity[1] += y / 96;
            this->velocity[2] += z / 96;
        }
        
        /// Check collision with another object
        /// Returns velocity multiplier (-0.2 for collision, 1.0 for no collision) // Gibt Geschwindigkeitsmultiplikator zurück (-0.2 bei Kollision, 1.0 ohne)
        float CheckCollision(const Object& other) {
            float dx = other.position[0] - this->position[0];
            float dy = other.position[1] - this->position[1];
            float dz = other.position[2] - this->position[2];
            float distance = std::pow(dx*dx + dy*dy + dz*dz, (1.0f/2.0f)); // Euclidean distance // Euklidische Distanz
            if (other.radius + this->radius > distance){
                return -0.2f; // Collision detected, reverse velocity // Kollision erkannt, Geschwindigkeit umkehren
            }
            return 1.0f; // No collision // Keine Kollision
        }
};

std::vector<Object> objs = {}; // Container for all objects in simulation // Container für alle Objekte in der Simulation

// Function declaration for grid generation // Funktionsdeklaration für Gittergenerierung
std::vector<float> CreateGridVertices(float size, int divisions, const std::vector<Object>& objs);

GLuint gridVAO, gridVBO; // Grid vertex array and buffer objects // Gitter-Vertex-Array und Buffer-Objekte

/// Main function
/// Entry point of the gravity simulation // Einstiegspunkt der Gravitationssimulation
int main() {
    GLFWwindow* window = StartGLU(); // Initialize graphics and create window // Grafik initialisieren und Fenster erstellen
    GLuint shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource); // Create shader program // Shader-Programm erstellen

    // Get shader uniform locations // Shader-Uniform-Locations abrufen
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");
    glUseProgram(shaderProgram);

    // Set up input callbacks // Eingabe-Callbacks einrichten
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Hide cursor for FPS-style camera // Cursor für FPS-Stil-Kamera verstecken

    // Set up projection matrix // Projektionsmatrix einrichten
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 750000.0f); // FOV, aspect ratio, near/far planes // FOV, Seitenverhältnis, Nah-/Fernebenen
    GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    cameraPos = glm::vec3(0.0f, 1000.0f,  5000.0f); // Initial camera position // Anfängliche Kameraposition

    // Initialize objects (Moon and Earth example) // Objekte initialisieren (Mond- und Erde-Beispiel)
    objs = {
        Object(glm::vec3(3844, 0, 0), glm::vec3(0, 0, 228), 7.34767309*pow(10, 22), 3344), // Moon-like object // Mondähnliches Objekt
        // Object(glm::vec3(-250, 0, 0), glm::vec3(0, -50, 0), 7.34767309*pow(10, 22), 3344),
        Object(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), 5.97219*pow(10, 24), 5515), // Earth-like object // Erdähnliches Objekt
    };
    
    // Create grid for space-time visualization // Gitter für Raum-Zeit-Visualisierung erstellen
    std::vector<float> gridVertices = CreateGridVertices(100000.0f, 50, objs);
    CreateVBOVAO(gridVAO, gridVBO, gridVertices.data(), gridVertices.size());
    
    // Debug output // Debug-Ausgabe
    std::cout<<"Earth radius: "<<objs[1].radius<<std::endl;
    std::cout<<"Moon radius: "<<objs[0].radius<<std::endl;

    // Main render loop // Haupt-Render-Schleife
    while (!glfwWindowShouldClose(window) && running == true) {
        // Calculate frame time // Frame-Zeit berechnen
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Clear buffers // Buffer löschen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set up input callbacks (could be moved outside loop) // Eingabe-Callbacks einrichten (könnte außerhalb der Schleife verschoben werden)
        glfwSetKeyCallback(window, keyCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        UpdateCam(shaderProgram, cameraPos);
        
        // Handle object creation mass adjustment // Objekterstellung Massenanpassung behandeln
        if (!objs.empty() && objs.back().Initalizing) {
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
                // Increase mass by 100% per second // Masse um 100% pro Sekunde erhöhen
                objs.back().mass *= 1.0 + 1.0 * deltaTime;
                
                // Update radius based on new mass // Radius basierend auf neuer Masse aktualisieren
                objs.back().radius = pow(
                    (3 * objs.back().mass / objs.back().density) / 
                    (4 * 3.14159265359f), 
                    1.0f/3.0f
                ) / 100000.0f;
                
                // Update vertex data // Vertex-Daten aktualisieren
                objs.back().UpdateVertices();
            }
        }

        // Draw the grid // Gitter zeichnen
        glUseProgram(shaderProgram);
        glUniform4f(objectColorLoc, 1.0f, 1.0f, 1.0f, 0.25f); // White color with 25% transparency // Weiße Farbe mit 25% Transparenz
        gridVertices = CreateGridVertices(10000.0f, 50, objs); // Update grid with gravitational distortion // Gitter mit Gravitationsverzerrung aktualisieren
        glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
        glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_DYNAMIC_DRAW);
        DrawGrid(shaderProgram, gridVAO, gridVertices.size());

        // Draw and update all objects // Alle Objekte zeichnen und aktualisieren
        for(auto& obj : objs) {
            glUniform4f(objectColorLoc, obj.color.r, obj.color.g, obj.color.b, obj.color.a);

            // Calculate gravitational forces between all objects // Gravitationskräfte zwischen allen Objekten berechnen
            for(auto& obj2 : objs){
                if(&obj2 != &obj && !obj.Initalizing && !obj2.Initalizing){
                    // Calculate distance vector // Distanzvektor berechnen
                    float dx = obj2.GetPos()[0] - obj.GetPos()[0];
                    float dy = obj2.GetPos()[1] - obj.GetPos()[1];
                    float dz = obj2.GetPos()[2] - obj.GetPos()[2];
                    float distance = sqrt(dx * dx + dy * dy + dz * dz);

                    if (distance > 0) {
                        // Normalize direction vector // Richtungsvektor normalisieren
                        std::vector<float> direction = {dx / distance, dy / distance, dz / distance};
                        distance *= 1000; // Convert to meters // In Meter umrechnen
                        
                        // Newton's law of universal gravitation: F = G * m1 * m2 / r² // Newtons Gravitationsgesetz: F = G * m1 * m2 / r²
                        double Gforce = (G * obj.mass * obj2.mass) / (distance * distance);
                        
                        // Calculate acceleration: a = F / m // Beschleunigung berechnen: a = F / m
                        float acc1 = Gforce / obj.mass;
                        std::vector<float> acc = {direction[0] * acc1, direction[1]*acc1, direction[2]*acc1};
                        
                        // Apply acceleration if not paused // Beschleunigung anwenden, wenn nicht pausiert
                        if(!pause){
                            obj.accelerate(acc[0], acc[1], acc[2]);
                        }

                        // Check and handle collisions // Kollisionen prüfen und behandeln
                        obj.velocity *= obj.CheckCollision(obj2);
                    }
                }
            }
            
            // Update radius for objects being created // Radius für erstellte Objekte aktualisieren
            if(obj.Initalizing){
                obj.radius = pow(((3 * obj.mass/obj.density)/(4 * 3.14159265359)), (1.0f/3.0f)) / 100000;
                obj.UpdateVertices();
            }

            // Update positions if not paused // Positionen aktualisieren, wenn nicht pausiert
            if(!pause){
                obj.UpdatePos();
            }
            
            // Set up model matrix for rendering // Modellmatrix für Rendering einrichten
            glm::mat4 model = glm::mat4(1.0f); // Identity matrix // Einheitsmatrix
            model = glm::translate(model, obj.position); // Apply position transformation // Positionstransformation anwenden
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            
            // Draw the object // Objekt zeichnen
            glBindVertexArray(obj.VAO);
            glDrawArrays(GL_TRIANGLES, 0, obj.vertexCount / 3);
        }
        
        // Swap buffers and poll events // Buffer tauschen und Ereignisse abfragen
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup resources // Ressourcen aufräumen
    for (auto& obj : objs) {
        glDeleteVertexArrays(1, &obj.VAO);
        glDeleteBuffers(1, &obj.VBO);
    }

    glDeleteVertexArrays(1, &gridVAO);
    glDeleteBuffers(1, &gridVBO);

    glDeleteProgram(shaderProgram);
    glfwTerminate(); // Terminate GLFW // GLFW beenden

    return 0;
}

/// Initialize GLFW and create OpenGL context
/// Sets up window and OpenGL state // Richtet Fenster und OpenGL-Zustand ein
GLFWwindow* StartGLU() {
    // Initialize GLFW // GLFW initialisieren
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW, panic" << std::endl;
        return nullptr;
    }
    
    // Create window // Fenster erstellen
    GLFWwindow* window = glfwCreateWindow(800, 600, "3D_TEST", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window); // Make OpenGL context current // OpenGL-Kontext aktuell machen

    // Initialize GLEW // GLEW initialisieren
    glewExperimental = GL_TRUE; // Enable experimental features // Experimentelle Funktionen aktivieren
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW." << std::endl;
        glfwTerminate();
        return nullptr;
    }

    // Set up OpenGL state // OpenGL-Zustand einrichten
    glEnable(GL_DEPTH_TEST); // Enable depth testing // Tiefentest aktivieren
    glViewport(0, 0, 800, 600); // Set viewport size // Viewport-Größe festlegen
    glEnable(GL_BLEND); // Enable blending for transparency // Blending für Transparenz aktivieren
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Standard blending for transparency // Standard-Blending für Transparenz

    return window;
}

/// Create and compile shader program
/// Links vertex and fragment shaders // Verknüpft Vertex- und Fragment-Shader
GLuint CreateShaderProgram(const char* vertexSource, const char* fragmentSource) {
    // Compile vertex shader // Vertex-Shader kompilieren
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);

    // Check compilation status // Kompilierungsstatus prüfen
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
    }

    // Compile fragment shader // Fragment-Shader kompilieren
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
    }

    // Link shader program // Shader-Programm verknüpfen
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check linking status // Verknüpfungsstatus prüfen
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
    }

    // Clean up individual shaders // Einzelne Shader aufräumen
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

/// Create Vertex Array Object and Vertex Buffer Object
/// Sets up vertex data for rendering // Richtet Vertex-Daten für Rendering ein
void CreateVBOVAO(GLuint& VAO, GLuint& VBO, const float* vertices, size_t vertexCount) {
    glGenVertexArrays(1, &VAO); // Generate VAO // VAO generieren
    glGenBuffers(1, &VBO); // Generate VBO // VBO generieren

    glBindVertexArray(VAO); // Bind VAO // VAO binden
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // Bind VBO // VBO binden
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), vertices, GL_STATIC_DRAW); // Upload vertex data // Vertex-Daten hochladen

    // Set vertex attribute pointer // Vertex-Attribut-Zeiger setzen
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // Position attribute // Positionsattribut
    glEnableVertexAttribArray(0); // Enable attribute // Attribut aktivieren
    glBindVertexArray(0); // Unbind VAO // VAO entbinden
}

/// Update camera view matrix
/// Creates look-at matrix from camera position and orientation // Erstellt Look-at-Matrix aus Kameraposition und -ausrichtung
void UpdateCam(GLuint shaderProgram, glm::vec3 cameraPos) {
    glUseProgram(shaderProgram);
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp); // Create view matrix // Ansichtsmatrix erstellen
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view)); // Upload to shader // An Shader übertragen
}

/// Keyboard input handler
/// Processes camera movement and simulation controls // Verarbeitet Kamerabewegung und Simulationssteuerung
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    float cameraSpeed = 1000.0f * deltaTime; // Speed adjusted by frame time // Geschwindigkeit nach Frame-Zeit angepasst
    bool shiftPressed = (mods & GLFW_MOD_SHIFT) != 0;
    Object& lastObj = objs[objs.size() - 1]; // Reference to newest object // Referenz zum neuesten Objekt
    
    // Camera movement controls // Kamera-Bewegungssteuerung
    if (glfwGetKey(window, GLFW_KEY_W)==GLFW_PRESS){
        cameraPos += cameraSpeed * cameraFront; // Move forward // Vorwärts bewegen
    }
    if (glfwGetKey(window, GLFW_KEY_S)==GLFW_PRESS){
        cameraPos -= cameraSpeed * cameraFront; // Move backward // Rückwärts bewegen
    }

    if (glfwGetKey(window, GLFW_KEY_A)==GLFW_PRESS){
        cameraPos -= cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp)); // Strafe left // Nach links bewegen
    }
    if (glfwGetKey(window, GLFW_KEY_D)==GLFW_PRESS){
        cameraPos += cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp)); // Strafe right // Nach rechts bewegen
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
        cameraPos += cameraSpeed * cameraUp; // Move up // Nach oben bewegen
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
        cameraPos -= cameraSpeed * cameraUp; // Move down // Nach unten bewegen
    }

    // Simulation controls // Simulationssteuerung
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS){
        pause = true; // Pause simulation // Simulation pausieren
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_RELEASE){
        pause = false; // Resume simulation // Simulation fortsetzen
    }
    
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS){
        glfwTerminate();
        glfwWindowShouldClose(window);
        running = false; // Quit application // Anwendung beenden
    }

    // Object positioning during creation // Objektpositionierung während der Erstellung
    if(!objs.empty() && objs[objs.size() - 1].Initalizing){
        if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)){
            if (!shiftPressed) {
                objs[objs.size()-1].position[1] += 0.5; // Move object up // Objekt nach oben bewegen
            }
        };
        if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
            if (!shiftPressed) {
                objs[objs.size()-1].position[1] -= 0.5; // Move object down // Objekt nach unten bewegen
            }
        }
        if(key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)){
            objs[objs.size()-1].position[0] += 0.5; // Move object right // Objekt nach rechts bewegen
        };
        if(key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT)){
            objs[objs.size()-1].position[0] -= 0.5; // Move object left // Objekt nach links bewegen
        };
        if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
            objs[objs.size()-1].position[2] += 0.5; // Move object forward // Objekt nach vorne bewegen
        };
        if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
            objs[objs.size()-1].position[2] -= 0.5; // Move object backward // Objekt nach hinten bewegen
        }
    };
};

/// Mouse movement handler
/// Implements FPS-style camera rotation // Implementiert FPS-Stil Kameradrehung
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    // Calculate mouse movement delta // Mausbewegungsdelta berechnen
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Inverted for natural feel // Invertiert für natürliches Gefühl
    lastX = xpos;
    lastY = ypos;

    // Apply sensitivity // Empfindlichkeit anwenden
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    // Update camera angles // Kamerawinkel aktualisieren
    yaw += xoffset;
    pitch += yoffset;

    // Clamp pitch to prevent flipping // Pitch begrenzen, um Umkippen zu verhindern
    if(pitch > 89.0f) pitch = 89.0f;
    if(pitch < -89.0f) pitch = -89.0f;

    // Calculate new camera front vector // Neuen Kamera-Vorwärtsvektor berechnen
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

/// Mouse button handler
/// Handles object creation and mass adjustment // Behandelt Objekterstellung und Massenanpassung
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods){
    if (button == GLFW_MOUSE_BUTTON_LEFT){
        if (action == GLFW_PRESS){
            // Create new object at origin // Neues Objekt am Ursprung erstellen
            objs.emplace_back(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0f, 0.0f, 0.0f), initMass);
            objs[objs.size()-1].Initalizing = true; // Mark as being created // Als in Erstellung markieren
        };
        if (action == GLFW_RELEASE){
            objs[objs.size()-1].Initalizing = false; // Finalize object // Objekt finalisieren
            objs[objs.size()-1].Launched = true; // Mark as launched // Als gestartet markieren
        };
    };
    // Mass adjustment during creation (commented out) // Massenanpassung während der Erstellung (auskommentiert)
    // if (!objs.empty() && button == GLFW_MOUSE_BUTTON_RIGHT && objs[objs.size()-1].Initalizing) {
    //     if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    //         objs[objs.size()-1].mass *= 1.2;}
    //         std::cout<<"MASS: "<<objs[objs.size()-1].mass<<std::endl;
    // }
};

/// Mouse scroll handler
/// Zooms camera in/out // Zoomt Kamera ein/aus
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
    float cameraSpeed = 50000.0f * deltaTime;
    if(yoffset>0){
        cameraPos += cameraSpeed * cameraFront; // Zoom in // Einzoomen
    } else if(yoffset<0){
        cameraPos -= cameraSpeed * cameraFront; // Zoom out // Auszoomen
    }
}

/// Convert spherical to Cartesian coordinates
/// Used for sphere mesh generation // Für Kugel-Mesh-Generierung verwendet
/// @param r Radius
/// @param theta Polar angle (0 to π) // Polarwinkel (0 bis π)
/// @param phi Azimuthal angle (0 to 2π) // Azimutwinkel (0 bis 2π)
glm::vec3 sphericalToCartesian(float r, float theta, float phi){
    float x = r * sin(theta) * cos(phi);
    float y = r * cos(theta);
    float z = r * sin(theta) * sin(phi);
    return glm::vec3(x, y, z);
};

/// Draw the grid
/// Renders the space-time grid visualization // Rendert die Raum-Zeit-Gitter-Visualisierung
void DrawGrid(GLuint shaderProgram, GLuint gridVAO, size_t vertexCount) {
    glUseProgram(shaderProgram);
    glm::mat4 model = glm::mat4(1.0f); // Identity matrix for the grid // Einheitsmatrix für das Gitter
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(gridVAO);
    glPointSize(5.0f); // Set point size for grid points // Punktgröße für Gitterpunkte setzen
    glDrawArrays(GL_LINES, 0, vertexCount / 3); // Draw as lines // Als Linien zeichnen
    glBindVertexArray(0);
}

/// Create grid vertices with gravitational distortion
/// Generates a 2D grid that bends based on object masses // Generiert ein 2D-Gitter, das sich basierend auf Objektmassen biegt
/// @param size Grid size in world units // Gittergröße in Welteinheiten
/// @param divisions Number of grid divisions // Anzahl der Gitterunterteilungen
/// @param objs Objects that distort the grid // Objekte, die das Gitter verzerren
std::vector<float> CreateGridVertices(float size, int divisions, const std::vector<Object>& objs) {
    std::vector<float> vertices;
    float step = size / divisions; // Distance between grid lines // Abstand zwischen Gitterlinien
    float halfSize = size / 2.0f; // Half grid size for centering // Halbe Gittergröße zum Zentrieren

    // Generate horizontal grid lines (X-axis) // Horizontale Gitterlinien generieren (X-Achse)
    for (int yStep = 3; yStep <= 3; ++yStep) { // Only one Y level // Nur eine Y-Ebene
        float y = -halfSize*0.3f + yStep * step;
        for (int zStep = 0; zStep <= divisions; ++zStep) {
            float z = -halfSize + zStep * step;
            for (int xStep = 0; xStep < divisions; ++xStep) {
                float xStart = -halfSize + xStep * step;
                float xEnd = xStart + step;
                // Line start point // Linien-Startpunkt
                vertices.push_back(xStart); vertices.push_back(y); vertices.push_back(z);
                // Line end point // Linien-Endpunkt
                vertices.push_back(xEnd);   vertices.push_back(y); vertices.push_back(z);
            }
        }
    }

    // Generate vertical grid lines (Z-axis) // Vertikale Gitterlinien generieren (Z-Achse)
    for (int xStep = 0; xStep <= divisions; ++xStep) {
        float x = -halfSize + xStep * step;
        for (int yStep = 3; yStep <= 3; ++yStep) {
            float y = -halfSize*0.3f + yStep * step;
            for (int zStep = 0; zStep < divisions; ++zStep) {
                float zStart = -halfSize + zStep * step;
                float zEnd = zStart + step;
                // Line start point // Linien-Startpunkt
                vertices.push_back(x); vertices.push_back(y); vertices.push_back(zStart);
                // Line end point // Linien-Endpunkt
                vertices.push_back(x); vertices.push_back(y); vertices.push_back(zEnd);
            }
        }
    }
    
    // Apply gravitational distortion to grid // Gravitationsverzerrung auf Gitter anwenden
    // This simulates space-time curvature visualization // Dies simuliert die Visualisierung der Raum-Zeit-Krümmung
    float minz = 0.0f;
    for (int i = 0; i < vertices.size(); i += 3) {
        glm::vec3 vertexPos(vertices[i], vertices[i+1], vertices[i+2]);
        glm::vec3 totalDisplacement(0.0f);
        
        // Calculate displacement for each object // Verschiebung für jedes Objekt berechnen
        for (const auto& obj : objs) {
            glm::vec3 toObject = obj.GetPos() - vertexPos;
            float distance = glm::length(toObject);

            float distance_m = distance * 1000.0f; // Convert to meters // In Meter umrechnen
            float rs = (2*G*obj.mass)/(c*c); // Schwarzschild radius // Schwarzschild-Radius

            // Calculate gravitational "dip" in spacetime // Gravitationelle "Delle" in der Raumzeit berechnen
            float z = 2 * sqrt(rs*(distance_m - rs)) * 100.0f;
            totalDisplacement += z;
        }
        
        vertexPos += totalDisplacement;
        // Apply vertical displacement to simulate space-time curvature // Vertikale Verschiebung anwenden, um Raum-Zeit-Krümmung zu simulieren
        vertices[i+1] = vertexPos[1] / 15.0f - 3000.0f;
    }
    
    return vertices;
}
