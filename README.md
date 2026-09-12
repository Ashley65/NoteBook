# NoteBook

NoteBook is a high-performance productivity application designed for seamless workspace, project, task, and note management. Built with C++20 and Qt 6 for desktop platforms, and modern web browser support.

---

## Features

- **Workspace Management**: Create and switch between isolated workspaces.
- **Project & Task Tracking**: Granular task tracking with priority levels (Low, Medium, High), due dates, and Kanban status columns (To Do, In Progress, Completed).
- **Markdown Note Taking**: Integrated note-taking system with live Markdown rendering, pinning, and searching.
- **Robust Persistence**: SQLite integration with automatic migrations, schema initialization, and cascade operations.
- **Cross-Platform**:
  - **Windows**: Native Mica style integration and Windows deployment automation.
  - **macOS**: Native application bundle (`NoteBook.app`).
  - **Linux**: Standard Qt 6 desktop integration.

---

## Desktop Prerequisites & Build (C++ / Qt)

### Prerequisites
- **CMake** (v3.16 or higher)
- **Qt 6** (modules: `Core`, `Gui`, `Widgets`, `Svg`, `Quick`, `QuickWidgets`, `QuickControls2`, `Network`, `Sql`, `Test`)
- **Compiler**: MSVC 2022 (Windows), GCC 11+ / Clang (Linux/macOS)

### Build Desktop App
```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build NoteBook
cmake --build build --config Release --target NoteBook
```

### Run Automated Tests
```bash
# Run the test suite via CTest
ctest --test-dir build -C Release --output-on-failure
```

The test suite covers:
- `DatabaseManagerTest`: Core SQLite schema, CRUD, cascade deletes, and transactions.
- `WorkspaceRepositoryTest`: Workspace, project, task, and note repository operations.
- `TaskManagerTest`: Task creation, status updates, completion states, and subtasks.
- `NoteManagerTest`: Note authoring, pinning, archiving, and retrieval.

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
