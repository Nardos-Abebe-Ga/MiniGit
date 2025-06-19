# MiniGit

MiniGit is a simplified version control system built in **C++**, designed to mimic basic features of Git. It allows users to track changes, switch between versions, and manage branches using a command-line interface.



##  Features Implemented

*  Repository Initialization (`init`)
*  Staging Files (`add`)
*  Committing (`commit`)
*  Viewing Log History (`log`)
*  Branch Creation and Switching (`branch`, `checkout`)
*  Merge with Conflict Detection (`merge`)
*  Diff Viewer (line-by-line file comparison)
*  File Restoration from Old Commits



##  Project Structure


MiniGitProject/
├── include/              # Header files
│   └── minigit.hpp
├── src/                  # Source files
│   └── minigit.cpp
├── .minigit/             # Hidden internal repo (auto-generated)
├── main.cpp              # Entry point with CLI interface
├── README.md             # Project overview (this file)
├── MiniGit_Report.txt    # Short report on design/DSA usage
├── .gitignore            # Files to exclude from Git




##  How to Compile and Run

###  On Windows (with g++)


g++ -std=c++17 main.cpp src/minigit.cpp -Iinclude -o minigit.exe
./minigit.exe


### On Linux/macOS:


g++ -std=c++17 main.cpp src/minigit.cpp -Iinclude -o minigit
./minigit




##  Usage Instructions


MiniGit > Choose a command:
1. add <filename>
2. commit
3. log
4. branch <branch-name>
5. checkout <branch-name or commit-hash>
6. merge <branch-name>
7. restore <commit-hash> <filename>
8. diff <filename> <commitA> <commitB>
0. exit






## Concepts & Data Structures

| Component        | DSA Concepts          |
| ---------------- | --------------------- |
| Blob             | Hashing, File I/O     |
| Commit Node      | Linked List / DAG     |
| Branch Reference | Hash Map              |
| Staging Area     | Set / Hash Table      |
| Log Traversal    | Linked List Traversal |



##  Video Demo

A short video demonstrating MiniGit is available at: [Demo Link Here](#)





> Thank you for checking out MiniGit!
