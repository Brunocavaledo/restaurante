#pragma once
#include <string>
#include "chef.hpp"

class Mesa {
private:
    bool estadoMesa;        // false = mesa livre, true = em atendimento
    std::string pedido;     // armazena o *primeiro* pedido se a mesa for para a fila de espera
    Chef* chef;             // ponteiro para um chef (null = nenhum)

public:
    Mesa(); // construtor

    bool mesaTemChef();             // verifica se a mesa tem chef
    void adicionarChef(Chef* c);    // associa um chef à mesa
    void removerChef();             // remove o chef (coloca nullptr)
    Chef* getChef();                // retorna o chef associado

    void adicionarPedido(std::string pedido);   // registra o pedido (para fila de espera)
    std::string removerPedido();                // limpa e retorna o pedido

    bool getEstadoMesa();           // retorna o estado
    void setEstadoMesaTrue();       // marca mesa como ocupada
};