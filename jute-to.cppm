export module jute:to;
import :view;

namespace jute {
  export struct invalid_number {};
  export constexpr auto to_u32(jute::view v) {
    int res = 0;
    for (auto c : v) {
      if (c < '0' || c > '9') throw invalid_number {};
      res = res * 10 + (c - '0');
    }
    return res;
  }
  static_assert(to_u32("0") == 0);
  static_assert(to_u32("129302") == 129302);

  export constexpr float to_f(jute::view v) {
    float res = 0;
    int decimals = -1;
    bool negative = v[0] == '-';
    if (negative) v = v.subview(1).after;
    for (auto c : v) {
      if (c == '.' && decimals == -1) {
        decimals = 0;
        continue;
      } else if (c == '.') throw invalid_number {};
      if (c < '0' || c > '9') throw invalid_number {};
  
      res = res * 10 + (c - '0');
      
      if (decimals >= 0) decimals++;
    }
    for (auto i = 0; i < decimals; i++) res /= 10;
    return negative ? -res : res;
  }
  static_assert(to_f("0") == 0.0f);
  static_assert(to_f("129302") == 129302.0f);
  static_assert(to_f("129.302") == 129.302f);
  static_assert(to_f("-129.302") == -129.302f);
}

