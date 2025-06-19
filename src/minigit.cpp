#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <string>
#include <chrono>
#include <ctime>
#include "../include/minigit.hpp"
#include <unordered_set>
#include <vector>
#include <unordered_map>

namespace fs = std::filesystem;

// ---------------- Utility Functions ----------------

std::string generateHash(const std::string& content) {
    std::hash<std::string> hasher;
    return std::to_string(hasher(content));
}

std::string getCurrentTime() {
    auto now = std::time(nullptr);
    std::string timeStr = std::ctime(&now);
    if (!timeStr.empty() && timeStr.back() == '\n') {
        timeStr.pop_back();
    }
    return timeStr;
}

std::string generateCommitHash(const std::string& message, const std::string& time) {
    std::string combined = message + time;
    return std::to_string(std::hash<std::string>{}(combined));
}

// ---------------- Initialization ----------------

void initMiniGit() {
    std::string root = ".minigit";
    std::string objectsDir = root + "/objects";
    std::string refsDir = root + "/refs";

    if (fs::exists(root)) {
        std::cout << "MiniGit repository already initialized.\n";
        return;
    }

    fs::create_directory(root);
    fs::create_directory(objectsDir);
    fs::create_directory(refsDir);

    std::ofstream head(".minigit/HEAD");
    head << "ref: refs/master";
    head.close();

    std::ofstream master(".minigit/refs/master");
    master << "";
    master.close();
}

// ---------------- Add File ----------------

void addFile(const std::string& filename) {
    std::ifstream inFile(filename);
    if (!inFile) {
        std::cerr << "Error: File not found: " << filename << "\n";
        return;
    }

    std::string content((std::istreambuf_iterator<char>(inFile)),
                         std::istreambuf_iterator<char>());
    inFile.close();

    std::string hash = generateHash(content);

    std::string objectPath = ".minigit/objects/" + hash;
    std::ofstream outFile(objectPath);
    if (!outFile) {
        std::cerr << "Error: Could not write to " << objectPath << "\n";
        return;
    }
    outFile << content;
    outFile.close();

    std::ofstream index(".minigit/index", std::ios::app);
    if (index) {
        index << filename << " " << hash << "\n";
        index.close();
        std::cout << "File '" << filename << "' staged successfully.\n";
    } else {
        std::cerr << "Error: Could not open .minigit/index\n";
    }
}

// ---------------- Commit ----------------

void commit(const std::string& message) {
    std::ifstream index(".minigit/index");
    if (!index) {
        std::cerr << "No files staged. Nothing to commit.\n";
        return;
    }

    std::string indexContents((std::istreambuf_iterator<char>(index)),
                              std::istreambuf_iterator<char>());
    index.close();

    if (indexContents.empty()) {
        std::cout << "Nothing to commit. Staging area is empty.\n";
        return;
    }

    std::string time = getCurrentTime();
    std::string commitHash = generateCommitHash(message, time);

    std::ifstream headFile(".minigit/HEAD");
    std::string refPath;
    std::getline(headFile, refPath);
    headFile.close();

    if (refPath.rfind("ref: ", 0) != 0) {
        std::cerr << "HEAD is not pointing to a branch.\n";
        return;
    }
    refPath = refPath.substr(5); // "refs/master"

    std::string parentHash;
    std::ifstream branchRef(".minigit/" + refPath);
    if (branchRef) std::getline(branchRef, parentHash);
    branchRef.close();

    fs::create_directory(".minigit/commits");
    std::ofstream commitFile(".minigit/commits/" + commitHash);
    if (!commitFile) {
        std::cerr << "Failed to create commit file.\n";
        return;
    }

    commitFile << "Commit: " << commitHash << "\n";
    if (!parentHash.empty())
        commitFile << "Parent: " << parentHash << "\n";
    commitFile << "Date: " << time << "\n";
    commitFile << "Message: " << message << "\n";
    commitFile << "Files:\n" << indexContents;
    commitFile.close();

    std::ofstream updateBranch(".minigit/" + refPath);
    updateBranch << commitHash;
    updateBranch.close();

    std::ofstream clearIndex(".minigit/index", std::ofstream::trunc);
    clearIndex.close();

    std::cout << "Commit successful! Hash: " << commitHash << "\n";
}

