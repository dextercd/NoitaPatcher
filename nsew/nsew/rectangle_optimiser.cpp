// Rectangle optimiser functions exposed for LuaJIT-FFI

#include <nsew/rectangle_optimiser.hpp>
#include <nsew_native_export.h>

extern "C" NSEW_NATIVE_EXPORT
nsew::rectangle_optimiser* rectangle_optimiser_new()
{
    return new nsew::rectangle_optimiser{};
}

extern "C" NSEW_NATIVE_EXPORT
void rectangle_optimiser_delete(nsew::rectangle_optimiser* rectangle_optimiser)
{
    delete rectangle_optimiser;
}

extern "C" NSEW_NATIVE_EXPORT
void rectangle_optimiser_reset(nsew::rectangle_optimiser* rectangle_optimiser)
{
    return rectangle_optimiser->reset();
}

extern "C" NSEW_NATIVE_EXPORT
void rectangle_optimiser_submit(nsew::rectangle_optimiser* rectangle_optimiser, nsew::rectangle* rectangle)
{
    return rectangle_optimiser->submit(*rectangle);
}

extern "C" NSEW_NATIVE_EXPORT
void rectangle_optimiser_scan(nsew::rectangle_optimiser* rectangle_optimiser)
{
    rectangle_optimiser->rectangles = rectangle_optimiser->scan();
}

extern "C" NSEW_NATIVE_EXPORT
std::int32_t rectangle_optimiser_size(const nsew::rectangle_optimiser* rectangle_optimiser)
{
    return rectangle_optimiser->rectangles.size();
}

extern "C" NSEW_NATIVE_EXPORT
const nsew::rectangle* rectangle_optimiser_get(const nsew::rectangle_optimiser* rectangle_optimiser, std::int32_t index)
{
    return &rectangle_optimiser->rectangles[index];
}
