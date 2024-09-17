# Vault

[![Release](https://img.shields.io/badge/Release-v0.1-blueviolet)](https://github.com/Horizon-NTH/Vault/releases)
[![Language](https://img.shields.io/badge/Language-C%2B%2B-0052cf)](https://en.wikipedia.org/wiki/C++)
[![Licence](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

## Introduction

A small, portable file system with encryption capabilities.

## Features

- **Vault Creation** : Create a vault to store files securely.
- **Vault Opening** : Open an existing vault to access its contents.
- **Vault Closing** : Close a vault and save its contents to a single file.
- **Vault Encryption _[Coming Soon]_** : Encrypt and decrypt files stored in a vault with a password.
- **Vault Compression _[Coming Soon]_** : Compress and decompress files stored in a vault.

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

### Commands

- `open` : Open a vault to access its contents.
- `close` : Close a vault and save its contents to a single file.

### Options

#### Command `open`

- `-v, --vault <path>` : Path to the vault (required).
- `-d, --destination <path>` : Destination path for the output (optional).

Exemple :

```bash
./vault open -v /path/to/vault -d /output/path
```

#### Command `close`

- `-v, --vault <path>` : Path to the vault (required).
- `-d, --destination <path>` : Destination path for the output (optional).
- `-e, --extension <ext>` : Extension of the output file (optional).

Exemple :

```bash
./vault close --vault=/path/to/vault --destination=/output/path --extension=.vlt
```

### Special Commands

To display **help** information, use one the following commands:

```bash
./vault [ --help | -h ]
```

To display the **version** of the application, use one the following commands:

```bash
./vault [ --version | -v ]
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
