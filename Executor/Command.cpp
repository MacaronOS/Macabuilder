#include "Command.h"

#include <array>
#include <fcntl.h>
#include <unistd.h>

Command::Command()
{
    // opening stdout file descriptors
    int res = pipe(m_out_fds);
    if (res < 0) {
        exit(1);
    }

    // opening stderr file descriptors
    res = pipe(m_err_fds);
    if (res < 0) {
        exit(1);
    }

    // setting non blocking mode to read file descriptors
    if (fcntl(m_out_fds[0], F_SETFL, O_NONBLOCK) < 0) {
        exit(1);
    }
    if (fcntl(m_err_fds[0], F_SETFL, O_NONBLOCK) < 0) {
        exit(1);
    }
}

Command::~Command()
{
    close(m_out_fds[read_ptr]);
    close(m_out_fds[write_ptr]);
    close(m_err_fds[read_ptr]);
    close(m_err_fds[write_ptr]);
}

void Command::execute(const std::string& command)
{
    m_done = false;
    m_exit_status = 0;
    m_std_out.clear();
    m_std_err.clear();

    m_command_pid = fork();
    if (m_command_pid < 0) {
        exit(1);
    }

    if (m_command_pid == 0) {
        dup2(m_out_fds[write_ptr], STDOUT_FILENO);
        dup2(m_err_fds[write_ptr], STDERR_FILENO);

        execl("/bin/bash", "bash", "-c", command.c_str(), nullptr);
    }
}

bool Command::done()
{
    if (m_done) {
        return true;
    }

    int status = 0;

    if (waitpid(m_command_pid, &status, WNOHANG) <= 0) {
        return false;
    }

    if (WIFEXITED(status)) {
        m_exit_status = WEXITSTATUS(status);
    }

    std::array<char, 256> buffer {};

    while (true) {
        int out_bytes = read(m_out_fds[read_ptr], buffer.data(), buffer.size());
        if (out_bytes < 0) {
            break;
        }
        m_std_out.append(buffer.data(), out_bytes);
    }

    while (true) {
        int err_bytes = read(m_err_fds[read_ptr], buffer.data(), buffer.size());
        if (err_bytes < 0) {
            break;
        }
        m_std_err.append(buffer.data(), err_bytes);
    }

    m_done = true;
    return true;
}
