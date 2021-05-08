#include "Context.h"
#include "Executor/Executor.h"
#include "Finder/Finder.h"
#include "Logger.h"

#include <thread>

int main()
{
    auto beegn_files = Finder::FindRootBeegnFiles();

    if (beegn_files.size() > 1) {
        Log(Color::Red, "multiple root files are presented");
        exit(1);
    }

    auto root = beegn_files[0];

    auto context = Context(root, Context::Operation::Build);
    context.run();

    Executor::the().run();

    while (!context.done()) {
        std::this_thread::yield();
    }

    Executor::the().stop();
    Executor::the().await();

    return 0;
}
