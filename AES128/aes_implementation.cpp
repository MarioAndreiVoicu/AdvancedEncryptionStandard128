/**
 * @file aes_implementation.cpp
 * @brief Implementation of AES-128 encryption and decryption methods.
 *
 * This file implements the AES class defined in the aes_interface module.
 * It includes key expansion, encryption and decryption of a 16-byte block,
 * and various internal transformations.
 */

#include <iostream>
#include <iomanip>
#include <array>
#include <random>
#include <stdexcept>
#include <cstdint>
import aes_interface;

AES::AES(const Key& key) { SetKey(key); }

void AES::SetKey(const Key& key) {
	mainKey = key;
	KeyExpansion();
}

Key AES::GenerateRandomKey() {
    std::string charString = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+[{]}|;:',<.>/?";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, charString.size() - 1);
    std::array<Byte, BLOCK_SIZE> keyArray{};
    for (size_t i = 0; i < BLOCK_SIZE; i++) {
        keyArray[i] = static_cast<Byte>(charString[dis(gen)]);
    }
    return ArrayToState(keyArray);
}

State AES::ArrayToState(const std::array<Byte, BLOCK_SIZE>& arr) {
    State state{};
    for (size_t j = 0; j < MATRIX_DIM; j++) {
        for (size_t i = 0; i < MATRIX_DIM; i++) {
            state[i][j] = arr[j * MATRIX_DIM + i];
        }
    }
    return state;
}

std::array<Byte, BLOCK_SIZE> AES::StateToArray(const State& state) {
    std::array<Byte, BLOCK_SIZE> arr{};
    for (size_t j = 0; j < MATRIX_DIM; j++) {
        for (size_t i = 0; i < MATRIX_DIM; i++) {
            arr[j * MATRIX_DIM + i] = state[i][j];
        }
    }
    return arr;
}

void AES::KeyExpansion() {
    roundKeys[0] = mainKey;
    std::array<Word, MATRIX_DIM> prev4Words{};
    for (size_t j = 0; j < MATRIX_DIM; j++) {
        for (size_t i = 0; i < MATRIX_DIM; i++) {
            prev4Words[j][i] = mainKey[i][j];
        }
    }

    Word lastWord = prev4Words[MATRIX_DIM - 1];
    for (size_t indexKey = 1; indexKey < NUM_ROUND_KEYS; indexKey++) {
        for (size_t indexWord = 0; indexWord < MATRIX_DIM; indexWord++) {
            WordForRoundKey(lastWord, prev4Words[indexWord], 4 * indexKey + indexWord);
            CopyWordInKey(lastWord, roundKeys[indexKey], indexWord);
            prev4Words[indexWord] = lastWord;
        }
    }
}

void AES::RotWord(Word& word) {
    Byte temp = word[0];
    word[0] = word[1];
    word[1] = word[2];
    word[2] = word[3];
    word[3] = temp;
}

void AES::SubWord(Word& word) {
    for (Byte& byte : word) {
        int row = (byte >> 4) & 0x0F;   // extracts the first digit and converts it to an integer
        int col = byte & 0x0F;  // extracts the second digit and converts it to an integer
        byte = sBox[row][col];
    }
}

Byte AES::Rcon(size_t round) {
    static const std::array<Byte, 10> roundConstants = { 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1b,0x36 };
    return roundConstants[round - 1];
}

void AES::XORwords(Word& word1, const Word& word2) {
    for (size_t i = 0; i < MATRIX_DIM; i++) {
        word1[i] ^= word2[i];
    }
}

void AES::WordForRoundKey(Word& word, const Word& wordPrev, size_t wordNumber) {
    if (wordNumber % MATRIX_DIM == 0) {
        RotWord(word);
        SubWord(word);
        Byte roundConstant = Rcon(wordNumber / MATRIX_DIM);
        word[0] ^= roundConstant;
    }
    XORwords(word, wordPrev);
}

void AES::CopyWordInKey(const Word& word, Key& key, size_t wordNumber) {
    for (size_t i = 0; i < MATRIX_DIM; i++) {
        key[i][wordNumber] = word[i];
    }
}

void AES::SubBytes(State& state) {
    for (size_t i = 0; i < MATRIX_DIM; i++) {
        SubWord(state[i]);
    }
}

void AES::ShiftRows(State& state)
{
    for (size_t i = 1; i < MATRIX_DIM; i++) {
        for (size_t shift = 0; shift < i; shift++) {
            Byte temp = state[i][0];
            for (size_t j = 0; j < MATRIX_DIM - 1; j++) {
                state[i][j] = state[i][j + 1];
            }
            state[i][MATRIX_DIM - 1] = temp;
        }
    }
}

