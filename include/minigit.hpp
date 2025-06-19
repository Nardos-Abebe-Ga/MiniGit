#ifndef MINIGIT_HPP
#define MINIGIT_HPP

#include <string>

void merge(const std::string& branchName);
std::string findCommonAncestor(const std::string& hash1, const std::string& hash2);
void initMiniGit();
void addFile(const std::string& filename);
void commit(const std::string& message);
void logHistory();
void restoreFile(const std::string& commitHash, const std::string& filename);
void createBranch(const std::string& branchName);
void checkoutBranch(const std::string& branchName);
void diffFile(const std::string& filename, const std::string& commitA, const std::string& commitB);



#endif
