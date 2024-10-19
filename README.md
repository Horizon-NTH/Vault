# Vault

[![Release](https://img.shields.io/badge/Release-v3.1-blueviolet)](https://github.com/Horizon-NTH/Vault/releases)
[![Language](https://img.shields.io/badge/Language-C%2B%2B-0052cf)](https://en.wikipedia.org/wiki/C++)
[![Licence](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

## Introduction

A small, portable file system with encryption capabilities.

## Features

- **Vault Opening** : Open an existing vault to access its contents.
- **Vault Closing** : Close a directory and save its contents to a single file.
- **Vault Encryption** : Encrypt and decrypt the vault with a password.
- **Vault Compression**: Compress and decompress files stored in a vault.

## Installation

> You can also simply install a pre-built version [here](https://github.com/Horizon-NTH/Vault/releases).

### Get Source Code

You first need to clone the repository with [git](https://git-scm.com).

```bash
git clone https://github.com/Horizon-NTH/Vault.git
```

### Build

Ensure you have [CMake](https://cmake.org/) and [Conan](https://conan.io/) are installed.  
Generate the build environment using CMake, Conan will automatically install all the
dependencies needed so you can build the application.

> You can also enable the [_tests_](https://github.com/google/googletest) by setting the `ENABLE_TESTS` option to `ON`.

```bash
mkdir build && cd build
cmake .. [-DENABLE_TESTS=ON]
make 
```

You can also install the application on your system using the following command:

```bash
sudo make install
```

### Execute

You can now run the application with the following command:

```bash
vault <command> [options]
```

## Usage

To have a list of all available commands, you can use the `--help` option.

```bash
vault [help | --help | -h]
```

### Close a Vault

To close a vault and save its contents to a single file, you can use the `close` command.
You can also specify the path where the vault will be saved and the extension of the file.
If you want to **encrypt** the vault, you can use the `-E | --encrypt` option.

```bash
vault close <vault_name> [-E | --encrypt] [--destination <path>] [--extension <ext>]
```

> [!NOTE]
> If you choose to encrypt the vault, you will be prompted to enter a password.

### Open a Vault

To open an existing vault file, you can use the `open` command. You can also specify the path where the vault will be
opened.

```bash
vault open <vault_name> [--destination <path>]
```

> [!NOTE]
> If the vault is encrypted, you will be prompted to enter the password.

## Dependencies

- **[Botan](https://botan.randombit.net/)** is used for encryption and password derivation.
- **[GoogleTest (gtest)](https://github.com/google/googletest)** is used for testing.
- **[CLI11](https://github.com/CLIUtils/CLI11)** is used for command-line argument parsing.
- **[PugiXML](https://pugixml.org/)** is used for XML parsing.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
