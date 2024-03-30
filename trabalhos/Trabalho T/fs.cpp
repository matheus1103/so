/**
 * Implemente aqui as funções dos sistema de arquivos que simula EXT3
 */

#include "fs.h"
#include <fstream>
#include <math.h>

void initFs(std::string fsFileName, int blockSize, int numBlocks, int numInodes){
    
    // Inicializando o arquivo para leitura
    std::fstream file(fsFileName, std::ios::out | std::ios::binary | std::ios::trunc);
    
    //
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
      
}