export module jute:heap;
import :view;

export namespace jute {
struct no_copy {};

/// Manages a string that maybe allocated on the heap. It will deallocate heap
/// strings in its dtor, but it won't try to deallocate if the string is not
/// from heap.
class heap {
  view m_view{};
  unsigned *m_refcnt{};

  constexpr heap(view v, unsigned *r) : m_view{v}, m_refcnt{r} {}

  constexpr void inc_ref() {
    if (m_refcnt != nullptr) (*m_refcnt)++;
  }
  constexpr void dec_ref() {
    if (m_refcnt != nullptr && (--(*m_refcnt) == 0)) {
      delete[] m_view.data();
      delete m_refcnt;
    }
  }
  constexpr void reset() {
    m_view = jute::view{"dev error: usage of string after ref is moved"};
    m_refcnt = nullptr;
  }

public:
  constexpr heap() = default;
  constexpr ~heap() { dec_ref(); }

  explicit constexpr heap(view v) : m_refcnt{new unsigned{1}} {
    auto *data = new char[v.size()]; // NOLINT
    m_view = view{data, v.size()};
    for (auto c : v) *data++ = c;
  }
  constexpr heap(no_copy, view v) : m_view{v} {}

  // TODO: any way to safely use "no_copy"?
  template <unsigned N> constexpr heap(const char (&c)[N]) : heap(view { c }) {}

  constexpr heap(const heap &o) : m_view{o.m_view}, m_refcnt{o.m_refcnt} {
    inc_ref();
  }
  constexpr heap(heap &&o) : m_view{o.m_view}, m_refcnt{o.m_refcnt} {
    o.reset();
  }
  constexpr heap &operator=(const heap &o) {
    if (m_refcnt == o.m_refcnt && m_refcnt) return *this;

    dec_ref();
    m_view = o.m_view;
    m_refcnt = o.m_refcnt;
    inc_ref();
    return *this;
  }
  constexpr heap &operator=(heap &&o) {
    if (m_refcnt == o.m_refcnt && m_refcnt) return *this;

    dec_ref();
    m_view = o.m_view;
    m_refcnt = o.m_refcnt;
    o.reset();
    return *this;
  }

  [[nodiscard]] constexpr view operator*() const { return m_view; }
  [[nodiscard]] constexpr auto begin() const { return m_view.begin(); }
  [[nodiscard]] constexpr auto end() const { return m_view.end(); }
  [[nodiscard]] constexpr auto data() const { return m_view.data(); }
  [[nodiscard]] constexpr auto size() const { return m_view.size(); }

  [[nodiscard]] constexpr bool operator==(const heap &o) const {
    return **this == *o;
  }
};
} // namespace jute

export namespace jute::literals {
[[nodiscard]] inline constexpr heap operator""_hs(const char *c, decltype(sizeof(0)) len) {
  return heap{no_copy{}, view{c, len}};
}
} // namespace jute::literals

namespace {
using namespace jute::literals;
static_assert((*""_hs).size() == 0);
static_assert(*"a"_hs == "a");
static_assert("test"_hs == "test"_hs);
static_assert(*(traits::move("a"_hs)) == "a");
static_assert(jute::heap{"asd"_s} == "asd"_hs);
static_assert(jute::heap{"alright"} == "alright"_hs);
} // namespace
