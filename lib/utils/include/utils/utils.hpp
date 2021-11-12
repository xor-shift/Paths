#pragma once

//from cppreference
template<class... Ts>
struct overloaded : Ts ... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

#define ConstCastCallNoArg(T, fn) const_cast<T>(const_cast<const std::decay_t<decltype(*this)> *>(this)->fn())
#define ConstCastCall(T, fn, ...) const_cast<T>(const_cast<const std::decay_t<decltype(*this)> *>(this)->fn(__VA_ARGS__))
