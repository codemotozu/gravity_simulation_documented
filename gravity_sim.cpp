/// Gravity Simulation
/// 
/// A 3D gravitational physics simulation using OpenGL that visualizes celestial bodies interacting through gravity.
/// Allows users to create objects with customizable mass and observe their gravitational interactions in real-time.
/// 
/// Usage:
/// ```cpp
/// // Compile with: g++ gravity_sim.cpp -lglfw3 -lopengl32 -lgdi32 -lglew32
/// // Run: ./gravity_sim.exe
/// ```
/// 
/// EN: Creates an interactive 3D environment where objects follow Newton's law of universal gravitation with visual effects.
/// DE: Erstellt eine interaktive 3D-Umgebung, in der Objekte dem Newtonschen Gravitationsgesetz mit visuellen Effekten folgen.

#include <GL/glew.h> // OpenGL Extension Wrangler Library for managing OpenGL extensions. // OpenGL Extension Wrangler Library zur Verwaltung von OpenGL-Erweiterungen.
#include <GLFW/glfw3.h> // GLFW library for window creation and input handling. // GLFW-Bibliothek für Fenstererstellung und Eingabeverarbeitung.
#include <glm/glm.hpp> // OpenGL Mathematics library for vector and matrix operations. // OpenGL Mathematics-Bibliothek für Vektor- und Matrixoperationen.
#include <glm/gtc/matrix_transform.hpp> // GLM extension for transformation matrices. // GLM-Erweiterung für Transformationsmatrizen.
#include <glm/gtc/type_ptr.hpp> // GLM extension for pointer access to matrices. // GLM-Erweiterung für Zeigerzugriff auf Matrizen.
#include <vector> // Standard library for dynamic arrays. // Standardbibliothek für dynamische Arrays.
#include <iostream> // Standard library for input/output operations. // Standardbibliothek für Ein-/Ausgabeoperationen.

/// Vertex shader source code in GLSL
/// EN: Transforms vertices and calculates lighting intensity based on position
/// DE: Transformiert Vertices und berechnet Lichtintensität basierend auf Position
const char* vertexShaderSource = R"glsl(
#version 330 core
layout(location=0) in vec3 aPos; // Input vertex position. // Eingangs-Vertex-Position.
uniform mat4 model; // Model transformation matrix. // Modell-Transformationsmatrix.
uniform mat4 view; // View transformation matrix. // Ansichts-Transformationsmatrix.
uniform mat4 projection; // Projection matrix. // Projektionsmatrix.
out float lightIntensity; // Output light intensity to fragment shader. // Ausgabe der Lichtintensität an Fragment-Shader.
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0); // Transform vertex to clip space. // Transformiere Vertex in Clip-Space.
    vec3 worldPos = (model * vec4(aPos, 1.0)).xyz; // Calculate world position. // Berechne Weltposition.
    vec3 normal = normalize(aPos); // Use position as normal for sphere. // Verwende Position als Normale für Kugel.
    vec3 dirToCenter = normalize(-worldPos); // Direction to world center. // Richtung zum Weltzentrum.
    lightIntensity = max(dot(normal, dirToCenter), 0.15); // Calculate diffuse lighting. // Berechne diffuse Beleuchtung.
})glsl";

/// Fragment shader source code in GLSL
/// EN: Determines pixel colors with lighting effects and special rendering modes
/// DE: Bestimmt Pixelfarben mit Lichteffekten und speziellen Rendering-Modi
const char* fragmentShaderSource = R"glsl(
#version 330 core
in float lightIntensity; // Input light intensity from vertex shader. // Eingangs-Lichtintensität vom Vertex-Shader.
out vec4 FragColor; // Output fragment color. // Ausgabe-Fragmentfarbe.
uniform vec4 objectColor; // Base color of the object. // Grundfarbe des Objekts.
uniform bool isGrid; // Flag for grid rendering. // Flag für Grid-Rendering.
uniform bool GLOW; // Flag for glowing objects. // Flag für leuchtende Objekte.
void main() {
    if (isGrid) {
        FragColor = objectColor; // Grid uses flat color. // Grid verwendet flache Farbe.
    } else if(GLOW){
        FragColor = vec4(objectColor.rgb * 100000, objectColor.a); // Extreme brightness for glow. // Extreme Helligkeit für Leuchten.
    }else {
        float fade = smoothstep(0.0, 10.0, lightIntensity*10); // Smooth lighting transition. // Sanfter Beleuchtungsübergang.
        FragColor = vec4(objectColor.rgb * fade, objectColor.a); // Apply lighting to color. // Wende Beleuchtung auf Farbe an.
    }
})glsl";

// Global simulation state variables. // Globale Simulationszustandsvariablen.
bool running = true; // Main loop control flag. // Hauptschleifen-Kontrollflag.
bool pause = true; // Simulation pause state. // Simulationspausenzustand.
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  1.0f); // Camera position in world space. // Kameraposition im Weltraum.
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // Camera forward direction. // Kamera-Vorwärtsrichtung.
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f); // Camera up vector. // Kamera-Aufwärtsvektor.
float lastX = 400.0, lastY = 300.0; // Last mouse position. // Letzte Mausposition.
float yaw = -90; // Camera yaw angle in degrees. // Kamera-Gierwinkel in Grad.
float pitch = 0.0; // Camera pitch angle in degrees. // Kamera-Nickwinkel in Grad.
float deltaTime = 0.0; // Time between frames. // Zeit zwischen Frames.
float lastFrame = 0.0; // Time of last frame. // Zeit des letzten Frames.

// Physical constants. // Physikalische Konstanten.
const double G = 6.6743e-11; // Gravitational constant in m^3 kg^-1 s^-2. // Gravitationskonstante in m^3 kg^-1 s^-2.
const float c = 299792458.0; // Speed of light in m/s. // Lichtgeschwindigkeit in m/s.
float initMass = float(pow(10, 22)); // Initial mass for new objects in kg. // Anfangsmasse für neue Objekte in kg.
float sizeRatio = 30000.0f; // Scale factor for visual representation. // Skalierungsfaktor für visuelle Darstellung.

