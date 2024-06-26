/**
 * Implemente aqui as funções dos sistema de arquivos que simula EXT3
 */

#include "fs.h"
#include <fstream>
#include <math.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <string.h>
#include <cstring>
#include <bitset>
#include <memory>
#define BITS_IN_BYTE 8

// Inverte os bits do mapa de bits
char invertBits(char mapBits, const unsigned char directBlocks[3])
{
    // Converte o mapBits para uma representação binária de 8 bits
    std::bitset<BITS_IN_BYTE> bits(mapBits);

    // Para cada valor em directBlocks, se diferente de 0, inverte o bit correspondente
    for (int i = 0; i < 3; ++i)
    {
        if (directBlocks[i] != 0)
        {
            int position = directBlocks[i];
            if (position >= 0 && position < BITS_IN_BYTE)
            {
                bits.flip(position);
            }
        }
    }

    // Converte de volta para um char e retorna
    return static_cast<char>(bits.to_ulong());
}

// extrai o nome do diretório do caminho
std::string getPath(const std::string &path)
{
    // Encontrar a última barra
    size_t lastSlash = path.rfind('/');

    // Caso especial para quando há apenas um diretório
    if (lastSlash == 0)
    {
        return "/";
    }

    // Encontrar a penúltima barra
    size_t penultimateSlash = path.rfind('/', lastSlash - 1);
    if (penultimateSlash == std::string::npos)
    {
        // Somente um diretório no caminho
        return "/";
    }

    // Retornar o penúltimo diretório
    return path.substr(penultimateSlash + 1, lastSlash - penultimateSlash - 1);
}

// extrai o nome do arquivo do caminho
std::string getFileName(const std::string &path)
{
    // Encontrar a última ocorrência da barra '/' no caminho
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos)
    {
        // Se não houver barra, retorna o caminho inteiro
        return path;
    }
    // Retorna a parte do caminho após a última barra
    return path.substr(pos + 1);
}

// Encontra o primeiro bloco livre no mapa de bits
int findFirstFreeBlock(unsigned int value)
{
    int bitCount = sizeof(value) * 8; // Total de bits no tipo de 'value' (geralmente 32 bits para um unsigned int)

    for (int i = 0; i < bitCount; i++)
    {
        if (!(value & (1 << i)))
        { // Verifica se o i-ésimo bit é 0
            return i;
        }
    }

    return -1;
}


void initFs(std::string fsFileName, int blockSize, int numBlocks, int numInodes)
{

    // Inicializando o arquivo para leitura
    std::fstream file(fsFileName, std::ios::out | std::ios::binary | std::ios::trunc);

    int mapBitSize = std::ceil(numBlocks / 8.0);
    char *mapBits = new char[mapBitSize]{};
    mapBits[0] = 0x01;
    INODE raiz{};
    raiz.IS_DIR = 0x01;
    raiz.IS_USED = 0x01;
    raiz.NAME[0] = '/';

    file.write((char *)&blockSize, sizeof(char));
    file.write((char *)&numBlocks, sizeof(char));
    file.write((char *)&numInodes, sizeof(char));
    file.write(mapBits, mapBitSize * sizeof(char));
    file.write((char *)&raiz, sizeof(INODE));

    char a(0x00);

    // Inicializa os inodes e blocos com 0
    for (int i(1); i < numInodes; i++)
    {
        for (int j(0); j < sizeof(INODE); j++)
        {
            file.write((char *)&a, sizeof(char));
        }
    }
    // Inicializa os blocos com 0
    file.write((char *)&a, sizeof(char));
    for (int i(0); i < numBlocks; i++)
    {
        for (int j(0); j < blockSize; j++)
        {
            file.write((char *)&a, sizeof(char));
        }
    }
    
    //limpar mapa de bits para evitar vazamento de memoria
    delete[] mapBits;
    mapBits = nullptr;
    
    file.close();
}

/**
 * @brief Adiciona um novo arquivo dentro do sistema de arquivos que simula EXT3. O sistema já deve ter sido inicializado.
 * @param fsFileName arquivo que contém um sistema sistema de arquivos que simula EXT3.
 * @param filePath caminho completo novo arquivo dentro sistema de arquivos que simula EXT3.
 * @param fileContent conteúdo do novo arquivo
 */
