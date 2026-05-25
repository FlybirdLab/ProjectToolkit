# Contributing Guide

ToolkitProject is a private SDK repository. Contributions are accepted only from authorized maintainers or approved internal collaborators.

## Contribution principles

1. Keep changes scoped.
2. Preserve module boundaries.
3. Avoid hidden coupling between Kits.
4. Do not introduce engine-level concepts into framework-level modules.
5. Prefer small, reviewable commits over large mixed rewrites.
6. Update tests and documentation when behavior changes.

## Before starting work

Check the target module and confirm the change belongs there:

- Core abstractions belong in CoreKit only when they are broadly reusable.
- Math-only logic belongs in MathKit.
- GraphicsKit is RHI only.
- Backend implementation details must not leak into public RHI headers.
- ReflectKit-related metadata generation must not require checked-in generated sources.
- NetKit RPC and replication work must be staged behind ReflectKit capability readiness.

## Change requirements

Every non-trivial change should include:

- clear commit message following the repository convention;
- test coverage or a short explanation for why tests are not practical;
- build-system updates when new files, modules, or dependencies are added;
- documentation updates when public API, module boundary, or workflow changes.

## Review expectations

Reviewers should check:

- API stability;
- ownership and lifetime rules;
- allocation behavior;
- thread-safety assumptions;
- platform-specific behavior;
- CMake integration;
- whether generated files or local paths were accidentally committed.

## Prohibited changes without prior design approval

- Moving renderer, scene, asset, scripting, editor, or UI systems into GraphicsKit/CoreKit.
- Adding global state to core modules without lifecycle design.
- Adding public dependency on heavy third-party libraries.
- Committing generated files, local absolute paths, build outputs, or binary dependency artifacts.
- Changing public ABI/API naming conventions as part of unrelated fixes.
