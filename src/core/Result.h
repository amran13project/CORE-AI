#pragma once
#include <string>
#include <utility>
#include <optional>

namespace coreai {

enum class ErrorCode {
    Ok = 0,
    InvalidArgument,
    ConfigInvalid,
    StorageOpenFailed,
    StorageUnavailable,
    StorageCorrupt,
    PermissionDenied,
    ProviderUnavailable,
    ModelUnavailable,
    NetworkTimeout,
    ParseError,
    NotImplemented,
    NotVerified,
    Internal
};

struct Error {
    ErrorCode code{ErrorCode::Internal};
    std::string message;
    std::string component;
    std::string operation;
    bool recoverable{false};
    bool retryable{false};
};

template <typename T>
class Result {
public:
    static Result success(T v) { return Result(std::move(v)); }
    static Result failure(Error e) { Result r; r.error_ = std::move(e); return r; }
    bool ok() const { return value_.has_value(); }
    const T& value() const { return *value_; }
    T& value() { return *value_; }
    const Error& error() const { return *error_; }
private:
    Result() = default;
    explicit Result(T v) : value_(std::move(v)) {}
    std::optional<T> value_;
    std::optional<Error> error_;
};

template <>
class Result<void> {
public:
    static Result success() { return Result(true); }
    static Result failure(Error e) { Result r(false); r.error_ = std::move(e); return r; }
    bool ok() const { return ok_; }
    const Error& error() const { return *error_; }
private:
    explicit Result(bool ok) : ok_(ok) {}
    bool ok_{false};
    std::optional<Error> error_;
};

inline Error error(ErrorCode code, std::string message, std::string component, std::string operation,
                   bool recoverable=false, bool retryable=false) {
    return Error{code, std::move(message), std::move(component), std::move(operation), recoverable, retryable};
}

} // namespace coreai
