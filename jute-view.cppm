export module jute:view;

namespace jute {
export class view {
  const char *m_data{};
  unsigned long m_len{};

public:
  constexpr view() noexcept = default;
  constexpr view(const char *v, unsigned long s) noexcept
      : m_data{v}, m_len{s} {}

  [[nodiscard]] constexpr auto data() const noexcept { return m_data; }
  [[nodiscard]] constexpr auto size() const noexcept { return m_len; }

  [[nodiscard]] constexpr char operator[](unsigned idx) const noexcept {
    if (idx >= m_len)
      return 0;
    return m_data[idx];
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

namespace jute::operators {
[[nodiscard]] constexpr view operator""_s(const char *v,
                                          unsigned long size) noexcept {
  return view{v, size};
}

static_assert("abcde"_s[0] == 'a');
static_assert("abcde"_s[3] == 'd');
static_assert("abcde"_s[4] == 'e');

static_assert("a"_s == "a"_s);
static_assert("abacabb"_s == "abacabb"_s);

static_assert("aa"_s != "aaaa"_s);
static_assert("aaaa"_s != "aa"_s);
static_assert("a"_s != "b"_s);
static_assert("aaaaaaaa"_s != "aaaaaaab"_s);
} // namespace jute::operators