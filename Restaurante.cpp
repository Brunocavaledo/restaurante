#include <fstream>
#include <iostream>   // usado para mensagens de erro/feedback
#include <stdexcept>  // usado para tratamento de exceções
#include <sstream>    // IMPORTANTE: usado para o stringstream (parsing de strings)
#include "Restaurante.hpp"

// -----------------------------------------------------------------------------
// Construtor do Restaurante
// -----------------------------------------------------------------------------
Restaurante::Restaurante(unsigned int qtdMesas, unsigned int qtdChefs) {
    // Dimensiona o vetor de mesas para ter o tamanho exato solicitado.
    this->mesas.resize(qtdMesas);

    // Loop para criar os chefs e seus arquivos de log
    for (unsigned int i = 0; i < qtdChefs; ++i) {
        // Cria um novo Chef com ID 'i' e adiciona na fila de disponíveis
        this->filaDeChefs.push(new Chef(i));
        
        // Monta o nome do arquivo de log desse chef (ex: "ChefeCozinha 0.txt")
        std::string nomeArquivo = "ChefeCozinha " + std::to_string(i) + ".txt";
        
        // Abre o arquivo no modo padrão (sem append), o que APAGA o conteúdo anterior.
        // Isso garante que começamos com logs limpos a cada execução do programa.
        std::ofstream logFile(nomeArquivo);
        if (logFile.is_open()) { 
            logFile << "ChefeCozinha " << i << std::endl; // Cabeçalho do arquivo
            logFile << std::endl; // Pula uma linha
            logFile.close();      // Fecha o arquivo (será reaberto pelos filhos depois)
        }
    }
}

// -----------------------------------------------------------------------------
// Métodos Básicos de Gerenciamento de Recursos
// -----------------------------------------------------------------------------

// Tenta pegar um chef da fila de disponíveis
Chef* Restaurante::getChefLivre() {
    if (this->filaDeChefs.empty()) 
        return nullptr; // Retorna nulo se não tem ninguém livre
    
    Chef* c = this->filaDeChefs.front(); // Pega o primeiro da fila
    this->filaDeChefs.pop();             // Remove ele da fila (agora ele está ocupado)
    return c;                            // Entrega o ponteiro do chef
}

// Devolve um chef que terminou o serviço para a fila
void Restaurante::liberarChef(Chef* chef) {
    this->filaDeChefs.push(chef); // Coloca ele de volta no final da fila
}

// Coloca uma mesa na lista de espera pq não tinha um chef disponível
void Restaurante::esperarAtendimento(int numeroMesa) {
    this->filaDeMesasEspera.push(numeroMesa); //coloca a mesa na fila de espera
}

// -----------------------------------------------------------------------------
// MÉTODOS AUXILIARES DE LÓGICA
// -----------------------------------------------------------------------------

// Função Central: Começa um atendimento
void Restaurante::iniciarAtendimentoDaMesa(int mesaId, Mesa& mesa, Chef* chef) {
    
    // 1. Vincula o Chef ao objeto Mesa (na memória do programa principal)
    mesa.adicionarChef(chef);
    
    // 2. O Chef cria o processo FILHO (fork) e prepara o Pipe
    chef->iniciarAtendimento(mesaId);

    // 3. Envia o cabeçalho "Mesa X:" pelo pipe para o filho escrever no txt
    chef->prepararPedido("\nMesa " + std::to_string(mesaId) + ":");
    
    // 4. Envia o pedido que estava guardado na mesa.
    chef->prepararPedido(" " + mesa.removerPedido() + "\n");
}

// Verifica se depois de liberar um chef existe alguém esperando na fila
void Restaurante::verificarFilaDeEspera() {
    // Se tem gente esperando...
    if (!this->filaDeMesasEspera.empty()) {
        // ... tenta pegar um chef livre (acabou de liberar um, então deve ter né)
        Chef* chefLivre = this->getChefLivre();
        
        if (chefLivre) {
            // Pega o ID da mesa que está a mais tempo esperando (primeira da fila)
            int mesaEmEsperaId = this->filaDeMesasEspera.front();
            this->filaDeMesasEspera.pop(); // Tira da fila de espera
            
            // Pega a referência real do objeto mesa
            Mesa &mesaEmEspera = this->mesas[mesaEmEsperaId];
            
            // Reutiliza a função central para iniciar os trabalhos
            iniciarAtendimentoDaMesa(mesaEmEsperaId, mesaEmEspera, chefLivre);
        }
    }
}

