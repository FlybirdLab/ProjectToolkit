# Release and Versioning Policy

ToolkitProject is a private SDK. Versioning is used for internal snapshots and downstream integration, not public distribution.

## Version format

Use semantic-style versions:

```text
MAJOR.MINOR.PATCH[-tag]
```

Examples:

```text
0.1.0-internal
0.2.0-dev
0.2.1-hotfix
```

## Version meaning

| Part | Meaning |
| --- | --- |
| MAJOR | Large incompatible SDK/API change. |
| MINOR | New module capability or backend feature. |
| PATCH | Bug fix, regression fix, small compatibility improvement. |

Before `1.0.0`, API compatibility is best-effort but breaking changes must still be documented.

## Snapshot checklist

Before cutting an internal snapshot:

- build supported core modules;
- run relevant tests;
- verify generated files are excluded;
- verify dependency binaries are not accidentally included;
- update changelog;
- record known broken modules/backends;
- tag the commit if needed.

## Changelog format

Use categories:

```markdown
## 0.2.0-internal

### Added
- Added Vulkan memory checkpoint logging.

### Changed
- Reduced default Vulkan queue requests to one queue per family.

### Fixed
- Fixed CoreKit path normalization edge case.

### Known Issues
- NetKit RPC layer is not complete until ReflectKit invocation metadata is ready.
```