// Function declarations. // Funktionsdeklarationen.
GLFWwindow* StartGLU(); // Initializes OpenGL context and window. // Initialisiert OpenGL-Kontext und Fenster.
GLuint CreateShaderProgram(const char* vertexSource, const char* fragmentSource); // Creates shader program. // Erstellt Shader-Programm.
void CreateVBOVAO(GLuint& VAO, GLuint& VBO, const float* vertices, size_t vertexCount); // Creates vertex buffers. // Erstellt Vertex-Buffer.
void UpdateCam(GLuint shaderProgram, glm::vec3 cameraPos); // Updates camera matrices. // Aktualisiert Kameramatrizen.
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods); // Handles keyboard input. // Verarbeitet Tastatureingaben.
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods); // Handles mouse buttons. // Verarbeitet Maustasten.
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset); // Handles mouse scroll. // Verarbeitet Mausrad.
void mouse_callback(GLFWwindow* window, double xpos, double ypos); // Handles mouse movement. // Verarbeitet Mausbewegung.
glm::vec3 sphericalToCartesian(float r, float theta, float phi); // Converts spherical to Cartesian coordinates. // Konvertiert sphärische zu kartesischen Koordinaten.
void DrawGrid(GLuint shaderProgram, GLuint gridVAO, size_t vertexCount); // Renders the grid. // Rendert das Gitter.

/// Object Class
/// 
/// Represents a celestial body in the simulation with physical properties and rendering data.
/// Each object has mass, velocity, position, and visual representation as a sphere.
/// 
/// EN: Manages individual gravitational objects with physics simulation and OpenGL rendering.
/// DE: Verwaltet einzelne Gravitationsobjekte mit Physiksimulation und OpenGL-Rendering.
class Object {
    public:
        GLuint VAO, VBO; // OpenGL vertex array and buffer objects. // OpenGL Vertex-Array- und Buffer-Objekte.
        glm::vec3 position = glm::vec3(400, 300, 0); // Current position in world space. // Aktuelle Position im Weltraum.
        glm::vec3 velocity = glm::vec3(0, 0, 0); // Current velocity vector. // Aktueller Geschwindigkeitsvektor.
        size_t vertexCount; // Number of vertices in the mesh. // Anzahl der Vertices im Mesh.
        glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // RGBA color of the object. // RGBA-Farbe des Objekts.

        bool Initalizing = false; // Object is being created by user. // Objekt wird vom Benutzer erstellt.
        bool Launched = false; // Object has been released. // Objekt wurde freigegeben.
        bool target = false; // Object is targeted (unused). // Objekt ist anvisiert (unbenutzt).

        float mass; // Mass of the object in kg. // Masse des Objekts in kg.
        float density;  // Density in kg/m^3. // Dichte in kg/m^3.
        float radius; // Visual radius of the sphere. // Visueller Radius der Kugel.

        glm::vec3 LastPos = position; // Previous position (unused). // Vorherige Position (unbenutzt).
        bool glow; // Whether object should glow. // Ob Objekt leuchten soll.

