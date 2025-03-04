/**
 * @file fileio_implementation.cpp
 * @brief Implementation of the file I/O module for AES encryption/decryption.
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
     * @brief Applies PKCS#7 padding to a BLOCK_SIZE-byte block.
     *
     * If the block is partially filled, the remaining bytes are set to the value
     * (BLOCK_SIZE - bytesFilled).
     *
     * @param block The block to pad.
     * @param bytesFilled Number of bytes already filled in the block.
     */
    void AddPadding(std::array<Byte, BLOCK_SIZE>& block, size_t bytesFilled) {
        Byte padding = BLOCK_SIZE - bytesFilled;
        for (size_t i = bytesFilled; i < BLOCK_SIZE; i++) {
            block[i] = padding;
        }
    }
}


void EncryptFile(const std::string& inputFileName, const std::string& outputFileName, AES& aesInstance) {
    std::ifstream inFile(inputFileName, std::ios::binary);
    if (!inFile)
        throw std::runtime_error("Error opening input file for encryption.");

    std::ofstream outFile(outputFileName, std::ios::binary);
    if (!outFile)
        throw std::runtime_error("Error opening output file for encryption.");

    std::array<Byte, BLOCK_SIZE> buffer{};
    while (true) {
        inFile.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
        std::streamsize bytesRead = inFile.gcount();
        if (bytesRead <= 0) {
            break;
        }

        // Apply PKCS#7 padding if this is the last (possibly partial) block
        if (static_cast<size_t>(bytesRead) < BLOCK_SIZE) {
            AddPadding(buffer, static_cast<size_t>(bytesRead));
        }

        // Encrypt the block
        State state = AES::ArrayToState(buffer);
        aesInstance.EncryptBlock(state);
        auto outBlock = AES::StateToArray(state);

        // Write each byte as two-digit hexadecimal values, separated by a space.
        for (auto byte : outBlock) {
            outFile << std::hex
                << std::setw(2)
                << std::setfill('0')
                << static_cast<int>(byte)
                << " ";
        }
    }
}

void DecryptFile(const std::string& inputFileName, const std::string& outputFileName, AES& aesInstance) {
    std::ifstream inFile(inputFileName);
    if (!inFile) {
        throw std::runtime_error("Error opening input file for decryption.");
    }

    std::ofstream outFile(outputFileName, std::ios::binary);
    if (!outFile) {
        throw std::runtime_error("Error opening output file for decryption.");
    }

    std::vector<std::array<Byte, BLOCK_SIZE>> allDecryptedBlocks;

    while (true) {
        std::array<Byte, BLOCK_SIZE> buffer{};
        bool gotFullBlock = true;

        for (size_t i = 0; i < BLOCK_SIZE; i++) {
            std::string byteStr;
            if (!(inFile >> byteStr)) {
                gotFullBlock = false;
                break;
            }
            buffer[i] = static_cast<Byte>(std::stoul(byteStr, nullptr, 16));
        }

        if (!gotFullBlock) {
            break;
        }

        // Decrypt the block.
        State state = AES::ArrayToState(buffer);
        aesInstance.DecryptBlock(state);
        allDecryptedBlocks.push_back(AES::StateToArray(state));
    }

    // Remove PKCS#7 padding from only the last block.
    if (!allDecryptedBlocks.empty()) {
        auto& lastBlock = allDecryptedBlocks.back();
        Byte padding = lastBlock[BLOCK_SIZE - 1];

        if (padding > 0 && padding <= BLOCK_SIZE) {
            for (size_t i = BLOCK_SIZE - padding; i < BLOCK_SIZE; i++)
                if (lastBlock[i] != padding) {
                    throw std::runtime_error("Invalid PKCS#7 padding detected.");
                }
        }
        else {
            padding = 0;
        }
        
        // Write all blocks to the output file, trimming padding in the final block.
        for (size_t b = 0; b < allDecryptedBlocks.size(); b++) {
            auto& block = allDecryptedBlocks[b];
            size_t bytesToWrite = BLOCK_SIZE;

            // On the final block, skip the padded bytes
            if (b == allDecryptedBlocks.size() - 1) {
                bytesToWrite = BLOCK_SIZE - padding;
            }

            outFile.write(reinterpret_cast<const char*>(block.data()), bytesToWrite);
        }
    }
}

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