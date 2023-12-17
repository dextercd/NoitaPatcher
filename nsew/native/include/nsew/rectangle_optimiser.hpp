#ifndef NSEW_RECTANGLE_OPTIMISER_HPP
#define NSEW_RECTANGLE_OPTIMISER_HPP

#include <cstdint>
#include <vector>

#include <absl/container/btree_set.h>

namespace nsew {

// Right must be greater than left and bottom must be greater that top.
//
// -      x +
//   .-------->
//   |
// y |
// + |
//   v

struct coord {
    std::int32_t x;
    std::int32_t y;
};

constexpr coord translated_by(coord p, coord o) {
    return {
        p.x + o.x, p.y + o.y
    };
}

struct rectangle {
    std::int32_t left;
    std::int32_t top;
    std::int32_t right;
    std::int32_t bottom;

    bool operator==(rectangle const&) const = default;
};

constexpr rectangle translated_by(rectangle r, coord o) {
    return {
        r.left + o.x,
        r.top + o.y,
        r.right + o.x,
        r.bottom + o.y,
    };
}

constexpr bool has_area(rectangle r) {
    return (
        r.left < r.right &&
        r.top < r.bottom
    );
}

constexpr std::int32_t area(rectangle r) {
    return (r.right - r.left) * (r.bottom - r.top);
}

enum class edge_side : char {
    left, right
};

struct edge {
    edge_side side;
    std::int32_t position;
    std::int32_t start;
    std::int32_t stop;

    bool operator==(edge const&) const = default;

    template <typename H>
    friend H AbslHashValue(H h, edge e) {
        return H::combine(std::move(h), e.side, e.position, e.start, e.stop);
    }
};

struct segment {
    std::int32_t position;
    std::int32_t start;
    std::int32_t stop;

    bool operator==(segment const&) const = default;

    template <typename H>
    friend H AbslHashValue(H h, segment s) {
        return H::combine(std::move(h), s.position, s.start, s.stop);
    }
};

struct range {
    std::int32_t start;
    std::int32_t stop;

    bool operator==(range const&) const = default;
};

struct segment_stop_cmp {
    constexpr bool operator()(segment const& a, segment const& b) const {
        return a.stop < b.stop;
    }
};

struct sweep_alg {
    absl::btree_set<segment, segment_stop_cmp> segments;
    std::vector<rectangle> output;

    void reset()
    {
        segments.clear();
        output.clear();
    }

    void next(const std::vector<range>& ranges, int position);
};

struct rectangle_optimiser {
    std::vector<rectangle> rectangles;
    std::vector<edge> edges;
    std::vector<range> all_ranges;
    std::vector<range> active_ranges;
    sweep_alg alg;

    void reset()
    {
        alg.reset();
        rectangles.clear();
        edges.clear();
        active_ranges.clear();
    }

    void submit(rectangle rect) {
        if (!has_area(rect))
            return;

        rectangles.push_back(rect);
    }

    std::vector<rectangle> scan();
};

} // namespace nsew

#endif // header guard
