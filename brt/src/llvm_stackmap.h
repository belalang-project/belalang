#ifndef BRT_LLVM_STACKMAP_H_
#define BRT_LLVM_STACKMAP_H_

#include <stddef.h>
#include <stdint.h>

struct LLVMStkMapHeader {
  uint8_t version;
  uint8_t reserved0;
  uint16_t reserved1;
  uint32_t num_functions;
  uint32_t num_constants;
  uint32_t num_records;
} __attribute__((packed));

struct LLVMStkMapSizeRecord {
  uint64_t function_address;
  uint64_t stack_size;
  uint64_t record_count;
} __attribute__((packed));

struct LLVMStkMapConstant {
  uint64_t large_constant;
} __attribute__((packed));

struct LLVMStkMapLocation {
  uint8_t kind;
  uint8_t reserved0;
  uint16_t location_size;
  uint16_t dwarf_regnum;
  uint16_t reserved1;
  int32_t offset_or_small_constant;
} __attribute__((packed));

struct LLVMStkMapRecordHeader {
  uint64_t patch_point_id;
  uint32_t instruction_offset;
  uint16_t reserved;
  uint16_t num_locations;
} __attribute__((packed));

struct LLVMStkMapLiveOutsHeader {
  uint16_t padding;
  uint16_t num_live_outs;
} __attribute__((packed));

struct LLVMStkMapLiveOut {
  uint16_t dwarf_regnum;
  uint8_t reserved;
  uint8_t size_in_bytes;
} __attribute__((packed));

struct LLVMStkMapSection {
  const uint8_t *start;
  size_t size;
};

struct LLVMStkMapSection get_llvm_stkmap_section();

#endif // BRT_LLVM_STACKMAP_H_
