export module jute:to;
import :heap;
import :twine;
import :view;

namespace jute {
  template<typename T>
  struct pair {
    T result;
    bool consumed_all;
  };
  export constexpr pair<unsigned> to_u32(view v) {
    unsigned res = 0;
    for (auto c : v) {
      if (c < '0' || c > '9') return { res, false };
      res = res * 10 + (c - '0');
    }
    return { res, true };
  }
  static_assert(to_u32("0").result == 0);
  static_assert(to_u32("129302").result == 129302);

  export constexpr pair<signed> to_i32(view v) {
    int mult = (v.size() && v[0] != '-') ? 1 : -1;
    auto vv = (v.size() && v[0] != '-') ? v : v.subview(1).after;
    auto [res, all] = to_u32(vv);
    return { mult * static_cast<int>(res), all };
  }
  static_assert(to_i32("0").result == 0);
  static_assert(to_i32("830").result == 830);
  static_assert(to_i32("-123").result == -123);

  // TODO: accept partials
  export constexpr pair<float> to_f(view v) {
    const auto take = [&] { v = v.subview(1).after; };

    bool negative = v[0] == '-';
    if (negative) take();
    
    // TODO: what to do with zeroes at left, like 0100? Octal or error?
    float mantissa = 0;
    while (v.size()) {
      // TODO: should we accept scientific notation without decimals, like "1e1"?
      if (v[0] == '.') break;
      if (v[0] < '0' || v[0] > '9') return { 0, false };
      mantissa = mantissa * 10 + (v[0] - '0');
      take();
    }

    int decimals = 0;
    if (v.size() && v[0] == '.') {
      take();

      while (v.size()) {
        if ((v[0] | 0x20) == 'e') break;
        if (v[0] < '0' || v[0] > '9') return { 0, false };
        mantissa = mantissa * 10 + (v[0] - '0');
        decimals++;
        take();
      }
    }

    if (v.size() && (v[0] | 0x20) == 'e') {
      take();

      auto eneg = v[0] == '-';
      if (eneg || v[0] == '+') take();

      int e = 0;
      while (v.size()) {
        if (v[0] < '0' || v[0] > '9') return { 0, false };
        e = e * 10 + (v[0] - '0');
        take();
      }
      decimals -= eneg ? -e : e;
    }

    float div = negative ? -1 : 1;
    for (auto i = 0; i < decimals; i++) div *= 10;
    for (auto i = decimals; i < 0; i++) div /= 10;
    return { mantissa / div, true };
  }

  static_assert(to_f("0").result == 0.0f);
  static_assert(to_f("129302").result == 129302.0f);

  static_assert(to_f("0.0").result == 0.0f);
  static_assert(to_f("129.302").result == 129.302f);
  static_assert(to_f("-129.302").result == -129.302f);

  static_assert(to_f("-10.0e0").result == -10.0f);
  static_assert(to_f("-0.1e2").result == -10.0f);
  static_assert(to_f(".1e+2").result == 10.0f);
  static_assert(to_f("100.0e-1").result == 10.0f);

  export constexpr heap to_s(unsigned long long n) {
    if (n == 0) return "0";

    static constexpr const auto sz = 32;
    char buf[sz] {};
    auto p = buf + sz - 1;
    while (n) {
      *p-- = (n % 10) + '0';
      n /= 10;
    }
    auto l = static_cast<unsigned>(buf + sz - 1 - p);
    return heap { view { p + 1, l } };
  }
  static_assert(to_s(0ULL) == "0");
  static_assert(to_s(1ULL) == "1");
  static_assert(to_s(123ULL) == "123");

  export constexpr heap to_s(long long val) {
    if (val == 0) return "0";

    auto negative = val < 0;
    unsigned long long n = negative ? -val : val;

    static constexpr const auto sz = 32;
    char buf[sz] {};
    auto p = buf + sz - 1;
    while (n) {
      *p-- = (n % 10) + '0';
      n /= 10;
    }
    if (negative) *p-- = '-';
    auto l = static_cast<unsigned>(buf + sz - 1 - p);
    return heap { view { p + 1, l } };
  }
  static_assert(to_s(0LL) == "0");
  static_assert(to_s(1LL) == "1");
  static_assert(to_s(123LL) == "123");
  static_assert(to_s(-98LL) == "-98");

  export constexpr heap to_s(double val) {
    auto is = to_s(static_cast<long long>(val));

    auto tmp = to_s(static_cast<long long>(val * 1000.0 + 1000.0));
    auto fs = (*tmp).subview(tmp.size() - 3).after;

    return (is + "." + fs).heap();
  }
  static_assert(to_s(0.0) == "0.000");
  static_assert(to_s(1.0) == "1.000");
  static_assert(to_s(123.0) == "123.000");
  static_assert(to_s(-98.0) == "-98.000");
  static_assert(to_s(12.3) == "12.300"); // 12.300001
  static_assert(to_s(2.3) == "2.300"); // 2.299999
  static_assert(to_s(0.001) == "0.001");

  export constexpr inline view to_s(sv val) { return val; }

  export constexpr inline heap to_s(int val) { return to_s(static_cast<long long>(val)); }
  export constexpr inline heap to_s(unsigned val) { return to_s(static_cast<unsigned long long>(val)); }
}