void addFile(std::string fsFileName, std::string filePath, std::string fileContent)
{
    std::fstream file(fsFileName, std::ios::in | std::ios::out | std::ios::binary);
    file.seekg(0);
    INODE Directory{};
    INODE File{};

    int blockSize = 0x00;
    int numBlocks = 0x00;
    int numInodes = 0x00;
    int sizePai = 0x00;
    int indicePai{0};
    int indiceNovoInode{0};

    file.read((char *)&blockSize, sizeof(char));
    file.read((char *)&numBlocks, sizeof(char));
    file.read((char *)&numInodes, sizeof(char));

    int mapBitSize = std::ceil(numBlocks / 8.0);
    char *mapBits = new char[mapBitSize]{};
    char diretorioPai[10]{};

    file.read(mapBits, mapBitSize * sizeof(char));

    // extrai o nome do diretório pai
    std::string dir = getPath(filePath);
    strcpy(diretorioPai, dir.c_str());

    // quatidade de blocos que o fileContent vai usar
    int quantityBlocks = std::ceil(static_cast<double>(fileContent.size()) / blockSize);

    // encontrar o primeiro bloco livre e escrever o fileContent
    int freeBlock = findFirstFreeBlock(*mapBits);

    file.seekg(5 + numInodes * sizeof(INODE) + freeBlock * blockSize);

    for (int i(0); i < fileContent.size(); i++)
    {
        file.write((char *)&fileContent[i], sizeof(char));
    }

    // escreve o inode do arquivo
    File.IS_USED = 0x01;
    File.IS_DIR = 0x00;
    File.SIZE = fileContent.size();

    std::string fileName = getFileName(filePath);
    strcpy(File.NAME, fileName.c_str());

    // atualiza o inode com os blocos que o fileContent está
    int blockIndex = 0;
    int aux = freeBlock;
    for (freeBlock; freeBlock < quantityBlocks + aux; freeBlock++)
    {
        File.DIRECT_BLOCKS[blockIndex] = freeBlock;
        blockIndex++;
    }
    file.seekg(4);

    // escreve o inode do diretório pai
    for (; indicePai < numInodes; indicePai++)
    {
        if (file.read((char *)&Directory, sizeof(INODE)))
        {
            int result = std::strcmp(diretorioPai, Directory.NAME);
            if (result == 0)
            {
                file.seekg(16 + indicePai * sizeof(INODE));
                break;
            }
        }
    }

    // achar o primeiro inode livre e escrever o INODE
    
    file.seekg(4);
    for (; indiceNovoInode < numInodes; indiceNovoInode++)
    {
        if (file.read((char *)&Directory, sizeof(INODE)))
        {
            if (Directory.IS_USED == 0x00)
            {
                break;
            }
        }
    }
    file.seekg(4 + indicePai * sizeof(INODE));
    file.read((char *)&Directory, sizeof(INODE));

    bool isFind = false;
    for (int i(0); i < (sizeof(Directory.DIRECT_BLOCKS) / sizeof(Directory.DIRECT_BLOCKS[0])); i++)
    {
        file.seekg(5 + numInodes * sizeof(INODE) + Directory.DIRECT_BLOCKS[i] * blockSize);
        for (int j(0); j < blockSize; j++)
        {
            char aux;
            if (file.read((char *)&aux, sizeof(char)))
            {
                if (aux == 0x00 || Directory.SIZE == 0x00)
                {
                    file.seekg(5 + numInodes * sizeof(INODE) + Directory.DIRECT_BLOCKS[i] * blockSize + j);
                    file.write((char *)&indiceNovoInode, sizeof(char));
                    isFind = true;
                    break;
                }
            }
        }
        if (isFind)
        {
            break;
        }
    }
    if (!isFind)
    {
        if (Directory.DIRECT_BLOCKS[1] == 0x00)
        {
            Directory.DIRECT_BLOCKS[1] = freeBlock;
            file.seekg(5 + numInodes * sizeof(INODE) + Directory.DIRECT_BLOCKS[1] * blockSize);
            file.write((char *)&indiceNovoInode, sizeof(char));
            *mapBits = invertBits(*mapBits, &Directory.DIRECT_BLOCKS[1]);
        }
    }
    Directory.SIZE += 0x01;
    file.seekg(4 + indicePai * sizeof(INODE));
    file.write((char *)&Directory, sizeof(INODE));

    file.seekg(4 + indiceNovoInode * sizeof(INODE));
    file.write((char *)&File, sizeof(INODE));

    // atualiza o mapa de bits
    file.seekg(3);

    *mapBits = invertBits(*mapBits, File.DIRECT_BLOCKS);

    file.write(mapBits, mapBitSize * sizeof(char));
    
    //limpar mapa de bits para evitar vazamento de memoria
    delete[] mapBits;
    mapBits = nullptr;
    
    file.close();
}

