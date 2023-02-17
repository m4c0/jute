export module jute:heap;
import :view;
import traits;

export namespace jute {
/// Manages a string that maybe allocated on the heap. It will deallocate heap
/// strings in its dtor, but it won't try to deallocate if the string is not
/// from heap.
class heap {
  view m_view;
  bool m_heap = false;

  [[nodiscard]] static constexpr view clone(view sv) noexcept {
    char *data = new char[sv.size()]; // NOLINT
    view res{data, sv.size()};
    for (auto c : sv) {
      *data++ = c;
    }
    return res;
  }

  constexpr heap(view v, bool h) noexcept : m_view{v}, m_heap{h} {}

public:
  constexpr heap() noexcept = default;
  constexpr ~heap() noexcept {
    if (m_heap)
      delete[] m_view.data();
  }

  constexpr heap(view v) noexcept : heap(v, false) {}

  constexpr heap(const heap &o) noexcept = delete;
  constexpr heap(heap &&o) noexcept : heap{o.m_view, o.m_heap} {
    o.m_heap = false;
  }
  constexpr heap &operator=(const heap &o) noexcept = delete;
  constexpr heap &operator=(heap &&o) noexcept {
    if (this == &o)
      return *this;
    m_view = o.m_view;
    m_heap = o.m_heap;
    o.m_heap = false;
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
    return heap{v, true};
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

  [[nodiscard]] constexpr heap clone() const noexcept {
    return heap{clone(m_view), true};
  }
};

[[nodiscard]] inline constexpr heap
operator"" _ks(const char *c, traits::size_t len) noexcept {
  return heap{view{c, len}};
}
} // namespace jute

namespace {
using namespace jute;
static_assert((*heap{view{"ok"}}.clone()).size() == 2);
} // namespace
