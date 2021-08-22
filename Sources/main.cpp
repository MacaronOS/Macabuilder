#include "Config.h"
#include "Context.h"
#include "Executor/Executor.h"
#include "Finder/Finder.h"
#include "Utils/Logger.h"

#include <thread>

int main(int argc, char** argv)
{
    Config::the().process_arguments(argc, argv);

    auto maca_files = Finder::FindRootMacaFiles();

    if (maca_files.size() > 1) {
        Log(Color::Red, "multiple root files are presented");
        exit(1);
    }

    if (maca_files.empty()) {
        Log(Color::Red, "no root files are presented");
        exit(1);
    }

    auto context = Context(maca_files.front(), Context::Operation::Build, true);
    context.run();

    Executor::the().run();

    while (!context.done()) {
        std::this_thread::yield();
    }

    return 0;
}
