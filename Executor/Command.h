#pragma once

#include <string>
#include <unistd.h>

class Command {
public:
    Command();
    ~Command();

public:
    bool done();
    void execute(const std::string& command);

public:
    int exit_status() const { return m_exit_status; }
    std::string& std_out() { return m_std_out; }
    [[nodiscard]] const std::string& std_out() const { return m_std_out; }

    std::string& std_err() { return m_std_err; }
    [[nodiscard]] const std::string& std_err() const { return m_std_err; }

private:
    int m_command_pid { -1 };
    bool m_done {};
    int8_t m_exit_status {};

    int m_out_fds[2] {};
    int m_err_fds[2] {};
    constexpr static auto read_ptr = 0, write_ptr = 1;

    std::string m_std_out {};
    std::string m_std_err {};
};
