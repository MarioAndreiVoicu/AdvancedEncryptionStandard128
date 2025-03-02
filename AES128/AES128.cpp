#include <iostream>
#include <iomanip>
#include <random>
#include <array>
#include<fstream>
#include<string>

typedef std::array<uint8_t, 4> Word;  //a word is made of 4 bytes
typedef std::array<Word, 4> State;    //the state matrix is a 4x4 matrix where each column is a word
typedef std::array<std::array<uint8_t, 4>, 4> Key;

State ArrayToMatrix(const std::array<uint8_t, 16>& array)
{
    Key matrix;
    for (int j = 0; j < 4; j++)
        for (int i = 0; i < 4; i++)
            matrix[i][j] = array[j * 4 + i];

    return matrix;
}

// step 1) generate random key if needed
Key generateRandomKey()
{
    const std::string charString = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+[{]}|;:',<.>/?";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, charString.length() - 1);

    std::string keyString;
    for (int i = 0; i < 16; i++)
        keyString.push_back(charString[dis(gen)]);

    std::array<uint8_t, 16> keyArray;
    for (int i = 0; i < 16; i++)
        keyArray[i] = static_cast<uint8_t>(keyString[i]);

    Key key = ArrayToMatrix(keyArray);

    return key;
}

std::array<uint8_t, 16> MatrixToArray(const State& matrix)
{
    std::array<uint8_t, 16> array;

    for (int j = 0; j < 4; j++)
        for (int i = 0; i < 4; i++)
            array[j * 4 + i] = matrix[i][j];

    return array;
}

//step 2: Generate the round keys
//step 2.1  RotWord: performs a left circular shift by rotating the bytes in each word of each round key
void RotWord(Word& word)
{
    uint8_t temp = word[0];
    word[0] = word[1];
    word[1] = word[2];
    word[2] = word[3];
    word[3] = temp;
}

//step 2.2 each byte in the word is replaced using this substition box. The first value from the byte is the row and the second one is the column so 0xcf becomes the value stored at row c(=12) column f(=15) so 0x8a
void SubWord(Word& word)
{
    const std::array<std::array<uint8_t, 16>, 16> substitutionBox =
    { {
        {0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76},
        {0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0},
        {0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15},
        {0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75},
        {0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84},
        {0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf},
        {0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8},
        {0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2},
        {0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73},
        {0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb},
        {0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79},
        {0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08},
        {0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a},
        {0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e},
        {0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf},
        {0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16}
    } };

    for (uint8_t& byte : word)
    {
        int row = (byte >> 4) & 0x0F;  // extracts the first digit and converts it to an integer,which will be the row
        int column = byte & 0x0F;  // extracts the second digit and converts it to an integer,which will be the column
        byte = substitutionBox[row][column];
    }
}

//step 2.3: the first byte of the first word of each roundKey is XOR'd with round constant that changes for every key
uint8_t Rcon(int round)
{
    const std::array<uint8_t, 10> roundConstants = { 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1b,0x36 };

    return roundConstants[round - 1];
}

//step 2.4: each word is XOR'd with the previous word to make the final word for the key
void XORwords(Word& word1, Word& word2)
{
    for (size_t i = 0; i < word1.size(); i++)
        word1[i] = word1[i] ^ word2[i];
}

//this function transforms the words for the keys
void WordForRoundKey(Word& word, Word& wordPrev4Indexes, int wordNumber)  //the parameter word will have the value of the previous word.So word 6 will be created from word 5
{
    if (wordNumber % 4 == 0) //these three apply only to the first word of each key
    {
        RotWord(word);
        SubWord(word);
        uint8_t roundConstant = Rcon(wordNumber / 4);
        word[0] = word[0] ^ roundConstant;
    }
    XORwords(word, wordPrev4Indexes);  //this applies to all words
}

//when generating the round keys,the words are stored in an array,so on the rows.But in the key they will be stored on the columns
void CopyWordInKey(const Word& word, std::array<Key, 11>& roundKeys, int roundKeyNumber, int wordNumber)  //the word is stored in a row and it needs to be transfered in a column in a key
{
    for (int i = 0; i < 4; i++)
        roundKeys[roundKeyNumber][i][wordNumber] = word[i];
}

