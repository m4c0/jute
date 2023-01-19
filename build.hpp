#include "../hai/build.hpp"
#include "../traits/build.hpp"
#include "ecow.hpp"

auto jute() {
  using namespace ecow;

  auto m = unit::create<mod>("jute");
  m->add_wsdep("hai", hai());
  m->add_wsdep("traits", traits());
  m->add_part("view");
  m->add_part("twine");

  return m;
}
