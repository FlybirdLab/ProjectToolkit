# Code Review Guidelines

Review should protect ToolkitProject as a stable private SDK, not just check whether code compiles.

## Review checklist

### API and module boundary

- Does the change belong in this module?
- Does it expose backend details through public headers?
- Does it introduce engine-layer concepts into SDK-layer modules?
- Is the public API naming consistent?

### Correctness

- Are invalid inputs handled?
- Are failure paths tested?
- Are native handles released exactly once?
- Are descriptor lifetimes safe?
- Are move/copy semantics correct?

### Memory and performance

- Does the change add hidden allocations?
- Does it grow per-frame memory?
- Does it keep resources alive longer than necessary?
- For Vulkan/DX backends, does it distinguish driver memory from SDK-owned memory?

### Threading

- Are thread-safety assumptions documented?
- Are callbacks invoked on the expected thread?
- Are blocking waits acceptable in context?

### Build system

- Are new files included in CMake?
- Are dependencies private unless required by public headers?
- Are platform-specific libraries guarded by platform checks?
- Are local paths or generated files accidentally committed?

### Tests and docs

- Are tests updated?
- Are skipped tests justified?
- Is documentation updated for workflow/API changes?

## Blocking review comments

A review should block merge when:

- public API is changed without reason;
- module boundaries are violated;
- generated or local machine-specific files are committed;
- security-sensitive parsing/networking code has no validation;
- native resources can leak or double-release;
- build breaks a supported platform or backend.