//this function generates the round keys based on the main key
void RoundKeys(const Key& mainKey, std::array<Key, 11>& roundKeys)
{
    roundKeys[0] = mainKey;  //the first key is the mainKey

    std::array<Word, 4> prev4Words; //this contains the last 4 words before the one that is being worked on now

    for (int j = 0; j < 4; j++)        //the key provides the 4 words and the rest of the words are made from these 4
    {
        for (int i = 0; i < 4; i++)  //each column from the key is a word,so column 0 is word 0,column1 is word 1
        {
            prev4Words[i][j] = mainKey[j][i];  //prev4Words is a 4x4 matrix that contains the last 4 words,but the words are in each row instead of each column like it is in the state
        }
    }

    Word lastWord = prev4Words[3];
    for (int indexKey = 1; indexKey < 11; indexKey++)
    {
        for (int indexWord = 0; indexWord < 4; indexWord++)
        {
            WordForRoundKey(lastWord, prev4Words[indexWord], 4 * indexKey + indexWord);
            CopyWordInKey(lastWord, roundKeys, indexKey, indexWord);
            prev4Words[indexWord] = lastWord;
        }
    }
}

//step 3
//step 3.1: each byte in the state matrix is replaced using the S-box
void SubBytes(State& stateMatrix)
{
    for (int i = 0; i < 4; i++)
        SubWord(stateMatrix[i]);  //SubBytes does the same thing as SubWord but on the whole state matrix,it takes each byte and replaces it with the corresponding byte from the substitution box
}

//step 3.2: A left circualr shift is performed on each row of the matrix.Row 0 by 0 bytes,row 1 by 1 byte,row 2 by 2 bytes and row 3 by 3 bytes
void ShiftRows(State& stateMatrix)
{
    for (int indexRow = 0; indexRow < 4; indexRow++)
        for (int shiftCount = 0; shiftCount < indexRow; shiftCount++)
            RotWord(stateMatrix[indexRow]);  //it rotates the row circularly,so the RotWord can be used to achieve the same result
}

//step 3.3 The key in the 2d matrix form is multiplied by another 4x4 matrix with constant values
uint8_t gmul(uint8_t num1, uint8_t num2)  //this function multiplies 2 values in the Galois Field 
{
    uint8_t result = 0;
    uint8_t counter;
    uint8_t hiBitSet;

    for (counter = 0; counter < 8; counter++)
    {
        if ((num2 & 1) == 1)
            result ^= num1;

        hiBitSet = (num1 & 0x80);
        num1 <<= 1;

        if (hiBitSet == 0x80)
            num1 ^= 0x1b; // irreducible polynomial

        num2 >>= 1;
    }
    return result;
}

// MixColumns operation on a single column
void MixColumn(Word& column)
{
    uint8_t temp[4];
    for (int i = 0; i < 4; i++)
    {
        temp[i] = column[i];
    }
    column[0] = gmul(temp[0], 2) ^ gmul(temp[3], 1) ^ gmul(temp[2], 1) ^ gmul(temp[1], 3);
    column[1] = gmul(temp[1], 2) ^ gmul(temp[0], 1) ^ gmul(temp[3], 1) ^ gmul(temp[2], 3);
    column[2] = gmul(temp[2], 2) ^ gmul(temp[1], 1) ^ gmul(temp[0], 1) ^ gmul(temp[3], 3);
    column[3] = gmul(temp[3], 2) ^ gmul(temp[2], 1) ^ gmul(temp[1], 1) ^ gmul(temp[0], 3);
}

//this function does the MixColumn operation on the state matrix,which means the state matrix is multiplied by a specific 4x4 matrix
void MixColumns(State& state)
{
    for (int i = 0; i < 4; i++)
    {
        Word column;
        for (int j = 0; j < 4; j++)
        {
            column[j] = state[j][i];
        }
        MixColumn(column);
        for (int j = 0; j < 4; j++) {
            state[j][i] = column[j];
        }
    }
}

//this function is needed to XOR the values between the state matrix and the round key
void MatrixesXOR(State& state, Key& roundKey)
{
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            state[i][j] = state[i][j] ^ roundKey[i][j];
}

