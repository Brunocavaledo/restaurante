#include <fstream>
#include <iostream>   // usado para mensagens de erro/feedback
#include <stdexcept>  // usado pela função stoi

#include "Restaurante.hpp"

// -----------------------------------------------------------------------------
// Construtor do restaurante
// -----------------------------------------------------------------------------
Restaurante::Restaurante(unsigned int qtdMesas, unsigned int qtdChefs) {

    // Cria 'qtdMesas' objetos Mesa no vetor
    this->mesas.resize(qtdMesas); //resize determina o tamanho do vetor de acordo com a qtde de mesas

    // Cria cada chef e coloca na fila de disponíveis
    for (unsigned int i = 0; i < qtdChefs; ++i) {
        this->filaDeChefs.push(new Chef(i)); // O ID do chef vai ser o i, add ele na fila
        std::string nomeArquivo = "ChefeCozinha " + std::to_string(i) + ".txt"; //cria o arquivo
        std::ofstream logFile(nomeArquivo);{  // Abre em modo padrão, daí vai apagar logs antigos
        if (logFile.is_open())    //se o arquivo abriu de boas
            logFile << "ChefeCozinha " << i << std::endl;  // escreve no cabeçalho do arquivo: ChefeCozinha "tal", e pula uma linha
            logFile << std::endl; // Adiciona mais uma linha em branco
            logFile.close();  //fecha o arquivo de log
        }
    }
}


// -----------------------------------------------------------------------------
// Retorna um chef disponível (O(1))
// -----------------------------------------------------------------------------
Chef* Restaurante::getChefLivre() {
    if (this->filaDeChefs.empty())
        return nullptr; // Retorna ponteiro nulo se não houver chefs disponíveis
    
    Chef* c = this->filaDeChefs.front(); // Pega o primeiro da fila
    this->filaDeChefs.pop();             // Remove da fila
    return c;                            // Retorna o ponteiro desse chef
}

// -----------------------------------------------------------------------------
// Devolve um chef para a fila de disponíveis
// -----------------------------------------------------------------------------
void Restaurante::liberarChef(Chef* chef) {
    this->filaDeChefs.push(chef); // Adiciona o chef de volta ao fim da fila
}

// -----------------------------------------------------------------------------
// Coloca uma mesa na fila de espera
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

    // Pega a referência da mesa
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

            // 4. Lógica da Fila de Espera: Após chef ser liberado ele atende o próximo
            if (!this->filaDeMesasEspera.empty()) { //Se a fila de mesas não estiver vazia
                int mesaEmEsperaId = this->filaDeMesasEspera.front(); //armazena a primeira mesa da fila
                this->filaDeMesasEspera.pop(); //remove a que tirou da fila, atualizando a fila
                Mesa &mesaEmEspera = this->mesas[mesaEmEsperaId]; //Cria a referência mesaEmEspera que aponta para o objeto Mesa dentro do vetor mesas no índice [mesaEmEsperaId]
                Chef* chefLivre = this->getChefLivre(); //Pega o primeiro chefe livre e armazena o retorno da getChefLivre

                //Aqui abaixo escreve qual chefe pegou e qual mesa ele tá atendendo, e que ela veio lá da fila de espera.
                std::cout << ">> Chef " << chefLivre->getId() << " atendendo mesa " << mesaEmEsperaId << " da fila de espera." << std::endl;

                mesaEmEspera.adicionarChef(chefLivre); //Associa um chefe à uma mesa
                chefLivre->iniciarAtendimento(mesaEmEsperaId); //Cria la na classe chef um processo filho com fork pra essa chefe e essa mesa

                chefLivre->prepararPedido("\nMesa " + std::to_string(mesaEmEsperaId) + ":"); //manda a string "mesa tal" pelo pipe pro filho, que lê ela e escreve no log
                chefLivre->prepararPedido(mesaEmEspera.removerPedido()); //Remove o pedido da mesa e retornando esse pedido passa ele profilho escrever no log
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
std::pair<int, std::string> Restaurante::interpretarLinha(std::string linha) { // vai retornar um  par de dois valores
    size_t pos = linha.find(' '); //procura o espaço dentro da string e guardar onde ele fica

    if (pos == std::string::npos) //se não achar retorna -1 e string vazia, que é erro
        return { -1, "" };

    try {
        // Converte a primeira parte para inteiro (número da mesa)
        int mesa = std::stoi(linha.substr(0, pos)); //pega do começo até o espaço e converte em int e guarda
        std::string pedido = linha.substr(pos + 1); //pega do espaço até o final e guarda
        return { mesa, pedido }; // retorna tudo separado
    } catch (const std::exception& e) {  //se digitar mesa e pedido errados, vai retorna erro
        return { -1, "" }; 
    }
}