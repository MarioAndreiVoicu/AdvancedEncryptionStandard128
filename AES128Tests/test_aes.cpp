#include "pch.h"
import aes_interface;
TEST(EncryptionDecryption, BasicEncryptionDecryption) {
    // Initialize AES with a random key.
    Key key = AES::GenerateRandomKey();
    AES aes(key);

    // Define a 16-byte plaintext block.
    State plaintext = { {
        {0x00, 0x11, 0x22, 0x33},
        {0x44, 0x55, 0x66, 0x77},
        {0x88, 0x99, 0xAA, 0xBB},
        {0xCC, 0xDD, 0xEE, 0xFF}
    } };
    State original = plaintext;

    aes.EncryptBlock(plaintext);
    aes.DecryptBlock(plaintext);

    EXPECT_EQ(original, plaintext);
}