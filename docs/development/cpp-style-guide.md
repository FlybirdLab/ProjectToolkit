# C++ Style Guide

This guide defines the default style for ToolkitProject C++ code. Existing module-local style may be preserved during migration, but new code should follow this document.

## Language level

- Use modern C++ where it reduces complexity.
- Avoid using advanced language features only for cleverness.
- Prefer explicit ownership and lifecycle over hidden global state.
- Public headers should be conservative and stable.

## File layout

Preferred structure:

```text
modules/<KitName>/
  include/toolkit/<kit>/...
  src/toolkit/<kit>/...
  src/toolkit/platforms/<backend-or-platform>/...
```

Backend implementation details belong under `src`, not public headers.

## Naming

Use the existing Toolkit naming style consistently inside a module.

General recommendations:

- Types: `PascalCase`
- Functions/methods: follow existing module style; do not mix styles in one file.
- Private members: follow existing module style; keep naming consistent.
- Constants/macros: `UPPER_SNAKE_CASE` only for preprocessor definitions.
- Public APIs should avoid abbreviations unless already established, such as `Gfx`, `RHI`, `RPC`.

## Includes

- Include only what is needed.
- Prefer forward declarations in headers when possible.
- Do not include backend headers from public RHI headers.
- Do not include heavy third-party headers in public headers unless unavoidable.
- Keep platform SDK includes inside platform/backend implementation files.

## Ownership

- Document ownership when passing raw pointers across module boundaries.
- Prefer RAII wrappers for native handles.
- Avoid `delete this` unless the type is explicitly designed and documented for intrusive lifetime management.
- Avoid storing references to temporary data passed from descriptors.
- Validate descriptor lifetime assumptions in constructors.

## Error handling

- Fail early for invalid descriptors and unsupported backend states.
- Use project assertions for programmer errors.
- Return explicit failure results for runtime/environment errors.
- Do not silently fall back to another backend unless the caller requested fallback.

## Memory and allocation

- Avoid unexpected heap allocation in hot paths.
- Keep allocator behavior explicit in CoreKit-level types.
- Do not allocate large temporary buffers in per-frame paths.
- For GraphicsKit, distinguish host memory, device memory, staging memory, and driver-owned memory when logging.

## Threading

- Document thread ownership for objects that touch platform APIs or GPU/Audio backends.
- Avoid hidden cross-thread callbacks.
- Avoid blocking waits in worker code unless the scheduling design explicitly allows it.
- Prefer task-system-friendly synchronization primitives over sleep polling.

## Platform code

- Platform-specific code must be isolated behind narrow interfaces.
- Keep `#ifdef` blocks out of common logic where possible.
- Do not let one platform backend define the public API shape for all platforms.

## Comments

Use comments to explain non-obvious decisions, not to repeat the code.

Good comment:

```cpp
// Request one queue per family. The RHI currently uses only queue index 0,
// and requesting all queues increases driver-side initialization overhead.
```

Bad comment:

```cpp
// Set queue count to 1.
```
