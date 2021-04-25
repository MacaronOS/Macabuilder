#include "Context.h"
#include <thread>
int main()
{
    auto context = Context("examples/wisteria/wisteria.bgn", Context::Operation::Parse);
    context.run();
    while (!context.done()) {
        std::this_thread::yield();
    }

    return 0;
}
