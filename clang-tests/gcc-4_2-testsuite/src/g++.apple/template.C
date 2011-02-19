/* APPLE LOCAL file templated static data 6298605 */
/* Radar 6298605 */

namespace {
  template<int>
  struct ElfSizes {
    static const int sym_size = 5;  /* { dg-error "has internal linkage but is not defined" } */
    static const int ehdr_size = 5;  /* { dg-error "has internal linkage but is not defined" } */
  };

  template<int>
  struct SizedElfFileData {
    SizedElfFileData();

    virtual void ReadSymbols();
  };

  template<int Size>
  SizedElfFileData<Size>::SizedElfFileData() {
    ElfSizes<Size>::ehdr_size; /* { dg-error "note: used here" } */
  }

  template<int Size>
  void SizedElfFileData<Size>::ReadSymbols() {
    ElfSizes<Size>::sym_size; /* { dg-error "note: used here" } */
  }

  void Open() {
    SizedElfFileData<32> foo = SizedElfFileData<32>();
  }
}