// Lógica executada quando o pedido é "fim"
void Restaurante::tratarPedidoFim(int numeroMesa, Mesa& mesa) {
    // Só faz sentido finalizar se a mesa tiver um chef
    if (mesa.mesaTemChef()) {
        Chef* chef = mesa.getChef(); // Descobre quem é o chef responsável

        chef->encerrarAtendimento(); // Mata o processo filho (SIGKILL) e fecha pipes
        mesa.removerChef();          // Desvincula o chef da mesa (mesa fica livre)
        this->liberarChef(chef);     // Devolve o chef para a fila de livres

        // IMPORTANTE: Como um chef ficou livre, vamos ver se a fila anda!
        verificarFilaDeEspera();
    } else {
        std::cout << ">> Mesa " << numeroMesa << " esta na fila de espera e nao tem chef para ser finalizada." << std::endl;
    }
}

// Lógica executada quando é um pedido normal para uma mesa JÁ ATENDIDA
void Restaurante::tratarPedidoComChef(int numeroMesa, Mesa& mesa, std::string pedido) {
    Chef* chef = mesa.getChef(); 
    // Apenas formata e manda pelo pipe. O processo filho lá no chef.cpp recebe e grava.
    chef->prepararPedido(" " + pedido + "\n");
}

// Lógica executada quando é um NOVO cliente (mesa sem chef)
void Restaurante::tratarNovaMesa(int numeroMesa, Mesa& mesa, std::string pedido) {
    // 1. Verifica se a mesa já estava na fila de espera (tem pedido guardado, mas sem chef)
    std::string pedidoAntigo = mesa.removerPedido();
    if (!pedidoAntigo.empty()) {
        mesa.adicionarPedido(pedidoAntigo); // Devolve o pedido original para não perder
        return;
    }

    // 2. Se é um cliente novo mesmo, guarda o pedido na mesa temporariamente
    mesa.adicionarPedido(pedido);

    // 3. Tenta contratar um chef
    Chef* chef = this->getChefLivre();

    if (chef) {
        // Se achou chef, começa o atendimento (fork, logs, pipes)
        iniciarAtendimentoDaMesa(numeroMesa, mesa, chef);
    } else {
        // Se não tem chef, manda pra fila de espera
        esperarAtendimento(numeroMesa);
    }
}

// -----------------------------------------------------------------------------
// Método Principal ("Gerente")
// Recebe a ordem da main e decide para qual função auxiliar enviar
// -----------------------------------------------------------------------------
void Restaurante::pedidoRecebido(int numeroMesa, std::string pedido) {
    Mesa& mesa = this->mesas[numeroMesa]; // Pega referência da mesa

    // Roteamento da decisão
    if (pedido == "fim") {
        tratarPedidoFim(numeroMesa, mesa);
    } 
    else if (mesa.mesaTemChef()) {
        // Se não é fim e já tem chef, é um pedido adicional
        tratarPedidoComChef(numeroMesa, mesa, pedido);
    } 
    else {
        // Se não tem chef, é um início de atendimento (ou vai pra fila)
        tratarNovaMesa(numeroMesa, mesa, pedido);
    }
}

// -----------------------------------------------------------------------------
// Interpreta uma entrada usando STRINGSTREAM
// -----------------------------------------------------------------------------
std::pair<int, std::string> Restaurante::interpretarLinha(std::string linha) {
    std::stringstream ss(linha);
    int mesa;
    std::string pedido;

    //Tenta extrair o número da mesa
    if (!(ss >> mesa)) {
        return { -1, "" }; // Se falhar (ex: digitou "abc hamburguer"), manda um erro
    }

    //Lê tudo o que sobrou no buffer até o final
    std::getline(ss, pedido);

    // O comando abaixo remove os espaços em branco do início da string, que era o espaço entre o número da mesa e o pedido.
    size_t inicio = pedido.find_first_not_of(" \t");
    
    if (inicio == std::string::npos) {
        // Só havia espaços ou a string estava vazia (ex: digitou apenas "5")
        pedido = ""; 
    } else {
        pedido = pedido.substr(inicio);
    }

    return { mesa, pedido };
}