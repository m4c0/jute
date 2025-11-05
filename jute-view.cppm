export module jute:view;
import hai;
import sv;

export namespace jute {
  using size_t = decltype(sizeof(0));
  
  template<typename T>
  concept stringish = requires (T t, const char * c, size_t len) {
    c = t.data();
    len = t.size();
  };
  
  /// Holds a pointer to a string (aka char array) and its size. This is intended
  /// to be a non-owning pointer holder. As consequences, it should not outlive
  /// the original pointer.
  struct view : public sv {
    using sv::sv;

    ///////////////////////////////////////////////////////////////////////////
    // Bridge functions, to allow an easier transition from jute::view to sv
    ///////////////////////////////////////////////////////////////////////////
 
    constexpr view(sv o) : sv { o } {}

    [[nodiscard]] static constexpr view unsafe(const char * cstr) {
      return view { sv::unsafe(cstr) };
    }
 
    ///////////////////////////////////////////////////////////////////////////

    [[nodiscard]] constexpr auto cstr() const {
      hai::cstr res { static_cast<unsigned>(size()) };
      auto ptr = res.begin();
      for (auto c : *this) *ptr++ = c;
      return res;
    }
  };
}

export namespace jute::literals {
  [[nodiscard]] constexpr view operator""_s(const char *v, size_t size) {
    return view { v, size };
  }
}

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

static_assert("aabaa"_s.index_of('b') == 2);
static_assert("aabaa"_s.index_of('c') == -1);

static_assert("abcd"_s.starts_with("ab"));
static_assert("abcd"_s.starts_with("abcd"));
static_assert(!"abcd"_s.starts_with("abcde"));
static_assert(!"x"_s.starts_with("y"));

static_assert("abcd"_s.ends_with("cd"));
static_assert("abcd"_s.ends_with("abcd"));
static_assert(!"abcd"_s.ends_with("abcde"));
static_assert(!"x"_s.ends_with("y"));

static_assert([] {
  const auto &[a, b] = "jute"_s.subview(2);
  return a == "ju"_s && b == "te"_s;
}());

static_assert([] {
  const auto &[a, b, c] = "jute"_s.subview(2, 1);
  return a == "ju"_s && b == "t"_s && c == "e"_s;
}());
static_assert([] {
  const auto &[a, b, c] = "jute"_s.subview(2, 0);
  return a == "ju"_s && b == ""_s && c == "te"_s;
}());

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

static_assert("abc"_s.trim() == "abc");
static_assert("  abc"_s.trim() == "abc");
static_assert("abc   "_s.trim() == "abc");
static_assert("   abc  "_s.trim() == "abc");
static_assert("   abc  "_s.trim() == "abc");
static_assert(" abc 234 "_s.trim() == "abc 234");
static_assert("   "_s.trim() == "");
static_assert(""_s.trim() == "");

static_assert(  "abc"_s  <  "bcd"_s  );
static_assert(  "abc"_s  <  "bcde"_s );
static_assert(!("bcd"_s  <  "abc"_s ));
static_assert(!("bcde"_s <  "abc"_s ));
static_assert(  "bcd"_s  >  "abc"_s  );
static_assert(  "bcde"_s >  "abc"_s  );
static_assert(!("abc"_s  >  "bcd"_s ));
static_assert(!("abc"_s  >  "bcde"_s));
static_assert(  "abc"_s  <= "abc"_s  );
static_assert(!("abcd"_s <= "abc"_s ));
static_assert(  "abc"_s  >= "abc"_s  );
static_assert(!("abc"_s  >= "abcd"_s));
} // namespace

static_assert([] {
  hai::cstr c{20};
  jute::view v{c};
  return v.data() == c.data() && c.size() == v.size();
}());
static_assert([] {
  jute::view v{"testing"};
  hai::cstr c = v.cstr();
  return v == jute::view{c};
}());