//this function takes an input of 16 bytes and a key of 16 bytes and encrypts it using AES-128
void EncryptAES128(std::array<std::array<uint8_t, 4>, 4>& input, Key& mainKey)
{
    std::array<Key, 11> roundKeys;
    RoundKeys(mainKey, roundKeys);

    MatrixesXOR(input, roundKeys[0]); //for the first iteration only the XOR with the round key is performed
    for (int i = 1; i < 11; i++)
    {
        SubBytes(input);
        ShiftRows(input);

        if (i != 10)  //for the last iteration the columns won't be mixed because it won't make a difference
            MixColumns(input);
        MatrixesXOR(input, roundKeys[i]);
    }
}



//Decryption part

// Inverse ShiftRows function
void InverseShiftRows(State& state)
{
    for (int i = 1; i < 4; i++)
    {
        for (int k = 0; k < i; k++)
        {
            uint8_t temp = state[i][3];
            state[i][3] = state[i][2];
            state[i][2] = state[i][1];
            state[i][1] = state[i][0];
            state[i][0] = temp;
        }
    }
}

// Inverse SubBytes function
void InverseSubBytes(State& state)
{
    const std::array<std::array<uint8_t, 16>, 16> inverseSubstitutionBox = {
    {
        { 0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb },
        { 0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb },
        { 0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e },
        { 0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25 },
        { 0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92 },
        { 0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84 },
        { 0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06 },
        { 0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b },
        { 0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73 },
        { 0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e },
        { 0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b },
        { 0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4 },
        { 0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f },
        { 0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef },
        { 0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61 },
        { 0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d }
    }
    };

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            uint8_t byte = state[i][j];
            int row = (byte >> 4) & 0x0F;
            int column = byte & 0x0F;
            state[i][j] = inverseSubstitutionBox[row][column];
        }
    }
}

// InverseMixColumns function: here the state matrix is multiplied by a constant 4x4 matrix
void InverseMixColumns(State& state)
{
    const std::array<std::array<uint8_t, 4>, 4> inverseMixColumnMatrix = {
        { {0x0e, 0x0b, 0x0d, 0x09},
          {0x09, 0x0e, 0x0b, 0x0d},
          {0x0d, 0x09, 0x0e, 0x0b},
          {0x0b, 0x0d, 0x09, 0x0e} }
    };

    Word temp;  // each column is stored in temp so it can be calculated sparately

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            temp[j] = state[j][i];
        }

        for (int j = 0; j < 4; j++)
        {
            state[j][i] = gmul(inverseMixColumnMatrix[j][0], temp[0]) ^
                gmul(inverseMixColumnMatrix[j][1], temp[1]) ^
                gmul(inverseMixColumnMatrix[j][2], temp[2]) ^
                gmul(inverseMixColumnMatrix[j][3], temp[3]);
        }
    }
}

void AddRoundKey(State& state, const Key& roundKey)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            state[i][j] ^= roundKey[i][j];
        }
    }
}

void DecryptAES128(State& input, Key& mainKey)
{
    std::array<Key, 11> roundKeys;
    RoundKeys(mainKey, roundKeys);

    AddRoundKey(input, roundKeys[10]);

    for (int round = 9; round >= 1; round--)
    {
        InverseShiftRows(input);
        InverseSubBytes(input);
        AddRoundKey(input, roundKeys[round]);
        InverseMixColumns(input);
    }

    InverseShiftRows(input);
    InverseSubBytes(input);
    AddRoundKey(input, roundKeys[0]);
}

void WriteEncryptedDataToFile(const std::array<uint8_t, 16>& encryptedData, std::ofstream& outFile, bool lastBlock = false)
{
    // Write the hexadecimal representation of each byte to the file
    if (!lastBlock)
    {
        for (const auto& byte : encryptedData)
        {
            outFile << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
        }
    }
    else  //if it is the last block of data,then the empty space after the last byte won't be written in the file
    {
        for (int i = 0; i < 15; i++)
            outFile << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(encryptedData[i]) << " ";
        outFile << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(encryptedData[15]);

    }
}

