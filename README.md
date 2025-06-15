> 🧠 **Note on Algorithms Used:**  
> This project utilizes a variation of **Dijkstra’s algorithm** (inspired by greedy selection and minimum spanning tree techniques) to compute optimal file ordering for compression. Although the algorithm implementation resembles Dijkstra, it is functionally closer to **Prim’s algorithm** for MST generation — helping improve archive efficiency.

# 📦 Archive Manager (macOS Desktop App)

A macOS desktop application implementing **Dijkstra's** algorithm for efficient file path resolution when creating and extracting ZIP archives — built with **C++** and **wxWidgets**.

---

## 🧩 Features

- 🗜️ **Create Archive**  
  Select multiple files or folders and compress them into a single `.zip` file at your desired location using optimized path resolution.

- 📂 **Extract Archive**  
  Unzip files instantly with smart path determination — no nested folders, just direct access to the actual contents.

- 💡 **Batch Extraction**  
  Extract multiple ZIP files at once with a single click using parallel processing.

> Designed with algorithmic efficiency in mind, **Archive Manager** combines Dijkstra's pathfinding with compression for frustration-free archiving.

---

## 🖥️ User Interface Preview

<img width="500" alt="Main Interface" src="https://github.com/user-attachments/assets/b34a3801-3e22-4104-ba3d-c8762caf1713">
<img width="500" alt="Extraction Screen" src="https://github.com/user-attachments/assets/9540b382-58c6-4d7b-8448-3288411a8e21">

---

## ⬇️ Download

> ⚠️ This app requires local compilation and is **only compatible with macOS** due to licensing limitations. Therefore, Option 1 will only show the UI without operation

Download the prebuilt app to preview the user interface:  
🔗 [ArchiveManager.zip](https://github.com/user-attachments/files/20624900/ArchiveManager.zip)

---

## 🚀 Quick Start (macOS only)

## Option 1: Use Prebuilt App 
1. Download [ArchiveManager.zip](https://github.com/user-attachments/files/20624900/ArchiveManager.zip)
2. Unzip the file:
   ```bash
   unzip ArchiveManager.zip -d ~/Applications/
3. Lauch the application
   ```bash
   open ~/Applications/ArchiveManager.app
## Option 2: Build from Source (Need a compiler and wxWidgets)
1. Clone the repository:
   ```bash
   git clone https://github.com/Lumb3/ArchiveManager.git
   cd ArchiveManager
2. Build the project (requires wxWidgets):
   ```bash
   make  # or follow build instructions in the repository
3. Run the application:
   ```bash
   ./ArchiveManager

## 📽️ Watch the application in action:
  https://www.youtube.com/watch?v=k7_x3RX6FfE
