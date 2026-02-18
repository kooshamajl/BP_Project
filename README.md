# 📌 Terminal Task Manager (C + ncurses)

A terminal-based Task Manager written in **C** using the **ncurses** library.  
Developed as a Programming Fundamentals course project.

---

## 🧠 Features

- Add / Edit / Delete Tasks  
- Add / Delete Subtasks  
- Add / Delete Categories  
- Task Description  
- Deadline with countdown  
- Mark tasks as complete  
- Sort (Alphabetical / Deadline)  
- Save & Load from file  

---

## 🖥️ Interface

Multi-window terminal UI built with **ncurses**:

- Tasks
- Subtasks
- Description
- Categories
- Deadline
- Help

Fully keyboard-driven navigation.

---

## 🎮 Controls

| Key | Action |
|------|--------|
| `a` | Add |
| `d` | Delete |
| `e` | Edit |
| `space` | Complete |
| `n` | Set deadline |
| `r` | Edit description |
| `j / k` | Navigate |
| `l / h` | Subtask mode |
| `c` | Category mode |
| `b` | Sort (A-Z) |
| `s` | Sort (Deadline) |
| `w` | Save |
| `q` | Quit |

---

## 💾 Data

Tasks are stored in a binary file:


Loaded automatically at startup.

---

## 🛠️ Tech Stack

- C  
- ncurses  
- stdio / stdlib / string / time  

---

## ▶️ Compile & Run

```bash
gcc main.c -o taskmanager -lncurses
./taskmanager

```

👨‍💻 Author
Koosha Majlessi