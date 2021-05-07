#include "Context.h"
#include "Executor/Executor.h"

#include <thread>

int main()
{
    auto context = Context("examples/wisteria/wisteria.bgn", Context::Operation::Parse);
    context.run();

    Executor::the().run();

    while (!context.done()) {
        std::this_thread::yield();
    }

    Executor::the().stop();
    Executor::the().await();

    return 0;
}
