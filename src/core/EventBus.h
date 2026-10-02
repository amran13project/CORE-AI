#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace core {
class EventBus {
public:
    using Handler = std::function<void(const std::string&)>;
    void subscribe(const std::string& event, Handler handler);
    void publish(const std::string& event, const std::string& payload = {});
private:
    std::unordered_map<std::string, std::vector<Handler>> handlers_;
};
}
