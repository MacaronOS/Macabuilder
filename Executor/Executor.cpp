#include "Executor.h"
#include "../Context.h"
#include "../Logger.h"
#include "ExecutableUnit.h"

#include <iostream>
#include <thread>

void Executor::run()
{
    m_thread = new std::thread([this]() {
        for (size_t cpu = 0; cpu < m_free_processes; cpu++) {
            m_commands[cpu].open_descriptors();
        }

        const auto fetch_command = [](Command& cmd) {
            if (cmd.executable_unit()->op == Operation::Compile) {
                if (cmd.exit_status()) {
                    Log(Color::Red, "Build error:", cmd.executable_unit()->src);
                } else {
                    if (!cmd.std_out().empty() || !cmd.std_err().empty()) {
                        Log(Color::Yellow, "Built with warnings:", cmd.executable_unit()->src);
                    } else {
                        Log(Color::Green, "Built:", cmd.executable_unit()->src);
                    }
                }

                cmd.executable_unit()->ctx->compile_counter--;
            } else {
                if (cmd.exit_status()) {
                    Log(Color::Red, "Link error:", cmd.executable_unit()->src);
                } else {
                    if (!cmd.std_out().empty() || !cmd.std_err().empty()) {
                        Log(Color::Yellow, "Linked with warnings:", cmd.executable_unit()->src);
                    } else {
                        Log(Color::Green, "Linked:", *cmd.executable_unit()->binary);
                    }
                }

                cmd.executable_unit()->ctx->done_linker = true;
            }

            if (!cmd.std_out().empty()) {
                std::cout << cmd.std_out() << "\n";
            }

            if (!cmd.std_err().empty()) {
                std::cout << cmd.std_err() << "\n";
            }

            cmd.fetch();
        };

        std::shared_ptr<ExecutableUnit> unit {};

        while (m_running || m_units.size_approx()) {
            int fetched = -1;

            for (size_t at = 0; at < m_commands.size(); at++) {
                auto& cmd = m_commands[at];

                if (cmd.done() && !cmd.fetched()) {
                    fetch_command(cmd);
                }

                if (cmd.fetched()) {
                    fetched = at;
                }
            }

            if (fetched < 0) {
                continue;
            }

            if (!m_units.dequeue(unit)) {
                std::this_thread::yield();
                continue;
            }

            process_unit(unit, m_commands[fetched]);
        }

        for (auto& cmd : m_commands) {
            while (!cmd.done()) { }

            if (!cmd.fetched()) {
                fetch_command(cmd);
            }
        }
    });
}

void Executor::enqueue(const std::shared_ptr<ExecutableUnit>& u3)
{
    m_units.enqueue(u3);
}

void Executor::await()
{
    m_thread->join();
}

void Executor::process_unit(const std::shared_ptr<ExecutableUnit>& unit, Command& cmd)
{
    if (unit->op == Operation::Compile) {
        unit->ctx->compile_counter++;
    }
    cmd.set_executable_unit(unit);
    cmd.execute(*unit->callee, unit->args);
}
