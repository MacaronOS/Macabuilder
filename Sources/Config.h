#pragma once

#include <string>
#include <vector>

class Config {
public:
    enum class Mode {
        Generate,
        Default,
        CommandList,
    };

public:
    static inline auto& the()
    {
        static auto instance = Config();
        return instance;
    }

public:
    void process_arguments(int argc, char** argv);

public:
    const auto& filename() const { return m_filename; }
    const auto& arguments() const { return m_arguments; }
    Mode mode() const { return m_mode; }
    int timestamp() const { return m_timestamp; }

private:
    Config();

private:
    int m_timestamp;
    std::string m_filename {};
    std::vector<std::string> m_arguments {};
    Mode m_mode {};
};