        /// Constructor for creating a new gravitational object
        /// EN: Initializes object with physical properties and generates sphere mesh
        /// DE: Initialisiert Objekt mit physikalischen Eigenschaften und generiert Kugel-Mesh
        Object(glm::vec3 initPosition, glm::vec3 initVelocity, float mass, float density = 3344, glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), bool Glow = false) {   
            this->position = initPosition; // Set initial position. // Setze Anfangsposition.
            this->velocity = initVelocity; // Set initial velocity. // Setze Anfangsgeschwindigkeit.
            this->mass = mass; // Set object mass. // Setze Objektmasse.
            this->density = density; // Set material density. // Setze Materialdichte.
            this->radius = pow(((3 * this->mass/this->density)/(4 * 3.14159265359)), (1.0f/3.0f)) / sizeRatio; // Calculate radius from mass and density. // Berechne Radius aus Masse und Dichte.
            this->color = color; // Set object color. // Setze Objektfarbe.
            this->glow = Glow; // Set glow effect. // Setze Leuchteffekt.
            
            std::vector<float> vertices = Draw(); // Generate sphere vertices. // Generiere Kugel-Vertices.
            vertexCount = vertices.size(); // Store vertex count. // Speichere Vertex-Anzahl.

            CreateVBOVAO(VAO, VBO, vertices.data(), vertexCount); // Create OpenGL buffers. // Erstelle OpenGL-Buffer.
        }

        /// Generates sphere mesh vertices
        /// EN: Creates triangulated sphere using spherical coordinates
        /// DE: Erstellt triangulierte Kugel mit sphärischen Koordinaten
        std::vector<float> Draw() {
            std::vector<float> vertices; // Vertex data container. // Vertex-Daten-Container.
            int stacks = 10; // Horizontal subdivisions. // Horizontale Unterteilungen.
            int sectors = 10; // Vertical subdivisions. // Vertikale Unterteilungen.

            // Generate sphere triangles. // Generiere Kugel-Dreiecke.
            for(float i = 0.0f; i <= stacks; ++i){
                float theta1 = (i / stacks) * glm::pi<float>(); // Current latitude angle. // Aktueller Breitengrad-Winkel.
                float theta2 = (i+1) / stacks * glm::pi<float>(); // Next latitude angle. // Nächster Breitengrad-Winkel.
                for (float j = 0.0f; j < sectors; ++j){
                    float phi1 = j / sectors * 2 * glm::pi<float>(); // Current longitude angle. // Aktueller Längengrad-Winkel.
                    float phi2 = (j+1) / sectors * 2 * glm::pi<float>(); // Next longitude angle. // Nächster Längengrad-Winkel.
                    
                    // Convert spherical to Cartesian coordinates. // Konvertiere sphärische zu kartesischen Koordinaten.
                    glm::vec3 v1 = sphericalToCartesian(this->radius, theta1, phi1);
                    glm::vec3 v2 = sphericalToCartesian(this->radius, theta1, phi2);
                    glm::vec3 v3 = sphericalToCartesian(this->radius, theta2, phi1);
                    glm::vec3 v4 = sphericalToCartesian(this->radius, theta2, phi2);

                    // Triangle 1: v1-v2-v3. // Dreieck 1: v1-v2-v3.
                    vertices.insert(vertices.end(), {v1.x, v1.y, v1.z});
                    vertices.insert(vertices.end(), {v2.x, v2.y, v2.z});
                    vertices.insert(vertices.end(), {v3.x, v3.y, v3.z});
                    
                    // Triangle 2: v2-v4-v3. // Dreieck 2: v2-v4-v3.
                    vertices.insert(vertices.end(), {v2.x, v2.y, v2.z});
                    vertices.insert(vertices.end(), {v4.x, v4.y, v4.z});
                    vertices.insert(vertices.end(), {v3.x, v3.y, v3.z});
                }   
            }
            return vertices; // Return generated vertices. // Gebe generierte Vertices zurück.
        }
        
        /// Updates object position based on velocity
        /// EN: Integrates velocity to update position with fixed timestep
        /// DE: Integriert Geschwindigkeit zur Positionsaktualisierung mit festem Zeitschritt
        void UpdatePos(){
            this->position[0] += this->velocity[0] / 94; // Update X position. // Aktualisiere X-Position.
            this->position[1] += this->velocity[1] / 94; // Update Y position. // Aktualisiere Y-Position.
            this->position[2] += this->velocity[2] / 94; // Update Z position. // Aktualisiere Z-Position.
            this->radius = pow(((3 * this->mass/this->density)/(4 * 3.14159265359)), (1.0f/3.0f)) / sizeRatio; // Recalculate radius. // Berechne Radius neu.
        }
        
        /// Updates vertex buffer with new sphere data
        /// EN: Regenerates sphere mesh when radius changes
        /// DE: Regeneriert Kugel-Mesh wenn sich Radius ändert
        void UpdateVertices() {
            std::vector<float> vertices = Draw(); // Generate new vertices. // Generiere neue Vertices.
            
            glBindBuffer(GL_ARRAY_BUFFER, VBO); // Bind vertex buffer. // Binde Vertex-Buffer.
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW); // Upload new data. // Lade neue Daten hoch.
        }
        
        /// Returns current position
        /// EN: Getter for object position
        /// DE: Getter für Objektposition
        glm::vec3 GetPos() const {
            return this->position;
        }
        
        /// Applies acceleration to velocity
        /// EN: Updates velocity based on forces with timestep integration
        /// DE: Aktualisiert Geschwindigkeit basierend auf Kräften mit Zeitschritt-Integration
        void accelerate(float x, float y, float z){
            this->velocity[0] += x / 96; // Apply X acceleration. // Wende X-Beschleunigung an.
            this->velocity[1] += y / 96; // Apply Y acceleration. // Wende Y-Beschleunigung an.
            this->velocity[2] += z / 96; // Apply Z acceleration. // Wende Z-Beschleunigung an.
        }
        
        /// Checks collision with another object
        /// EN: Returns velocity damping factor if objects collide
        /// DE: Gibt Geschwindigkeitsdämpfungsfaktor zurück wenn Objekte kollidieren
        float CheckCollision(const Object& other) {
            float dx = other.position[0] - this->position[0]; // X distance. // X-Abstand.
            float dy = other.position[1] - this->position[1]; // Y distance. // Y-Abstand.
            float dz = other.position[2] - this->position[2]; // Z distance. // Z-Abstand.
            float distance = std::pow(dx*dx + dy*dy + dz*dz, (1.0f/2.0f)); // Calculate Euclidean distance. // Berechne euklidischen Abstand.
            if (other.radius + this->radius > distance){ // Check if spheres overlap. // Prüfe ob Kugeln überlappen.
                return -0.2f; // Return damping factor. // Gebe Dämpfungsfaktor zurück.
            }
            return 1.0f; // No collision. // Keine Kollision.
        }
};

std::vector<Object> objs = {}; // Container for all objects in simulation. // Container für alle Objekte in der Simulation.

// Grid function declarations. // Grid-Funktionsdeklarationen.
std::vector<float> CreateGridVertices(float size, int divisions, const std::vector<Object>& objs); // Creates grid mesh. // Erstellt Grid-Mesh.
std::vector<float> UpdateGridVertices(std::vector<float> vertices, const std::vector<Object>& objs); // Updates grid deformation. // Aktualisiert Grid-Verformung.

GLuint gridVAO, gridVBO; // OpenGL objects for grid rendering. // OpenGL-Objekte für Grid-Rendering.

