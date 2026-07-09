# Commit Message Convention

ToolkitProject uses a Conventional Commits-style format with stricter scope naming.

## Format

```text
<type>(<scope>): <summary>
```

Examples:

```text
feat(graphics): add lightweight Vulkan memory checkpoint logging
fix(core): correct realloc handling for zero-sized allocation
refactor(reflect): move generated metadata output to build directory
test(math): add frustum containment edge cases
chore(cmake): normalize backend option names
```

## Types

| Type | Meaning |
| --- | --- |
| feat | New user-visible or module-visible capability. |
| fix | Bug fix. |
| refactor | Internal rewrite without intended behavior change. |
| perf | Performance or memory optimization. |
| test | Test-only change. |
| docs | Documentation-only change. |
| style | Formatting or naming-only change. |
| build | Build system, compiler flags, dependency wiring. |
| ci | CI or automation. |
| chore | Maintenance that does not fit other types. |
| revert | Reverts a previous commit. |

## Recommended scopes

Use module names or tool names:

```text
core
math
window
graphics
vulkan
dx11
dx12
audio
net
locale
reflect
rkgen
shadercompiler
video
cmake
tests
docs
```

For cross-module changes, use the highest-level affected area:

```text
refactor(cmake): split common module setup helpers
fix(tests): avoid generated file dependency in clean checkout
```

## Summary rules

- Use lowercase where practical.
- Use imperative mood: `add`, `fix`, `remove`, `split`, `avoid`.
- Keep the first line under 72 characters when possible.
- Do not end the summary with a period.
- Do not mix unrelated changes in one commit.

## Body rules

Use a body when the reason is not obvious:

```text
perf(vulkan): request one queue per family during device creation

The backend currently requests all queues from each selected queue family, but
only queue index 0 is used. This increases driver-side setup cost and may add
unnecessary host memory overhead during initialization.
```

## Breaking changes

Use `BREAKING CHANGE:` in the body when public API or expected behavior changes:

```text
refactor(graphics): rename GfxShaderProgramDesc fields

BREAKING CHANGE: shader program creation now requires precompiled shader stages.
```

## AI-assisted commits

When an AI agent produced or heavily edited the change, the commit still needs normal review quality. Do not use vague summaries such as:

```text
fix: update files
chore: ai changes
refactor: cleanup
```

The commit must describe the actual technical change.
