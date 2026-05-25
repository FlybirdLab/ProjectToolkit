# Security Policy

ToolkitProject is a private SDK. Security issues should be reported through private project channels only.

## Supported scope

Security-sensitive areas include:

- file and path handling;
- serialization and deserialization;
- network protocol parsing;
- shader compiler input handling;
- native backend initialization;
- dynamic library loading;
- generated reflection metadata;
- memory allocators and ownership boundaries.

## Reporting

Do not disclose vulnerabilities publicly. Report the issue with:

- affected module;
- reproduction steps;
- expected and actual behavior;
- platform, compiler, and build configuration;
- crash logs or sanitizer output if available;
- suspected impact.

## Handling rules

- Security fixes should be isolated from unrelated refactors.
- Do not include exploit-like test data in public or shared locations unless required for regression testing and approved.
- Avoid logging secrets, access tokens, machine-specific paths, or private asset locations.
- Treat dependency updates as security-sensitive when they affect native parsing, networking, compiler, or compression code.
