# 🚦 Traffic Light — OpenGL Project

A real-time interactive traffic light simulation built with **OpenGL 3.3 Core**, **GLFW**, and **GLEW** in C++.  
Lights cycle automatically (Red → Yellow → Green), a car moves and stops at the traffic light, and the scene responds to keyboard input.

---

## ⌨️ Controls

| Key | Action |
|-----|--------|
| `W` | Increase active light brightness |
| `S` | Decrease active light brightness |
| `A` | Move traffic light left |
| `D` | Move traffic light right |
| `ESC` | Close the window |

> Lights switch automatically: **Red (5s) → Yellow (2s) → Green (20s) → repeat**

---

## 🚗 Car Behaviour

- The car moves from left to right across the road continuously
- On **red or yellow**: the car stops at the stop line (just left of the traffic light)
- On **green**: the car moves freely and resets to the left edge when it exits the screen
- The stop line adjusts automatically if the traffic light is moved with **A / D**

---

## 🚀 How to Run

### 1. Clone the repository
```bash
git clone https://github.com/Hamza-dev3/TrafficLight.git
```
Or download the ZIP from GitHub and extract it.

### 2. Open the solution
1. Open **Visual Studio 2022** (or later)
2. Click **Open a project or solution**
3. Open **`TrafficLight.slnx`** from the cloned folder

### 3. Build and Run

## ⚠️ Requirements

- **Visual Studio 2022 or later** is required to open and build this project.  
  The solution and project files are generated for VS 2022 and may not be compatible with older versions.  
  If you are using an earlier version, create a new project manually and add `main.cpp` to it.

1. Set platform to **x64**
2. Press **Ctrl + F5**
3. The window opens immediately ✅

> All dependencies (GLFW, GLEW) are already configured and linked via `$(SolutionDir)` — no manual setup required.

---

## 📁 Project Structure

```
TrafficLight/
├── main.cpp             # All source code (single file)
├── TrafficLight.slnx    # Visual Studio solution file
├── TrafficLight.vcxproj # Visual Studio project file
└── README.md
```

---

## 🧠 Concepts Used

| Concept | How it's used |
|---------|---------------|
| **VAO / VBO** | One VAO + VBO stores all shape vertices |
| **Shaders** | Vertex shader for position, Fragment shader for color + alpha |
| **Uniforms** | `uXOffset`, `uYOffset`, `uColor`, `uAlpha` control each shape per draw call |
| **Blending** | `GL_SRC_ALPHA / GL_ONE_MINUS_SRC_ALPHA` for dim/bright lights and transparency |
| **glfwGetTime()** | Drives the automatic light switching timer |
| **GL_LINES** | Used for road center dashes instead of triangles |
| **Shape reuse** | Light square drawn 3× with different uniforms for Red, Yellow, Green |

---

## 🎨 Scene Elements

- 🌑 Dark blue-grey background (night/dusk atmosphere)
- 🛣️ Dark asphalt road with yellow dashed center line
- 🚦 Traffic light pole, housing box, and 3 colored lights
- 🚗 Blue car with roof that stops at red and moves on green

---

## 👤 Author

**Hamza Abdulrahman**  
Computer Graphics Course — 2026
