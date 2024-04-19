# AES Encryption Implementation

## Project Overview
- Implemented the Advanced Encryption Standard (AES) algorithm from scratch without relying on existing AES libraries.
- Utilized basic C++ Standard Template Library (STL) functionalities like arrays, file handling, and string operations.
- Chose to implement without AES libraries to learn more about cryptography and better understand how the algorithm works

## AES Overview
- AES is a symmetric encryption algorithm widely used to secure data.
- AES-128 encrypts and decrypts data in blocks of 16 bytes using a 128-bit key.

## Features
- Supports encrypting and decrypting files using AES-128.
- Can generate a random key or use a predefined key.
- Provides a command-line user interface for key selection (randomly generated or predefined) and encryption/decryption.

## Implementation Details
- Followed official documentation for AES,  available [here](https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.197-upd1.pdf).
- Encrypted data (ciphertext) represented with hexadecimal values.
- Processes input in blocks of 16 characters for encryption/decryption.
- Utilizes PKCS#7 padding scheme for encryption.
- Achieves encryption/decryption speed of 15MB/minute on my laptop (a text file with 15 million characters).

## Encryption Process
- Key expansion generates 11 round keys from the original key.
- Takes input in blocks of 16 characters, encrypts it and writes the ciphertext to the file.
- Applies padding to the last block to ensure all blocks are of size 16.

## Decryption Process
- Similar to encryption but applies the inverse of encryption functions to transform ciphertext back into plaintext.