/// Main function
/// EN: Entry point that sets up OpenGL, creates initial objects, and runs the simulation loop
/// DE: Einstiegspunkt der OpenGL einrichtet, Anfangsobjekte erstellt und Simulationsschleife ausführt
int main() {
    GLFWwindow* window = StartGLU(); // Initialize OpenGL and create window. // Initialisiere OpenGL und erstelle Fenster.
    GLuint shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource); // Compile shaders. // Kompiliere Shader.

    // Get shader uniform locations. // Hole Shader-Uniform-Positionen.
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model"); // Model matrix location. // Model-Matrix-Position.
    GLint objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor"); // Color uniform location. // Farb-Uniform-Position.
    glUseProgram(shaderProgram); // Activate shader program. // Aktiviere Shader-Programm.

    // Set up input callbacks. // Richte Eingabe-Callbacks ein.
    glfwSetCursorPosCallback(window, mouse_callback); // Mouse movement callback. // Mausbewegung-Callback.
    glfwSetScrollCallback(window, scroll_callback); // Mouse wheel callback. // Mausrad-Callback.
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Hide and capture cursor. // Verstecke und fange Cursor.

    // Set up projection matrix. // Richte Projektionsmatrix ein.
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 750000.0f); // Create perspective projection. // Erstelle perspektivische Projektion.
    GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection"); // Get projection uniform location. // Hole Projektions-Uniform-Position.
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection)); // Upload projection matrix. // Lade Projektionsmatrix hoch.
    cameraPos = glm::vec3(0.0f, 1000.0f, 5000.0f); // Set initial camera position. // Setze Anfangs-Kameraposition.

    // Initialize celestial objects. // Initialisiere Himmelskörper.
    objs = {
        Object(glm::vec3(-5000, 650, -350), glm::vec3(0, 0, 1500), 5.97219*pow(10, 22), 5515, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)), // Blue object orbiting. // Blaues Objekt in Umlaufbahn.
        Object(glm::vec3(5000, 650, -350), glm::vec3(0, 0, -1500), 5.97219*pow(10, 22), 5515, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)), // Blue object orbiting opposite. // Blaues Objekt in Gegenumlaufbahn.
        Object(glm::vec3(0, 0, -350), glm::vec3(0, 0, 0), 1.989 * pow(10, 25), 5515, glm::vec4(1.0f, 0.929f, 0.176f, 1.0f), true), // Central glowing star. // Zentraler leuchtender Stern.
    };
    
    // Create grid mesh. // Erstelle Grid-Mesh.
    std::vector<float> gridVertices = CreateGridVertices(20000.0f, 25, objs); // Generate grid vertices. // Generiere Grid-Vertices.
    CreateVBOVAO(gridVAO, gridVBO, gridVertices.data(), gridVertices.size()); // Create grid buffers. // Erstelle Grid-Buffer.

    // Main render loop. // Haupt-Render-Schleife.
    while (!glfwWindowShouldClose(window) && running == true) {
        // Calculate frame timing. // Berechne Frame-Timing.
        float currentFrame = glfwGetTime(); // Get current time. // Hole aktuelle Zeit.
        deltaTime = currentFrame - lastFrame; // Calculate delta time. // Berechne Delta-Zeit.
        lastFrame = currentFrame; // Update last frame time. // Aktualisiere letzte Frame-Zeit.

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear framebuffer. // Lösche Framebuffer.

        // Set up callbacks. // Richte Callbacks ein.
        glfwSetKeyCallback(window, keyCallback); // Keyboard callback. // Tastatur-Callback.
        glfwSetMouseButtonCallback(window, mouseButtonCallback); // Mouse button callback. // Maustasten-Callback.
        UpdateCam(shaderProgram, cameraPos); // Update camera view matrix. // Aktualisiere Kamera-Ansichtsmatrix.
        
        // Handle object creation with right mouse. // Verarbeite Objekterstellung mit rechter Maus.
        if (!objs.empty() && objs.back().Initalizing) {
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
                objs.back().mass *= 1.0 + 1.0 * deltaTime; // Increase mass over time. // Erhöhe Masse über Zeit.
                
                // Update radius based on new mass. // Aktualisiere Radius basierend auf neuer Masse.
                objs.back().radius = pow(
                    (3 * objs.back().mass / objs.back().density) / 
                    (4 * 3.14159265359f), 
                    1.0f/3.0f
                ) / sizeRatio;
                
                objs.back().UpdateVertices(); // Regenerate sphere mesh. // Regeneriere Kugel-Mesh.
            }
        }

        // Draw the grid. // Zeichne das Gitter.
        glUseProgram(shaderProgram); // Activate shader. // Aktiviere Shader.
        glUniform4f(objectColorLoc, 1.0f, 1.0f, 1.0f, 0.25f); // Set grid color with transparency. // Setze Grid-Farbe mit Transparenz.
        glUniform1i(glGetUniformLocation(shaderProgram, "isGrid"), 1); // Enable grid rendering mode. // Aktiviere Grid-Rendering-Modus.
        glUniform1i(glGetUniformLocation(shaderProgram, "GLOW"), 0); // Disable glow for grid. // Deaktiviere Leuchten für Grid.
        gridVertices = UpdateGridVertices(gridVertices, objs); // Update grid deformation. // Aktualisiere Grid-Verformung.
        glBindBuffer(GL_ARRAY_BUFFER, gridVBO); // Bind grid buffer. // Binde Grid-Buffer.
        glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_DYNAMIC_DRAW); // Upload grid data. // Lade Grid-Daten hoch.
        DrawGrid(shaderProgram, gridVAO, gridVertices.size()); // Render grid. // Rendere Grid.

        // Draw all objects. // Zeichne alle Objekte.
        for(auto& obj : objs) {
            glUniform4f(objectColorLoc, obj.color.r, obj.color.g, obj.color.b, obj.color.a); // Set object color. // Setze Objektfarbe.

            // Calculate gravitational forces between objects. // Berechne Gravitationskräfte zwischen Objekten.
            for(auto& obj2 : objs){
                if(&obj2 != &obj && !obj.Initalizing && !obj2.Initalizing){ // Skip self and initializing objects. // Überspringe Selbst und initialisierende Objekte.
                    float dx = obj2.GetPos()[0] - obj.GetPos()[0]; // X distance. // X-Abstand.
                    float dy = obj2.GetPos()[1] - obj.GetPos()[1]; // Y distance. // Y-Abstand.
                    float dz = obj2.GetPos()[2] - obj.GetPos()[2]; // Z distance. // Z-Abstand.
                    float distance = sqrt(dx * dx + dy * dy + dz * dz); // Calculate distance. // Berechne Abstand.

                    if (distance > 0) { // Avoid division by zero. // Vermeide Division durch Null.
                        std::vector<float> direction = {dx / distance, dy / distance, dz / distance}; // Normalized direction. // Normalisierte Richtung.
                        distance *= 1000; // Convert to meters. // Konvertiere zu Metern.
                        double Gforce = (G * obj.mass * obj2.mass) / (distance * distance); // Newton's law of gravitation. // Newtonsches Gravitationsgesetz.
                        
                        float acc1 = Gforce / obj.mass; // Calculate acceleration. // Berechne Beschleunigung.
                        std::vector<float> acc = {direction[0] * acc1, direction[1]*acc1, direction[2]*acc1}; // Acceleration vector. // Beschleunigungsvektor.
                        if(!pause){
                            obj.accelerate(acc[0], acc[1], acc[2]); // Apply acceleration if not paused. // Wende Beschleunigung an wenn nicht pausiert.
                        }

                        obj.velocity *= obj.CheckCollision(obj2); // Apply collision damping. // Wende Kollisionsdämpfung an.
                        std::cout<<"radius: "<<obj.radius<<std::endl; // Debug output. // Debug-Ausgabe.
                    }
                }
            }
            
            // Update object during initialization. // Aktualisiere Objekt während Initialisierung.
            if(obj.Initalizing){
                obj.radius = pow(((3 * obj.mass/obj.density)/(4 * 3.14159265359)), (1.0f/3.0f)) / 1000000; // Smaller radius during creation. // Kleinerer Radius während Erstellung.
                obj.UpdateVertices(); // Update mesh. // Aktualisiere Mesh.
            }

            // Update positions if not paused. // Aktualisiere Positionen wenn nicht pausiert.
            if(!pause){
                obj.UpdatePos();
            }
            
            // Set up rendering state for object. // Richte Rendering-Zustand für Objekt ein.
            glm::mat4 model = glm::mat4(1.0f); // Identity matrix. // Einheitsmatrix.
            model = glm::translate(model, obj.position); // Apply object position. // Wende Objektposition an.
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); // Upload model matrix. // Lade Model-Matrix hoch.
            glUniform1i(glGetUniformLocation(shaderProgram, "isGrid"), 0); // Disable grid mode. // Deaktiviere Grid-Modus.
            
            // Set glow effect. // Setze Leuchteffekt.
            if(obj.glow){
                glUniform1i(glGetUniformLocation(shaderProgram, "GLOW"), 1); // Enable glow. // Aktiviere Leuchten.
            } else {
                glUniform1i(glGetUniformLocation(shaderProgram, "GLOW"), 0); // Disable glow. // Deaktiviere Leuchten.
            }
            
            glBindVertexArray(obj.VAO); // Bind object vertex array. // Binde Objekt-Vertex-Array.
            glDrawArrays(GL_TRIANGLES, 0, obj.vertexCount / 3); // Draw triangles. // Zeichne Dreiecke.
        }
        
        glfwSwapBuffers(window); // Swap front and back buffers. // Tausche Vorder- und Hintergrundpuffer.
        glfwPollEvents(); // Process window events. // Verarbeite Fenster-Events.
    }

    // Clean up OpenGL resources. // Räume OpenGL-Ressourcen auf.
    for (auto& obj : objs) {
        glDeleteVertexArrays(1, &obj.VAO); // Delete vertex array. // Lösche Vertex-Array.
        glDeleteBuffers(1, &obj.VBO); // Delete vertex buffer. // Lösche Vertex-Buffer.
    }

    glDeleteVertexArrays(1, &gridVAO); // Delete grid vertex array. // Lösche Grid-Vertex-Array.
    glDeleteBuffers(1, &gridVBO); // Delete grid buffer. // Lösche Grid-Buffer.

    glDeleteProgram(shaderProgram); // Delete shader program. // Lösche Shader-Programm.
    glfwTerminate(); // Terminate GLFW. // Beende GLFW.

    return 0; // Exit successfully. // Beende erfolgreich.
}

