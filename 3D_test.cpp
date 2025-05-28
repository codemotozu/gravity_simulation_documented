/// 3D_test.cpp
/// 
/// A simple OpenGL application that renders an orange triangle using modern OpenGL (3.3 Core Profile). // Eine einfache OpenGL-Anwendung, die ein orangefarbenes Dreieck mit modernem OpenGL (3.3 Core Profile) rendert.
/// Demonstrates basic concepts: window creation, shader compilation, VAO/VBO setup, and rendering loop. // Demonstriert grundlegende Konzepte: Fenstererstellung, Shader-Kompilierung, VAO/VBO-Setup und Render-Schleife.
/// 
/// Usage:
/// ```bash
/// g++ 3D_test.cpp -o 3D_test -lGL -lGLEW -lglfw -lglm
/// ./3D_test
/// ```
/// 
/// EN: Creates a window with an orange triangle rendered using vertex and fragment shaders.
/// DE: Erstellt ein Fenster mit einem orangefarbenen Dreieck, das mit Vertex- und Fragment-Shadern gerendert wird.

#include <GL/glew.h> // Includes GLEW for loading OpenGL extensions. // Inkludiert GLEW zum Laden von OpenGL-Erweiterungen.
#include <GLFW/glfw3.h> // Includes GLFW for window creation and input handling. // Inkludiert GLFW für Fenstererstellung und Eingabeverarbeitung.
#include <glm/glm.hpp> // Includes GLM for mathematical operations with vectors and matrices. // Inkludiert GLM für mathematische Operationen mit Vektoren und Matrizen.
#include <glm/gtc/matrix_transform.hpp> // Includes GLM transformation functions. // Inkludiert GLM-Transformationsfunktionen.
#include <glm/gtc/type_ptr.hpp> // Includes GLM type pointer functions for OpenGL compatibility. // Inkludiert GLM-Typ-Zeiger-Funktionen für OpenGL-Kompatibilität.
#include <iostream> // Includes standard I/O stream for console output. // Inkludiert Standard-I/O-Stream für Konsolenausgabe.

// Function declarations // Funktionsdeklarationen
GLFWwindow* StartGLU(); // Initializes GLFW, GLEW and creates a window. // Initialisiert GLFW, GLEW und erstellt ein Fenster.
void CreateVBOVAO(GLuint& VAO, GLuint& VBO, const float* vertices, size_t vertexCount); // Creates and binds VAO and VBO. // Erstellt und bindet VAO und VBO.
GLuint CreateShaderProgram(const char* vertexSource, const char* fragmentSource); // Compiles and links shader program. // Kompiliert und verknüpft Shader-Programm.

/// Vertex Shader Source Code
/// EN: Transforms vertex positions from model space to clip space. Simply passes through the position.
/// DE: Transformiert Vertex-Positionen vom Modellraum in den Clip-Raum. Gibt die Position einfach weiter.
const char* vertexShaderSource = R"glsl(
    #version 330 core
    layout (location = 0) in vec3 aPos; // Input vertex position attribute at location 0. // Eingabe-Vertex-Positionsattribut an Position 0.
    void main() {
        gl_Position = vec4(aPos, 1.0); // Convert 3D position to 4D homogeneous coordinates. // Konvertiert 3D-Position in 4D homogene Koordinaten.
    }
)glsl";

/// Fragment Shader Source Code
/// EN: Determines the color of each pixel. Sets all pixels to orange.
/// DE: Bestimmt die Farbe jedes Pixels. Setzt alle Pixel auf Orange.
const char* fragmentShaderSource = R"glsl(
    #version 330 core
    out vec4 FragColor; // Output fragment color. // Ausgabe-Fragment-Farbe.
    void main() {
        FragColor = vec4(1.0, 0.5, 0.2, 1.0); // Orange color (R=1.0, G=0.5, B=0.2, A=1.0). // Orangene Farbe (R=1.0, G=0.5, B=0.2, A=1.0).
    }
)glsl";