Byte AES::gmul(Byte a, Byte b) {
    Byte result = 0;
    for (size_t i = 0; i < 8; i++) {
        if (b & 1)
            result ^= a;
        Byte hi_bit = a & 0x80;
        a <<= 1;
        if (hi_bit)
            a ^= 0x1b;
        b >>= 1;
    }
    return result;
}

void AES::MixColumn(Word& column) {
    Word temp = column;
    column[0] = gmul(temp[0], 2) ^ gmul(temp[3], 1) ^ gmul(temp[2], 1) ^ gmul(temp[1], 3);
    column[1] = gmul(temp[1], 2) ^ gmul(temp[0], 1) ^ gmul(temp[3], 1) ^ gmul(temp[2], 3);
    column[2] = gmul(temp[2], 2) ^ gmul(temp[1], 1) ^ gmul(temp[0], 1) ^ gmul(temp[3], 3);
    column[3] = gmul(temp[3], 2) ^ gmul(temp[2], 1) ^ gmul(temp[1], 1) ^ gmul(temp[0], 3);
}

void AES::MixColumns(State& state) {
    for (size_t i = 0; i < MATRIX_DIM; i++) {
        Word col{};
        for (size_t j = 0; j < MATRIX_DIM; j++) {
            col[j] = state[j][i];
        }
        MixColumn(col);
        for (size_t j = 0; j < MATRIX_DIM; j++) {
            state[j][i] = col[j];
        }
    }
}

void AES::InverseShiftRows(State& state) {
    for (size_t i = 1; i < MATRIX_DIM; i++) {
        for (size_t shift = 0; shift < i; shift++) {
            Byte temp = state[i][MATRIX_DIM - 1];
            for (size_t j = MATRIX_DIM - 1; j > 0; j--) {
                state[i][j] = state[i][j - 1];
            }
            state[i][0] = temp;
        }
    }
}

void AES::InverseSubBytes(State& state) {
    for (size_t i = 0; i < MATRIX_DIM; i++) {
        for (size_t j = 0; j < MATRIX_DIM; j++) {
            Byte byte = state[i][j];
            int row = (byte >> 4) & 0x0F;
            int col = byte & 0x0F;
            state[i][j] = invSBox[row][col];
        }
    }
}

void AES::InverseMixColumns(State& state) {
    const std::array<std::array<Byte, MATRIX_DIM>, MATRIX_DIM> invMix{ {
        {0x0e, 0x0b, 0x0d, 0x09},
        {0x09, 0x0e, 0x0b, 0x0d},
        {0x0d, 0x09, 0x0e, 0x0b},
        {0x0b, 0x0d, 0x09, 0x0e}
    } };
    for (size_t i = 0; i < MATRIX_DIM; i++) {
        Word col{};
        for (size_t j = 0; j < MATRIX_DIM; j++) {
            col[j] = state[j][i];
        }
        for (size_t j = 0; j < MATRIX_DIM; j++) {
            state[j][i] = gmul(invMix[j][0], col[0]) ^
                gmul(invMix[j][1], col[1]) ^
                gmul(invMix[j][2], col[2]) ^
                gmul(invMix[j][3], col[3]);
        }
    }
}

void AES::AddRoundKey(State& state, const Key& roundKey) {
    for (size_t i = 0; i < MATRIX_DIM; i++) {
        for (size_t j = 0; j < MATRIX_DIM; j++) {
            state[i][j] ^= roundKey[i][j];
        }
    }
}

void AES::EncryptBlock(State& state) {
    AddRoundKey(state, roundKeys[0]);
    for (size_t round = 1; round < NUM_ROUND_KEYS; round++) {
        SubBytes(state);
        ShiftRows(state);
        if (round != NUM_ROUND_KEYS - 1) {
            MixColumns(state);
        }
        AddRoundKey(state, roundKeys[round]);
    }
}

void AES::DecryptBlock(State& state) {
    AddRoundKey(state, roundKeys[NUM_ROUND_KEYS - 1]);
    for (int round = static_cast<int>(NUM_ROUND_KEYS) - 2; round >= 1; round--) {
        InverseShiftRows(state);
        InverseSubBytes(state);
        AddRoundKey(state, roundKeys[round]);
        InverseMixColumns(state);
    }
    InverseShiftRows(state);
    InverseSubBytes(state);
    AddRoundKey(state, roundKeys[0]);
}