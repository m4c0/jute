export module jute:twine;
import :view;
import hai;
import traits;

export namespace jute {
class twine {
  using next_t = hai::sptr<twine>;

  view m_v{};
  next_t m_next{};

  constexpr twine(const view &v, next_t n) noexcept : m_v{v}, m_next{n} {}

public:
  constexpr twine() noexcept = default;
  constexpr twine(const view &v) noexcept : m_v{v} {}

  [[nodiscard]] constexpr const view &self() const noexcept { return m_v; }
  [[nodiscard]] constexpr const auto &next() const noexcept { return m_next; }

  [[nodiscard]] constexpr twine substr(unsigned idx) const noexcept {
    if (idx < m_v.size())
      return {m_v.subview(idx), m_next};
    if (!m_next)
      return {};
    return m_next->substr(idx - m_v.size());
  }

  [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }
  [[nodiscard]] constexpr unsigned size() const noexcept {
    unsigned s = m_v.size();
    return m_next ? (s + m_next->size()) : s;
  }

  [[nodiscard]] constexpr char operator[](unsigned idx) const noexcept {
    if (idx < m_v.size())
      return m_v[idx];
    if (!m_next)
      return 0;
    return (*m_next)[idx - m_v.size()];
  }

  [[nodiscard]] constexpr twine operator+(const view &o) noexcept {
    twine n = m_next ? (*m_next + o) : o;
    return twine{m_v, next_t::make(n)};
  }
  [[nodiscard]] constexpr twine operator+(const twine &o) noexcept {
    twine n = m_next ? (*m_next + o) : o;
    return twine{m_v, next_t::make(n)};
  }
};

[[nodiscard]] constexpr twine operator+(const view &a, const view &b) noexcept {
  return twine{a} + b;
}
[[nodiscard]] constexpr bool operator==(const twine &a,
                                        const twine &b) noexcept {
  const auto as = a.size();
  const auto bs = b.size();
  if (as != bs)
    return false;

  for (auto i = 0; i < as; i++) {
    if (a[i] != b[i])
      return false;
  }

  return true;
}
} // namespace jute

namespace {
using namespace jute;
using namespace jute::operators;
static_assert(twine{} == twine{});
static_assert(twine{"a"_s} != twine{});
static_assert(twine{} != twine{"a"_s});
static_assert(twine{"a"_s} == twine{"a"_s});
static_assert(twine{"ab"_s} != twine{"aa"_s});

static_assert(twine{"twine"_s}.substr(3) == twine{"ne"_s});
static_assert(("ju"_s + "te"_s).substr(3) == "e"_s);

static_assert(twine{""_s}.size() == 0);
static_assert(twine{"twine"_s}.size() == 5);
static_assert(("aa"_s + "b"_s).size() == 3);
static_assert(("aa"_s + ""_s).size() == 2);
static_assert((""_s + ""_s + "a"_s).size() == 1);
static_assert(("ju"_s + "t"_s + "e"_s).size() == 4);

static_assert("ju"_s + "te"_s == "ju"_s + "te"_s);
static_assert("ju"_s + "te"_s == "jut"_s + "e"_s);
static_assert("j"_s + "ute"_s == "jute"_s + ""_s);
static_assert("j"_s + "u"_s + "te"_s == twine{"jute"_s});
static_assert("j"_s + "u"_s + "t"_s + "e"_s == twine{"jute"_s});
static_assert(("j"_s + "u"_s) + ("t"_s + "e"_s) == twine{"jute"_s});
static_assert(twine{"j"_s} + twine{"u"_s} + twine{"t"_s} + twine{"e"_s} ==
              twine{"jute"_s});
static_assert("ju"_s + "te"_s != "aut"_s + "e"_s);
static_assert("ju"_s + "te"_s != "jut"_s + "a"_s);
} // namespace
