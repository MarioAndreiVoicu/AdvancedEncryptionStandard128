#include "pch.h"
import aes_interface;

// Test AES encryption and decryption with known data (FIPS 197).
TEST(AESKnownAnswerTest, EncryptDecrypt) {
    // AES-128 key: 2B 7E 15 16 28 AE D2 A6 AB F7 15 88 09 CF 4F 3C
    std::array<Byte, BLOCK_SIZE> keyArray = {
        0x2B, 0x7E, 0x15, 0x16,
        0x28, 0xAE, 0xD2, 0xA6,
        0xAB, 0xF7, 0x15, 0x88,
        0x09, 0xCF, 0x4F, 0x3C
    };
    Key key = AES::ArrayToState(keyArray);
    AES aes(key);

    // Known plaintext: 32 43 F6 A8 88 5A 30 8D 31 31 98 A2 E0 37 07 34
    std::array<Byte, BLOCK_SIZE> plaintextArray = {
        0x32, 0x43, 0xF6, 0xA8,
        0x88, 0x5A, 0x30, 0x8D,
        0x31, 0x31, 0x98, 0xA2,
        0xE0, 0x37, 0x07, 0x34
    };
    State plaintext = AES::ArrayToState(plaintextArray);
    State original = plaintext;

    // Expected ciphertext: 39 25 84 1D 02 DC 09 FB DC 11 85 97 19 6A 0B 32
    std::array<Byte, BLOCK_SIZE> expectedCiphertextArray = {
        0x39, 0x25, 0x84, 0x1D,
        0x02, 0xDC, 0x09, 0xFB,
        0xDC, 0x11, 0x85, 0x97,
        0x19, 0x6A, 0x0B, 0x32
    };
    State expectedCiphertext = AES::ArrayToState(expectedCiphertextArray);

    aes.EncryptBlock(plaintext);
    EXPECT_EQ(expectedCiphertext, plaintext);

    aes.DecryptBlock(plaintext);
    EXPECT_EQ(original, plaintext);
}

// Verify that conversion functions are inverses.
TEST(AESConversionTest, ArrayToStateAndBack) {
    std::array<Byte, BLOCK_SIZE> arr = {
        0x01, 0x23, 0x45, 0x67,
        0x89, 0xAB, 0xCD, 0xEF,
        0xFE, 0xDC, 0xBA, 0x98,
        0x76, 0x54, 0x32, 0x10
    };
    State state = AES::ArrayToState(arr);
    std::array<Byte, BLOCK_SIZE> arr2 = AES::StateToArray(state);
    EXPECT_EQ(arr, arr2);
}

// Ensure random key generation produces different keys.
TEST(AESRandomKeyTest, GenerateRandomKeyProducesDifferentKeys) {
    Key key1 = AES::GenerateRandomKey();
    Key key2 = AES::GenerateRandomKey();
    EXPECT_NE(key1, key2);
}