// AES128 encryption works on blocks of size 16,so if the plaintext doesn't have 16 bytes padding is needed
void AddPadding(std::array<uint8_t, 16>& array, int bytesWritten)  //the padding method is PKCS#7
{
    uint8_t paddingValue = static_cast<uint8_t>(16 - bytesWritten);

    for (int i = bytesWritten; i < 16; i++)
        array[i] = paddingValue;
}

//this function encrypts a file's data and puts the output in byte form in a specified file
void EncryptFileAES(const std::string& inputFileName, const std::string& outputFileName, Key& mainKey)
{
    std::ifstream inputFile(inputFileName);
    if (!inputFile.is_open())
        throw std::runtime_error("Error: The file with data that needs to be encrypted could not be opened.");

    std::ofstream file(outputFileName, std::ofstream::out | std::ofstream::trunc);     // open the file where the encrypted data will be written in output mode to clear its contents
    if (!file.is_open())
        throw std::runtime_error("Error: The file for writing the encrypted data could not be opened.");
    file.close();

    std::ofstream outFile(outputFileName, std::ios::app);  //the file is opened in append mode for multiple blocks of 16 bytes to be written
    if (!outFile.is_open())
        throw std::runtime_error("Error: The output file for the encrypted data could not be opened.");

    char character;
    std::array<uint8_t, 16> inputArray;
    int size = 0;

    while (inputFile.get(character)) // get() is used to read each character including spaces
    {
        std::array<uint8_t, 16> output;
        if (size != 16)
        {
            uint8_t byte = static_cast<uint8_t>(character); //read the characters in the file and convert to uint8_t
            inputArray[size] = byte;
            size++;
        }
        else
        {
            State input = ArrayToMatrix(inputArray);
            EncryptAES128(input, mainKey);
            output = MatrixToArray(input);
            WriteEncryptedDataToFile(output, outFile);

            uint8_t byte = static_cast<uint8_t>(character); //read the characters in the file and convert to uint8_t
            inputArray[0] = byte;
            size = 1;
        }
    }
    inputFile.close();

    if (size == 16)  //encrypt the last block and add a padding block
    {
        State input = ArrayToMatrix(inputArray);
        EncryptAES128(input, mainKey);
        std::array<uint8_t, 16> output = MatrixToArray(input);
        WriteEncryptedDataToFile(output, outFile);

        std::array<uint8_t, 16> paddingBlock;
        AddPadding(paddingBlock, 0);  //a block made of padding is made so the decryption algorithm knows how many characters to remove from the final plaintext
        State paddingMatrix = ArrayToMatrix(paddingBlock);  //the block is transformed into a matrix so it can be encrypted
        EncryptAES128(paddingMatrix, mainKey);
        output = MatrixToArray(paddingMatrix);
        WriteEncryptedDataToFile(output, outFile, true);
    }
    else  //for the case where the text is not exactly a multiple of 16,then padding is added to the block to make it 16 bytes
    {
        AddPadding(inputArray, size);
        State input = ArrayToMatrix(inputArray);
        EncryptAES128(input, mainKey);
        std::array<uint8_t, 16> output = MatrixToArray(input);
        WriteEncryptedDataToFile(output, outFile, true);
    }

    outFile.close();
}

void WriteDecryptedDataToFile(const std::array<uint8_t, 16>& decryptedData, std::ofstream& outFile, bool hasPadding = false)
{
    if (!hasPadding)
    {
        for (const uint8_t& byte : decryptedData)
            outFile << static_cast<char>(byte);  //the bytes are converted into chars so the plaintext is displayed
    }
    else  //if it is the last block,it has padding and it won't be displayed
    {
        int paddingAdded = decryptedData[15];
        for (int i = 0; i < 16 - paddingAdded; i++)
        {
            uint8_t byte = decryptedData[i];
            outFile << static_cast<char>(byte);
        }
    }
}

