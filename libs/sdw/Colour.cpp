#include "Colour.h"
#include <cmath>
#include <cstdint>
#include <utility>

Colour::Colour() = default;
Colour::Colour(int r, int g, int b) : red(r), green(g), blue(b) {}
Colour::Colour(std::string n, int r, int g, int b) : name(std::move(n)), red(r), green(g), blue(b) {}

std::ostream &operator<<(std::ostream &os, const Colour &colour) {
    os << colour.name << " [" << colour.red << ", " << colour.green << ", " << colour.blue << "]";
    return os;
}

uint32_t Colour::value(float brightness) {
    return int(red * brightness) << 16 | int(green * brightness) << 8 | int(blue * brightness);
}
