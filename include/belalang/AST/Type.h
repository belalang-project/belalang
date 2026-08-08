#ifndef BELALANG_AST_TYPE_H_
#define BELALANG_AST_TYPE_H_

namespace belalang {
namespace ast {

class Type {
public:
  Type() = delete;
  Type(const Type &) = delete;
  Type(Type &&) = delete;
  Type &operator=(const Type &) = delete;
  Type &operator=(Type &&) = delete;

  enum class TypeKind {
    Builtin,
  };

  TypeKind getKind() const { return kind; }

protected:
  Type(TypeKind k) : kind(k) {}

private:
  TypeKind kind;
};

class BuiltinType : public Type {
public:
  enum class Kind {
    String,
    Integer,
    Float,
    Boolean,
    None,
  };

  BuiltinType(Kind k) : Type(TypeKind::Builtin) {}

  static bool classof(const Type *t) {
    return t->getKind() == TypeKind::Builtin;
  }
};

} // namespace ast
} // namespace belalang

#endif // BELALANG_AST_TYPE_H_