// ---------------- Log History ----------------

void logHistory() {
    std::ifstream head(".minigit/HEAD");
    if (!head) {
        std::cerr << "Error: HEAD file not found.\n";
        return;
    }

    std::string refLine;
    std::getline(head, refLine);
    head.close();

    if (refLine.rfind("ref: ", 0) != 0) {
        std::cerr << "Invalid HEAD format.\n";
        return;
    }

    std::string branchPath = ".minigit/" + refLine.substr(5);
    std::ifstream branch(branchPath);
    std::string currentHash;
    std::getline(branch, currentHash);
    branch.close();
    std::unordered_set<std::string> visited;

    while (!currentHash.empty()&& visited.find(currentHash)== visited.end()) {
         visited.insert(currentHash); 
        std::ifstream commitFile(".minigit/commits/" + currentHash);
        if (!commitFile) {
            std::cerr << "Error: Commit file missing for hash " << currentHash << "\n";
            break;
        }

        std::string line, parentHash;
        std::cout << "---------------------------\n";
        while (std::getline(commitFile, line)) {
             if (line.find("Parent: ") == 0) {
                parentHash = line.substr(8);
            }
            if (line.find("Files:") == 0) break;
            std::cout << line << "\n";
        }
        std::cout << "---------------------------\n\n";
        commitFile.close();
         if (parentHash.empty()) break; // Stop when there’s no parent

        currentHash = parentHash;

    
    }
}

// ---------------- Restore File ----------------

void restoreFile(const std::string& commitHash, const std::string& filename) {
    std::ifstream commitFile(".minigit/commits/" + commitHash);
    if (!commitFile) {
        std::cerr << "Commit not found: " << commitHash << "\n";
        return;
    }

    std::string line;
    bool foundFile = false;
    std::string fileHash;

    while (std::getline(commitFile, line)) {
        if (line.rfind(filename + " ", 0) == 0) {
            fileHash = line.substr(filename.length() + 1);
            foundFile = true;
            break;
        }
    }
    commitFile.close();

    if (!foundFile) {
        std::cerr << "File '" << filename << "' not found in commit " << commitHash << "\n";
        return;
    }

    std::ifstream blob(".minigit/objects/" + fileHash);
    if (!blob) {
        std::cerr << "Could not find blob for hash: " << fileHash << "\n";
        return;
    }

    std::ofstream outFile(filename);
    outFile << blob.rdbuf();
    outFile.close();
    blob.close();

    std::cout << "Restored '" << filename << "' from commit " << commitHash << "\n";
}
// ---------------- Branching ----------------

void createBranch(const std::string& branchName) {
    std::ifstream head(".minigit/HEAD");
    std::string refPath;
    std::getline(head, refPath);
    head.close();

    if (refPath.rfind("ref: ", 0) == 0)
        refPath = refPath.substr(5);
    else {
        std::cerr << "HEAD is not pointing to a branch.\n";
        return;
    }

    std::ifstream currentBranch(".minigit/" + refPath);
    std::string currentHash;
    std::getline(currentBranch, currentHash);
    currentBranch.close();

    std::ofstream newBranch(".minigit/refs/" + branchName);
    newBranch << currentHash;
    newBranch.close();

    std::cout << "Created branch '" << branchName << "' at " << currentHash << "\n";
}

void checkoutBranch(const std::string& branchName) {
    std::string branchPath = ".minigit/refs/" + branchName;
    std::ifstream branch(branchPath);
    if (!branch) {
        std::cerr << "Branch not found: " << branchName << "\n";
        return;
    }

    std::ofstream head(".minigit/HEAD");
    head << "ref: refs/" << branchName;
    head.close();

    std::cout << "Switched to branch '" << branchName << "'\n";
}

