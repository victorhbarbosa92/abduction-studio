#pragma once
#include <vector>
#include <memory>
#include <iostream>

namespace KuroUI {

    class ICommand {
    public:
        virtual ~ICommand() = default;
        virtual void execute() = 0;
        virtual void undo() = 0;
    };

    class CommandManager {
    private:
        std::vector<std::shared_ptr<ICommand>> undo_stack;
        std::vector<std::shared_ptr<ICommand>> redo_stack;
        const size_t MAX_HISTORY = 50;

    public:
        void executeCommand(std::shared_ptr<ICommand> cmd) {
            cmd->execute();
            undo_stack.push_back(cmd);
            redo_stack.clear(); // Limpa o redo quando uma nova ação é feita
            
            if (undo_stack.size() > MAX_HISTORY) {
                undo_stack.erase(undo_stack.begin());
            }
        }

        void undo() {
            if (!undo_stack.empty()) {
                auto cmd = undo_stack.back();
                cmd->undo();
                redo_stack.push_back(cmd);
                undo_stack.pop_back();
                std::cout << "[COMMAND] Undo realizado.\n";
            }
        }

        void redo() {
            if (!redo_stack.empty()) {
                auto cmd = redo_stack.back();
                cmd->execute();
                undo_stack.push_back(cmd);
                redo_stack.pop_back();
                std::cout << "[COMMAND] Redo realizado.\n";
            }
        }
    };

}
