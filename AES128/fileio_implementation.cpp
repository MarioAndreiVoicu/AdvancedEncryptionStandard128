/**
 * @file fileio_implementation.cpp
 * @brief Implementation of the file I/O module for AES encryption and decryption.
 *
 * This file implements functions for encrypting and decrypting files using AES.
 * Data is processed in fixed-size blocks (BLOCK_SIZE), with PKCS#7 padding applied on encryption.
 * Decryption uses a lookahead strategy to process and write each block immediately,
 * ensuring minimal memory usage and correct removal of padding on the final block.
 */

#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <iostream>
#include <array>
#include <vector>
import aes_interface;
import fileio_interface;

namespace {
    /**
     * @brief Applies PKCS#7 padding to a block.
     *
     * If the block is partially filled, the remaining bytes are set to the value (BLOCK_SIZE - bytesFilled).
     *
     * @param block The block to pad.
     * @param bytesFilled The number of bytes already filled in the block.
     */
    void AddPadding(std::array<Byte, BLOCK_SIZE>& block, size_t bytesFilled) {
        Byte padding = BLOCK_SIZE - bytesFilled;
        for (size_t i = bytesFilled; i < BLOCK_SIZE; i++) {
            block[i] = padding;
        }
    }
}

/**
 * @brief Encrypts a file using AES encryption.
 *
 * Reads the plaintext input file in BLOCK_SIZE-byte blocks.
 * If the final block is incomplete, PKCS#7 padding is applied.
 * Each block is then encrypted using the provided AES instance and written to the output file as
 * two-digit hexadecimal values separated by spaces.
 *
 * @param inputFileName The name of the plaintext file.
 * @param outputFileName The name of the output file to write the encrypted data.
 * @param aesInstance An initialized AES encryption instance.
 */
void EncryptFile(const std::string& inputFileName, const std::string& outputFileName, AES& aesInstance) {
    std::ifstream inFile(inputFileName, std::ios::binary);
    if (!inFile) {
        throw std::runtime_error("Error opening input file for encryption.");
    }

    std::ofstream outFile(outputFileName, std::ios::binary);
    if (!outFile) {
        throw std::runtime_error("Error opening output file for encryption.");
    }

    std::array<Byte, BLOCK_SIZE> buffer{};
    while (true) {
        inFile.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
        std::streamsize bytesRead = inFile.gcount();
        if (bytesRead <= 0) {
            break;
        }

        // Apply PKCS#7 padding if this is the final (possibly partial) block.
        if (static_cast<size_t>(bytesRead) < BLOCK_SIZE) {
            AddPadding(buffer, static_cast<size_t>(bytesRead));
        }

        // Encrypt the block.
        State state = AES::ArrayToState(buffer);
        aesInstance.EncryptBlock(state);
        auto outBlock = AES::StateToArray(state);

        // Write each byte as two-digit hexadecimal values, separated by a space.
        for (auto byte : outBlock) {
            outFile << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(byte) << " ";
        }
    }
}

/**
 * @brief Decrypts a file encrypted with AES.
 *
 * Reads the input file containing hexadecimal-encoded encrypted data in BLOCK_SIZE-byte blocks.
 * Uses a lookahead strategy: the first block is decrypted, then the next block is read to determine
 * if the current block is final. If not, the decrypted block is written immediately.
 * For the final block, PKCS#7 padding is removed before writing the valid plaintext bytes.
 *
 * @param inputFileName The name of the file containing encrypted data.
 * @param outputFileName The name of the file to write the decrypted plaintext.
 * @param aesInstance An initialized AES instance.
 */
void DecryptFile(const std::string& inputFileName, const std::string& outputFileName, AES& aesInstance) {
    std::ifstream inFile(inputFileName);
    if (!inFile) {
        throw std::runtime_error("Error opening input file for decryption.");
    }

    std::ofstream outFile(outputFileName, std::ios::binary);
    if (!outFile) {
        throw std::runtime_error("Error opening output file for decryption.");
    }

    // Buffer for the current encrypted block.
    std::array<Byte, BLOCK_SIZE> currentEncryptedBlock{};
    // Buffer for the decrypted version of the current block.
    std::array<Byte, BLOCK_SIZE> currentDecryptedBlock{};

    // Read and decrypt the first block.
    bool gotFullBlock = true;
    for (size_t i = 0; i < BLOCK_SIZE; i++) {
        std::string byteStr;
        if (!(inFile >> byteStr)) {
            gotFullBlock = false;
            break;
        }
        currentEncryptedBlock[i] = static_cast<Byte>(std::stoul(byteStr, nullptr, 16));
    }

    if (!gotFullBlock) {
        // No complete block found; nothing to decrypt.
        return;
    }

    {
        // Convert the current block to a state, decrypt it, and convert it back.
        State state = AES::ArrayToState(currentEncryptedBlock);
        aesInstance.DecryptBlock(state);
        currentDecryptedBlock = AES::StateToArray(state);
    }

    // Process remaining blocks using a lookahead strategy.
    while (true) {
        // Buffer for the next encrypted block.
        std::array<Byte, BLOCK_SIZE> nextEncryptedBlock{};
        bool gotNextBlock = true;
        for (size_t i = 0; i < BLOCK_SIZE; i++) {
            std::string byteStr;
            if (!(inFile >> byteStr)) {
                gotNextBlock = false;
                break;
            }
            nextEncryptedBlock[i] = static_cast<Byte>(std::stoul(byteStr, nullptr, 16));
        }

        if (gotNextBlock) {
            // A subsequent block exists; the current block is not final.
            // Write the decrypted current block as-is.
            outFile.write(reinterpret_cast<const char*>(currentDecryptedBlock.data()), BLOCK_SIZE);

            // Decrypt the next block and update the current block.
            currentEncryptedBlock = nextEncryptedBlock;
            State state = AES::ArrayToState(currentEncryptedBlock);
            aesInstance.DecryptBlock(state);
            currentDecryptedBlock = AES::StateToArray(state);
        }
        else {
            // Final block reached; remove PKCS#7 padding.
            Byte padding = currentDecryptedBlock[BLOCK_SIZE - 1];
            if (padding > 0 && padding <= BLOCK_SIZE) {
                for (size_t i = BLOCK_SIZE - padding; i < BLOCK_SIZE; i++) {
                    if (currentDecryptedBlock[i] != padding) {
                        throw std::runtime_error("Invalid PKCS#7 padding detected.");
                    }
                }
            }
            else {
                padding = 0;
            }
            size_t bytesToWrite = BLOCK_SIZE - padding;
            outFile.write(reinterpret_cast<const char*>(currentDecryptedBlock.data()), bytesToWrite);
            break;
        }
    }
}

/**
 * @brief Reads a 16-character key from standard input and converts it to AES key format.
 *
 * Prompts the user for a key of exactly BLOCK_SIZE characters and converts it into the internal
 * state (matrix) format required by AES.
 *
 * @param key Output parameter to store the key in the AES state format.
 */
void ReadKey(Key& key) {
    std::cout << "Enter key (16 characters): ";
    std::string keyStr;
    std::getline(std::cin, keyStr);
    if (keyStr.size() != BLOCK_SIZE) {
        throw std::runtime_error("Key must be exactly 16 characters long.");
    }
    std::array<Byte, BLOCK_SIZE> keyArray{};
    for (size_t i = 0; i < keyStr.size(); i++) {
        keyArray[i] = static_cast<Byte>(keyStr[i]);
    }
    key = AES::ArrayToState(keyArray);
}
