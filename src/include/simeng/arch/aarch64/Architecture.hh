#pragma once

#include <forward_list>
#include <queue>
#include <unordered_map>

#include "simeng/arch/Architecture.hh"
#include "simeng/arch/aarch64/ExceptionHandler.hh"
#include "simeng/arch/aarch64/Instruction.hh"
#include "simeng/kernel/Linux.hh"

using csh = size_t;

namespace simeng {
namespace arch {
namespace aarch64 {

/* A basic ARMv8-a implementation of the `Architecture` interface. */
class Architecture : public arch::Architecture {
 public:
  Architecture(kernel::Linux& kernel, YAML::Node config);
  ~Architecture();
  /** Pre-decode instruction memory into a macro-op of `Instruction`
   * instances. Returns the number of bytes consumed to produce it (always 4),
   * and writes into the supplied macro-op vector. */
  uint8_t predecode(const void* ptr, uint8_t bytesAvailable,
                    uint64_t instructionAddress, BranchPrediction prediction,
                    MacroOp& output, std::string& disasm) const override;

  /** Returns an ARMv8-a register file structure description. */
  std::vector<RegisterFileStructure> getRegisterFileStructures() const override;

  /** Returns a zero-indexed register tag for a system register encoding. */
  uint16_t getSystemRegisterTag(uint16_t reg) const;

  /** Create an exception handler for the exception generated by `instruction`,
   * providing the core model object and a reference to process memory.
   * Returns a smart pointer to an `ExceptionHandler` which may be ticked until
   * the exception is resolved, and results then obtained. */
  std::shared_ptr<arch::ExceptionHandler> handleException(
      const std::shared_ptr<simeng::Instruction>& instruction, const Core& core,
      MemoryInterface& memory) const override;

  /** Retrieve the initial process state. */
  ProcessStateChange getInitialState() const override;

  /** Retrieve any updates to the process state. */
  ProcessStateChange getUpdateState() const override;

  /** Returns the maximum size of a valid instruction in bytes. */
  uint8_t getMaxInstructionSize() const override;

  /** Returns the current vector length set by the provided configuration. */
  uint64_t getVectorLength() const;

 private:
  /** Retrieve an executionInfo object for the requested instruction. If a
   * opcode-based override has been defined for the latency and/or
   * port information, return that instead of the group-defined execution
   * information. */
  executionInfo getExecutionInfo(Instruction& insn) const;

  /** A decoding cache, mapping an instruction word to a previously decoded
   * instruction. Instructions are added to the cache as they're decoded, to
   * reduce the overhead of future decoding. */
  static std::unordered_map<uint32_t, Instruction> decodeCache;
  /** A decoding metadata cache, mapping an instruction word to a previously
   * decoded instruction metadata bundle. Metadata is added to the cache as it's
   * decoded, to reduce the overhead of future decoding. */
  static std::forward_list<InstructionMetadata> metadataCache;

  /** A mapping from system register encoding to a zero-indexed tag. */
  std::unordered_map<uint16_t, uint16_t> systemRegisterMap_;

  /** A map to hold the relationship between aarch64 instruction groups and
   * user-defined execution information. */
  std::unordered_map<uint16_t, executionInfo> groupExecutionInfo_;

  /** A map to hold the relationship between aarch64 instruction opcode and
   * user-defined execution information. */
  std::unordered_map<uint16_t, executionInfo> opcodeExecutionInfo_;

  /** A Capstone decoding library handle, for decoding instructions. */
  csh capstoneHandle;

  /** A reference to a Linux kernel object to forward syscalls to. */
  kernel::Linux& linux_;

  /** The vector length used by the SVE extension in bits. */
  uint64_t VL_;
};

}  // namespace aarch64
}  // namespace arch
}  // namespace simeng
