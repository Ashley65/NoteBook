# NoteBook / Taskhelper

NoteBook is a high-performance productivity application designed for seamless workspace, project, task, and note management. Built with C++20 and Qt 6 for desktop platforms, and HTML5/JS/CSS for modern Web browsers.

---

## 🌟 Features

- **Workspace Management**: Create and switch between isolated workspaces.
- **Project & Task Tracking**: Granular task tracking with priority levels (Low, Medium, High), due dates, and Kanban status columns (To Do, In Progress, Completed).
- **Markdown Note Taking**: Integrated note-taking system with live Markdown rendering, pinning, and searching.
- **LocalStorage & Data Backup**: Client-side offline data persistence with instant JSON Export and Import capabilities.
- **Cloud Readiness**: Storage layer abstracted via `StorageAdapter` for seamless connection to a C# REST API backend.
- **Cross-Platform & Web**:
  - **Desktop**: C++20 + Qt 6 (Windows Mica dark mode, macOS native bundle).
  - **Web Application**: Responsive glassmorphism web UI runnable locally or hostable on Cloudflare Pages.

---

## 🌐 Web Version (`web/`)

The web application resides in the `web/` directory and can be used in any modern browser.

### 1. Run Locally
You can run the web version locally using Node / NPX or any static web server:

```bash
cd web
npm run dev
```
Or with Python:
```bash
cd web
python -m http.server 3000
```
Then open [http://localhost:3000](http://localhost:3000) in your browser.

### 2. Host on Cloudflare Pages
Deploying to Cloudflare Pages is zero-config using Wrangler:

```bash
cd web
npx wrangler pages deploy . --project-name=notebook-app
```
Alternatively, link your GitHub repository to Cloudflare Pages dashboard with `web` as the build output directory.

---

## 💻 Desktop Prerequisites & Build (C++ / Qt)

### Prerequisites
- **CMake** (v3.16 or higher)
- **Qt 6.11.0** (modules: `qtbase`, `qtsvg`, `qtdeclarative`, `qtnetwork`)
- **Compiler**: MSVC 2022 (Windows), GCC/Clang (Linux/macOS)

### Build Desktop App
```bash
# Configure
cmake -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.11.0/msvc2022_64"

# Build
cmake --build build --config Release
```
