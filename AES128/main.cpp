#include <iostream>
#include <string>
import aes_interface;
import fileio_interface;

int main() {
    Key key;
    std::string inputFileName = "input.txt";             // Plaintext file.
    std::string encryptedFileName = "encrypteddata.txt"; // Encrypted output.
    std::string decryptedFileName = "decrypteddata.txt"; // Decrypted output.

    std::cout << "Enter 1 to generate a random key, or 2 to input your key manually: ";
    std::string option;
    std::getline(std::cin, option);
    if (option == "1") {
        key = AES::GenerateRandomKey();
        auto keyArray = AES::StateToArray(key);
        std::cout << "Generated key: ";
        for (auto byte : keyArray)
            std::cout << static_cast<char>(byte);
        std::cout << "\n";
    }
    else if (option == "2") {
        try {
            ReadKey(key);
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    }
    else {
        std::cerr << "Invalid option.\n";
        return 1;
    }

    AES aesInstance(key);

    std::cout << "Enter 1 for encryption or 2 for decryption: ";
    std::getline(std::cin, option);
    try {
        if (option == "1") {
            EncryptFile(inputFileName, encryptedFileName, aesInstance);
            std::cout << "Encryption complete. Data written to " << encryptedFileName << "\n";
        }
        else if (option == "2") {
            DecryptFile(encryptedFileName, decryptedFileName, aesInstance);
            std::cout << "Decryption complete. Data written to " << decryptedFileName << "\n";
        }
        else {
            std::cerr << "Invalid option.\n";
            return 1;
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "Error during file operation: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}