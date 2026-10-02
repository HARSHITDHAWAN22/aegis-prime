#pragma once
#include <string>

namespace aegis::kv {

// Placeholder interface — filled in once SPEC.md is written.
class Store {
public:
    virtual ~Store() = default;
    virtual bool put(const std::string& key, const std::string& value) = 0;
    virtual bool get(const std::string& key, std::string& out_value) = 0;
    virtual bool remove(const std::string& key) = 0;
};

}  // namespace aegis::kv