#pragma once
#include "uranium/core/config.hpp"
#include "uranium/math/matrix.hpp"
#include "uranium/math/quaternion.hpp"
#include "uranium/math/vector.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <variant>
#include <vector>
namespace uranium {
namespace physics {
using Vec3 = Vector<double, 3>;
using Mat3 = Matrix<double, 3, 3>;
using Quat = Quaternion<double>;
struct SphereShape {
    double radius = 0.5;
    Vec3 offset{0.0, 0.0, 0.0};
};

struct BoxShape {
    Vec3 half_extents{0.5, 0.5, 0.5};
    Vec3 offset{0.0, 0.0, 0.0};
};

using Shape = std::variant<SphereShape, BoxShape>;
struct Plane {
    Vec3 normal{0.0, 1.0, 0.0};
    double offset = 0.0;
    Plane() = default;
    Plane(Vec3 n, double d) : normal(normalize(n)), offset(d) {}
    static Plane at_point(Vec3 point, Vec3 normal) {
        Vec3 n = normalize(normal);
        return Plane(n, dot(n, point));
    }

    URANIUM_NODISCARD double signed_distance(const Vec3& p) const { return dot(normal, p) - offset; }
};

struct Contact {
    Vec3 point{0.0, 0.0, 0.0};
    Vec3 normal{0.0, 1.0, 0.0};
    double depth = 0.0;
    int a = -1;
    int b = -1;
};

namespace detail {
inline Vec3 closest_point_on_obb(const Vec3& p, const Vec3& center, const Vec3& half_extents, const Mat3& rotation) {
    Vec3 local = rotation.transposed() * (p - center);
    Vec3 clamped{std::max(-half_extents.x, std::min(half_extents.x, local.x)),
                 std::max(-half_extents.y, std::min(half_extents.y, local.y)),
                 std::max(-half_extents.z, std::min(half_extents.z, local.z))};
    return center + rotation * clamped;
}

inline std::array<Vec3, 8> obb_corners(const Vec3& center, const Vec3& half_extents, const Mat3& rotation) {
    std::array<Vec3, 8> corners{};
    for (int i = 0; i < 8; ++i) {
        Vec3 local{(i & 1) ? half_extents.x : -half_extents.x, (i & 2) ? half_extents.y : -half_extents.y,
                   (i & 4) ? half_extents.z : -half_extents.z};
        corners[static_cast<std::size_t>(i)] = center + rotation * local;
    }
    return corners;
}

inline std::vector<Vec3> clip_polygon(const std::vector<Vec3>& poly, const Vec3& normal, double offset) {
    std::vector<Vec3> out;
    out.reserve(poly.size() + 1);
    for (std::size_t i = 0; i < poly.size(); ++i) {
        const Vec3& p = poly[i];
        const Vec3& q = poly[(i + 1) % poly.size()];
        double dp = dot(p, normal) - offset;
        double dq = dot(q, normal) - offset;
        if (dp <= 0.0) out.push_back(p);
        if (dp * dq < 0.0) {
            double s = dp / (dp - dq);
            out.push_back(p + (q - p) * s);
        }
    }
    return out;
}
}

inline bool collide_sphere_sphere(const Vec3& pa, double ra, const Vec3& pb, double rb, Contact& out) {
    Vec3 d = pb - pa;
    double dist = d.norm();
    double rsum = ra + rb;
    if (dist >= rsum || dist < 1e-12) return false;
    out.normal = d * (1.0 / dist);
    out.depth = rsum - dist;
    out.point = pa + out.normal * (ra - out.depth * 0.5);
    return true;
}

inline bool collide_sphere_plane(const Vec3& center, double radius, const Plane& plane, Contact& out) {
    double dist = plane.signed_distance(center);
    if (dist > radius) return false;
    out.normal = plane.normal * -1.0;
    out.depth = radius - dist;
    out.point = center - plane.normal * dist;
    return true;
}

inline bool collide_box_plane(const Vec3& center, const Vec3& half_extents, const Mat3& rotation, const Plane& plane, std::vector<Contact>& out) {
    auto corners = detail::obb_corners(center, half_extents, rotation);
    Vec3 centroid{0.0, 0.0, 0.0};
    double max_depth = 0.0;
    int n = 0;
    for (const auto& c : corners) {
        double d = plane.signed_distance(c);
        if (d < 0.0) {
            centroid += c;
            ++n;
            max_depth = std::max(max_depth, -d);
        }
    }
    if (n == 0) return false;
    Contact ct;
    ct.normal = plane.normal * -1.0;
    ct.depth = max_depth;
    ct.point = centroid * (1.0 / static_cast<double>(n));
    out.push_back(ct);
    return true;
}

inline bool collide_sphere_box(const Vec3& center, double radius, const Vec3& box_center, const Vec3& half_extents, const Mat3& rotation, Contact& out) {
    Vec3 closest = detail::closest_point_on_obb(center, box_center, half_extents, rotation);
    Vec3 d = center - closest;
    double dist = d.norm();
    if (dist > radius) return false;
    if (dist < 1e-12) {
        Vec3 inside = rotation.transposed() * (center - box_center);
        double dx = half_extents.x - std::fabs(inside.x);
        double dy = half_extents.y - std::fabs(inside.y);
        double dz = half_extents.z - std::fabs(inside.z);
        Vec3 local_n{0.0, 0.0, 0.0};
        double depth = dx;
        local_n.x = inside.x >= 0.0 ? 1.0 : -1.0;
        if (dy < depth) {
            depth = dy;
            local_n = Vec3{0.0, inside.y >= 0.0 ? 1.0 : -1.0, 0.0};
        }
        if (dz < depth) {
            depth = dz;
            local_n = Vec3{0.0, 0.0, inside.z >= 0.0 ? 1.0 : -1.0};
        }
        out.normal = rotation * local_n;
        out.depth = depth + radius;
        out.point = center;
        return true;
    }
    out.normal = d * (1.0 / dist);
    out.depth = radius - dist;
    out.point = closest;
    return true;
}

inline bool collide_obb_obb(const Vec3& ca, const Vec3& ha, const Mat3& ra, const Vec3& cb, const Vec3& hb, const Mat3& rb, std::vector<Contact>& out) {
    std::array<Vec3, 3> axes_a{};
    std::array<Vec3, 3> axes_b{};
    for (int i = 0; i < 3; ++i) {
        axes_a[static_cast<std::size_t>(i)] = Vec3{ra(0, i), ra(1, i), ra(2, i)};
        axes_b[static_cast<std::size_t>(i)] = Vec3{rb(0, i), rb(1, i), rb(2, i)};
    }
    std::array<Vec3, 15> axes{};
    axes[0] = axes_a[0];
    axes[1] = axes_a[1];
    axes[2] = axes_a[2];
    axes[3] = axes_b[0];
    axes[4] = axes_b[1];
    axes[5] = axes_b[2];
    std::size_t idx = 6;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            Vec3 c = cross(axes_a[static_cast<std::size_t>(i)], axes_b[static_cast<std::size_t>(j)]);
            double n = c.norm();
            axes[idx++] = n > 1e-9 ? c * (1.0 / n) : Vec3{0.0, 0.0, 1.0};
        }
    }