/// Main Function
/// EN: Entry point of the application. Sets up OpenGL context and renders a triangle.
/// DE: Einstiegspunkt der Anwendung. Richtet OpenGL-Kontext ein und rendert ein Dreieck.
int main() {
    GLFWwindow* window = StartGLU(); // Initialize graphics libraries and create window. // Initialisiere Grafikbibliotheken und erstelle Fenster.
    GLuint shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource); // Create and compile shader program. // Erstelle und kompiliere Shader-Programm.

    // Vertex data for triangle // Vertex-Daten für Dreieck
    float vertices[] = {
        -0.5f, -0.5f, 0.0f, // Bottom left vertex (x, y, z). // Unterer linker Vertex (x, y, z).
         0.5f, -0.5f, 0.0f, // Bottom right vertex (x, y, z). // Unterer rechter Vertex (x, y, z).
         0.0f,  0.5f, 0.0f  // Top center vertex (x, y, z). // Oberer mittlerer Vertex (x, y, z).
    };

    // Create VAO and VBO // Erstelle VAO und VBO
    GLuint VAO, VBO; // Declare Vertex Array Object and Vertex Buffer Object IDs. // Deklariere Vertex Array Object und Vertex Buffer Object IDs.
    CreateVBOVAO(VAO, VBO, vertices, sizeof(vertices) / sizeof(float)); // Setup vertex data on GPU. // Richte Vertex-Daten auf GPU ein.

    // Main render loop // Haupt-Render-Schleife
    while (!glfwWindowShouldClose(window)) { // Continue until user closes window. // Fortfahren bis Benutzer Fenster schließt.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers. // Lösche Farb- und Tiefenpuffer.

        glUseProgram(shaderProgram); // Activate shader program for rendering. // Aktiviere Shader-Programm für Rendering.

        glBindVertexArray(VAO); // Bind VAO containing vertex data. // Binde VAO mit Vertex-Daten.

        glDrawArrays(GL_TRIANGLES, 0, 3); // Draw triangle using 3 vertices starting at index 0. // Zeichne Dreieck mit 3 Vertices beginnend bei Index 0.

        glfwSwapBuffers(window); // Swap front and back buffers (double buffering). // Tausche Vorder- und Hinterpuffer (Double Buffering).
        glfwPollEvents(); // Process window events (keyboard, mouse, etc.). // Verarbeite Fenster-Ereignisse (Tastatur, Maus, etc.).
    }

    // Cleanup resources // Ressourcen aufräumen
    glDeleteVertexArrays(1, &VAO); // Delete Vertex Array Object. // Lösche Vertex Array Object.
    glDeleteBuffers(1, &VBO); // Delete Vertex Buffer Object. // Lösche Vertex Buffer Object.
    glDeleteProgram(shaderProgram); // Delete shader program. // Lösche Shader-Programm.

    glfwTerminate(); // Terminate GLFW and free resources. // Beende GLFW und gebe Ressourcen frei.
    return 0; // Exit successfully. // Erfolgreich beenden.
}

/// StartGLU Function
/// EN: Initializes GLFW and GLEW libraries, creates a window with OpenGL context.
/// DE: Initialisiert GLFW- und GLEW-Bibliotheken, erstellt ein Fenster mit OpenGL-Kontext.
/// 
/// @return GLFWwindow* Pointer to created window, or nullptr on failure. // Zeiger auf erstelltes Fenster, oder nullptr bei Fehler.
GLFWwindow* StartGLU() {
    if (!glfwInit()) { // Initialize GLFW library. // Initialisiere GLFW-Bibliothek.
        std::cout << "Failed to initialize GLFW, panic" << std::endl; // Print error message. // Gebe Fehlermeldung aus.
        return nullptr; // Return null pointer to indicate failure. // Gebe Null-Zeiger zurück um Fehler anzuzeigen.
    }
    GLFWwindow* window = glfwCreateWindow(800, 600, "3D_TEST", NULL, NULL); // Create 800x600 window titled "3D_TEST". // Erstelle 800x600 Fenster mit Titel "3D_TEST".
    if (!window) { // Check if window creation failed. // Prüfe ob Fenstererstellung fehlgeschlagen ist.
        std::cerr << "Failed to create GLFW window." << std::endl; // Print error to stderr. // Gebe Fehler auf stderr aus.
        glfwTerminate(); // Clean up GLFW. // Räume GLFW auf.
        return nullptr; // Return null pointer. // Gebe Null-Zeiger zurück.
    }
    glfwMakeContextCurrent(window); // Make the window's OpenGL context current. // Mache OpenGL-Kontext des Fensters aktuell.

    glewExperimental = GL_TRUE; // Enable experimental GLEW features for core profile. // Aktiviere experimentelle GLEW-Features für Core Profile.
    if (glewInit() != GLEW_OK) { // Initialize GLEW to load OpenGL function pointers. // Initialisiere GLEW um OpenGL-Funktionszeiger zu laden.
        std::cerr << "Failed to initialize GLEW." << std::endl; // Print error message. // Gebe Fehlermeldung aus.
        glfwTerminate(); // Clean up GLFW. // Räume GLFW auf.
        return nullptr; // Return null pointer. // Gebe Null-Zeiger zurück.
    }

    glEnable(GL_DEPTH_TEST); // Enable depth testing for 3D rendering. // Aktiviere Tiefentest für 3D-Rendering.

    return window; // Return pointer to created window. // Gebe Zeiger auf erstelltes Fenster zurück.
}

