#ifndef POINT_H
#define POINT_H

#include <cmath>
#include <concepts>
#include <utility>

namespace detail
{
template <typename T, typename... U>
concept one_of = (std::same_as<T, U> || ...);

template <typename T>
concept number = one_of<T, int, double, size_t>;
} // namespace detail

template <typename T> struct PointT {
    T x;
    T y;

    PointT() = default;
    template <detail::number P1, detail::number P2>
    PointT(P1 _x, P2 _y) : x(_x), y(_y)
    {
    }

    std::pair<T, T> asPair() const
    {
        return {x, y};
    }

    auto operator<=>(const PointT<T> &) const = default;
    PointT<T> &operator+=(PointT<T> rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
    PointT<T> &operator-=(PointT<T> rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }
    PointT<T> operator+(PointT<T> rhs) const
    {
        return rhs += *this;
    }
    PointT<T> operator-(PointT<T> rhs) const
    {
        return PointT{*this} -= rhs;
    }
};

typedef PointT<int> Point;
typedef PointT<double> Pointf;

template <typename T> double hypot(PointT<T> a, PointT<T> b)
{
    auto [x, y] = a - b;
    return std::hypot(x, y);
}

#endif // POINT_H
