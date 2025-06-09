# 📦 Archive Manager (macOS Desktop App)

A sleek and lightweight macOS desktop application for effortlessly creating and extracting ZIP archives — built with **C++** and **wxWidgets**.

---

## 🧩 Features

- 🗜️ **Create Archive**  
  Select multiple files or folders and compress them into a single `.zip` file at your desired location.

- 📂 **Extract Archive**  
  Unzip files instantly — no nested folders, just direct access to the actual contents.

- 💡 **Batch Extraction**  
  Extract multiple ZIP files at once with a single click.

> Designed with simplicity and speed in mind, **Archive Manager** keeps archiving clean and frustration-free.

---

## 🖥️ User Interface Preview

<img width="500" alt="Main Interface" src="https://github.com/user-attachments/assets/b34a3801-3e22-4104-ba3d-c8762caf1713">
<img width="500" alt="Extraction Screen" src="https://github.com/user-attachments/assets/9540b382-58c6-4d7b-8448-3288411a8e21">

---

## ⬇️ Download

> ⚠️ This app requires local compilation and is **only compatible with macOS** due to licensing limitations.

Download the prebuilt app to preview the user interface:  
🔗 [ArchiveManager.zip](https://github.com/user-attachments/files/20624900/ArchiveManager.zip)

---

## � Quick Start (macOS only)

### Option 1: Use Prebuilt App
1. Download [ArchiveManager.zip](https://github.com/user-attachments/files/20624900/ArchiveManager.zip)
2. Unzip the file:
   ```bash
   unzip ArchiveManager.zip -d ~/Applications/
   When prompted about __MACOSX/._ArchiveManager.app, type A then press Enter

Launch the application:

bash
open ~/Applications/ArchiveManager.app
Option 2: Build from Source
bash
# 1. Clone the repository
git clone https://github.com/Lumb3/ArchiveManager.git
cd ArchiveManager

# 2. Build the project (requires wxWidgets)
make  # or follow build instructions in the repository

# 3. Run the application
./ArchiveManager
