#include <iostream>
#include <random>
#include <stdexcept>
#include <algorithm>
#include <array>
#include <fstream>

#include <nsew/rectangle_optimiser.hpp>

std::ostream& operator<<(std::ostream& os, nsew::rectangle rect)
{
    return os
        << "{" << rect.left << "," << rect.top << ","
        << rect.right << "," << rect.bottom << "}";
}


void start_svg(std::ostream& os)
{
    os << R"(
    <svg version="1.1"
     width="5000" height="4000"
     xmlns="http://www.w3.org/2000/svg">
    )";
}

void end_svg(std::ostream& os)
{
    os << "</svg>";
}

void write_line(std::ostream& os, int x, int y, int x2, int y2)
{
    os
        << "<line x1=\"" << x
        << "\" y1=\"" << y
        <<"\" x2=\"" << x2
        << "\" y2=\"" << y2
        << "\" stroke=\"black\" />\n";
}

std::array<nsew::coord, 4> get_points(nsew::rectangle rect)
{
    return {{
        {rect.left, rect.top},
        {rect.right, rect.top},
        {rect.right, rect.bottom},
        {rect.left, rect.bottom},
    }};
}

void write_rectangle(std::ostream& os, nsew::rectangle rect)
{
    os << R"(<polygon points=")";

    for (auto p : get_points(rect)) {
        os << p.x << ',' << p.y << " ";
    }

    os << R"(" fill="none" stroke="black" />)";
}

void write_rectangles(std::ostream& os, std::vector<nsew::rectangle> const& rects, nsew::coord offset)
{
    for (auto rect : rects) {
        write_rectangle(os, translated_by(rect, offset));
    }
}

std::mt19937 random_gen;

class test_error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

void check(bool res, char const* message = "")
{
    if (!res)
        throw test_error{message};
}

auto constexpr position_limit = 5000;
auto constexpr extent_limit = 5000;

template<class Generator>
nsew::rectangle random_rectangle(Generator& g, int pos=position_limit, int ext=extent_limit)
{
    auto position_dist = std::uniform_int_distribution(-pos, pos);

    // Generate rectangles with at least 1 unit of width and height
    auto extent_dist = std::uniform_int_distribution(1, ext);

    auto x = position_dist(g);
    auto y = position_dist(g);
    auto width = extent_dist(g);
    auto height = extent_dist(g);

    return {
        x,
        y,
        x + width,
        y + height
    };
}

void zero_extent_is_empty()
{
    std::bernoulli_distribution bool_dist;
    for (int i = 0; i != 10000; ++i) {
        auto rectopt = nsew::rectangle_optimiser{};
        auto rect = random_rectangle(random_gen);
        if (bool_dist(random_gen)) {
            rect.right = rect.left;
        } else {
            rect.top = rect.bottom;
        }

        rectopt.submit(rect);
        auto results = rectopt.scan();
        check(results.empty(), "results not empty");
    }
}

void single_rect_is_noop()
{
    for (int i = 0; i != 10000; ++i) {
        auto rectopt = nsew::rectangle_optimiser{};
        auto rect = random_rectangle(random_gen);

        rectopt.submit(rect);
        auto results = rectopt.scan();
        check(results.size() == 1, "results size is not 1");
        check(results[0] == rect, "got a different rectangle back");
    }
}

std::int64_t total_area(std::vector<nsew::rectangle> const& rects)
{
    std::int64_t total{};
    for (auto r : rects)
        total += nsew::area(r);

    return total;
}

void area_is_less_or_equal()
{
    auto const multiplier = 5;
    auto rect_count_dist = std::uniform_int_distribution(50 * multiplier, 200 * multiplier);
    std::vector<nsew::rectangle> rects;
    auto rectopt = nsew::rectangle_optimiser{};
    for (int i = 0; i != 2000; ++i) {
        rectopt.reset();
        rects.resize(rect_count_dist(random_gen));
        auto const area_size = 1072;
        std::generate(std::begin(rects), std::end(rects),
            []() { return random_rectangle(random_gen, area_size, 67); });
        for (auto rect : rects)
            rectopt.submit(rect);

        auto optimised = rectopt.scan();

        #if 0
            std::ofstream svg{"t.svg"};
            start_svg(svg);
            write_rectangles(svg, rectopt.rectangles, {area_size, area_size});
            write_rectangles(svg, optimised, {(int)(3.5 * area_size), area_size});
            end_svg(svg);
            svg.close();
            std::cin.get();
        #endif

        check(total_area(optimised) <= total_area(rects));
    }
}

int main()
{
    zero_extent_is_empty();
    single_rect_is_noop();
    area_is_less_or_equal();

    return 0;
}
