export module jute:view;
import hai;
import traits;

export namespace jute {
using traits::size_t;

/// Holds a pointer to a string (aka char array) and its size. This is intended
/// to be a non-owning pointer holder. As consequences, it should not outlive
/// the original pointer.
class view {
  const char *m_data{};
  size_t m_len{};

public:
  constexpr view() noexcept = default;
  constexpr view(const char *v, size_t s) noexcept : m_data{v}, m_len{s} {}
  constexpr view(const hai::cstr &str) noexcept
      : m_data{str.data()}, m_len{str.size()} {}

  template <unsigned N> constexpr view(const char (&c)[N]) : view(c, N - 1) {}

  [[nodiscard]] constexpr auto data() const noexcept { return m_data; }
  [[nodiscard]] constexpr auto size() const noexcept { return m_len; }

  [[nodiscard]] constexpr auto begin() const noexcept { return m_data; }
  [[nodiscard]] constexpr auto end() const noexcept { return m_data + m_len; }

  [[nodiscard]] constexpr auto cstr() const noexcept {
    hai::cstr res{size()};
    auto ptr = res.begin();
    for (auto c : *this) {
      *ptr++ = c;
    }
    return res;
  }

  [[nodiscard]] constexpr auto subview(unsigned idx) const noexcept {
    struct pair {
      view before;
      view after;
    };
    if (idx >= m_len)
      return pair{*this, {}};
    return pair{
        .before = {m_data, idx},
        .after = {m_data + idx, m_len - idx},
    };
  }
  [[nodiscard]] constexpr auto split(char c) const noexcept {
    struct pair {
      view before;
      view after;
    };
    for (auto i = 0U; i < m_len; i++) {
      if (m_data[i] != c)
        continue;

      return pair{.before = {m_data, i},
                  .after = {m_data + i + 1, m_len - i - 1}};
    }
    return pair{*this, {}};
  }
  [[nodiscard]] constexpr auto rsplit(char c) const noexcept {
    struct pair {
      view before;
      view after;
    };
    for (auto i = m_len; i > 0; i--) {
      auto j = i - 1;
      if (m_data[j] != c)
        continue;

      return pair{.before = {m_data, j},
                  .after = {m_data + j + 1, m_len - j - 1}};
    }
    return pair{{}, *this};
  }

  [[nodiscard]] constexpr char operator[](unsigned idx) const noexcept {
    if (idx >= m_len)
      return 0;
    return m_data[idx];
  }

  [[nodiscard]] static constexpr view unsafe(const char *str) {
    auto i = 0U;
    while (str[i])
      i++;
    return view{str, i};
  }
};

[[nodiscard]] constexpr bool operator==(const view &a, const view &b) noexcept {
  if (a.size() != b.size())
    return false;

  for (auto i = 0; i < a.size(); i++)
    if (a[i] != b[i])
      return false;

  return true;
}
} // namespace jute

export namespace jute::literals {
[[nodiscard]] constexpr view operator""_s(const char *v, size_t size) noexcept {
  return view{v, size};
}
} // namespace jute::literals

namespace {
using namespace jute::literals;

static_assert("abcde"_s[0] == 'a');
static_assert("abcde"_s[3] == 'd');
static_assert("abcde"_s[4] == 'e');

static_assert("a"_s == "a"_s);
static_assert("abacabb"_s == "abacabb"_s);

static_assert("check"_s == jute::view{"check"});
static_assert("abacabb"_s == jute::view::unsafe("abacabb"));

static_assert("aa"_s != "aaaa"_s);
static_assert("aaaa"_s != "aa"_s);
static_assert("a"_s != "b"_s);
static_assert("aaaaaaaa"_s != "aaaaaaab"_s);

static_assert([] {
  const auto &[a, b] = "jute"_s.subview(2);
  return a == "ju"_s && b == "te"_s;
});

static_assert([] {
  const auto &[a, b] = "love"_s.split('/');
  return a == "love"_s && b == ""_s;
}());
static_assert([] {
  const auto &[a, b] = "jute twine etc"_s.split(' ');
  return a == "jute"_s && b == "twine etc"_s;
}());

static_assert([] {
  const auto &[a, b] = "love"_s.rsplit('/');
  return a == ""_s && b == "love"_s;
}());
static_assert([] {
  const auto &[a, b] = "jute twine etc"_s.rsplit(' ');
  return a == "jute twine"_s && b == "etc"_s;
}());
} // namespace

static_assert([] {
  hai::cstr c{20};
  jute::view v{c};
  return v.data() == c.data() && c.size() == v.size();
}());
