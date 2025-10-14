#pragma once

#include <flecs.h>

#include <vector>

namespace reflection {
    // Reusable reflection support for std::vector
    template <typename Elem, typename Vector = std::vector<Elem>> 
    flecs::opaque<Vector, Elem> std_vector_support(flecs::world& world);
}

