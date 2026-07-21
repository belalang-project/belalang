#ifndef BIR_TOOLS_BIR_TBLGEN_TABLEGENBACKENDS_H_
#define BIR_TOOLS_BIR_TBLGEN_TABLEGENBACKENDS_H_

namespace llvm {
class raw_ostream;
class RecordKeeper;
} // namespace llvm

namespace belalang::bir {

void emitRustBindingDecls(const llvm::RecordKeeper &, llvm::raw_ostream &);
void emitCXXBindingDecls(const llvm::RecordKeeper &, llvm::raw_ostream &);
void emitCXXBindingDefs(const llvm::RecordKeeper &, llvm::raw_ostream &);

} // namespace belalang::bir

#endif // BIR_TOOLS_BIR_TBLGEN_TABLEGENBACKENDS_H_
