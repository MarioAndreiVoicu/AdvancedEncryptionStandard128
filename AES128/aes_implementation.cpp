#include <iostream>
#include <iomanip>
#include <array>
#include <random>
#include <stdexcept>
#include <cstdint>
import aes_interface;


AES::AES(const Key& key) { SetKey(key); }

void AES::SetKey(const Key& key)
{
	mainKey = key;
	KeyExpansion();
}

Key AES::GenerateRandomKey()
{
    std::string charString = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+[{]}|;:',<.>/?";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, charString.size() - 1);
    std::array<Byte, 16> keyArray{};
    for (size_t i = 0; i < 16; i++) {
        keyArray[i] = static_cast<Byte>(charString[dis(gen)]);
    }
    return ArrayToState(keyArray);
}

State AES::ArrayToState(const std::array<Byte, 16>& arr)
{
    State state{};
    for (int j = 0; j < 4; j++)
        for (int i = 0; i < 4; i++)
            state[i][j] = arr[j * 4 + i];
    return state;
}

std::array<Byte, 16> AES::StateToArray(const State& state)
{
    std::array<Byte, 16> arr{};
    for (int j = 0; j < 4; j++)
        for (int i = 0; i < 4; i++)
            arr[j * 4 + i] = state[i][j];
    return arr;
}

void AES::KeyExpansion()
{
    roundKeys[0] = mainKey;
    std::array<Word, 4> prev4Words{};
    for (int j = 0; j < 4; j++)
        for (int i = 0; i < 4; i++)
            prev4Words[j][i] = mainKey[i][j];

    Word lastWord = prev4Words[3];
    for (int indexKey = 1; indexKey < 11; indexKey++)
    {
        for (int indexWord = 0; indexWord < 4; indexWord++)
        {
            WordForRoundKey(lastWord, prev4Words[indexWord], 4 * indexKey + indexWord);
            CopyWordInKey(lastWord, roundKeys[indexKey], indexWord);
            prev4Words[indexWord] = lastWord;
        }
    }
}

void AES::RotWord(Word& word)
{
    Byte temp = word[0];
    word[0] = word[1];
    word[1] = word[2];
    word[2] = word[3];
    word[3] = temp;
}

void AES::SubWord(Word& word)
{
    for (Byte& byte : word)
    {
        int row = (byte >> 4) & 0x0F;   // extracts the first digit and converts it to an integer
        int col = byte & 0x0F;  // extracts the second digit and converts it to an integer
        byte = sBox[row][col];
    }
}

Byte AES::Rcon(int round)
{
    static const std::array<Byte, 10> roundConstants = { 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1b,0x36 };
    return roundConstants[round - 1];
}

void AES::XORwords(Word& word1, const Word& word2)
{
    for (size_t i = 0; i < 4; i++)
        word1[i] ^= word2[i];
}

void AES::WordForRoundKey(Word& word, const Word& wordPrev, int wordNumber)
{
    if (wordNumber % 4 == 0)
    {
        RotWord(word);
        SubWord(word);
        Byte roundConstant = Rcon(wordNumber / 4);
        word[0] ^= roundConstant;
    }
    XORwords(word, wordPrev);
}

void AES::CopyWordInKey(const Word& word, Key& key, int wordNumber)
{
    for (int i = 0; i < 4; i++)
        key[i][wordNumber] = word[i];
}

void AES::SubBytes(State& state)
{
    for (int i = 0; i < 4; i++)
        SubWord(state[i]);
}

void AES::ShiftRows(State& state)
{
    for (int i = 1; i < 4; i++)
    {
        for (int shift = 0; shift < i; shift++)
        {
            Byte temp = state[i][0];
            for (int j = 0; j < 3; j++)
                state[i][j] = state[i][j + 1];
            state[i][3] = temp;
        }
    }
}

Byte AES::gmul(Byte a, Byte b) {
    Byte result = 0;
    for (int i = 0; i < 8; i++) {
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

void AES::MixColumn(Word& column)
{
    Word temp = column;
    column[0] = gmul(temp[0], 2) ^ gmul(temp[3], 1) ^ gmul(temp[2], 1) ^ gmul(temp[1], 3);
    column[1] = gmul(temp[1], 2) ^ gmul(temp[0], 1) ^ gmul(temp[3], 1) ^ gmul(temp[2], 3);
    column[2] = gmul(temp[2], 2) ^ gmul(temp[1], 1) ^ gmul(temp[0], 1) ^ gmul(temp[3], 3);
    column[3] = gmul(temp[3], 2) ^ gmul(temp[2], 1) ^ gmul(temp[1], 1) ^ gmul(temp[0], 3);
}

void AES::MixColumns(State& state)
{
    for (int i = 0; i < 4; i++)
{
        Word col{};
        for (int j = 0; j < 4; j++)
            col[j] = state[j][i];
        MixColumn(col);
        for (int j = 0; j < 4; j++)
            state[j][i] = col[j];
    }
}

void AES::InverseShiftRows(State& state)
{
    for (int i = 1; i < 4; i++)
    {
        for (int shift = 0; shift < i; shift++)
        {
            Byte temp = state[i][3];
            for (int j = 3; j > 0; j--)
                state[i][j] = state[i][j - 1];
            state[i][0] = temp;
        }
    }
}

void AES::InverseSubBytes(State& state) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            Byte byte = state[i][j];
            int row = (byte >> 4) & 0x0F;
            int col = byte & 0x0F;
            state[i][j] = invSBox[row][col];
        }
}

void AES::InverseMixColumns(State& state)
{
    const std::array<std::array<Byte, 4>, 4> invMix{ {
        {0x0e, 0x0b, 0x0d, 0x09},
        {0x09, 0x0e, 0x0b, 0x0d},
        {0x0d, 0x09, 0x0e, 0x0b},
        {0x0b, 0x0d, 0x09, 0x0e}
    } };
    for (int i = 0; i < 4; i++)
    {
        Word col{};
        for (int j = 0; j < 4; j++)
            col[j] = state[j][i];
        for (int j = 0; j < 4; j++)
            state[j][i] = gmul(invMix[j][0], col[0]) ^
            gmul(invMix[j][1], col[1]) ^
            gmul(invMix[j][2], col[2]) ^
            gmul(invMix[j][3], col[3]);
    }
}

void AES::AddRoundKey(State& state, const Key& roundKey)
{
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            state[i][j] ^= roundKey[i][j];
}

void AES::EncryptBlock(State& state)
{
    AddRoundKey(state, roundKeys[0]);
    for (int round = 1; round < 11; round++)
    {
        SubBytes(state);
        ShiftRows(state);
        if (round != 10)
            MixColumns(state);
        AddRoundKey(state, roundKeys[round]);
    }
}

void AES::DecryptBlock(State& state)
{
    AddRoundKey(state, roundKeys[10]);
    for (int round = 9; round >= 1; round--)
    {
        InverseShiftRows(state);
        InverseSubBytes(state);
        AddRoundKey(state, roundKeys[round]);
        InverseMixColumns(state);
    }
    InverseShiftRows(state);
    InverseSubBytes(state);
    AddRoundKey(state, roundKeys[0]);
}