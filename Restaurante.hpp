#pragma once
#include <vector>        // usado para armazenar mesas por número
#include <queue>         // usado para fila de chefs livres
#include <string>
#include <fstream>
#include "Mesa.hpp"
#include "chef.hpp"

class Restaurante {
private:
    std::vector<Mesa> mesas;             // vetor de mesas
    std::queue<Chef*> filaDeChefs;       // fila de chefs disponíveis
    std::queue<int> filaDeMesasEspera;   // fila de mesas em espera

// Lógica centralizada para vincular um chef a uma mesa e iniciar os logs
    void iniciarAtendimentoDaMesa(int mesaId, Mesa& mesa, Chef* chef);

    // Verifica se há mesas na fila e chefs livres para atendê-las
    void verificarFilaDeEspera();

    // Trata especificamente o comando "fim"
    void tratarPedidoFim(int numeroMesa, Mesa& mesa);

    // Trata pedidos para mesas que já têm chef (encaminhamento)
    void tratarPedidoComChef(int numeroMesa, Mesa& mesa, std::string pedido);

    // Trata o primeiro pedido de uma mesa (pode ir pra fila ou conseguir chef)
    void tratarNovaMesa(int numeroMesa, Mesa& mesa, std::string pedido);

public:
    Restaurante(unsigned int qtdMesas, unsigned int qtdCozinheiros);

    Chef* getChefLivre();                  // entrega um chef se disponível
    
    void liberarChef(Chef* chef); // Libera o chef após terminar de atender alguém

    std::pair<int, std::string> interpretarLinha(std::string linha);  // separa informações

    void esperarAtendimento(int numeroMesa); // Bota a mesa na fila

    // Ee método será apenas um "gerente" que delega as tarefas
    void pedidoRecebido(int numeroMesa, std::string pedido);
};