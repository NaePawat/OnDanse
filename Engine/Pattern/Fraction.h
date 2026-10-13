#pragma once

#include <compare>
#include <cstdint>
#include <numeric>
#include <ostream>
#include <stdexcept>

namespace alors
{

class Fraction
{
public:
    constexpr Fraction() = default;

    constexpr Fraction (std::int64_t numerator, std::int64_t denominator = 1)
        : num (numerator), den (denominator)
    {
        if (den == 0)
            throw std::invalid_argument ("Fraction denominator must not be zero");

        normalise();
    }

    constexpr std::int64_t numerator() const   { return num; }
    constexpr std::int64_t denominator() const { return den; }

    constexpr double toDouble() const { return static_cast<double> (num) / static_cast<double> (den); }

    constexpr Fraction floor() const
    {
        auto q = num / den;
        if (num % den != 0 && num < 0)
            --q;
        return { q, 1 };
    }

    constexpr Fraction ceil() const
    {
        auto f = floor();
        return f == *this ? f : f + 1;
    }

    constexpr Fraction cyclePosition() const { return *this - floor(); }

    friend constexpr Fraction operator+ (Fraction a, Fraction b) { return { a.num * b.den + b.num * a.den, a.den * b.den }; }
    friend constexpr Fraction operator- (Fraction a, Fraction b) { return { a.num * b.den - b.num * a.den, a.den * b.den }; }
    friend constexpr Fraction operator* (Fraction a, Fraction b) { return { a.num * b.num, a.den * b.den }; }
    friend constexpr Fraction operator/ (Fraction a, Fraction b) { return { a.num * b.den, a.den * b.num }; }
    constexpr Fraction operator-() const { return { -num, den }; }

    constexpr Fraction& operator+= (Fraction other) { return *this = *this + other; }
    constexpr Fraction& operator-= (Fraction other) { return *this = *this - other; }

    friend constexpr bool operator== (Fraction a, Fraction b) { return a.num == b.num && a.den == b.den; }

    friend constexpr std::strong_ordering operator<=> (Fraction a, Fraction b)
    {
        return a.num * b.den <=> b.num * a.den;
    }

    friend std::ostream& operator<< (std::ostream& os, Fraction f)
    {
        os << f.num;
        if (f.den != 1)
            os << '/' << f.den;
        return os;
    }

private:
    constexpr void normalise()
    {
        if (den < 0)
        {
            num = -num;
            den = -den;
        }

        if (auto g = std::gcd (num, den); g > 1)
        {
            num /= g;
            den /= g;
        }
    }

    std::int64_t num = 0;
    std::int64_t den = 1;
};

constexpr Fraction min (Fraction a, Fraction b) { return a < b ? a : b; }
constexpr Fraction max (Fraction a, Fraction b) { return a < b ? b : a; }

}
