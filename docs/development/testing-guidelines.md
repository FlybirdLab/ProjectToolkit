# Testing Guidelines

Tests are required for stable SDK behavior. The goal is to keep low-level modules reliable while allowing backend-specific work to remain staged.

## Test categories

| Category | Scope |
| --- | --- |
| Unit tests | Core containers, math, serialization, reflection metadata, parsing. |
| Integration tests | Window/input, audio device, graphics device, shader compiler, network driver. |
| Backend tests | Vulkan, DX11, DX12, OpenAL, XAudio, platform-specific behavior. |
| Regression tests | Reproductions for fixed bugs. |

## Rules

- Add tests for behavior, not implementation details.
- Keep tests deterministic when possible.
- Avoid tests that depend on local absolute paths.
- Backend tests should detect unsupported environments and skip cleanly.
- Do not mark unfinished tests as passing.
- Skipped tests must explain why they are skipped.

## Graphics tests

GraphicsKit is an RHI. Tests should verify:

- device creation/destruction;
- swapchain creation when a window is available;
- buffer and texture allocation;
- shader/pipeline creation;
- command submission;
- synchronization behavior;
- resource deletion safety;
- memory tracking where supported.

For Vulkan memory work, prefer checkpoint logs around:

```text
volkInitialize
vkCreateInstance
vkCreateDevice
vmaCreateAllocator
swapchain creation
first pipeline creation
first draw
shutdown
```

## Reflect tests

ReflectKit tests should verify:

- type registration;
- generated metadata loading;
- field and method lookup;
- invocation argument passing;
- object handle behavior;
- generated output not depending on local absolute paths.

## Net tests

NetKit should be tested in layers:

1. driver-only send/receive;
2. connection and disconnect events;
3. packet ordering and reliability if supported by the driver;
4. serialization;
5. RPC only after ReflectKit invocation is ready;
6. replication only after reflected field metadata is stable.
