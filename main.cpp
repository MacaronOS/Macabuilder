#include "Context.h"
#include "Executor/Executor.h"
#include "Finder/Finder.h"
#include "Logger.h"

#include <thread>

int main()
{
    auto beelder_files = Finder::FindRootBeelderFiles();

    if (beelder_files.size() > 1) {
        Log(Color::Red, "multiple root files are presented");
        exit(1);
    }

    auto root = beelder_files[0];

    auto context = Context(root, Context::Operation::Build);
    context.run();

    Executor::the().run();

    while (!context.done()) {
        std::this_thread::yield();
    }

    return 0;
}
