#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class BuildField {
public:
    enum class Type {
        StaticLib,
        Executable,
    };

    struct ExtensionOption {
        std::string const* compiler {};
        std::vector<std::string const*> flags {};
    };

public:
    BuildField() = default;

    inline bool set_type(const std::string& type)
    {
        if (type == "StaticLib") {
            m_type = Type::StaticLib;
        } else if (type == "Executable") {
            m_type = Type::Executable;
        } else {
            return false;
        }
        return true;
    }

    inline void add_dependency(const std::string& dependency)
    {
        m_depends.push_back(&dependency);
    }

    inline void add_source(const std::string& source)
    {
        m_sources.push_back(&source);
    }

    inline bool set_compiler_to_extension(const std::string& extension, const std::string& compiler)
    {
        if (m_extensions[&extension].compiler) [[unlikely]] {
            return false;
        }
        m_extensions[&extension].compiler = &compiler;
        return true;
    }

    inline void add_flag_to_extension(const std::string& extension, const std::string& flag)
    {
        m_extensions[&extension].flags.push_back(&flag);
    }

private:
    Type m_type {};
    std::vector<std::string const*> m_depends {};
    std::vector<std::string const*> m_sources {};
    std::unordered_map<std::string const*, ExtensionOption> m_extensions;
};