/// Initializes GLFW and GLEW, creates window
/// EN: Sets up OpenGL context with depth testing and alpha blending
/// DE: Richtet OpenGL-Kontext mit Tiefentest und Alpha-Blending ein
GLFWwindow* StartGLU() {
    if (!glfwInit()) { // Initialize GLFW library. // Initialisiere GLFW-Bibliothek.
        std::cout << "Failed to initialize GLFW, panic" << std::endl; // Error message. // Fehlermeldung.
        return nullptr;
    }
    GLFWwindow* window = glfwCreateWindow(800, 600, "3D_TEST", NULL, NULL); // Create 800x600 window. // Erstelle 800x600 Fenster.
    if (!window) { // Check window creation. // Prüfe Fenstererstellung.
        std::cerr << "Failed to create GLFW window." << std::endl; // Error message. // Fehlermeldung.
        glfwTerminate(); // Clean up GLFW. // Räume GLFW auf.
        return nullptr;
    }
    glfwMakeContextCurrent(window); // Make OpenGL context current. // Mache OpenGL-Kontext aktuell.

    glewExperimental = GL_TRUE; // Enable experimental features. // Aktiviere experimentelle Features.
    if (glewInit() != GLEW_OK) { // Initialize GLEW. // Initialisiere GLEW.
        std::cerr << "Failed to initialize GLEW." << std::endl; // Error message. // Fehlermeldung.
        glfwTerminate(); // Clean up. // Aufräumen.
        return nullptr;
    }

    glEnable(GL_DEPTH_TEST); // Enable depth testing. // Aktiviere Tiefentest.
    glViewport(0, 0, 800, 600); // Set viewport size. // Setze Viewport-Größe.
    glEnable(GL_BLEND); // Enable alpha blending. // Aktiviere Alpha-Blending.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Set blend function. // Setze Blend-Funktion.

    return window; // Return created window. // Gebe erstelltes Fenster zurück.
}