/**
 * @brief Adiciona um novo diretório dentro do sistema de arquivos que simula EXT3. O sistema já deve ter sido inicializado.
 * @param fsFileName arquivo que contém um sistema sistema de arquivos que simula EXT3.
 * @param dirPath caminho completo novo diretório dentro sistema de arquivos que simula EXT3.
 */
void addDir(std::string fsFileName, std::string dirPath)
{
    std::fstream file(fsFileName, std::ios::in | std::ios::out | std::ios::binary);

    INODE inodes{};
    INODE Directory{};

    int blockSize = 0x00;
    int numBlocks = 0x00;
    int numInodes = 0x00;
    int sizePai = 0x00;

    file.read((char *)&blockSize, sizeof(char));
    file.read((char *)&numBlocks, sizeof(char));
    file.read((char *)&numInodes, sizeof(char));

    int mapBitSize = std::ceil(numBlocks / 8.0);
    char *mapBits = new char[mapBitSize]{};
    char diretorioPai[10]{};

    file.read(mapBits, mapBitSize * sizeof(char));

    std::string dir = getPath(dirPath);
    strcpy(diretorioPai, dir.c_str());

    int indicePai{0};

    // escreve o inode do diretório pai e atualiza o tamanho
    for (; indicePai < numInodes; indicePai++)
    {
        if (file.read((char *)&inodes, sizeof(INODE)))
        {
            int result = std::strcmp(diretorioPai, inodes.NAME);
            if (result == 0)
            {
                inodes.SIZE += 0x01;
                file.seekg(16 + indicePai * sizeof(INODE));
                file.write((char *)&inodes.SIZE, sizeof(char));
                break;
            }
        }
    }

    // achar o primeiro inode livre e escrever o INODE
    int indiceNovoInode{0};

    file.seekg(4);
    for (; indiceNovoInode < numInodes; indiceNovoInode++)
    {
        if (file.read((char *)&inodes, sizeof(INODE)))
        {
            if (inodes.IS_USED == 0x00)
            {
                break;
            }
        }
    }
    file.seekg(4 + indicePai * sizeof(INODE));
    file.read((char *)&inodes, sizeof(INODE));

    bool isFind = false;
    for (int i(0); i < (sizeof(inodes.DIRECT_BLOCKS) / sizeof(inodes.DIRECT_BLOCKS[0])); i++)
    {
        file.seekg(5 + numInodes * sizeof(INODE) + inodes.DIRECT_BLOCKS[i] * blockSize);
        for (int j(0); j < blockSize; j++)
        {
            char aux;
            if (file.read((char *)&aux, sizeof(char)))
            {
                if (aux == 0x00)
                {
                    file.seekg(5 + numInodes * sizeof(INODE) + inodes.DIRECT_BLOCKS[i] * blockSize + j);
                    file.write((char *)&indiceNovoInode, sizeof(char));
                    isFind = true;
                    break;
                }
            }
        }
        if (isFind)
        {
            break;
        }
    }

    Directory.IS_USED = 0x01;
    Directory.IS_DIR = 0x01;
    Directory.SIZE = 0x00;

    // remove o / do nome do arquivo e escreve no inode
    dirPath.erase(0, 1);
    strcpy(Directory.NAME, dirPath.c_str());

    int freeBlock = findFirstFreeBlock(*mapBits);

    Directory.DIRECT_BLOCKS[0] = freeBlock;
    // atualiza o inode com os blocos que o fileContent está
    int blockIndex = 0;

    file.seekg(4 + indiceNovoInode * sizeof(INODE));
    file.write((char *)&Directory, sizeof(INODE));

    // atualiza o mapa de bits
    file.seekg(3);
    *mapBits = invertBits(*mapBits, Directory.DIRECT_BLOCKS);
    file.write(mapBits, mapBitSize * sizeof(char));
    
    //limpar mapa de bits para evitar vazamento de memoria
    delete[] mapBits;
    mapBits = nullptr;
    
    file.close();
}

