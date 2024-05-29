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

std::string getPath(const std::string& path) {
    // Encontrar a última barra
    size_t lastSlash = path.rfind('/');

    // Caso especial para quando há apenas um diretório
    if (lastSlash == 0) {
        return "/";
    }

    // Encontrar a penúltima barra
    size_t penultimateSlash = path.rfind('/', lastSlash - 1);
    if (penultimateSlash == std::string::npos) {
        // Somente um diretório no caminho
        return "/";
    }

    // Retornar o penúltimo diretório
    return path.substr(penultimateSlash + 1, lastSlash - penultimateSlash - 1);
}

std::string getNameFile(const std::string& path) {
    // Encontrar a última barra
    size_t lastSlash = path.rfind('/');

    // Caso especial para quando há apenas um diretório
    if (lastSlash == 0) {
        return "/";
    }

    // Encontrar a penúltima barra
    size_t penultimateSlash = path.rfind('/', lastSlash - 1);
    if (penultimateSlash == std::string::npos) {
        // Somente um diretório no caminho
        return "/";
    }

    // Retornar o Ultimo diretório
    return path.substr(lastSlash + 1, path.size() - lastSlash - 1);

    // return path.substr(penultimateSlash + 1, lastSlash - penultimateSlash - 1);
}

int findFirstFreeBlock(unsigned int value) {
    int bitCount = sizeof(value) * 8;  // Total de bits no tipo de 'value' (geralmente 32 bits para um unsigned int)

    for (int i = 0; i < bitCount; i++) {
        if (!(value & (1 << i))) {  // Verifica se o i-ésimo bit é 0
            return i;
        }
    }

    return -1;
}

unsigned int calculateNewBitMap(unsigned int value, int quantityBlocksUsed) {
    int bitCount = sizeof(value) * 8;  // Total de bits no tipo de 'value' (geralmente 32 bits para um unsigned int)

    for (int i = 0; i < bitCount; i++) {
        if (!(value & (1 << i))) {  // Verifica se o i-ésimo bit é 0
            for(int j = i; j < quantityBlocksUsed + i; j++){
                value |= (1 << j);  // Configura o i-ésimo bit como 1
            }
            return value;  // Retorna o valor atualizado
        }
    }

    return value;  // Retorna o valor original se todos os bits já estiverem configurados como 1
}

void initFs(std::string fsFileName, int blockSize, int numBlocks, int numInodes){
    
    // Inicializando o arquivo para leitura
    std::fstream file(fsFileName, std::ios::out | std::ios::binary | std::ios::trunc);
    
    int mapBitSize = std::ceil(numBlocks/8.0);
    char *mapBits = new char[mapBitSize]{};
    mapBits[0] = 0x01;
    INODE raiz{};
    raiz.IS_DIR = 0x01;
    raiz.IS_USED = 0x01;
    raiz.NAME[0] = '/';

    file.write((char*) &blockSize, sizeof(char));
    file.write((char*) &numBlocks, sizeof(char));
    file.write((char*) &numInodes, sizeof(char));
    file.write(mapBits, mapBitSize * sizeof(char));
    file.write((char*) &raiz, sizeof(INODE));

    char a(0x00);

    for (int i(1); i < numInodes; i++){
        for (int j(0); j < sizeof(INODE); j++){
            file.write((char*) &a, sizeof(char));
        }
    }

    file.write((char*) &a, sizeof(char));
    for (int i(0); i< numBlocks; i++){
        for (int j(0); j< blockSize; j++){
            file.write((char*) &a, sizeof(char));
        }
    }

    file.close();
      
}

/**
 * @brief Adiciona um novo arquivo dentro do sistema de arquivos que simula EXT3. O sistema já deve ter sido inicializado.
 * @param fsFileName arquivo que contém um sistema sistema de arquivos que simula EXT3.
 * @param filePath caminho completo novo arquivo dentro sistema de arquivos que simula EXT3.
 * @param fileContent conteúdo do novo arquivo
 */