/// Creates and links OpenGL shader program
/// EN: Compiles vertex and fragment shaders, links them into a program
/// DE: Kompiliert Vertex- und Fragment-Shader, verknüpft sie zu einem Programm
GLuint CreateShaderProgram(const char* vertexSource, const char* fragmentSource) {
    // Compile vertex shader. // Kompiliere Vertex-Shader.
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); // Create vertex shader object. // Erstelle Vertex-Shader-Objekt.
    glShaderSource(vertexShader, 1, &vertexSource, nullptr); // Set shader source. // Setze Shader-Quellcode.
    glCompileShader(vertexShader); // Compile shader. // Kompiliere Shader.

    GLint success; // Compilation status. // Kompilierungsstatus.
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success); // Check compilation. // Prüfe Kompilierung.
    if (!success) {
        char infoLog[512]; // Error message buffer. // Fehlermeldungs-Puffer.
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog); // Get error message. // Hole Fehlermeldung.
        std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl; // Print error. // Gebe Fehler aus.
    }

    // Compile fragment shader. // Kompiliere Fragment-Shader.
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Create fragment shader object. // Erstelle Fragment-Shader-Objekt.
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr); // Set shader source. // Setze Shader-Quellcode.
    glCompileShader(fragmentShader); // Compile shader. // Kompiliere Shader.

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success); // Check compilation. // Prüfe Kompilierung.
    if (!success) {
        char infoLog[512]; // Error message buffer. // Fehlermeldungs-Puffer.
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog); // Get error message. // Hole Fehlermeldung.
        std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl; // Print error. // Gebe Fehler aus.
    }

    // Link shader program. // Verknüpfe Shader-Programm.
    GLuint shaderProgram = glCreateProgram(); // Create program object. // Erstelle Programm-Objekt.
    glAttachShader(shaderProgram, vertexShader); // Attach vertex shader. // Hänge Vertex-Shader an.
    glAttachShader(shaderProgram, fragmentShader); // Attach fragment shader. // Hänge Fragment-Shader an.
    glLinkProgram(shaderProgram); // Link program. // Verknüpfe Programm.

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success); // Check linking. // Prüfe Verknüpfung.
    if (!success) {
        char infoLog[512]; // Error message buffer. // Fehlermeldungs-Puffer.
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog); // Get error message. // Hole Fehlermeldung.
        std::cerr << "Shader program linking failed: " << infoLog << std::endl; // Print error. // Gebe Fehler aus.
    }

    glDeleteShader(vertexShader); // Clean up vertex shader. // Räume Vertex-Shader auf.
    glDeleteShader(fragmentShader); // Clean up fragment shader. // Räume Fragment-Shader auf.

    return shaderProgram; // Return linked program. // Gebe verknüpftes Programm zurück.
}

/// Creates Vertex Array Object and Vertex Buffer Object
/// EN: Sets up OpenGL buffers for vertex data with position attributes
/// DE: Richtet OpenGL-Puffer für Vertex-Daten mit Positionsattributen ein
void CreateVBOVAO(GLuint& VAO, GLuint& VBO, const float* vertices, size_t vertexCount) {
    glGenVertexArrays(1, &VAO); // Generate vertex array object. // Generiere Vertex-Array-Objekt.
    glGenBuffers(1, &VBO); // Generate vertex buffer object. // Generiere Vertex-Buffer-Objekt.

    glBindVertexArray(VAO); // Bind VAO. // Binde VAO.
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // Bind VBO. // Binde VBO.
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), vertices, GL_STATIC_DRAW); // Upload vertex data. // Lade Vertex-Daten hoch.

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // Set vertex attribute. // Setze Vertex-Attribut.
    glEnableVertexAttribArray(0); // Enable attribute. // Aktiviere Attribut.
    glBindVertexArray(0); // Unbind VAO. // Löse VAO-Bindung.
}

/// Updates camera view matrix
/// EN: Calculates and uploads view matrix based on camera position and orientation
/// DE: Berechnet und lädt Ansichtsmatrix basierend auf Kameraposition und -ausrichtung
void UpdateCam(GLuint shaderProgram, glm::vec3 cameraPos) {
    glUseProgram(shaderProgram); // Activate shader program. // Aktiviere Shader-Programm.
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp); // Calculate view matrix. // Berechne Ansichtsmatrix.
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view"); // Get view uniform location. // Hole View-Uniform-Position.
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view)); // Upload view matrix. // Lade Ansichtsmatrix hoch.
}

/// Keyboard input handler
/// EN: Processes keyboard events for camera movement and object manipulation
/// DE: Verarbeitet Tastatur-Events für Kamerabewegung und Objektmanipulation
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    float cameraSpeed = 10000.0f * deltaTime; // Camera movement speed. // Kamerabewegungsgeschwindigkeit.
    bool shiftPressed = (mods & GLFW_MOD_SHIFT) != 0; // Check shift key. // Prüfe Shift-Taste.
    Object& lastObj = objs[objs.size() - 1]; // Reference to last object. // Referenz zum letzten Objekt.
    
    // WASD camera movement. // WASD-Kamerabewegung.
    if (glfwGetKey(window, GLFW_KEY_W)==GLFW_PRESS){
        cameraPos += cameraSpeed * cameraFront; // Move forward. // Bewege vorwärts.
    }
    if (glfwGetKey(window, GLFW_KEY_S)==GLFW_PRESS){
        cameraPos -= cameraSpeed * cameraFront; // Move backward. // Bewege rückwärts.
    }
    if (glfwGetKey(window, GLFW_KEY_A)==GLFW_PRESS){
        cameraPos -= cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp)); // Move left. // Bewege links.
    }
    if (glfwGetKey(window, GLFW_KEY_D)==GLFW_PRESS){
        cameraPos += cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp)); // Move right. // Bewege rechts.
    }

    // Vertical camera movement. // Vertikale Kamerabewegung.
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
        cameraPos += cameraSpeed * cameraUp; // Move up. // Bewege nach oben.
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
        cameraPos -= cameraSpeed * cameraUp; // Move down. // Bewege nach unten.
    }

    // Pause control. // Pausensteuerung.
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS){
        pause = true; // Pause simulation. // Pausiere Simulation.
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_RELEASE){
        pause = false; // Resume simulation. // Setze Simulation fort.
    }
    
    // Quit application. // Beende Anwendung.
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS){
        glfwTerminate(); // Terminate GLFW. // Beende GLFW.
        glfwWindowShouldClose(window); // Signal window close. // Signalisiere Fensterschließung.
        running = false; // Stop main loop. // Stoppe Hauptschleife.
    }

    // Object positioning during initialization. // Objektpositionierung während Initialisierung.
    if(!objs.empty() && objs[objs.size() - 1].Initalizing){
        if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)){
            if (!shiftPressed) {
                objs[objs.size()-1].position[1] += objs[objs.size() - 1].radius * 0.2; // Move up. // Bewege nach oben.
            }
        };
        if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
            if (!shiftPressed) {
                objs[objs.size()-1].position[1] -= objs[objs.size() - 1].radius * 0.2; // Move down. // Bewege nach unten.
            }
        }
        if(key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)){
            objs[objs.size()-1].position[0] += objs[objs.size() - 1].radius * 0.2; // Move right. // Bewege nach rechts.
        };
        if(key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT)){
            objs[objs.size()-1].position[0] -= objs[objs.size() - 1].radius * 0.2; // Move left. // Bewege nach links.
        };
        if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
            objs[objs.size()-1].position[2] += objs[objs.size() - 1].radius * 0.2; // Move forward. // Bewege vorwärts.
        };
        if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
            objs[objs.size()-1].position[2] -= objs[objs.size() - 1].radius * 0.2; // Move backward. // Bewege rückwärts.
        }
    };
};

