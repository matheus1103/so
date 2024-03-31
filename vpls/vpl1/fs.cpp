#ifndef fs_h
#define fs_h
#include <string>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <string.h>
#include <cstring>

/**
 * @param arquivoDaLista nome do arquivo em disco que contem a lista encadeada
 * @param novoNome nome a ser adicionado apos depoisDesteNome
 * @param depoisDesteNome um nome presente na lista
 */

#include <vector>

bool blocoNulo(const std::vector<char>& block) {
    for (char byte : block) {
        if (byte != 0) {
            return false;
        }
    }
    return true;
}

int procurarBlocoNulo(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Erro ao abrir o arquivo." << std::endl;
        return -1;
    }

    // Pular os primeiros 4 bytes do arquivo
    file.seekg(4, std::ios::beg);

    std::vector<char> block(28);
    int position = 4;

    while (file.read(block.data(), block.size())) {
        if (blocoNulo(block)) {
            return position;
        }
        position += block.size();
    }

    // Verificar o último bloco, que pode ser menor que 28 bytes
    if (file.gcount() > 0 && blocoNulo(block)) {
        return position;
    }

    return -1; // Não encontrou um bloco de 28 bytes nulos
}

void adiciona(std::string arquivoDaLista, std::string novoNome, std::string depoisDesteNome)
{
    // variáveis
    //posicao inicial do nodo
    int posicaoInicial;
    int valorInicial;
    int posicaoAnterior = -1;
    
    //posicao atual do nodo
    int posicaoAtual;

    //final do nodo
    int proximoNodo;

    //aux
    int aux;
    char nomeAdd[20];
    char nomeLista[20];
    char depoisNome[20];
    char bloco[28];

    // abre o arquivo
    std::fstream file(arquivoDaLista, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) {
        std::cerr << "Erro ao abrir o arquivo." << std::endl;
        return;
    }

    //transforma as strings em array de char
    std::strncpy(depoisNome, depoisDesteNome.c_str(), 20);
    std::strncpy(nomeAdd, novoNome.c_str(), 20);
   
    // Lê os primeiros 4 bytes do arquivo pos int tem 4 bytes de tamanho por padrão
    file.read(reinterpret_cast<char*>(&posicaoInicial), sizeof(posicaoInicial));
    posicaoAtual = posicaoInicial;
    file.seekg(posicaoAtual);


    // procura o nodo que contem o nome depoisDesteNome
    while(aux >= 0) {
        file.seekg(posicaoAtual);
        file.read(reinterpret_cast<char*>(&valorInicial), sizeof(valorInicial));
        if(valorInicial > 1){
            file.seekg(valorInicial);
            file.read(reinterpret_cast<char*>(&posicaoInicial), sizeof(posicaoInicial));
        }
        file.read(nomeLista, sizeof(nomeLista));

        if(std::strcmp(nomeLista, depoisNome) == 0){
            aux = -1;        
        }
        file.seekg(posicaoAtual + 24);
        file.read(reinterpret_cast<char*>(&proximoNodo), sizeof(proximoNodo));
        posicaoAnterior = posicaoAtual;
        posicaoAtual = proximoNodo;

    }
    // procura bloco de 28 bytes nulos
    int novoNodo = procurarBlocoNulo(arquivoDaLista);
    

    //reescreve o novo ponteiro
    file.seekg(posicaoAnterior + 24);
    file.write(reinterpret_cast<char*>(&novoNodo), sizeof(novoNodo));
    
    //cria novo nodo
    file.seekg(novoNodo);
    file.write(reinterpret_cast<char*>(&valorInicial), sizeof(valorInicial));
    
    file.write(nomeAdd, sizeof(nomeAdd));
    file.write(reinterpret_cast<char*>(&proximoNodo), sizeof(proximoNodo));


    file.close();

}

#endif /* fs_h */