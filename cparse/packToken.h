#ifndef PACKTOKEN_H_
#define PACKTOKEN_H_

#include <string>

namespace cparse {

// Encapsulate TokenBase* into a friendlier interface
class packToken {
  TokenBase* base;

 public:
  static const packToken& None();

  typedef std::string (*strFunc_t)(const TokenBase*, uint32_t);
  static strFunc_t& str_custom();

 public:
  packToken() : base(new TokenNone()) {}
  packToken(const TokenBase& t) : base(t.clone()) {}
  packToken(const packToken& t) : base(t.base->clone()) {}
  packToken(packToken&& t) : base(t.base) { t.base = 0; }
  packToken& operator=(const packToken& t);

  template<class C>
  packToken(C c, tokType type) : base(new Token<C>(c, type)) {}
  packToken(int i) : base(new Token<int64_t>(i, INT)) {}
  packToken(int64_t l) : base(new Token<int64_t>(l, INT)) {}
  packToken(bool b) : base(new Token<uint8_t>(b, BOOLEANO)) {}
  packToken(size_t s) : base(new Token<int64_t>(s, INT)) {}
  packToken(float f) : base(new Token<double>(f, REAL)) {}
  packToken(double d) : base(new Token<double>(d, REAL)) {}
  packToken(const char* s) : base(new Token<std::string>(s, STR)) {}
  packToken(const std::string& s) : base(new Token<std::string>(s, STR)) {}
  packToken(const TokenMap& map);
  packToken(const TokenList& list);
  ~packToken() { delete base; }

  TokenBase* operator->() const;
  bool operator==(const packToken& t) const;
  bool operator!=(const packToken& t) const;
  packToken& operator[](const std::string& key);
  packToken& operator[](const char* key);
  const packToken& operator[](const std::string& key) const;
  const packToken& operator[](const char* key) const;
  TokenBase* token() { return base; }
  const TokenBase* token() const { return base; }

  bool asBool() const;
  double asDouble() const;
  int64_t asInt() const;
  std::string& asString() const;
  TokenMap& asMap() const;
  TokenList& asList() const;
  Tuple& asTuple() const;
  STuple& asSTuple() const;
  Function* asFunc() const;

  // Specialize this template to your types, e.g.:
  // MyType& m = packToken.as<MyType>();
  template<typename T> T& as() const;

  // The nest argument defines how many times
  // it will recursively print nested structures:
  std::string str(uint32_t nest = 3) const;
  static std::string str(const TokenBase* t, uint32_t nest = 3);

 public:
  // This constructor makes sure the TokenBase*
  // will be deleted when the packToken destructor is called.
  //
  // If you still plan to use your TokenBase* use instead:
  //
  // - packToken(token->clone())
  //
  explicit packToken(TokenBase* t) : base(t) {}

 public:
  // Used to recover the original pointer.
  // The intance whose pointer was removed must be an rvalue.
  TokenBase* release() && {
    TokenBase* b = base;
    // Setting base to 0 leaves the class in an invalid state,
    // except for destruction.
    base = 0;
    return b;
  }
};

// To allow cout to print it:
std::ostream& operator<<(std::ostream& os, const packToken& t);

}  // namespace cparse

#endif  // PACKTOKEN_H_