/**
 * @brief Remove um arquivo ou diretório (recursivamente) de um sistema de arquivos que simula EXT3. O sistema já deve ter sido inicializado.
 * @param fsFileName arquivo que contém um sistema sistema de arquivos que simula EXT3.
 * @param path caminho completo do arquivo ou diretório a ser removido.
 */
void remove(std::string fsFileName, std::string path)
{
    std::fstream file(fsFileName, std::ios::in | std::ios::out | std::ios::binary);

    INODE File{};
    INODE RelativePath{};

    int blockSize = 0x00;
    int numBlocks = 0x00;
    int numInodes = 0x00;
    int sizePai = 0x00;

    file.read((char *)&blockSize, sizeof(char));
    file.read((char *)&numBlocks, sizeof(char));
    file.read((char *)&numInodes, sizeof(char));

    int mapBitSize = std::ceil(numBlocks / 8.0);
    char *mapBits = new char[mapBitSize]{};
    char fileNameInode[10]{};
    char diretorioPai[10]{};

    file.read(mapBits, mapBitSize * sizeof(char));

    //
    std::string fileName = getFileName(path);
    strcpy(fileNameInode, fileName.c_str());

    int fileIdex{0};

    // remove o is_used do inode
    for (; fileIdex < numInodes; fileIdex++)
    {
        if (file.read((char *)&File, sizeof(INODE)))
        {
            int result = std::strcmp(fileNameInode, File.NAME);
            if (result == 0)
            {
                File.IS_USED = 0x00;
                file.seekg(1 + fileIdex * sizeof(INODE));
                break;
            }
        }
    }
    // achar o inode do pai

    std::string dir = getPath(path);
    strcpy(diretorioPai, dir.c_str());

    int indicePai{0};
    file.seekg(4);
    // escreve o inode do diretório pai
    for (; indicePai < numInodes; indicePai++)
    {
        if (file.read((char *)&RelativePath, sizeof(INODE)))
        {
            int result = std::strcmp(diretorioPai, RelativePath.NAME);
            if (result == 0)
            {
                RelativePath.SIZE -= 0x01;
                file.seekg(16 + indicePai * sizeof(INODE));
                file.write((char *)&RelativePath.SIZE, sizeof(char));
                break;
            }
        }
    }
    bool isFound = false;
    int posicaoParaMudar;
    for (int i = 0; i < (sizeof(RelativePath.DIRECT_BLOCKS) / sizeof(RelativePath.DIRECT_BLOCKS[0])); ++i)
    {
        int blockStart = 5 + numInodes * sizeof(INODE) + RelativePath.DIRECT_BLOCKS[i] * blockSize;
        file.seekg(blockStart);

        for (int j = 0; j < blockSize; ++j)
        {
            char content, aux;

            if (file.read(&content, sizeof(char)))
            {
                if (content == fileIdex)
                {
                    if (j == 0)
                    {
                        file.seekg(blockStart + j + 1);
                        file.read(&aux, sizeof(char));

                        if (aux == 0x00)
                        {
                            break;
                        }

                        file.seekg(blockStart + j);
                        file.write(&aux, sizeof(char));
                        isFound = true;
                        posicaoParaMudar = j+1;
                        
                    }

                } else if (isFound && RelativePath.DIRECT_BLOCKS[i] > 0)
                {

                    file.seekg(5 + numInodes * sizeof(INODE) + RelativePath.DIRECT_BLOCKS[i - 1] * blockSize + posicaoParaMudar);
                    file.write(&content, sizeof(char));
                    *mapBits = invertBits(*mapBits, &RelativePath.DIRECT_BLOCKS[i]);
                    RelativePath.DIRECT_BLOCKS[i] = 0x00;
                    file.seekg(4 + indicePai * sizeof(INODE));
                    file.write((char *)&RelativePath, sizeof(INODE));
                    break;
                }
            }
        }
    }

    *mapBits = invertBits(*mapBits, File.DIRECT_BLOCKS);

    file.seekg(4 + fileIdex * sizeof(INODE));
    file.write((char *)&File, sizeof(INODE));
    file.seekg(3);
    file.write(mapBits, mapBitSize * sizeof(char));
    
    //limpar mapa de bits para evitar vazamento de memoria
    delete[] mapBits;
    mapBits = nullptr;
    
    file.close();
}

