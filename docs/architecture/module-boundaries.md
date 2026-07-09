# Module Boundaries

ToolkitProject should remain a private SDK/foundation layer. Module boundaries are strict by default.

## CoreKit

Allowed:

- primitive types;
- containers;
- allocators;
- threading primitives;
- filesystem and stream abstractions;
- logging and diagnostics;
- platform basics that are broadly useful.

Not allowed:

- graphics-specific resource concepts;
- renderer state;
- scene objects;
- game-object lifecycle;
- asset pipeline policy.

## MathKit

Allowed:

- numeric types;
- vectors/matrices/quaternions;
- geometry primitives;
- intersection and containment helpers.

Not allowed:

- rendering pipeline logic;
- physics world or solver logic;
- engine entity transforms.

## WindowKit

Allowed:

- windows;
- input;
- screens;
- clipboard/dialog support if platform-scoped;
- platform window handles behind controlled APIs.

Not allowed:

- UI framework;
- editor docking;
- renderer viewport management beyond handle passing.

## GraphicsKit

GraphicsKit is RHI only.

Allowed:

- devices;
- contexts/command buffers;
- buffers/textures/samplers;
- shaders and shader programs;
- pipeline state;
- swapchains;
- backend implementations such as Vulkan, DX11, DX12, Metal, OpenGL;
- backend memory and synchronization utilities.

Not allowed:

- renderer graph;
- material system;
- model loading;
- scene rendering;
- lighting system;
- post-processing pipeline;
- particle system;
- terrain;
- UI rendering policy;
- asset database.

If these systems are needed, create separate modules such as `RenderKit`, `AssetKit`, `SceneKit`, or `UiKit`.

## AudioKit

Allowed:

- audio device abstraction;
- audio clips/sources/listeners;
- decoding/encoding;
- platform audio backends;
- streaming playback infrastructure.

Not allowed:

- game-specific audio event systems;
- editor audio preview policy;
- asset import pipeline policy.

## NetKit

Allowed:

- network drivers;
- peers/connections;
- packet/message types;
- serialization streams;
- basic connection events.

Reflection-dependent features should be staged:

- RPC depends on ReflectKit method metadata and invocation.
- Replication depends on reflected field metadata and object registry.
- Do not force old engine scripting assumptions into NetKit.

## ReflectKit

Allowed:

- runtime type metadata;
- generated metadata registration;
- object/type handles;
- field/method metadata;
- invocation support;
- codegen integration.

Not allowed:

- engine object model policy;
- scripting runtime policy;
- serialization format ownership unless explicitly designed.

## LocaleKit

Allowed:

- culture/language identifiers;
- localized string tables;
- binary localization file parsing;
- fallback logic.

Not allowed:

- editor translation workflow;
- remote localization service policy.

## VideoKit

Allowed:

- video player abstraction;
- platform video backend wrappers;
- texture/frame delivery interfaces.

Not allowed:

- cutscene system;
- timeline/editor integration;
- asset import pipeline policy.
