#include "pch.h"
import aes_interface;
import fileio_interface;
#include <fstream>

// Read the entire file into a string.
std::string readFileToString(const std::string& filename) {
    std::ifstream ifs(filename, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(ifs)),
        std::istreambuf_iterator<char>());
}

TEST(FileIOTest, EncryptThenDecryptFile) {
    // Define fixed temporary file names.
    std::string inputFile = "temp_test_input.txt";
    std::string encryptedFile = "temp_test_encrypted.txt";
    std::string decryptedFile = "temp_test_decrypted.txt";

    std::string expectedContent = "This is a test input for AES encryption!";

    // Write the expected content to the input file.
    {
        std::ofstream outputFile(inputFile, std::ios::binary);
        outputFile << expectedContent;
    }

    Key key = AES::GenerateRandomKey();
    AES aes(key);

    EncryptFile(inputFile, encryptedFile, aes);
    DecryptFile(encryptedFile, decryptedFile, aes);

    std::string decryptedContent = readFileToString(decryptedFile);
    EXPECT_EQ(expectedContent, decryptedContent);

    // Clean up temporary files.
    std::remove(inputFile.c_str());
    std::remove(encryptedFile.c_str());
    std::remove(decryptedFile.c_str());
}