/**
 * @brief Move um arquivo ou diretório em um sistema de arquivos que simula EXT3. O sistema já deve ter sido inicializado.
 * @param fsFileName arquivo que contém um sistema sistema de arquivos que simula EXT3.
 * @param oldPath caminho completo do arquivo ou diretório a ser movido.
 * @param newPath novo caminho completo do arquivo ou diretório.
 */
void move(std::string fsFileName, std::string oldPath, std::string newPath) {
    std::fstream file(fsFileName, std::ios::in | std::ios::out | std::ios::binary);
    if (!file) {
        std::cerr << "Error: Could not open file." << std::endl;
        return;
    }

    INODE File{};
    INODE inodes{};

    int blockSize = 0x00;
    int numBlocks = 0x00;
    int numInodes = 0x00;

    file.read(reinterpret_cast<char*>(&blockSize), sizeof(char));
    file.read(reinterpret_cast<char*>(&numBlocks), sizeof(char));
    file.read(reinterpret_cast<char*>(&numInodes), sizeof(char));

    int mapBitSize = std::ceil(numBlocks / 8.0);
    std::unique_ptr<char[]> mapBits(new char[mapBitSize]{});
    char fileNameInode[10]{};
    char novoDiretorioPai[10]{};
    char antigoDiretorioPai[10]{};

    file.read(mapBits.get(), mapBitSize * sizeof(char));

    // NOVO PATH
    std::string novoDir = getPath(newPath);
    strcpy(novoDiretorioPai, novoDir.c_str());

    int indicePai{0};
    // encontrar o primeiro bloco livre e escrever o fileContent
    std::string fileName = getFileName(oldPath);
    strcpy(fileNameInode, fileName.c_str());

    // ANTIGO PATH
    std::string antigoDir = getPath(oldPath);
    strcpy(antigoDiretorioPai, antigoDir.c_str());

    // CASO MUDANÇA DE NOME
    int fileIdex{0};
    std::string newFileName = getFileName(newPath);

    // Acha o indice do inode a ser movido
    file.seekg(4);
    for (; fileIdex < numInodes; fileIdex++) {
        if (file.read(reinterpret_cast<char*>(&File), sizeof(INODE))) {
            int result = std::strcmp(fileNameInode, File.NAME);
            if (result == 0) {
                strcpy(File.NAME, newFileName.c_str());
                file.seekp(4 + fileIdex * sizeof(INODE));
                file.write(reinterpret_cast<char*>(&File), sizeof(INODE));
                break;
            }
        }
    }
    
    if (novoDir == antigoDir) {
        strcpy(File.NAME, newFileName.c_str());
        file.seekp(4 + fileIdex * sizeof(INODE));
        file.write(reinterpret_cast<char*>(&File), sizeof(INODE));
        return;
    }

    // Ler conteudo do fileName
    std::string fileContent;
    for (int i = 0; i < (sizeof(File.DIRECT_BLOCKS) / sizeof(File.DIRECT_BLOCKS[0])); i++) {
        if (File.DIRECT_BLOCKS[i] == 0x00) {
            break;
        }
        file.seekg(5 + numInodes * sizeof(INODE) + File.DIRECT_BLOCKS[i] * blockSize);
        for (int j = 0; j < blockSize; j++) {
            char content;
            if (file.read(&content, sizeof(char))) {
                if (content != 0x00) {
                    fileContent += content;
                }
            }
        }
    }

    file.close();
    
    // Movimentar o arquivo
    remove(fsFileName, oldPath);
    addFile(fsFileName, newPath, fileContent);
}