/// Mouse movement callback
/// EN: Updates camera orientation based on mouse movement
/// DE: Aktualisiert Kameraausrichtung basierend auf Mausbewegung
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    float xoffset = xpos - lastX; // Calculate X offset. // Berechne X-Versatz.
    float yoffset = lastY - ypos; // Calculate Y offset (inverted). // Berechne Y-Versatz (invertiert).
    lastX = xpos; // Update last X. // Aktualisiere letztes X.
    lastY = ypos; // Update last Y. // Aktualisiere letztes Y.

    float sensitivity = 0.1f; // Mouse sensitivity. // Mausempfindlichkeit.
    xoffset *= sensitivity; // Apply sensitivity. // Wende Empfindlichkeit an.
    yoffset *= sensitivity; // Apply sensitivity. // Wende Empfindlichkeit an.

    yaw += xoffset; // Update yaw angle. // Aktualisiere Gierwinkel.
    pitch += yoffset; // Update pitch angle. // Aktualisiere Nickwinkel.

    // Clamp pitch to prevent flipping. // Begrenze Nickwinkel um Überschlagen zu verhindern.
    if(pitch > 89.0f) pitch = 89.0f;
    if(pitch < -89.0f) pitch = -89.0f;

    // Calculate camera front vector. // Berechne Kamera-Vorwärtsvektor.
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch)); // X component. // X-Komponente.
    front.y = sin(glm::radians(pitch)); // Y component. // Y-Komponente.
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch)); // Z component. // Z-Komponente.
    cameraFront = glm::normalize(front); // Normalize direction. // Normalisiere Richtung.
}

/// Mouse button callback
/// EN: Handles object creation and launch with mouse buttons
/// DE: Verarbeitet Objekterstellung und -start mit Maustasten
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods){
    if (button == GLFW_MOUSE_BUTTON_LEFT){ // Left mouse button. // Linke Maustaste.
        if (action == GLFW_PRESS){
            objs.emplace_back(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0f, 0.0f, 0.0f), initMass); // Create new object. // Erstelle neues Objekt.
            objs[objs.size()-1].Initalizing = true; // Mark as initializing. // Markiere als initialisierend.
        };
        if (action == GLFW_RELEASE){
            objs[objs.size()-1].Initalizing = false; // End initialization. // Beende Initialisierung.
            objs[objs.size()-1].Launched = true; // Mark as launched. // Markiere als gestartet.
        };
    };
    if (!objs.empty() && button == GLFW_MOUSE_BUTTON_RIGHT && objs[objs.size()-1].Initalizing) { // Right mouse during init. // Rechte Maus während Init.
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            objs[objs.size()-1].mass *= 1.2; // Increase mass. // Erhöhe Masse.
            std::cout<<"MASS: "<<objs[objs.size()-1].mass<<std::endl; // Debug output. // Debug-Ausgabe.
        }
    }
};

/// Mouse scroll callback
/// EN: Zooms camera in/out with mouse wheel
/// DE: Zoomt Kamera ein/aus mit Mausrad
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
    float cameraSpeed = 250000.0f * deltaTime; // Zoom speed. // Zoom-Geschwindigkeit.
    if(yoffset>0){
        cameraPos += cameraSpeed * cameraFront; // Zoom in. // Hineinzoomen.
    } else if(yoffset<0){
        cameraPos -= cameraSpeed * cameraFront; // Zoom out. // Herauszoomen.
    }
}

/// Converts spherical to Cartesian coordinates
/// EN: Mathematical conversion for sphere vertex generation
/// DE: Mathematische Konvertierung für Kugel-Vertex-Generierung
glm::vec3 sphericalToCartesian(float r, float theta, float phi){
    float x = r * sin(theta) * cos(phi); // Calculate X coordinate. // Berechne X-Koordinate.
    float y = r * cos(theta); // Calculate Y coordinate. // Berechne Y-Koordinate.
    float z = r * sin(theta) * sin(phi); // Calculate Z coordinate. // Berechne Z-Koordinate.
    return glm::vec3(x, y, z); // Return Cartesian vector. // Gebe kartesischen Vektor zurück.
};

/// Renders the grid
/// EN: Draws grid lines with identity transformation
/// DE: Zeichnet Gitterlinien mit Einheitstransformation
void DrawGrid(GLuint shaderProgram, GLuint gridVAO, size_t vertexCount) {
    glUseProgram(shaderProgram); // Activate shader. // Aktiviere Shader.
    glm::mat4 model = glm::mat4(1.0f); // Identity matrix for grid. // Einheitsmatrix für Gitter.
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model"); // Get model uniform. // Hole Model-Uniform.
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); // Upload model matrix. // Lade Model-Matrix hoch.

    glBindVertexArray(gridVAO); // Bind grid VAO. // Binde Grid-VAO.
    glPointSize(5.0f); // Set point size. // Setze Punktgröße.
    glDrawArrays(GL_LINES, 0, vertexCount / 3); // Draw grid lines. // Zeichne Gitterlinien.
    glBindVertexArray(0); // Unbind VAO. // Löse VAO-Bindung.
}

