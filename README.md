# Advanced Encryption Standard (AES-128) Implementation

## Project Overview

This is an implementation of the Advanced Encryption Standard (AES-128) algorithm from scratch in C++.

## Project Structure

The solution is split into three components:

- **AES128Lib:**  
  Contains the core AES-128 implementation, including key expansion, encryption/decryption, and file I/O functionality, packaged as a library.

- **AES128Demo:**  
  A console application that demonstrates how to use AES128Lib. It includes main.cpp, which provides a command-line interface, and a Resources folder, which contains the text files (input.txt, encryptedData.txt, and decryptedData.txt) used during the demo.

- **AES128Tests:**  
  Contains unit tests built with Google Test to verify the library’s functionality and ensure that each component works as expected.

## Features

- **AES-128 Implementation:** Encrypts and decrypts 16-byte blocks using a 128-bit key.
- **File I/O Support:** Processes files in fixed-size blocks for encryption and decryption.
- **Key Options:** Supports both random key generation and manual key input.
- **Command-Line Interface:** The demo provides a simple command-line interface for encryption and decryption.
- **Testing:** Unit tests ensure the correctness of the AES and file operations.
- **Modern C++:** Uses C++20 modules.

## Implementation Details

I followed the official documentation for AES, available [here](https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.197-upd1.pdf).

- **Algorithm:**  
  Implements standard AES transformations (`SubBytes`, `ShiftRows`, `MixColumns`, `AddRoundKey`) and their inverses.

- **Key Expansion:**  
  Expands the initial 128-bit key into 11 round keys (one initial key plus 10 rounds).

- **File Handling:**  
  Reads input files in 16-byte blocks, applies PKCS#7 padding to incomplete blocks, and writes encrypted data as hexadecimal values.

### Encryption Process

1. Expand the original key into 11 round keys.
2. Process each 16-byte block of plaintext through 10 rounds of AES transformations.
3. Apply PKCS#7 padding to the final block if it is incomplete.
4. Write the encrypted data as hexadecimal values to the output file.

### Decryption Process

1. Read the hexadecimal-encoded ciphertext from the file.
2. Decrypt each block using the inverse AES operations.
3. Remove PKCS#7 padding from the final block.
4. Write the resulting plaintext to the output file.

> **Note:** I did this project as a learning exercise, not as a secure implementation for real-world use.

## Building and Running

### Prerequisites
- A C++20-compatible compiler (required for AES functionality and demo).
- Google Test (only needed for running unit tests).

### Files
- **AES128Lib:** This project is configured to be built as a static library.
- **AES128Demo:** This is set as the startup project.
- **AES128Tests:** This is integrated with Google Test and can be run via Visual Studio’s testing.

## How to Use

**Demo Application:**  
Run `AES128Demo` and follow the CLI prompts to generate or input a key, then choose to encrypt or decrypt data.

## Example

Below is an example demonstrating the core functionality:

```cpp
import aes_interface;
import fileio_interface;

int main() {
    // Generate a random 16-byte key and initialize the AES object
    Key key = AES::GenerateRandomKey();
    AES aes(key);

    // Encrypt the plaintext file and save the ciphertext
    EncryptFile("Resources/input.txt", "Resources/encryptedData.txt", aes);

    // Decrypt the ciphertext file and restore the plaintext
    DecryptFile("Resources/encryptedData.txt", "Resources/decryptedData.txt", aes);

    return 0;
}
```