#ifndef VEC2_H
#define VEC2_H

namespace Pilo
{
struct Vec2
{
    int x = 0;
    int y = 0;
};

constexpr Vec2 operator+(const Vec2& v1, const Vec2& v2)
{
    return {v1.x + v2.x, v1.y + v2.y};
}

constexpr Vec2 operator+(const Vec2& v, int x)
{
    return {v.x + x, v.y + x};
}

constexpr Vec2 operator-(const Vec2& v1, const Vec2& v2)
{
    return {v1.x - v2.x, v1.y - v2.y};
}

constexpr Vec2 operator-(const Vec2& v, int x)
{
    return {v.x - x, v.y - x};
}
}

#endif  // VEC2_H