/// Creates grid vertex data
/// EN: Generates a flat grid of lines in the XZ plane
/// DE: Generiert ein flaches Gitter aus Linien in der XZ-Ebene
std::vector<float> CreateGridVertices(float size, int divisions, const std::vector<Object>& objs) {
    std::vector<float> vertices; // Vertex container. // Vertex-Container.
    float step = size / divisions; // Grid cell size. // Gitterzellengröße.
    float halfSize = size / 2.0f; // Half grid size. // Halbe Gittergröße.

    // Generate X-axis lines. // Generiere X-Achsen-Linien.
    for (int yStep = 3; yStep <= 3; ++yStep) { // Fixed Y position. // Feste Y-Position.
        float y = -halfSize*0.3f + yStep * step; // Calculate Y coordinate. // Berechne Y-Koordinate.
        for (int zStep = 0; zStep <= divisions; ++zStep) {
            float z = -halfSize + zStep * step; // Calculate Z coordinate. // Berechne Z-Koordinate.
            for (int xStep = 0; xStep < divisions; ++xStep) {
                float xStart = -halfSize + xStep * step; // Line start X. // Linienstart X.
                float xEnd = xStart + step; // Line end X. // Linienende X.
                vertices.push_back(xStart); vertices.push_back(y); vertices.push_back(z); // Start vertex. // Startvertex.
                vertices.push_back(xEnd);   vertices.push_back(y); vertices.push_back(z); // End vertex. // Endvertex.
            }
        }
    }
    
    // Generate Z-axis lines. // Generiere Z-Achsen-Linien.
    for (int xStep = 0; xStep <= divisions; ++xStep) {
        float x = -halfSize + xStep * step; // Calculate X coordinate. // Berechne X-Koordinate.
        for (int yStep = 3; yStep <= 3; ++yStep) { // Fixed Y position. // Feste Y-Position.
            float y = -halfSize*0.3f + yStep * step; // Calculate Y coordinate. // Berechne Y-Koordinate.
            for (int zStep = 0; zStep < divisions; ++zStep) {
                float zStart = -halfSize + zStep * step; // Line start Z. // Linienstart Z.
                float zEnd = zStart + step; // Line end Z. // Linienende Z.
                vertices.push_back(x); vertices.push_back(y); vertices.push_back(zStart); // Start vertex. // Startvertex.
                vertices.push_back(x); vertices.push_back(y); vertices.push_back(zEnd); // End vertex. // Endvertex.
            }
        }
    }

    return vertices; // Return grid vertices. // Gebe Grid-Vertices zurück.
}

/// Updates grid vertices to show gravitational warping
/// EN: Deforms grid based on gravitational field of objects (spacetime curvature visualization)
/// DE: Verformt Gitter basierend auf Gravitationsfeld der Objekte (Raumzeit-Krümmungsvisualisierung)
std::vector<float> UpdateGridVertices(std::vector<float> vertices, const std::vector<Object>& objs){
    
    // Calculate center of mass. // Berechne Massenschwerpunkt.
    float totalMass = 0.0f; // Total system mass. // Gesamtsystemmasse.
    float comY = 0.0f; // Center of mass Y coordinate. // Massenschwerpunkt Y-Koordinate.
    for (const auto& obj : objs) {
        if (obj.Initalizing) continue; // Skip initializing objects. // Überspringe initialisierende Objekte.
        comY += obj.mass * obj.position.y; // Weighted Y position. // Gewichtete Y-Position.
        totalMass += obj.mass; // Sum masses. // Summiere Massen.
    }
    if (totalMass > 0) comY /= totalMass; // Calculate average. // Berechne Durchschnitt.
    
    // Find original grid height. // Finde ursprüngliche Grid-Höhe.
    float originalMaxY = -std::numeric_limits<float>::infinity(); // Initialize to minimum. // Initialisiere auf Minimum.
    for (int i = 0; i < vertices.size(); i += 3) {
        originalMaxY = std::max(originalMaxY, vertices[i+1]); // Find maximum Y. // Finde maximales Y.
    }

    float verticalShift = comY - originalMaxY; // Calculate shift. // Berechne Verschiebung.
    std::cout<<"vertical shift: "<<verticalShift<<" |         comY: "<<comY<<"|            originalmaxy: "<<originalMaxY<<std::endl; // Debug output. // Debug-Ausgabe.

    // Apply gravitational warping. // Wende Gravitationsverzerrung an.
    for (int i = 0; i < vertices.size(); i += 3) {
        glm::vec3 vertexPos(vertices[i], vertices[i+1], vertices[i+2]); // Current vertex position. // Aktuelle Vertex-Position.
        glm::vec3 totalDisplacement(0.0f); // Total warping. // Gesamtverzerrung.
        
        for (const auto& obj : objs) {
            glm::vec3 toObject = obj.GetPos() - vertexPos; // Vector to object. // Vektor zum Objekt.
            float distance = glm::length(toObject); // Distance to object. // Abstand zum Objekt.
            float distance_m = distance * 1000.0f; // Convert to meters. // Konvertiere zu Metern.
            float rs = (2*G*obj.mass)/(c*c); // Schwarzschild radius. // Schwarzschild-Radius.

            float dz = 2 * sqrt(rs * (distance_m - rs)); // Gravitational depression. // Gravitationssenkung.
            totalDisplacement.y += dz * 2.0f; // Apply warping. // Wende Verzerrung an.
        }
        vertices[i+1] = totalDisplacement.y + -abs(verticalShift); // Update Y coordinate. // Aktualisiere Y-Koordinate.
    }

    return vertices; // Return warped vertices. // Gebe verzerrte Vertices zurück.
}
