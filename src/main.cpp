#include <iostream>
#include "../include/minigit.hpp"

int main() {
    std::string command;
    bool running = true;

    std::cout << "\nWelcome to MiniGit!\n";
    initMiniGit(); // Automatically initialize the repo if not already done

    while (running) {
        std::cout << "\n============================\n";
        std::cout << "MiniGit > Choose a command:\n";
        std::cout << "1. add <filename>\n";
        std::cout << "2. commit\n";
        std::cout << "3. log\n";
        std::cout << "4. branch <branch-name>\n";
        std::cout << "5. checkout <branch-name or commit-hash>\n";
        std::cout << "6. merge <branch-name>\n";
        std::cout << "7. restore <commit-hash> <filename>\n";
        std::cout << "8. diff <filename> <commitA> <commitB>\n";
        std::cout << "0. exit\n";
        std::cout << "============================\n";
        std::cout << "Enter command: ";
        std::cin >> command;

        if (command == "1" || command == "add") {
            std::string filename;
            std::cout << "Enter filename to add: ";
            std::cin >> filename;
            addFile(filename);

        } else if (command == "2" || command == "commit") {
            std::cin.ignore();
            std::string message;
            std::cout << "Enter commit message: ";
            std::getline(std::cin, message);
            commit(message);

        } else if (command == "3" || command == "log") {
            logHistory();

        } else if (command == "4" || command == "branch") {
            std::string branchName;
            std::cout << "Enter new branch name: ";
            std::cin >> branchName;
            createBranch(branchName);

        } else if (command == "5" || command == "checkout") {
            std::string branchName;
            std::cout << "Enter branch name or commit hash: ";
            std::cin >> branchName;
            checkoutBranch(branchName);

        } else if (command == "6" || command == "merge") {
            std::string branchName;
            std::cout << "Enter branch name to merge into current: ";
            std::cin >> branchName;
            merge(branchName);

        } else if (command == "7" || command == "restore") {
            std::string hash, filename;
            std::cout << "Enter commit hash: ";
            std::cin >> hash;
            std::cout << "Enter filename to restore: ";
            std::cin >> filename;
            restoreFile(hash, filename);

        } else if (command == "8" || command == "diff") {
            std::string filename, a, b;
            std::cout << "Enter filename to compare: ";
            std::cin >> filename;
            std::cout << "Enter commit hash A: ";
            std::cin >> a;
            std::cout << "Enter commit hash B: ";
            std::cin >> b;
            diffFile(filename, a, b);

        } else if (command == "0" || command == "exit") {
            running = false;
            std::cout << "Exiting MiniGit. Goodbye!\n";
        } else {
            std::cout << "Invalid command. Try again.\n";
        }
    }

    return 0;
}
