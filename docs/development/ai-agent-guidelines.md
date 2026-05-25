# AI Agent Guidelines

AI tools may be used to assist development, but their output must be reviewed as normal engineering work.

## Allowed use

AI agents may help with:

- mechanical refactors;
- test generation;
- documentation drafts;
- CMake cleanup;
- migration of old engine code into Toolkit module style;
- static analysis reports;
- repetitive backend wrapper code.

## Required controls

Before accepting AI-generated changes:

- inspect every changed file;
- build affected targets;
- run relevant tests;
- check that generated files were not committed;
- check that no absolute local paths were introduced;
- check that no private code was copied into public locations;
- check that module boundaries are respected.

## Prompting rules for this repository

When asking an AI agent to modify ToolkitProject, include:

- target module;
- files allowed to edit;
- files forbidden to edit;
- whether generated files should be ignored;
- required tests/build command;
- expected commit scope.

Example:

```text
Refactor GraphicsKit Vulkan device initialization only.
Do not modify public RHI interfaces.
Do not add renderer-level concepts.
Do not commit generated files.
Run GraphicsTest if possible.
Commit scope: perf(vulkan).
```

## Prohibited AI output

Do not accept AI changes that:

- replace working architecture with generic template code;
- add unnecessary dependencies;
- rewrite unrelated files;
- remove comments explaining platform/backend details;
- silently change public APIs;
- add code copied from unknown sources without license review.