/// CreateShaderProgram Function
/// EN: Compiles vertex and fragment shaders, links them into a shader program.
/// DE: Kompiliert Vertex- und Fragment-Shader, verknüpft sie zu einem Shader-Programm.
/// 
/// @param vertexSource GLSL source code for vertex shader. // GLSL-Quellcode für Vertex-Shader.
/// @param fragmentSource GLSL source code for fragment shader. // GLSL-Quellcode für Fragment-Shader.
/// @return GLuint ID of the created shader program. // ID des erstellten Shader-Programms.
GLuint CreateShaderProgram(const char* vertexSource, const char* fragmentSource) {
    // Vertex shader compilation // Vertex-Shader-Kompilierung
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); // Create vertex shader object. // Erstelle Vertex-Shader-Objekt.
    glShaderSource(vertexShader, 1, &vertexSource, nullptr); // Set shader source code. // Setze Shader-Quellcode.
    glCompileShader(vertexShader); // Compile the vertex shader. // Kompiliere den Vertex-Shader.

    GLint success; // Variable to store compilation status. // Variable zum Speichern des Kompilierungsstatus.
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success); // Check compilation status. // Prüfe Kompilierungsstatus.
    if (!success) { // If compilation failed. // Falls Kompilierung fehlgeschlagen.
        char infoLog[512]; // Buffer for error message. // Puffer für Fehlermeldung.
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog); // Get error message. // Hole Fehlermeldung.
        std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl; // Print error. // Gebe Fehler aus.
    }

    // Fragment shader compilation // Fragment-Shader-Kompilierung
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Create fragment shader object. // Erstelle Fragment-Shader-Objekt.
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr); // Set shader source code. // Setze Shader-Quellcode.
    glCompileShader(fragmentShader); // Compile the fragment shader. // Kompiliere den Fragment-Shader.

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success); // Check compilation status. // Prüfe Kompilierungsstatus.
    if (!success) { // If compilation failed. // Falls Kompilierung fehlgeschlagen.
        char infoLog[512]; // Buffer for error message. // Puffer für Fehlermeldung.
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog); // Get error message. // Hole Fehlermeldung.
        std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl; // Print error. // Gebe Fehler aus.
    }

    // Shader program linking // Shader-Programm-Verknüpfung
    GLuint shaderProgram = glCreateProgram(); // Create shader program object. // Erstelle Shader-Programm-Objekt.
    glAttachShader(shaderProgram, vertexShader); // Attach vertex shader to program. // Hänge Vertex-Shader an Programm.
    glAttachShader(shaderProgram, fragmentShader); // Attach fragment shader to program. // Hänge Fragment-Shader an Programm.
    glLinkProgram(shaderProgram); // Link shaders into executable program. // Verknüpfe Shader zu ausführbarem Programm.

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success); // Check linking status. // Prüfe Verknüpfungsstatus.
    if (!success) { // If linking failed. // Falls Verknüpfung fehlgeschlagen.
        char infoLog[512]; // Buffer for error message. // Puffer für Fehlermeldung.
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog); // Get error message. // Hole Fehlermeldung.
        std::cerr << "Shader program linking failed: " << infoLog << std::endl; // Print error. // Gebe Fehler aus.
    }

    glDeleteShader(vertexShader); // Delete vertex shader (no longer needed after linking). // Lösche Vertex-Shader (nach Verknüpfung nicht mehr benötigt).
    glDeleteShader(fragmentShader); // Delete fragment shader (no longer needed after linking). // Lösche Fragment-Shader (nach Verknüpfung nicht mehr benötigt).

    return shaderProgram; // Return ID of created shader program. // Gebe ID des erstellten Shader-Programms zurück.
}

/// CreateVBOVAO Function
/// EN: Creates and configures Vertex Array Object (VAO) and Vertex Buffer Object (VBO) for storing vertex data on GPU.
/// DE: Erstellt und konfiguriert Vertex Array Object (VAO) und Vertex Buffer Object (VBO) zum Speichern von Vertex-Daten auf der GPU.
/// 
/// @param VAO Reference to store created VAO ID. // Referenz zum Speichern der erstellten VAO-ID.
/// @param VBO Reference to store created VBO ID. // Referenz zum Speichern der erstellten VBO-ID.
/// @param vertices Pointer to vertex data array. // Zeiger auf Vertex-Daten-Array.
/// @param vertexCount Number of float values in vertices array. // Anzahl der Float-Werte im Vertices-Array.
void CreateVBOVAO(GLuint& VAO, GLuint& VBO, const float* vertices, size_t vertexCount) {
    // VAO creation and binding // VAO-Erstellung und -Bindung
    glGenVertexArrays(1, &VAO); // Generate one VAO and store its ID. // Generiere ein VAO und speichere seine ID.
    glBindVertexArray(VAO); // Bind VAO to make it current. // Binde VAO um es aktuell zu machen.

    // VBO creation and data upload // VBO-Erstellung und Daten-Upload
    glGenBuffers(1, &VBO); // Generate one VBO and store its ID. // Generiere ein VBO und speichere seine ID.
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // Bind VBO as array buffer. // Binde VBO als Array-Puffer.
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), vertices, GL_STATIC_DRAW); // Upload vertex data to GPU (static = data won't change). // Lade Vertex-Daten auf GPU (static = Daten ändern sich nicht).

    // Vertex attribute configuration // Vertex-Attribut-Konfiguration
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // Configure attribute 0: 3 floats per vertex, no normalization, tightly packed. // Konfiguriere Attribut 0: 3 Floats pro Vertex, keine Normalisierung, dicht gepackt.
    glEnableVertexAttribArray(0); // Enable vertex attribute at location 0. // Aktiviere Vertex-Attribut an Position 0.

    glBindVertexArray(0); // Unbind VAO to prevent accidental modification. // Entbinde VAO um versehentliche Änderungen zu verhindern.
}