    Vec3 d = cb - ca;
    double min_overlap = std::numeric_limits<double>::max();
    Vec3 n{0.0, 1.0, 0.0};
    for (const auto& axis : axes) {
        double proj_a = std::fabs(dot(axes_a[0], axis)) * ha.x +
                        std::fabs(dot(axes_a[1], axis)) * ha.y +
                        std::fabs(dot(axes_a[2], axis)) * ha.z;
        double proj_b = std::fabs(dot(axes_b[0], axis)) * hb.x +
                        std::fabs(dot(axes_b[1], axis)) * hb.y +
                        std::fabs(dot(axes_b[2], axis)) * hb.z;
        double dist = std::fabs(dot(d, axis));
        double overlap = proj_a + proj_b - dist;
        if (overlap < 0.0) return false;
        if (overlap < min_overlap) {
            min_overlap = overlap;
            n = axis;
        }
    }
    if (dot(d, n) < 0.0) n = n * -1.0;
    bool ref_is_a = true;
    double best_a = 0.0;
    double best_b = 0.0;
    for (int i = 0; i < 3; ++i) {
        best_a = std::max(best_a, std::fabs(dot(n, axes_a[static_cast<std::size_t>(i)])));
        best_b = std::max(best_b, std::fabs(dot(n, axes_b[static_cast<std::size_t>(i)])));
    }
    if (best_b > best_a) ref_is_a = false;
    const Vec3& cr = ref_is_a ? ca : cb;
    const Vec3& hr = ref_is_a ? ha : hb;
    const auto& rot_r = ref_is_a ? ra : rb;
    const Vec3& ci = ref_is_a ? cb : ca;
    const Vec3& hi = ref_is_a ? hb : ha;
    const auto& rot_i = ref_is_a ? rb : ra;
    std::array<Vec3, 3> axes_r{};
    std::array<Vec3, 3> axes_i{};
    for (int i = 0; i < 3; ++i) {
        axes_r[static_cast<std::size_t>(i)] = Vec3{rot_r(0, i), rot_r(1, i), rot_r(2, i)};
        axes_i[static_cast<std::size_t>(i)] = Vec3{rot_i(0, i), rot_i(1, i), rot_i(2, i)};
    }

