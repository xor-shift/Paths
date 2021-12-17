#pragma once

// from cppreference
template<class... Ts> struct Overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;

#define CONST_CAST_CALL_NO_ARG(T, fn) const_cast<T>(const_cast<const std::decay_t<decltype(*this)> *>(this)->fn())
#define CONST_CAST_CALL(T, fn, ...) \
    const_cast<T>(const_cast<const std::decay_t<decltype(*this)> *>(this)->fn(__VA_ARGS__))

#include <functional>
#include <stdexcept>

namespace Utils::ScopeGuard {

enum ECondition { Failure, Success, Any };

template<ECondition condition, typename Callable> struct ScopeGuard {
    constexpr ScopeGuard() noexcept = delete;

    constexpr explicit ScopeGuard(Callable &&cb) noexcept(noexcept(Callable(std::forward<Callable>(cb))))
        : m_cb(std::forward<Callable>(cb)) { }

    constexpr explicit ScopeGuard(const Callable &cb) noexcept(noexcept(Callable(cb)))
        : m_cb(cb) { }

    constexpr ~ScopeGuard() noexcept {
        if (!m_armed)
            return;
        if ((condition == ECondition::Failure && std::current_exception())
            || (condition == ECondition::Success && !std::current_exception()) || (condition == ECondition::Any))
            std::invoke(m_cb);
    }

    bool m_armed = true;

private:
    Callable m_cb;
};

template<typename Callable> constexpr auto make_success_guard(Callable &&cb) {
    return ScopeGuard<ECondition::Success, Callable>(std::forward<Callable>(cb));
}

template<typename Callable> constexpr auto make_failure_guard(Callable &&cb) {
    return ScopeGuard<ECondition::Failure, Callable>(std::forward<Callable>(cb));
}

template<typename Callable> constexpr auto make_guard(Callable &&cb) {
    return ScopeGuard<ECondition::Any, Callable>(std::forward<Callable>(cb));
}

}
