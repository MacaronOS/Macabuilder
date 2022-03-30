/*
 * Context object is an internal representation of a Maca file,
 * which is being processed by a thread.
 */

#pragma once

#include "Executor/Executor.h"
#include "Finder/Finder.h"
#include "IncludeParser.h"
#include "Parser/Parser.h"
#include "Utils/Lock.h"
#include "Utils/Logger.h"

#include "Parser/Field/BuildField.h"
#include "Parser/Field/CommandsField.h"
#include "Parser/Field/DefaultField.h"
#include "Parser/Field/DefinesField.h"
#include "Parser/Field/IncludeField.h"

#include <cstdlib>
#include <filesystem>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>

constexpr size_t path_max = 4096;

enum class IncludeStatus {
    NotVisited,
    NeedsRecompilation,
    UpToDate,
};

class Context {
    friend class Parser;
    friend class Executor;

public:
    enum class State {
        NotStarted,
        ParseError,
        Parsed,
        BuildError,
        Built,
    };

    enum class Operation {
        Parse,
        Build,
    };

public:
    Context(std::filesystem::path path, Operation operation, bool root_ctx = false);

    void run();
    bool run_as_childs(const std::string& pattern, Operation operation);

    inline bool done() const { return m_done; }
    inline Operation operation() const { return m_operation; };
    inline std::filesystem::path directory() const { return m_path.parent_path(); }
    inline std::filesystem::path cwd() const
    {
        auto cwd = directory();
        if (cwd.empty()) {
            return ".";
        }
        return cwd;
    }
    inline std::string maca_path() const
    {
        auto dir = directory();
        if (dir.empty()) {
            return "MacaBuild";
        }
        return dir / "MacaBuild";
    }
    inline std::string static_library_path() const
    {
        size_t lastindex = m_path.string().find_last_of('.');
        std::string libname = m_path.string().substr(0, lastindex);
        return (maca_path() / std::filesystem::proximate(libname, directory())).string() + ".a";
    }
    inline std::string executable_path() const
    {
        size_t lastindex = m_path.string().find_last_of('.');
        std::string libname = m_path.string().substr(0, lastindex);
        return (maca_path() / std::filesystem::proximate(libname, directory())).string();
    }
    inline std::string timestamps_path() const
    {
        return std::filesystem::path(maca_path()) / "timestamps.macainfo";
    }
    inline bool root_ctx() const { return m_root_ctx; }
    inline std::string name() const { return std::filesystem::path(executable_path()).filename(); }
    inline bool root() const { return directory().empty(); }
    inline const auto& children() const { return m_children; }
    inline const auto& build_field() const { return m_build; }

private:
    void validate_fields();
    bool merge_children();
    bool build();
    void fill_timestamps();
    void dump_timestamps();
    void process_by_mode();

    IncludeStatus scan_include(const std::filesystem::path& file);

    inline void mark_source_as_failed(const std::string& failed_source)
    {
        m_failed_sources.insert(std::filesystem::proximate(directory(), failed_source));
    }

    inline void trigger_error(const std::string& error)
    {
        Log(Color::Red, m_path.string() + ":", error);
        exit(1);
    }

    static inline Context* get_context_by_path(const std::filesystem::path& path)
    {
        static char resolved_path[path_max];
        realpath(path.c_str(), resolved_path);
        return s_processing_contexts[resolved_path];
    }

    static inline void register_context(const std::filesystem::path& path, Context* context)
    {
        static char resolved_path[path_max];
        realpath(path.c_str(), resolved_path);
        s_processing_contexts[resolved_path] = context;
    }

private:
    std::filesystem::path m_path;
    Operation m_operation;
    bool m_root_ctx {};
    std::thread* m_thread {};
    State m_state { State::NotStarted };
    bool m_done {};

    // Parser
    Parser parser {};
    IncludeField m_include {};
    DefinesField m_defines {};
    CommandsField m_commands {};
    BuildField m_build {};
    DefaultField m_default {};

    // Executor
    std::atomic<int> compile_counter {};
    bool done_finalizer {};

    // Children options
    std::vector<Context*> m_children {};

    // If included context contains a build field it's going to be built separately
    std::vector<BuildField> m_children_builds {};

    bool m_was_any_recompilation {};
    std::unordered_map<std::string, int> m_timestamps {};
    std::unordered_map<std::string, IncludeStatus> m_include_status {};
    std::unordered_set<std::string> m_failed_sources {};
    std::vector<std::string> m_visited_stack {};
    std::unordered_map<std::string, size_t> m_path_to_visited_stack_index {};

    static SpinLock m_lock;
    static std::unordered_map<std::string, Context*> s_processing_contexts;
};