void addFile(std::string fsFileName, std::string filePath, std::string fileContent){
    std::fstream file(fsFileName, std::ios::in | std::ios::out | std::ios::binary);
    
    INODE inodes{};
    INODE File{};

    int blockSize = 0x00;
    int numBlocks = 0x00;
    int numInodes = 0x00;
    int sizePai = 0x00;

    file.read((char*) &blockSize, sizeof(char));
    file.read((char*) &numBlocks, sizeof(char));
    file.read((char*) &numInodes, sizeof(char));

    int mapBitSize = std::ceil(numBlocks/8.0);
    char *mapBits = new char[mapBitSize]{};
    char diretorioPai[10]{};

    file.read(mapBits, mapBitSize * sizeof(char));

    std::string dir = getPath(filePath);
    strcpy(diretorioPai, dir.c_str());

    int indicePai {0};

    // escreve o inode do diretório pai
    for (; indicePai < numInodes; indicePai++){
        if(file.read((char*) &inodes, sizeof(INODE))){
            int result = std::strcmp(diretorioPai, inodes.NAME);
            if( result == 0){
                inodes.SIZE += 0x01;
                file.seekg(16 + indicePai * sizeof(INODE));
                file.write((char*) &inodes.SIZE, sizeof(char));
                break;
            }
        }
    }

    int indiceNovoInode{0};
    
    file.seekg(4);
    for (; indiceNovoInode < numInodes; indiceNovoInode++){
        if(file.read((char*) &inodes, sizeof(INODE))){
            if(inodes.IS_USED == 0x00){
                break;
            }
        }
    }
    file.seekg(4 + indicePai * sizeof(INODE));
    file.read((char*) &inodes, sizeof(INODE));
    
    for (int i(0); i < (sizeof(inodes.DIRECT_BLOCKS) / sizeof(inodes.DIRECT_BLOCKS[0])); i++){
        bool isFind = false;
        file.seekg(5 + numInodes * sizeof(INODE) + inodes.DIRECT_BLOCKS[i] * blockSize);
        for (int j(0); j < blockSize; j++){
            char aux;
            if (file.read((char*) &aux, sizeof(char))){
                if(aux == 0x00){
                    file.seekg(5 + numInodes * sizeof(INODE) + inodes.DIRECT_BLOCKS[i] * blockSize + j);
                    file.write((char*) &indiceNovoInode, sizeof(char));
                    isFind = true;
                    break;
                }
            }
        }
        if (isFind){
            break;
        }
    }

    // quatidade de blocos que o fileContent vai usar
    int quantityBlocks = std::ceil(static_cast<double>(fileContent.size())/blockSize);
    
    // encontrar o primeiro bloco livre e escrever o fileContent
    int freeBlock = findFirstFreeBlock(*mapBits);

    file.seekg(5 + numInodes * sizeof(INODE) + freeBlock * blockSize);

    for (int i(0); i < fileContent.size(); i++){
        file.write((char*) &fileContent[i], sizeof(char));
    }


    File.IS_USED = 0x01;
    File.IS_DIR = 0x00;
    File.SIZE = fileContent.size();

    //remove o / do nome do arquivo e escreve no inode
    filePath.erase(0, 1);

    if (filePath.size() <= 10){
        for (int i(0); i < filePath.size(); i++){
            File.NAME[i] = filePath[i];
        }
    }

    // atualiza o inode com os blocos que o fileContent está
    int blockIndex = 0;
    
    for (freeBlock; freeBlock <= quantityBlocks; freeBlock++){
        File.DIRECT_BLOCKS[blockIndex] = freeBlock;
        blockIndex++;
    }

    file.seekg(4 + indiceNovoInode * sizeof(INODE));
    file.write((char*) &File, sizeof(INODE));
    

    //atualiza o mapa de bits
    file.seekg(3);
    *mapBits = calculateNewBitMap(*mapBits, quantityBlocks);
    file.write(mapBits, mapBitSize * sizeof(char));


    file.close();

}

/**
 * @brief Adiciona um novo diretório dentro do sistema de arquivos que simula EXT3. O sistema já deve ter sido inicializado.
 * @param fsFileName arquivo que contém um sistema sistema de arquivos que simula EXT3.
 * @param dirPath caminho completo novo diretório dentro sistema de arquivos que simula EXT3.
 */
