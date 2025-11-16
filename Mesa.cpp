#include "Mesa.hpp"
#include <string>

// Construtor inicializa a mesa como livre e sem cozinheiro
Mesa::Mesa() {
    this->estadoMesa = false;    // mesa começa livre
    this->chef = nullptr;      // e sem cozinheiro
}

// Retorna true se a mesa tem cozinheiro
bool Mesa::mesaTemChef() {
    return this->chef != nullptr;  //O ponteiro de chef dessa mesa é diferente de nulo?
}

// Associa um cozinheiro à mesa
void Mesa::adicionarChef(Chef* chefAdicionado) {
    this->chef = chefAdicionado;
}

// Remove o cozinheiro da mesa (coloca nullptr)
void Mesa::removerChef() {
    this->chef = nullptr; //aponta pra nullptr que significa q ele ta de boa
}

// Retorna o ponteiro para o chef associado
Chef* Mesa::getChef() {
    return this->chef;
}

// Armazena o pedido como string (usado pela fila de espera)
void Mesa::adicionarPedido(std::string pedido) {
    this->pedido = pedido;
}

// Retorna o pedido e o apaga da mesa
std::string Mesa::removerPedido() {
    std::string p = this->pedido; //armazena o pedido
    this->pedido.clear(); //limpa o pedido de dentro do objeto
    return p;
}

// Retorna o estado atual (livre/ocupada)
bool Mesa::getEstadoMesa() {
    return this->estadoMesa;
}

// Marca a mesa como ocupada
void Mesa::setEstadoMesaTrue() {
    this->estadoMesa = true;
}