#include <iostream>     // biblioteca para entrada/saída (cout, cin)
#include <string>       // para usar std::string

#include "Restaurante.hpp" 
#include "Mesa.hpp"
#include "chef.hpp"

// Função principal do programa
int main() {
    unsigned int qtdChefs, qtdMesas;

    // 1. Lê a configuração inicial (N chefs, M mesas)
    std::cout << "Digite a quantidade de chefs e mesas (ex: 5 20): ";
    std::cin >> qtdChefs >> qtdMesas;
    
    // Limpa o buffer do 'enter'
    std::string dummy;
    std::getline(std::cin, dummy); 

    // 2. Cria o restaurante
    Restaurante restaurante1(qtdMesas, qtdChefs);

    std::cout << "Restaurante aberto com " << qtdChefs << " chefs e " << qtdMesas << " mesas." << std::endl;

    std::string linha_completa;   // vai armazenar pedidos

    // 3. Loop principal de leitura de pedidos
    while(true){
        std::cout << "\nDigite o pedido (ex: 0 hamburguer), (ex: 2 fim) ou FIM para encerrar: ";
        std::getline(std::cin, linha_completa); 

        if(linha_completa == "FIM"){ 
            break;
        }

        // Interpreta a linha usando a função robusta da classe
        auto [mesa, pedido] = restaurante1.interpretarLinha(linha_completa); 
        
        if (mesa == -1) { // Checagem de erro
            std::cout << "Entrada inválida. Tente (Mesa Pedido) ou FIM." << std::endl;
            continue;
        }

        // Manda tudo para pedidoRecebido. 
        // A lógica de "fim" já está implementada lá dentro!
        restaurante1.pedidoRecebido(mesa, pedido);
    }

    return 0; 
}