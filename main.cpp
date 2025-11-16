#include <iostream>     // biblioteca para entrada/saída (cout, cin)
#include <string>       // para usar std::string
// Bibliotecas não usadas foram removidas

#include "Restaurante.hpp" 
#include "Mesa.hpp"
#include "chef.hpp"      // --- CORREÇÃO: "chef.hpp" (minúsculo) ---

// Função principal do programa
int main() {
    unsigned int qtdChefs, qtdMesas;

    // 1. Lê a configuração inicial (N chefs, M mesas)
    std::cout << "Digite a quantidade de chefs e mesas (ex: 5 20): ";
    std::cin >> qtdChefs >> qtdMesas;

    // Limpa o buffer do 'enter' depois do cin >>
    std::string dummy;
    std::getline(std::cin, dummy); 

    // 2. Cria o restaurante com os parâmetros lidos
    Restaurante restaurante1(qtdMesas, qtdChefs);

    std::cout << "Restaurante aberto com " << qtdChefs << " chefs (0-" << (qtdChefs - 1) 
              << ") e " << qtdMesas << " mesas (0-" << (qtdMesas - 1) << ")." << std::endl;

    std::string linha_completa;   // vai armazenar pedidos

    // 3. Loop principal de leitura de pedidos
    while(true){
        // Exemplo revertido para "0 hamburguer" e "2 fim"
        std::cout << "\nDigite o pedido (ex: 0 hamburguer), (ex: 2 fim) ou FIM para encerrar: ";
        std::getline(std::cin, linha_completa); // lê a linha completa

        if(linha_completa == "FIM"){ // "FIM" maiúsculo encerra o programa
            break;
        }

        // Interpreta a linha "mesa pedido"
        auto [mesa, pedido] = restaurante1.interpretarLinha(linha_completa); 
        if (mesa == -1) { // Checagem de erro
            std::cout << "Entrada inválida. Tente (Mesa Pedido) ou FIM." << std::endl;
            continue;
        }

        // Manda o pedido para o restaurante
        restaurante1.pedidoRecebido(mesa, pedido); 
    }

    std::cout << "Restaurante fechando. Bom trabalho!" << std::endl;
    return 0; 
}