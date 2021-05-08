#pragma once

#include <memory>
#include <string>
#include <vector>

enum class Operation {
    Compile,
    Link,
    Archive,
};

class Context;

struct ExecutableUnit {
    Operation op {};
    Context* ctx {};
    std::shared_ptr<std::string> callee {};
    std::string src {};
    std::shared_ptr<std::string> binary;
    std::vector<std::shared_ptr<std::string>> args {};
};
