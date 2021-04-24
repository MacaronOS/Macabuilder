#include "Context.h"

int main()
{
    auto context = Context("example.bgn", Context::Operation::Parse);
    context.run();
    return 0;
}
