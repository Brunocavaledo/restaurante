#pragma once
#include <vector>        // usado para armazenar mesas por número
#include <queue>      // usado para fila de chefs livres
#include <string>
#include <fstream>
#include "Mesa.hpp"
#include "chef.hpp"

class Restaurante {
private:
    std::vector<Mesa> mesas;                 // vetor de mesas (ex: mesas[5] = Mesa 5)
    std::queue<Chef*> filaDeChefs; // fila de chefs disponíveis
    std::queue<int> filaDeMesasEspera; // fila de mesas em espera
public:
    Restaurante(unsigned int qtdMesas, unsigned int qtdCozinheiros); //construtor da classe, cria mesas e cozinheiros

    Chef* getChefLivre();                  // entrega um chef se disponível
    void pedidoRecebido(int numeroMesa, std::string pedido); // registra pedido

    void liberarChef(Chef* chef); //Libera o chef após ternminar de atender alguém

    std::pair<int, std::string> interpretarLinha(std::string linha);  //vai separar informações dentro do pedido

    void esperarAtendimento(int numeroMesa); //Bota a mesa na fila esperando liberar um cozinheiro
};
