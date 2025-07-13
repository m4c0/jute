export module jute:to;
import :view;

namespace jute {
  export struct invalid_number {};
  export constexpr auto to_u32(jute::view v) {
    unsigned res = 0;
    for (auto c : v) {
      if (c < '0' || c > '9') throw invalid_number {};
      res = res * 10 + (c - '0');
    }
    return res;
  }
  static_assert(to_u32("0") == 0);
  static_assert(to_u32("129302") == 129302);

  export constexpr float to_f(jute::view v) {
    const auto take = [&] { v = v.subview(1).after; };

    bool negative = v[0] == '-';
    if (negative) take();
    
    // TODO: what to do with zeroes at left, like 0100? Octal or error?
    float mantissa = 0;
    while (v.size()) {
      // TODO: should we accept scientific notation without decimals, like "1e1"?
      if (v[0] == '.') break;
      if (v[0] < '0' || v[0] > '9') throw invalid_number {};
      mantissa = mantissa * 10 + (v[0] - '0');
      take();
    }

    int decimals = 0;
    if (v.size() && v[0] == '.') {
      take();

      while (v.size()) {
        if ((v[0] | 0x20) == 'e') break;
        if (v[0] < '0' || v[0] > '9') throw invalid_number {};
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
        if (v[0] < '0' || v[0] > '9') throw invalid_number {};
        e = e * 10 + (v[0] - '0');
        take();
      }
      decimals -= eneg ? -e : e;
    }

    float div = negative ? -1 : 1;
    for (auto i = 0; i < decimals; i++) div *= 10;
    for (auto i = decimals; i < 0; i++) div /= 10;
    return mantissa / div;
  }

  static_assert(to_f("0") == 0.0f);
  static_assert(to_f("129302") == 129302.0f);

  static_assert(to_f("0.0") == 0.0f);
  static_assert(to_f("129.302") == 129.302f);
  static_assert(to_f("-129.302") == -129.302f);

  static_assert(to_f("-10.0e0") == -10.0f);
  static_assert(to_f("-0.1e2") == -10.0f);
  static_assert(to_f(".1e+2") == 10.0f);
  static_assert(to_f("100.0e-1") == 10.0f);
}

