/**
 * @file fileio_interface.cppm
 * @brief Module interface for file input/output operations related to AES encryption.
 *
 * This module provides functions to read and write encrypted and decrypted data,
 * as well as a function to read a key from user input.
 */

export module fileio_interface;

#include <string>
#include <array>
import aes_interface;

/**
 * @brief Encrypts the contents of a file.
 *
 * Reads the file in BLOCK_SIZE-byte blocks, applies PKCS#7 padding on the final block if needed,
 * encrypts each block using the provided AES instance, and writes the encrypted data
 * as hexadecimal values to the output file.
 *
 * @param inputFileName The name of the plaintext input file.
 * @param outputFileName The name of the file to write the encrypted data.
 * @param aesInstance An initialized AES instance.
 */
export void EncryptFile(const std::string& inputFileName, const std::string& outputFileName, AES& aesInstance);

/**
 * @brief Decrypts the contents of a file.
 *
 * Reads the file containing hexadecimal-encoded encrypted data, decrypts each BLOCK_SIZE-byte block
 * using the provided AES instance, removes PKCS#7 padding from the final block, and writes
 * the resulting plaintext to the output file.
 *
 * @param inputFileName The name of the file with encrypted data.
 * @param outputFileName The name of the file to write the decrypted plaintext.
 * @param aesInstance An initialized AES instance.
 */
export void DecryptFile(const std::string& inputFileName, const std::string& outputFileName, AES& aesInstance);

/**
 * @brief Reads a 16-character key from standard input.
 *
 * Prompts the user to input a key of exactly BLOCK_SIZE characters and converts it into the
 * internal matrix format required by AES.
 *
 * @param key Output parameter to store the key in matrix form.
 */
export void ReadKey(Key& key);