void addDir(std::string fsFileName, std::string dirPath){
    std::fstream file(fsFileName, std::ios::in | std::ios::out | std::ios::binary);
    
    INODE inodes{};
    INODE Directory{};

    int blockSize = 0x00;
    int numBlocks = 0x00;
    int numInodes = 0x00;
    int sizePai = 0x00;

    file.read((char*) &blockSize, sizeof(char));
    file.read((char*) &numBlocks, sizeof(char));
    file.read((char*) &numInodes, sizeof(char));

    int mapBitSize = std::ceil(numBlocks/8.0);
    char *mapBits = new char[mapBitSize]{};
    char diretorioPai[10]{};

    file.read(mapBits, mapBitSize * sizeof(char));

    std::string dir = getPath(dirPath);
    strcpy(diretorioPai, dir.c_str());

    int indicePai {0};

    // escreve o inode do diretório pai
    for (; indicePai < numInodes; indicePai++){
        if(file.read((char*) &inodes, sizeof(INODE))){
            int result = std::strcmp(diretorioPai, inodes.NAME);
            if( result == 0){
                inodes.SIZE += 0x01;
                file.seekg(16 + indicePai * sizeof(INODE));
                file.write((char*) &inodes.SIZE, sizeof(char));
                break;
            }
        }
    }

    int indiceNovoInode{0};
    
    file.seekg(4);
    for (; indiceNovoInode < numInodes; indiceNovoInode++){
        if(file.read((char*) &inodes, sizeof(INODE))){
            if(inodes.IS_USED == 0x00){
                break;
            }
        }
    }
    file.seekg(4 + indicePai * sizeof(INODE));
    file.read((char*) &inodes, sizeof(INODE));
    
    for (int i(0); i < (sizeof(inodes.DIRECT_BLOCKS) / sizeof(inodes.DIRECT_BLOCKS[0])); i++){
        bool isFind = false;
        file.seekg(5 + numInodes * sizeof(INODE) + inodes.DIRECT_BLOCKS[i] * blockSize);
        for (int j(0); j < blockSize; j++){
            char aux;
            if (file.read((char*) &aux, sizeof(char))){
                if(aux == 0x00){
                    file.seekg(5 + numInodes * sizeof(INODE) + inodes.DIRECT_BLOCKS[i] * blockSize + j);
                    file.write((char*) &indiceNovoInode, sizeof(char));
                    isFind = true;
                    break;
                }
            }
        }
        if (isFind){
            break;
        }
    }

    Directory.IS_USED = 0x01;
    Directory.IS_DIR = 0x01;
    Directory.SIZE = 0x00;

    //remove o / do nome do arquivo e escreve no inode
    dirPath.erase(0, 1);

    if (dirPath.size() <= 10){
        for (int i(0); i < dirPath.size(); i++){
            Directory.NAME[i] = dirPath[i];
        }
    }
    int freeBlock = findFirstFreeBlock(*mapBits);

    Directory.DIRECT_BLOCKS[0] = freeBlock;
    // atualiza o inode com os blocos que o fileContent está
    int blockIndex = 0;
    

    file.seekg(4 + indiceNovoInode * sizeof(INODE));
    file.write((char*) &Directory, sizeof(INODE));
    

    //atualiza o mapa de bits
    file.seekg(3);
    *mapBits = calculateNewBitMap(*mapBits, 0x01);
    file.write(mapBits, mapBitSize * sizeof(char));


    file.close();

}

/**
 * @brief Remove um arquivo ou diretório (recursivamente) de um sistema de arquivos que simula EXT3. O sistema já deve ter sido inicializado.
 * @param fsFileName arquivo que contém um sistema sistema de arquivos que simula EXT3.
 * @param path caminho completo do arquivo ou diretório a ser removido.
 */
void remove(std::string fsFileName, std::string path){
    std::fstream file(fsFileName, std::ios::in | std::ios::out | std::ios::binary);
    
    INODE inodes{};
    INODE Directory{};

    int blockSize = 0x00;
    int numBlocks = 0x00;
    int numInodes = 0x00;
    int sizePai = 0x00;

    file.read((char*) &blockSize, sizeof(char));
    file.read((char*) &numBlocks, sizeof(char));
    file.read((char*) &numInodes, sizeof(char));

    int mapBitSize = std::ceil(numBlocks/8.0);
    char *mapBits = new char[mapBitSize]{};
    char fileNameInode[10]{};

    file.read(mapBits, mapBitSize * sizeof(char));

    std::string fileName = getNameFile(path);
    strcpy(fileNameInode, fileName.c_str());

    int fileIdex {0};

    // remove o is_used do inode
    for (; fileIdex < numInodes; fileIdex++){
        if(file.read((char*) &inodes, sizeof(INODE))){
            int result = std::strcmp(fileNameInode, inodes.NAME);
            if( result == 0){
                inodes.IS_USED += 0x00;
                file.seekg(1 + fileIdex * sizeof(INODE));
                file.write((char*) &inodes.IS_USED, sizeof(char));
                break;
            }
        }
    }
    // achar o inode do pai
    // decrementar o tamanho
    // ir para o mapbit do pai
    // procurar o bit com o numero do inode a ser deletado
    // rotacionar

    file.close();

}

/**
 * @brief Move um arquivo ou diretório em um sistema de arquivos que simula EXT3. O sistema já deve ter sido inicializado.
 * @param fsFileName arquivo que contém um sistema sistema de arquivos que simula EXT3.
 * @param oldPath caminho completo do arquivo ou diretório a ser movido.
 * @param newPath novo caminho completo do arquivo ou diretório.
 */
void move(std::string fsFileName, std::string oldPath, std::string newPath){}

