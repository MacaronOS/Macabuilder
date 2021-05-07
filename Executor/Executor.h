#pragma once

#include "../utils/ThreadQueue.h"
#include "Command.h"
#include "ExecutableUnit.h"

#include <atomic>
#include <memory>
#include <stack>
#include <thread>
#include <utility>
#include <vector>

class Context;

class Executor {
public:
    static Executor& the()
    {
        static auto instance = Executor();
        return instance;
    }

    void run();
    inline void stop() { m_running = false; }
    void await();
    void enqueue(const std::shared_ptr<ExecutableUnit>& u3);

private:
    Executor() = default;

private:
    static void process_unit(const std::shared_ptr<ExecutableUnit>& u3, Command& cmd);

private:
    std::thread* m_thread {};
    bool m_running { true };
    size_t m_free_processes { std::max((uint32_t)1, std::thread::hardware_concurrency()) };
    std::vector<Command> m_commands { std::vector<Command>(m_free_processes) };
    ThreadQueue<std::shared_ptr<ExecutableUnit>> m_units {};
};