std::string findCommonAncestor(const std::string& hash1, const std::string& hash2) {
    std::unordered_set<std::string> ancestors;

    // Traverse hash1's ancestors
    std::string current = hash1;
    while (!current.empty()) {
        ancestors.insert(current);
        std::ifstream file(".minigit/commits/" + current);
        if (!file) break;

        std::string line;
        while (std::getline(file, line)) {
            if (line.rfind("Parent: ", 0) == 0) {
                current = line.substr(8);
                break;
            }
        }
        if (file.eof()) break; // no parent found
    }

    // Traverse hash2's ancestors and find first match
    current = hash2;
    while (!current.empty()) {
        if (ancestors.count(current)) return current;

        std::ifstream file(".minigit/commits/" + current);
        if (!file) break;

        std::string line;
        while (std::getline(file, line)) {
            if (line.rfind("Parent: ", 0) == 0) {
                current = line.substr(8);
                break;
            }
        }
        if (file.eof()) break;
    }

    return ""; // no common ancestor found
}

void merge(const std::string& branchName) {
    // Step 1: Read current branch
    std::ifstream headFile(".minigit/HEAD");
    std::string currentRef;
    std::getline(headFile, currentRef);
    headFile.close();

    if (currentRef.rfind("ref: ", 0) != 0) {
        std::cerr << "HEAD is not pointing to a branch.\n";
        return;
    }

    std::string currentBranch = currentRef.substr(5);  // e.g., "refs/master"
    std::ifstream currentRefFile(".minigit/" + currentBranch);
    std::string currentCommitHash;
    std::getline(currentRefFile, currentCommitHash);
    currentRefFile.close();

    // Step 2: Read target branch
    std::string targetBranch = "refs/" + branchName;
    std::ifstream targetRefFile(".minigit/" + targetBranch);
    if (!targetRefFile) {
        std::cerr << "Branch '" << branchName << "' does not exist.\n";
        return;
    }

    std::string targetCommitHash;
    std::getline(targetRefFile, targetCommitHash);
    targetRefFile.close();

    std::cout << "Merging branch '" << branchName << "' into current branch...\n";
    
    // Step 3: Find LCA (we’ll do a simplified version)
    std::string lcaHash = findCommonAncestor(currentCommitHash, targetCommitHash);
    // Maps to hold filename → blob hash
std::unordered_map<std::string, std::string> lcaFiles;
std::unordered_map<std::string, std::string> currentFiles;
std::unordered_map<std::string, std::string> targetFiles;

// Helper to load files from a commit
auto loadFilesFromCommit = [](const std::string& hash) {
    std::unordered_map<std::string, std::string> fileMap;
    std::ifstream file(".minigit/commits/" + hash);
    std::string line;
    bool inFiles = false;
    while (std::getline(file, line)) {
        if (line == "Files:") {
            inFiles = true;
            continue;
        }
        if (inFiles && !line.empty()) {
            std::istringstream iss(line);
            std::string fname, fhash;
            iss >> fname >> fhash;
            fileMap[fname] = fhash;
        }
    }
    return fileMap;
};

// Load files from all three commits
if (!lcaHash.empty()) {
    lcaFiles = loadFilesFromCommit(lcaHash);
}
currentFiles = loadFilesFromCommit(currentCommitHash);
targetFiles = loadFilesFromCommit(targetCommitHash);

    std::cout << "Lowest Common Ancestor: " << lcaHash << "\n";

    // Steps 4–5 will go here next
    std::ofstream mergedIndex(".minigit/index", std::ofstream::trunc);
    std::vector<std::string> conflicts;

for (const auto& entry : targetFiles) {
    const std::string& file = entry.first;
    const std::string& targetHash = entry.second; 
    std::string baseHash = lcaFiles[file];
    std::string currentHash = currentFiles[file];

    if (currentHash == targetHash) {
        // No changes in either → skip
        continue;
    }

    if (currentHash == baseHash) {
        // Changed in target only → accept target
        mergedIndex << file << " " << targetHash << "\n";
    } else if (targetHash == baseHash) {
        // Changed in current only → accept current
        mergedIndex << file << " " << currentHash << "\n";
    } else {
        // Conflict!
        conflicts.push_back(file);
        mergedIndex << file << " " << currentHash << "\n"; // Keep current, mark conflict
    }
}

mergedIndex.close();
if (!conflicts.empty()) {
    std::cout << "CONFLICTS found in merge:\n";
    for (const std::string& file : conflicts) {
        std::cout << "CONFLICT: both modified " << file << "\n";
    }
    std::cout << "Please resolve conflicts manually before committing.\n";
    return; // stop here — user must fix files

}
// Finalize the merge as a commit
std::string time = getCurrentTime();
std::string mergeMessage = "Merged branch '" + branchName + "'";
std::string newHash = generateCommitHash(mergeMessage, time);

std::ofstream commitFile(".minigit/commits/" + newHash);
commitFile << "Commit: " << newHash << "\n";
commitFile << "Parent: " << currentCommitHash << "\n";
commitFile << "Parent: " << targetCommitHash << "\n"; // 2 parents = merge commit
commitFile << "Date: " << time << "\n";
commitFile << "Message: " << mergeMessage << "\n";
commitFile << "Files:\n";

std::ifstream index(".minigit/index");
std::string line;
while (std::getline(index, line)) {
    commitFile << line << "\n";
}
commitFile.close();
index.close();

// Update HEAD to point to the new merge commit
std::ofstream update(".minigit/" + currentBranch);
update << newHash;
update.close();

std::cout << "Merge complete! Commit: " << newHash << "\n";



}
void diffFile(const std::string& filename, const std::string& commitA, const std::string& commitB) {
    auto getFileContentFromCommit = [](const std::string& commitHash, const std::string& filename) {
        std::ifstream commitFile(".minigit/commits/" + commitHash);
        if (!commitFile) return std::string{};

        std::string line;
        bool inFiles = false;
        std::string hash;
        while (std::getline(commitFile, line)) {
            if (line == "Files:") {
                inFiles = true;
                continue;
            }
            if (inFiles) {
                std::istringstream iss(line);
                std::string fname, fhash;
                iss >> fname >> fhash;
                if (fname == filename) {
                    hash = fhash;
                    break;
                }
            }
        }

        if (hash.empty()) return std::string{};

        std::ifstream blob(".minigit/objects/" + hash);
        if (!blob) return std::string{};

        return std::string((std::istreambuf_iterator<char>(blob)), std::istreambuf_iterator<char>());
    };

    std::string contentA, contentB;

    if (commitA.empty()) {
        // Load from working directory
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "File not found in working directory.\n";
            return;
        }
        contentA = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    } else {
        contentA = getFileContentFromCommit(commitA, filename);
        if (contentA.empty()) {
            std::cerr << "File not found in commit " << commitA << "\n";
            return;
        }
    }

    contentB = getFileContentFromCommit(commitB, filename);
    if (contentB.empty()) {
        std::cerr << "File not found in commit " << commitB << "\n";
        return;
    }

    // Compare line by line
    std::istringstream streamA(contentA);
    std::istringstream streamB(contentB);
    std::string lineA, lineB;
    int lineNum = 1;
    bool differenceFound = false;

    while (std::getline(streamA, lineA) && std::getline(streamB, lineB)) {
        if (lineA != lineB) {
            differenceFound = true;
            std::cout << "Line " << lineNum << ":\n";
            std::cout << "- A: " << lineA << "\n";
            std::cout << "+ B: " << lineB << "\n";
        }
        ++lineNum;
    }

    // Handle extra lines
    while (std::getline(streamA, lineA)) {
        differenceFound = true;
        std::cout << "Line " << lineNum << ":\n";
        std::cout << "- A: " << lineA << "\n";
        ++lineNum;
    }

    while (std::getline(streamB, lineB)) {
        differenceFound = true;
        std::cout << "Line " << lineNum << ":\n";
        std::cout << "+ B: " << lineB << "\n";
        ++lineNum;
    }

    if (!differenceFound) {
        std::cout << "No differences found.\n";
    }
}


