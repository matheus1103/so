/**
 * Implemente aqui as funções dos sistema de arquivos que simula EXT3
 */

#include "fs.h"
#include <fstream>
#include <math.h>
int findFirstFreeBlock(unsigned int value) {
    int bitCount = sizeof(value) * 8;  // Total de bits no tipo de 'value' (geralmente 32 bits para um unsigned int)

    for (int i = 0; i < bitCount; i++) {
        if (!(value & (1 << i))) {  // Verifica se o i-ésimo bit é 0
            return i;
        }
    }

    return -1;
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
    
    int blockSize = 0x00;
    int numBlocks = 0x00;
    int numInodes = 0x00;

    file.read((char*) &blockSize, sizeof(char));
    file.read((char*) &numBlocks, sizeof(char));
    file.read((char*) &numInodes, sizeof(char));

    int mapBitSize = std::ceil(numBlocks/8.0);
    char *mapBits = new char[mapBitSize]{};


    file.read(mapBits, mapBitSize * sizeof(char));
    
    int indiceNovoInode{0};

    for (; indiceNovoInode < numInodes; indiceNovoInode++){
        if(file.read((char*) &inodes, sizeof(INODE))){
            if(inodes.IS_USED == 0x00){
                break;
            }
        }
    }
    
    // blocos 0 = filhos de raiz txt | agora é encontrar 1 e 2 = conteudo do txt
    int quantityBlocks = std::ceil(static_cast<double>(fileContent.size())/blockSize);
    int freeBlock = findFirstFreeBlock(*mapBits);

    file.seekg(5 + numInodes * sizeof(INODE) + freeBlock * blockSize);

    for (int i(0); i < fileContent.size(); i++){
        file.write((char*) &fileContent[i], sizeof(char));
    }
    int blockIndex = 0;
    for (freeBlock; freeBlock <= quantityBlocks; freeBlock++){
        inodes.DIRECT_BLOCKS[blockIndex] = freeBlock;
        blockIndex++;
    }

    inodes.IS_USED = 0x01;
    inodes.IS_DIR = 0x00;
    inodes.SIZE = fileContent.size();
    // TODO: atualizar mapa de bits apos inserir o arquivo novo
    //remove o / do nome do arquivo
    filePath.erase(0, 1);

    if (filePath.size() <= 10){
        for (int i(0); i < filePath.size(); i++){
            inodes.NAME[i] = filePath[i];
        }
    }
    // for (int i(0); i < fileContent.size(); i++){
    //     inodes.DIRECT_BLOCKS[i] = fileContent[i];
    // }


    file.seekg(4 + indiceNovoInode * sizeof(INODE));
    file.write((char*) &inodes, sizeof(INODE));
    
    //atualiza o mapa de bits
    file.seekg(5 + numInodes * sizeof(INODE));
    // file.write((char*)0x01, sizeof(char));
    for (int i(0); i< numBlocks; i++){
        bool flag = false;
        for (int j(0); j< blockSize; j++){
            char aux;
            if (file.read((char*) &aux, sizeof(char))){
                if(aux != 0x00){
                    flag = true;
                    break;
                }
            }
            
            // if (file.read((char*) &mapBits[i], sizeof(char))){
            
            // }
        }
        
        mapBits[i] = flag ? 0x01 : 0x00 ;
        
    }
    file.write(mapBits, mapBitSize * sizeof(char));
    
    file.seekg(4 + sizeof(INODE));





    file.close();

}

/**
 * @brief Adiciona um novo diretório dentro do sistema de arquivos que simula EXT3. O sistema já deve ter sido inicializado.
 * @param fsFileName arquivo que contém um sistema sistema de arquivos que simula EXT3.
 * @param dirPath caminho completo novo diretório dentro sistema de arquivos que simula EXT3.
 */
void addDir(std::string fsFileName, std::string dirPath){}

/**
 * @brief Remove um arquivo ou diretório (recursivamente) de um sistema de arquivos que simula EXT3. O sistema já deve ter sido inicializado.
 * @param fsFileName arquivo que contém um sistema sistema de arquivos que simula EXT3.
 * @param path caminho completo do arquivo ou diretório a ser removido.
 */
void remove(std::string fsFileName, std::string path){}

/**
 * @brief Move um arquivo ou diretório em um sistema de arquivos que simula EXT3. O sistema já deve ter sido inicializado.
 * @param fsFileName arquivo que contém um sistema sistema de arquivos que simula EXT3.
 * @param oldPath caminho completo do arquivo ou diretório a ser movido.
 * @param newPath novo caminho completo do arquivo ou diretório.
 */
void move(std::string fsFileName, std::string oldPath, std::string newPath){}

