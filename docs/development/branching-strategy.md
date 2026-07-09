# Branching Strategy

ToolkitProject uses a simple private-SDK branch model.

## Branches

| Branch | Purpose |
| --- | --- |
| main | Stable integration branch. Should build and pass core tests. |
| dev | Active development branch. Larger work is merged here first. |
| feature/* | Feature work. |
| fix/* | Bug fixes. |
| refactor/* | Refactoring work. |
| experiment/* | Temporary research branches. May be force-pushed or deleted. |
| release/* | Optional stabilization branch for internal SDK snapshots. |

## Rules

- `main` should only receive reviewed changes from `dev` or release branches.
- `dev` may receive feature branches after review and local test validation.
- Large migrations should be split by module and stage.
- Do not merge generated files, local dependency binaries, or build output.
- Keep backend-specific changes isolated where possible.

## Branch naming examples

```text
feature/vulkan-memory-checkpoints
fix/core-realloc-zero-size
refactor/net-driver-layer
experiment/dx12-rhi-bootstrap
release/0.2.0-internal
```

## Merge expectations

Before merging:

- build the affected modules;
- run relevant tests;
- inspect generated file changes;
- check CMake file lists;
- confirm no absolute local paths were added;
- update documentation if public behavior changed.
