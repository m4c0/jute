export module jute:heap;
import :view;
import traits;

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

  constexpr heap(view v) : m_refcnt{new unsigned{1}} {
    auto *data = new char[v.size()]; // NOLINT
    m_view = view{data, v.size()};
    for (auto c : v) *data++ = c;
  }
  constexpr heap(no_copy, view v) : m_view{v} {}

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
  [[nodiscard]] constexpr auto size() const { return m_view.size(); }

  [[nodiscard]] constexpr heap operator+(view o) const {
    const auto len = m_view.size() + o.size();
    auto *data = new char[len]; // NOLINT
    auto v = view{data, len};
    for (auto c : m_view) *data++ = c;
    for (auto c : o) *data++ = c;
    return heap{v, new unsigned{1}};
  }

  [[nodiscard]] constexpr heap operator+(const heap &o) const {
    return *this + *o;
  }
  [[nodiscard]] constexpr heap operator+(char c) const {
    return *this + view{&c, 1};
  }

  [[nodiscard]] constexpr bool operator==(const heap &o) const {
    return **this == *o;
  }
};
} // namespace jute

export namespace jute::literals {
[[nodiscard]] inline constexpr heap operator"" _hs(const char *c, traits::size_t len) {
  return heap{no_copy{}, view{c, len}};
}
} // namespace jute::literals

namespace {
using namespace jute::literals;
static_assert((*""_hs).size() == 0);
static_assert(*"a"_hs == "a");
static_assert("test"_hs == "test"_hs);
static_assert(jute::heap{} + "bb"_hs == "bb"_hs);
static_assert(jute::heap{} + "bb"_s == "bb"_hs);
static_assert("aaa"_hs + "bb"_hs == "aaabb"_hs);
static_assert(*(traits::move("a"_hs)) == "a");
static_assert(jute::heap{"asd"_s} == "asd"_hs);
static_assert("aaa"_hs + "bb"_s + "c" + 'd' == "aaabbcd"_hs);

static_assert([] {
  // Checks if we can copy heap-allocated over heap-allocated
  jute::heap a = "1"_hs + "2"_s;
  a = a + "3"_s;
  return a == "123"_hs;
}());
} // namespace
