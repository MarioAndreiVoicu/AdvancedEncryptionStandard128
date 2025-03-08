# Advanced Encryption Standard

## Project Overview

I implemented the Advanced Encryption Standard (AES) algorithm from scratch in C++ (without using any AES libraries) to discover how the algorithm works and learn about cryptography.

## Project Structure

The solution is composed of three projects:

- **AES128Lib:**  
  Contains the core AES-128 implementation, including key expansion, encryption/decryption, and file I/O functionality, packaged as a library.

- **AES128Demo:**  
  A console application demonstrating how to use AES128Lib. It includes main.cpp (providing a command-line interface) and a Resources folder containing the text files (input.txt, encryptedData.txt, and decryptedData.txt).

- **AES128Tests:**  
  Contains unit tests built with Google Test to verify the library’s functionality.

## Features

- **AES-128 Implementation:** Encrypts and decrypts 16-byte blocks using a 128-bit key.
- **File I/O Support:** Processes files in fixed-size blocks with PKCS#7 padding.
- **Key Options:** Supports random key generation or manual key input.
- **Command-Line Interface:** The demo provides a simple interface for encryption and decryption.
- **Testing:** Unit tests ensure the correctness of the AES implementation.
- **Modern C++:** Uses C++20 modules.

## Implementation Details

I followed the official documentation for AES, available [here](https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.197-upd1.pdf).

- **Algorithm:**  
  Implements standard AES transformations (SubBytes, ShiftRows, MixColumns, AddRoundKey) and their inverses.

- **Key Expansion:**  
  Generates 11 round keys from the original key.

- **File Handling:**  
  Reads input files in 16-byte blocks, applies PKCS#7 padding to incomplete blocks, and writes encrypted data as hexadecimal values.

### Encryption Process

- Expand the original key into 11 round keys.
- Process the plaintext block through 10 rounds of AES transformations.
- Write the output as hexadecimal values to the file.

### Decryption Process

- Read the ciphertext (in hexadecimal format) from the file.
- Apply AES inverse operations to recover the plaintext.
- Remove PKCS#7 padding and write the original data to the file.

> **Note:** I did this project as a learning exercise, not as a secure implementation for real-world use.

## Building and Running

- **AES128Lib:** This should be built as a static library.
- **AES128Demo:** This should be set as the startup project.
- **AES128Tests:** The unit tests can be run via Visual Studio's testing to verify that the functionality is correct.

## How to Use

**Demo Application:**  
Run AES128Demo and follow the CLI prompts to generate or input a key, then choose to encrypt or decrypt data.

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
