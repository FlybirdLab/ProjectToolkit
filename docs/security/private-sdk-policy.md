# Private SDK Policy

ToolkitProject is a private SDK. Treat all source code, documentation, build scripts, generated metadata, and internal artifacts as confidential unless explicitly approved for release.

## Access

- Access is granted only to approved collaborators.
- Do not mirror the repository to public remotes.
- Do not upload source files to public paste services or public issue trackers.
- Do not include private source excerpts in public support requests.

## Redistribution

Without explicit approval, do not redistribute:

- source code;
- headers;
- generated binaries;
- static libraries;
- private documentation;
- build artifacts;
- SDK packages.

## Third-party code

Before adding third-party code:

- verify license compatibility;
- record source and version;
- keep license notices;
- isolate the dependency under the dependency system;
- avoid copying large third-party source blocks into Toolkit modules.

## Generated files

Generated files should normally be excluded from source control. If a generated file must be committed for bootstrapping, document:

- generator name;
- generation command;
- source input;
- reason it must be committed.

## AI and external tools

When using external AI or analysis tools:

- avoid uploading private source unless the tool is approved for private-code use;
- remove secrets, tokens, local credentials, and private paths from prompts/logs;
- verify generated code for license and correctness risk.

## Secrets

Never commit:

- access tokens;
- private keys;
- service credentials;
- machine-specific paths containing usernames when avoidable;
- private server addresses unless intended by design.
