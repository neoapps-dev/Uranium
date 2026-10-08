#pragma once
#include <cstddef>
#include <vector>
namespace uranium {
using Index = std::size_t;
template <class T>
class Span {
public:
    constexpr Span() = default;
    constexpr Span(T* data, std::size_t size) : m_data(data), m_size(size) {}
    template <class A>
    constexpr Span(std::vector<A>& v) : m_data(v.data()), m_size(v.size()) {}
    template <class A>
    constexpr Span(const std::vector<A>& v) : m_data(v.data()), m_size(v.size()) {}
    template <std::size_t N>
    constexpr Span(T (&arr)[N]) : m_data(arr), m_size(N) {}
    constexpr T* data() const { return m_data; }
    constexpr std::size_t size() const { return m_size; }
    constexpr bool empty() const { return m_size == 0; }
    constexpr T& operator[](std::size_t i) const { return m_data[i]; }
    constexpr T* begin() const { return m_data; }
    constexpr T* end() const { return m_data + m_size; }
    constexpr Span subspan(std::size_t offset, std::size_t count) const {
        return Span(m_data + offset, count);
    }

private:
    T* m_data = nullptr;
    std::size_t m_size = 0;
};

template <class T>
Span<T> make_span(std::vector<T>& v) {
    return Span<T>(v.data(), v.size());
}

template <class T>
Span<const T> make_span(const std::vector<T>& v) {
    return Span<const T>(v.data(), v.size());
}
}
