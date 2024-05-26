export module jute:twine;
import :view;
import hai;
import traits;

export namespace jute {
template <unsigned N> class twine {
  view m_v[N];

  template <unsigned> friend class twine;

public:
  constexpr twine() noexcept = default;
  constexpr twine(view a) : m_v{a} {}
  constexpr twine(view a, view b) noexcept : m_v{a, b} {}

  [[nodiscard]] constexpr unsigned size() const noexcept {
    unsigned s{};
    for (auto v : m_v)
      s += v.size();
    return s;
  }

  [[nodiscard]] constexpr hai::cstr cstr() const noexcept {
    hai::cstr res{size()};
    auto ptr = res.begin();
    for (auto v : m_v) {
      for (auto c : v) {
        *ptr++ = c;
      }
    }
    return res;
  }

  [[nodiscard]] constexpr char operator[](unsigned idx) const noexcept {
    for (auto v : m_v) {
      if (idx < v.size())
        return v[idx];
      idx -= v.size();
    }
    return 0;
  }

  [[nodiscard]] constexpr twine<N + 1> operator+(const view &o) const noexcept {
    twine<N + 1> res{};
    for (auto i = 0; i < N; i++) {
      res.m_v[i] = m_v[i];
    }
    res.m_v[N] = o;
    return res;
  }
  template <unsigned M>
  [[nodiscard]] constexpr twine<M + N>
  operator+(const twine<M> &o) const noexcept {
    twine<N + M> res{};
    for (auto i = 0; i < N; i++) {
      res.m_v[i] = m_v[i];
    }
    for (auto i = 0; i < M; i++) {
      res.m_v[i + N] = o.m_v[i];
    }
    return res;
  }
};
twine()->twine<0>;
twine(view)->twine<1>;
twine(view, view)->twine<2>;

[[nodiscard]] constexpr twine<2> operator+(const view &a,
                                           const view &b) noexcept {
  return twine<2>{a, b};
}
} // namespace jute

namespace {
using namespace jute;
using namespace jute::literals;

template <unsigned A, unsigned B>
[[nodiscard]] constexpr bool operator==(const twine<A> &a,
                                        const twine<B> &b) noexcept {
  auto ca = a.cstr();
  auto cb = b.cstr();
  return view{ca} == view{cb};
}

static_assert(twine{} == twine{});
static_assert(twine{"a"_s} != twine{});
static_assert(twine{} != twine{"a"_s});
static_assert(twine{"a"_s} == twine{"a"_s});
static_assert(twine{"ab"_s} != twine{"aa"_s});

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

static_assert([] {
  // tests if we can use twine and its size in constexpr
  constexpr twine t{"jute"_s};
  [[maybe_unused]] constexpr char c[t.size()]{};
  return true;
}());

static_assert(view{("jute"_s + " "_s + "twine"_s).cstr()} == "jute twine"_s);
} // namespace
