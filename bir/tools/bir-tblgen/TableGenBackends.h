#ifndef BIR_TOOLS_BIR_TBLGEN_TABLEGENBACKENDS_H_
#define BIR_TOOLS_BIR_TBLGEN_TABLEGENBACKENDS_H_

namespace llvm {
class raw_ostream;
class RecordKeeper;
} // namespace llvm

namespace belalang::bir {

void emitRustBindings(const llvm::RecordKeeper &, llvm::raw_ostream &);
void emitCXXBindingsDecl(const llvm::RecordKeeper &, llvm::raw_ostream &);
void emitCXXBindingsDefs(const llvm::RecordKeeper &, llvm::raw_ostream &);

} // namespace belalang::bir

#endif // BIR_TOOLS_BIR_TBLGEN_TABLEGENBACKENDS_H_
