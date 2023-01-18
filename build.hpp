#include "ecow.hpp"

auto jute() {
  using namespace ecow;

  auto m = unit::create<mod>("jute");
  m->add_part("view");

  return m;
}