void DecryptFileAES(const std::string& inputFileName, const std::string& outputFileName, Key& mainKey)
{
    std::ifstream file(inputFileName, std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Error: The file with the encrypted data could not be opened.");

    std::ofstream fileClear(outputFileName, std::ofstream::out | std::ofstream::trunc);     // open the file where the decrypted data will be written in output mode to clear its contents
    if (!fileClear.is_open())
        throw std::runtime_error("Error: The file for writing the decrypted data could not be opened.");
    fileClear.close();

    std::ofstream outFile(outputFileName, std::ios::app);
    if (!outFile.is_open())
        throw std::runtime_error("Error: The output file for the decrypted data could not be opened.");

    std::array<uint8_t, 16> dataArray;
    size_t bytesRead = 0;
    while (!file.eof())
    {
        std::string byteString;
        file >> byteString;
        uint8_t byte = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));

        dataArray[bytesRead++] = byte;
        if (bytesRead == 16)
        {
            if (!file.eof())
            {
                std::array<uint8_t, 16> output;
                State input = ArrayToMatrix(dataArray);
                DecryptAES128(input, mainKey);
                output = MatrixToArray(input);
                WriteDecryptedDataToFile(output, outFile, false);
                bytesRead = 0; // Reset bytesRead for the next block
            }
            else  //if it reached the end of the file,then this is the last block and it contains padding
            {
                std::array<uint8_t, 16> output;
                State input = ArrayToMatrix(dataArray);
                DecryptAES128(input, mainKey);
                output = MatrixToArray(input);
                WriteDecryptedDataToFile(output, outFile, true);
                bytesRead = 0; // Reset bytesRead for the next block
            }
        }
    }
    if (bytesRead != 0)  //if there was a block of that with less that 16 bytes,the encrypted data was in the wrong format
        throw std::runtime_error("Error: Unexpected number of bytes read.");

    file.close();
    outFile.close();
}

void ReadKey(Key& mainKey)
{
    std::string option;  //the option is read as a string to cover invalid input cases like other values outside of 1 and 2 or multiple values inserted at once
    std::cout << "Insert 1 if you want to use a randomly generated key or insert 2 if you want to use your own key\n";
    std::getline(std::cin, option);

    while (option.size() != 1 || option[0] != '1' && option[0] != '2')
    {
        std::cout << "Invalid option,enter 1 or 2\n";
        std::getline(std::cin, option);
    }

    if (option[0] == '1')
    {
        mainKey = generateRandomKey();
        std::cout << "The key is between the \"\".Your key is: \"";
        std::array<uint8_t, 16> keyArray = MatrixToArray(mainKey);
        for (const uint8_t& byte : keyArray)
            std::cout << static_cast<char>(byte);
        std::cout << "\"\nMake sure you copy the value of the key,it will be needed for the decryption of the data\n";

        return;
    }
    else
    {
        std::string keyString;
        std::cout << "Enter your key in text format,it needs to be exactly 16 characters long including spaces\n";
        std::getline(std::cin, keyString);
        while (keyString.size() != 16)
        {
            std::cout << "The key is not 16 characters.Example for a key \"This is a key123\".Enter the key again";
            std::getline(std::cin, keyString);
        }

        std::array<uint8_t, 16> keyArray;
        for (int i = 0; i < 16; i++)
            keyArray[i] = static_cast<uint8_t>(keyString[i]); //convert the key from string to array of bytes
        mainKey = ArrayToMatrix(keyArray);  //then convert the key from array form to matrix form

        return;
    }
}

int main()
{
    Key mainKey;
    std::string inputFileName = "input.txt";    //here needs to be the name of the file that contains the plaintext
    std::string EncryptedDataFileName = "encryptedData.txt";  //here needs to be the name of the file where the encrypted data will be written at for the encryption and the encrypted data will be read from for the decryption
    std::string DecryptedDataFileName = "decryptedData.txt";  //here needs to be the name of the file where the decrypted data will be written

    ReadKey(mainKey);

    std::string option;
    std::cout << "Insert 1 for file encryption or 2 for file decryption\n";
    std::getline(std::cin, option);
    while (option.size() != 1 || option[0] != '1' && option[0] != '2')
    {
        std::cout << "Invalid option,enter 1 or 2\n";
        std::getline(std::cin, option);
    }

    if (option[0] == '1')
        EncryptFileAES(inputFileName, EncryptedDataFileName, mainKey);
    else
        DecryptFileAES(EncryptedDataFileName, DecryptedDataFileName, mainKey);

    return 0;
}