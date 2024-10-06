# Vault

[![Release](https://img.shields.io/badge/Release-v1.0-blueviolet)](https://github.com/Horizon-NTH/Vault/releases)
[![Language](https://img.shields.io/badge/Language-C%2B%2B-0052cf)](https://en.wikipedia.org/wiki/C++)
[![Licence](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

## Introduction

A small, portable file system with encryption capabilities.

## Features

- **Vault Creation** : Create a vault to store files securely.
- **Vault Opening** : Open an existing vault to access its contents.
- **Vault Closing** : Close a vault and save its contents to a single file.
- **Vault Encryption** _[Coming Soon]_ : Encrypt and decrypt files stored in a vault with a password.
- **Vault Compression** _[Coming Soon]_ : Compress and decompress files stored in a vault.

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

### Create a Vault

To create a new empty vault, you can use the `create` command.

```bash
vault create <vault_name>
```

By default, the vault will be created in the current directory. You can specify a destination path.

```bash
vault create <vault_name> --destination <path>
```

You can also choose a source directory so that the vault is created with the files in it.

```bash
vault create <vault_name> --from <path>
```

This will create a new vault, and move all the files from the source directory to the vault.

You also have the option to specify the extension of the closed vault file.

```bash
vault create <vault_name> --extension <ext>
```

> To use the next commands, you need to have a valid vault created with one of the previous commands.

### Close a Vault

To close a vault and save its contents to a single file, you can use the `close` command.
You can also specify the path where the vault will be saved.

```bash
vault close <vault_name> [--destination <path>]
```

### Open a Vault

To open an existing vault, you can use the `open` command. You can also specify the path where the vault will be opened.

```bash
vault open <vault_name> [--destination <path>]
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
