/**
 * @file fileio_implementation.cpp
 * @brief Implementation of the file I/O module for AES encryption/decryption.
 */

//module fileio_interface;

#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <iostream>
#include <array>
#include <vector>
import aes_interface;
import fileio_interface;


namespace
{
    /**
     * @brief Applies PKCS#7 padding to a 16-byte block.
     *
     * If the block is partially filled, the remaining bytes are set to the value
     * (16 - number_of_bytes_read).
     *
     * @param block The block to pad.
     * @param bytesFilled Number of bytes already filled in the block.
     */
    void AddPadding(std::array<Byte, 16>& block, int bytesFilled)
    {
        Byte padding = static_cast<Byte>(16 - bytesFilled);
        for (int i = bytesFilled; i < 16; i++) {
            block[i] = padding;
        }
    }
}


void EncryptFile(const std::string& inputFileName, const std::string& outputFileName, AES& aesInstance)
{
    std::ifstream inFile(inputFileName, std::ios::binary);
    if (!inFile)
        throw std::runtime_error("Error opening input file for encryption.");

    std::ofstream outFile(outputFileName, std::ios::binary);
    if (!outFile)
        throw std::runtime_error("Error opening output file for encryption.");

    std::array<Byte, 16> buffer{};
    while (true)
    {
        inFile.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
        std::streamsize bytesRead = inFile.gcount();
        if (bytesRead <= 0) break;

        // Apply PKCS#7 padding if this is the last (possibly partial) block
        if (bytesRead < 16)
            AddPadding(buffer, static_cast<int>(bytesRead));

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

void DecryptFile(const std::string& inputFileName, const std::string& outputFileName, AES& aesInstance)
{
    std::ifstream inFile(inputFileName);
    if (!inFile)
        throw std::runtime_error("Error opening input file for decryption.");

    std::ofstream outFile(outputFileName, std::ios::binary);
    if (!outFile)
        throw std::runtime_error("Error opening output file for decryption.");

    std::vector<std::array<Byte, 16>> allDecryptedBlocks;

    while (true)
    {
        std::array<Byte, 16> buffer{};
        bool gotFullBlock = true;

        for (int i = 0; i < 16; i++)
        {
            std::string byteStr;
            if (!(inFile >> byteStr)) {
                gotFullBlock = false;
                break;
            }
            buffer[i] = static_cast<Byte>(std::stoi(byteStr, nullptr, 16));
        }

        if (!gotFullBlock) break;

        // Decrypt the block
        State state = AES::ArrayToState(buffer);
        aesInstance.DecryptBlock(state);
        allDecryptedBlocks.push_back(AES::StateToArray(state));
    }

    // Remove PKCS#7 padding from only the last block
    if (!allDecryptedBlocks.empty())
    {
        auto& lastBlock = allDecryptedBlocks.back();
        Byte padding = lastBlock[15];

        if (padding > 0 && padding <= 16)
        {
            for (int i = 16 - padding; i < 16; i++)
                if (lastBlock[i] != padding)
                    throw std::runtime_error("Invalid PKCS#7 padding detected.");
        }
        else
            padding = 0;
        
        // Write all blocks to the output file, trimming the padding in the final block.
        for (size_t b = 0; b < allDecryptedBlocks.size(); b++)
        {
            auto& block = allDecryptedBlocks[b];
            int bytesToWrite = 16;

            // On the final block, skip the padded bytes
            if (b == allDecryptedBlocks.size() - 1)
            {
                bytesToWrite = 16 - static_cast<int>(padding);
            }

            outFile.write(reinterpret_cast<const char*>(block.data()), bytesToWrite);
        }
    }
}

void ReadKey(Key& key) {
    std::cout << "Enter key (16 characters): ";
    std::string keyStr;
    std::getline(std::cin, keyStr);
    if (keyStr.size() != 16)
        throw std::runtime_error("Key must be exactly 16 characters long.");
    std::array<Byte, 16> keyArray{};
    for (size_t i = 0; i < keyStr.size(); i++)
        keyArray[i] = static_cast<Byte>(keyStr[i]);
    key = AES::ArrayToState(keyArray);
}