    int ref_i = 0;
    double best = std::fabs(dot(n, axes_r[0]));
    for (int i = 1; i < 3; ++i) {
        double v = std::fabs(dot(n, axes_r[static_cast<std::size_t>(i)]));
        if (v > best) {
            best = v;
            ref_i = i;
        }
    }
    Vec3 rn = axes_r[static_cast<std::size_t>(ref_i)];
    if (dot(rn, ci - cr) < 0.0) rn = rn * -1.0;
    int inc_i = 0;
    best = std::fabs(dot(-rn, axes_i[0]));
    for (int i = 1; i < 3; ++i) {
        double v = std::fabs(dot(-rn, axes_i[static_cast<std::size_t>(i)]));
        if (v > best) {
            best = v;
            inc_i = i;
        }
    }
    Vec3 in = axes_i[static_cast<std::size_t>(inc_i)];
    if (dot(in, ci - cr) > 0.0) in = in * -1.0;
    std::vector<Vec3> poly;
    poly.reserve(4);
    for (int s1 : {-1, 1}) {
        for (int s2 : {-1, 1}) {
            int t1 = 0;
            int t2 = 0;
            if (inc_i == 0) { t1 = 1; t2 = 2; }
            else if (inc_i == 1) { t1 = 0; t2 = 2; }
            else { t1 = 0; t2 = 1; }
            Vec3 p = ci + in * hi[static_cast<std::size_t>(inc_i)] +
                     axes_i[static_cast<std::size_t>(t1)] * hi[static_cast<std::size_t>(t1)] * static_cast<double>(s1) +
                     axes_i[static_cast<std::size_t>(t2)] * hi[static_cast<std::size_t>(t2)] * static_cast<double>(s2);
            poly.push_back(p);
        }
    }

    for (int i = 0; i < 3; ++i) {
        if (i == ref_i) continue;
        Vec3 u = axes_r[static_cast<std::size_t>(i)];
        double h = hr[static_cast<std::size_t>(i)];
        poly = detail::clip_polygon(poly, u, dot(cr, u) + h);
        if (poly.size() < 3) break;
        poly = detail::clip_polygon(poly, u * -1.0, -dot(cr, u) + h);
        if (poly.size() < 3) break;
    }

    double offset_ref = dot(cr, rn) + hr[static_cast<std::size_t>(ref_i)];
    for (const Vec3& p : poly) {
        double depth = offset_ref - dot(p, rn);
        if (depth < 0.0) continue;
        Contact ct;
        ct.normal = n;
        ct.depth = depth;
        ct.point = p;
        out.push_back(ct);
        if (out.size() >= 4) break;
    }

    if (out.empty()) {
        Contact ct;
        ct.normal = n;
        ct.depth = min_overlap;
        ct.point = detail::closest_point_on_obb(cb, ca, ha, ra);
        out.push_back(ct);
    }
    return true;
}

inline bool collide_aabb_aabb(const Vec3& min_a, const Vec3& max_a, const Vec3& min_b, const Vec3& max_b, Contact& out) {
    if (max_a.x < min_b.x || min_a.x > max_b.x || max_a.y < min_b.y || min_a.y > max_b.y || max_a.z < min_b.z || min_a.z > max_b.z)return false;
    double overlaps[3] = {std::min(max_a.x, max_b.x) - std::max(min_a.x, min_b.x), std::min(max_a.y, max_b.y) - std::max(min_a.y, min_b.y), std::min(max_a.z, max_b.z) - std::max(min_a.z, min_b.z)};
    int axis = 0;
    if (overlaps[1] < overlaps[0]) axis = 1;
    if (overlaps[2] < overlaps[axis]) axis = 2;
    Vec3 n{0.0, 0.0, 0.0};
    Vec3 ca = (min_a + max_a) * 0.5;
    Vec3 cb = (min_b + max_b) * 0.5;
    n[axis] = cb[axis] >= ca[axis] ? 1.0 : -1.0;
    out.normal = n;
    out.depth = overlaps[axis];
    out.point = (ca + cb) * 0.5;
    return true;
}

inline Vec3 support_point(const Vec3& center, const Vec3& half_extents, const Mat3& rotation, const Vec3& direction) {
    Vec3 local = rotation.transposed() * direction;
    Vec3 sign{local.x >= 0.0 ? 1.0 : -1.0, local.y >= 0.0 ? 1.0 : -1.0, local.z >= 0.0 ? 1.0 : -1.0};
    return center + rotation * Vec3{half_extents.x * sign.x, half_extents.y * sign.y, half_extents.z * sign.z};
}
}
}
