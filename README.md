# ToolkitProject

ToolkitProject is a private C++ SDK and foundational runtime framework extracted from a legacy engine codebase. It provides reusable low-level modules for engine, tool, application, and runtime development.

This repository is not an open-source project and is not intended to be used as a public engine distribution. Access, copying, redistribution, and reuse are restricted to authorized maintainers and approved internal projects.

## Current module layout

| Module | Purpose |
| --- | --- |
| CoreKit | Core types, containers, memory, threading, filesystem, streams, logging, platform basics. |
| MathKit | Vectors, matrices, quaternions, transforms, geometry, bounds, colors. |
| WindowKit | Window creation, input, screens, platform window integration. |
| GraphicsKit | RHI layer only. Backend-specific APIs such as Vulkan, DirectX, Metal, OpenGL belong here, but renderer-level systems do not. |
| AudioKit | Audio device abstraction, clips, sources, listeners, decoding/encoding, backend integration. |
| NetKit | Networking foundation. RPC and replication are reflection-dependent and should be treated as staged work. |
| LocaleKit | Localization and binary string-table support. |
| ReflectKit | Runtime reflection, generated metadata, object/type handles, invocation foundation. |
| VideoKit | Video playback abstraction and platform backend entry points. |

## Repository rules

- Keep ToolkitProject as an SDK/framework layer, not a full engine layer.
- Do not move renderer, scene, asset pipeline, physics, editor, scripting, or UI systems into existing Kit modules unless a dedicated module boundary is defined first.
- Generated files should not be committed unless explicitly documented as required bootstrap files.
- Third-party libraries should stay isolated under the dependency system and should not leak into public headers unless unavoidable.
- Public headers must remain stable, minimal, and independent from backend implementation details.

## Recommended build flow

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

The exact dependency setup is project-local and may rely on private `libs` configuration that is intentionally not documented here.

## Documentation index

- [Contributing](CONTRIBUTING.md)
- [Commit message convention](docs/development/commit-message-convention.md)
- [Branching strategy](docs/development/branching-strategy.md)
- [C++ style guide](docs/development/cpp-style-guide.md)
- [CMake guidelines](docs/development/cmake-guidelines.md)
- [Testing guidelines](docs/development/testing-guidelines.md)
- [Code review guidelines](docs/development/review-guidelines.md)
- [Module boundaries](docs/architecture/module-boundaries.md)
- [Private SDK policy](docs/security/private-sdk-policy.md)
