export module jute:fmt;
import :heap;
import :to;
import sv;
import traits;

namespace {
  struct lit {
    const char * str;
    unsigned len;

    template<unsigned N>
      consteval lit(const char (&str)[N]) : str { str }, len { N - 1 } {}
  };
  template<traits::size_t N>
  struct lit2 {
    char str[N];
    unsigned len = N - 1;

    consteval lit2(const char (&s)[N]) {
      for (auto i = 0; i < N; i++) str[i] = s[i];
    }
  };

  template<typename T> consteval lit needle();
  template<> consteval lit needle<double   >() { return "f"; }
  template<> consteval lit needle<long long>() { return "d"; }
  template<> consteval lit needle<int      >() { return "d"; }
  template<> consteval lit needle<sv       >() { return "s"; }

  static consteval unsigned p_idx(const char * hs, unsigned hl, lit needle) {
    for (auto i = 0; i < hl - needle.len; i++) {
      if (hs[i] != '%') continue;

      i++;
      if (hs[i] == '%') continue; // Skip %%

      for (auto j = 0; j < needle.len; j++) {
        if (hs[i + j] == needle.str[j]) continue;
        throw "invalid mask in format string";
      }
      return i - 1;
    }
    // TODO: merge mask
    throw "missing mask in format string";
  }
  static_assert(p_idx("ok%lldok", 8, "lld") == 2);

  template<const char * Str, unsigned Len, typename... T>
    struct indices {
      sv p[sizeof...(T)];
      sv f;

      consteval indices() {
        lit needles[] { needle<traits::decay_t<T>>()... };
        auto str = Str;
        auto len = Len;
        for (auto i = 0; i < sizeof...(T); i++) {
          p[i] = { str, p_idx(str, len, needles[i]) };
          str += p[i].size() + 2;
          len -= p[i].size() + 2;
        }
        f = { str, len };
      }
    };
  template<lit2 M> struct i : indices<M.str, M.len, int, int> {};
  static_assert([] {
    i<"aa%daaa%daaaa"> ii {};
    auto [v0, v1] = ii.p;
    return v0 == "aa" && v1 == "aaa" && ii.f == "aaaa";
  }());

  static constexpr jute::heap to_s(auto && n) { return jute::to_s(n); }
  static constexpr jute::heap to_s(sv n) { return jute::heap { jute::no_copy {}, n }; }

  template<const char * Str, unsigned Len, typename... T>
  struct mask {
    static constexpr jute::heap fmt(T &&... n) {
      static constexpr const indices<Str, Len, T...> idxs {};
      jute::heap vals[] { to_s(traits::fwd<T>(n))... };

      unsigned len = idxs.f.size();
      for (auto & v : vals) len += v.size();
      for (auto l : idxs.p) len += l.size();

      char * buf = new char[len];
      jute::view vw { buf, len };
      for (auto i = 0; i < sizeof...(T); i++) {
        for (auto c : idxs.p[i]) *buf++ = c;
        for (auto c : vals[i]) *buf++ = c;
      }
      for (auto c : idxs.f) *buf++ = c;

      return jute::heap { jute::owned {}, vw };
    }
  };
}

namespace jute {
  export template<lit2 Msk, typename... T> constexpr jute::heap fmt(T &&... n) {
    return mask<Msk.str, Msk.len, T...>::fmt(traits::fwd<T>(n)...);
  }
  static_assert(fmt<"%d">(123) == "123");
  static_assert(fmt<"val = %d...">(123ll) == "val = 123...");
  static_assert(fmt<"val = %f...">(2.3) == "val = 2.300...");
  static_assert(fmt<"val = %s...">("ok"_sv) == "val = ok...");
  // static_assert(fmt<"val = %e...">(123) == ""); // compile-time error
  // static_assert(fmt<"val">(123) == ""); // compile-time error

  static_assert(fmt<"we got %d from %s at %f rate...">(23, "there"_sv, 6.7) == "we got 23 from there at 6.700 rate...");
}

