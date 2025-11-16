#include <fstream>
#include <iostream>   // usado para mensagens de erro/feedback
#include <stdexcept>  // usado pela função stoi

#include "Restaurante.hpp"

// -----------------------------------------------------------------------------
// Construtor do restaurante
// -----------------------------------------------------------------------------
Restaurante::Restaurante(unsigned int qtdMesas, unsigned int qtdChefs) {

    // Cria 'qtdMesas' objetos Mesa no vetor (acesso O(1))
    this->mesas.resize(qtdMesas); 

    // Cria cada chef e coloca na fila de disponíveis
    for (unsigned int i = 0; i < qtdChefs; ++i) {
        // O ID do chef agora é 'i' (começando em 0)
        this->filaDeChefs.push(new Chef(i)); //add o cara na fila

        // Limpa o arquivo de log deste chef para uma nova execução
        // O nome do arquivo usa 'i' (0, 1, 2...)
        std::string nomeArquivo = "ChefeCozinha " + std::to_string(i) + ".txt";

        // Abre em modo padrão que vai apagar logs antigos
        std::ofstream logFile(nomeArquivo); 
        if (logFile.is_open()) {
            // O cabeçalho usa 'i' (0, 1, 2...)
            logFile << "ChefeCozinha " << i << std::endl;
            logFile << std::endl; // Adiciona uma linha em branco
            logFile.close();
        }
    }
}


// -----------------------------------------------------------------------------
// Retorna um chef disponível (O(1))
// -----------------------------------------------------------------------------
Chef* Restaurante::getChefLivre() {
    if (this->filaDeChefs.empty())
        return nullptr; // Retorna nulo se não houver chefs
    
    Chef* c = this->filaDeChefs.front(); // Pega o primeiro da fila
    this->filaDeChefs.pop();             // Remove da fila
    return c;
}

// -----------------------------------------------------------------------------
// Devolve um chef para a fila de disponíveis (O(1))
// -----------------------------------------------------------------------------
void Restaurante::liberarChef(Chef* chef) {
    this->filaDeChefs.push(chef); // Adiciona o chef de volta ao fim da fila
}

// -----------------------------------------------------------------------------
// Coloca uma mesa na fila de espera (O(1))
// -----------------------------------------------------------------------------
void Restaurante::esperarAtendimento(int numeroMesa) {
    this->filaDeMesasEspera.push(numeroMesa);
    std::cout << ">> Mesa " << numeroMesa << " colocada na fila de espera." << std::endl;
}


// -----------------------------------------------------------------------------
// Lógica principal: Recebe e processa um pedido
// -----------------------------------------------------------------------------
void Restaurante::pedidoRecebido(int numeroMesa, std::string pedido) {

    // Validação da mesa, senão existe já manda um erro
    if (numeroMesa < 0 || numeroMesa >= this->mesas.size()) {
        std::cerr << "Erro: Mesa " << numeroMesa << " inexistente!\n"; // erro mesa TAL inexistente
        return;
    }

    // Pega a referência da mesa (acesso O(1))
    Mesa& mesa = this->mesas[numeroMesa];

    // ---------------------------------------------------
    // CASO 1: Pedido "fim"
    // ---------------------------------------------------
    if (pedido == "fim") {
        if (mesa.mesaTemChef()) {
            Chef* chef = mesa.getChef(); // Pega o chef associado

            chef->encerrarAtendimento(); // 1. Encerra o atendimento (mata o processo filho)
            mesa.removerChef();          // 2. Remove o chef da mesa
            this->liberarChef(chef);     // 3. Devolve o chef para a fila
            //Escreve que atendimento encerrou ali
            std::cout << "<< Mesa " << numeroMesa << " finalizada. Chef " << chef->getId() << " esta livre." << std::endl;

            // 4. Lógica da Fila de Espera: Chef liberado atende o próximo
            if (!this->filaDeMesasEspera.empty()) { //Se não tem mesas esperando
                int mesaEmEsperaId = this->filaDeMesasEspera.front(); //
                this->filaDeMesasEspera.pop();
                Mesa& mesaEmEspera = this->mesas[mesaEmEsperaId];
                Chef* chefLivre = this->getChefLivre(); 

                std::cout << ">> Chef " << chefLivre->getId() << " atendendo mesa " << mesaEmEsperaId << " da fila de espera." << std::endl;

                mesaEmEspera.adicionarChef(chefLivre);
                chefLivre->iniciarAtendimento(mesaEmEsperaId);

                chefLivre->prepararPedido("\nMesa " + std::to_string(mesaEmEsperaId) + ":");
                chefLivre->prepararPedido(mesaEmEspera.removerPedido());
            }
        } else {
            std::cout << ">> Mesa " << numeroMesa << " nao esta em atendimento." << std::endl;
        }
        return; 
    }

    // ---------------------------------------------------
    // CASO 2: Novo pedido para mesa JÁ ATENDIDA
    // ---------------------------------------------------
    if (mesa.mesaTemChef()) {
        Chef* chef = mesa.getChef(); 
        chef->prepararPedido(pedido); // Apenas envia o pedido pelo pipe
        std::cout << ">> Pedido '" << pedido << "' adicionado a mesa " << numeroMesa << std::endl;
    }

    // ---------------------------------------------------
    // CASO 3: Novo pedido para mesa SEM ATENDIMENTO
    // ---------------------------------------------------
    else {  // Verifica se a mesa já tem um pedido guardado (ou seja, se já está na fila)
        std::string pedidoAntigo = mesa.removerPedido(); // Pega o pedido antigo (e limpa)
        if (!pedidoAntigo.empty()) {
            std::cout << ">> Mesa " << numeroMesa << " ja esta na fila. Pedido '" << pedido << "' ignorado." << std::endl;
            // Devolve o pedido ANTIGO, não o novo
            mesa.adicionarPedido(pedidoAntigo); 
            return;
        }

        // Se estava vazio, armazena o NOVO pedido
        mesa.adicionarPedido(pedido);

        Chef* chef = this->getChefLivre(); // Tenta pegar um chef

        // 3a: Tem chef livre!
        if (chef) {
            std::cout << ">> Mesa " << numeroMesa << " será atendida pelo Chef " << chef->getId() << std::endl;
            
            mesa.adicionarChef(chef);
            chef->iniciarAtendimento(numeroMesa); // Cria o processo filho

            chef->prepararPedido("\nMesa " + std::to_string(numeroMesa) + ":"); 
            chef->prepararPedido(mesa.removerPedido()); // Envia o pedido (que limpa o da mesa)
        }
        // 3b: Não tem chef livre
        else {
            esperarAtendimento(numeroMesa); // Coloca o ID da MESA na fila de espera
        }
    }
}


// -----------------------------------------------------------------------------
// Interpreta uma entrada do usuário "N PEDIDO"
// -----------------------------------------------------------------------------
std::pair<int, std::string> Restaurante::interpretarLinha(std::string linha) {
    size_t pos = linha.find(' ');

    if (pos == std::string::npos)
        return { -1, "" };

    try {
        // Converte a primeira parte para inteiro (número da mesa)
        int mesa = std::stoi(linha.substr(0, pos));
        std::string pedido = linha.substr(pos + 1);
        return { mesa, pedido };
    } catch (const std::exception& e) {
        return { -1, "" };
    }
}