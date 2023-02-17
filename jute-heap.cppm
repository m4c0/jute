export module jute:heap;
import :view;
import traits;

export namespace jute {
/// Manages a string that maybe allocated on the heap. It will deallocate heap
/// strings in its dtor, but it won't try to deallocate if the string is not
/// from heap.
class heap {
  view m_view;
  unsigned *m_refcnt{};

  constexpr heap(view v, unsigned *h) noexcept : m_view{v}, m_refcnt{h} {
    if (h != nullptr)
      (*h)++;
  }

public:
  constexpr heap() noexcept = default;
  constexpr ~heap() noexcept {
    if (m_refcnt != nullptr && (--(*m_refcnt) == 0)) {
      delete[] m_view.data();
      delete m_refcnt;
    }
  }

  constexpr heap(view v) noexcept : heap(v, nullptr) {}

  constexpr heap(const heap &o) noexcept : heap{o.m_view, o.m_refcnt} {}
  constexpr heap(heap &&o) noexcept : m_view{o.m_view}, m_refcnt{o.m_refcnt} {
    o.m_refcnt = nullptr;
  }
  constexpr heap &operator=(const heap &o) noexcept {
    if (this == &o)
      return *this;
    m_view = o.m_view;
    m_refcnt = o.m_refcnt;
    if (m_refcnt != nullptr)
      (*m_refcnt)++;
    return *this;
  }
  constexpr heap &operator=(heap &&o) noexcept {
    if (this == &o)
      return *this;
    m_view = o.m_view;
    m_refcnt = o.m_refcnt;
    o.m_refcnt = nullptr;
    return *this;
  }

  [[nodiscard]] constexpr view operator*() const noexcept { return m_view; }

  [[nodiscard]] constexpr heap operator+(view o) const {
    const auto len = m_view.size() + o.size();
    auto *data = new char[len]; // NOLINT
    auto v = view{data, len};
    for (auto c : m_view) {
      *data++ = c;
    }
    for (auto c : o) {
      *data++ = c;
    }
    return heap{v, new unsigned{}};
  }

  [[nodiscard]] constexpr heap operator+(const heap &o) const {
    return *this + *o;
  }
  [[nodiscard]] constexpr heap operator+(char c) const {
    return *this + view{&c, 1};
  }

  [[nodiscard]] constexpr bool operator==(const heap &o) const noexcept {
    return **this == *o;
  }
};
} // namespace jute

export namespace jute::literals {
[[nodiscard]] inline constexpr heap
operator"" _hs(const char *c, traits::size_t len) noexcept {
  return heap{view{c, len}};
}
} // namespace jute::literals

namespace {
using namespace jute::literals;
static_assert((*""_hs).size() == 0);
static_assert(*"a"_hs == "a");
static_assert("test"_hs == "test"_hs);
static_assert("aaa"_hs + "bb"_hs == "aaabb"_hs);
static_assert(*(traits::move("a"_hs)) == "a");
static_assert(jute::heap{"asd"_s} == "asd"_hs);
static_assert("aaa"_hs + "bb"_s + "c" + 'd' == "aaabbcd"_hs);
} // namespace
