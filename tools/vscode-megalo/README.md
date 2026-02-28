# ReachVariantTool Megalo VS Code Extension

This extension adds language support for ReachVariantTool Megalo scripts (`.megalo`) in VS Code.

## Included

- Syntax highlighting for:
  - core keywords (`if`, `altif`, `for`, `function`, `on`, `declare`, etc.)
  - keyword phrases (`for each player randomly`, `on local init`, `with network priority`)
  - comments (`--` line comments and Lua-style long block comments)
  - strings (single/double quotes and long bracket strings)
  - operators (comparison + assignment operators, including MCC shift operators)
  - booleans, built-in constants, namespace identifiers, enum references, numbers
- Language configuration:
  - line comment toggling (`--`)
  - pair auto-closing
  - block indentation rules for `do`/`if`/`for`/`function`/`enum` + `end`/`alt`/`altif`
- Snippets for common structures (`on init`, loops, if/altif/alt, function, alias, declare)

## Install Locally (from source)

1. Install VS Code extension packaging tool:

```bash
npm install --global @vscode/vsce
```

2. Build a VSIX:

```bash
cd tools/vscode-megalo
vsce package
```

3. Install the generated package:

```bash
code --install-extension reachvarianttool-megalo-language-0.0.1.vsix
```

## Develop/Test

Open `tools/vscode-megalo` in VS Code and press `F5` to launch an Extension Development Host.
