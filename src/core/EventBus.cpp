#include "core/EventBus.h"
namespace core {
void EventBus::subscribe(const std::string& event, Handler handler) { handlers_[event].push_back(std::move(handler)); }
void EventBus::publish(const std::string& event, const std::string& payload) {
    if (auto it = handlers_.find(event); it != handlers_.end()) for (auto& h : it->second) h(payload);
}
}
