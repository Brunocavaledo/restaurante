#include <fstream>
#include "chef.hpp"

#include <iostream>
#include <csignal>
#include <unistd.h>

// --- CLASSE ATENDIMENTO ---

// Construtor: Cria o pipe e faz o fork()
Atendimento::Atendimento(unsigned int chefId, unsigned int mesaId) 
    : chefId(chefId), mesaId(mesaId) // Salva os IDs
{
    // Define "quemSou" como "Pai" por padrão.
    this->quemSou = "Pai";

    // 1. Cria o Pipe
    if (pipe(fd) < 0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // 2. Cria o Processo Filho (Fork)
    pid_t fork_pid = fork();
    this->pid = fork_pid; // Salva o PID na classe

    // 3. Checa Erro
    if (fork_pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    // 4. Lógica do Filho
    else if (fork_pid == 0) {
        this->quemSou = "Filho";
        iniciar(); // O filho entra no loop de escuta
        _exit(0);  // Garante que o filho morra se 'iniciar' retornar
    } 
    // 5. Lógica do Pai
    else {
        this->quemSou = "Pai";
        close(fd[0]); // O pai só escreve, então fecha a ponta de leitura
    }
}

// Destrutor: Executado pelo PAI
Atendimento::~Atendimento() {
    if (quemSou == "Pai") {
        close(fd[1]); // Fecha a ponta de escrita do pipe
        kill(pid, SIGKILL); // Mata o subprocesso filho
    }
}

// Executado pelo PAI para enviar um pedido ao FILHO
void Atendimento::prepararPedido(const std::string &pedido) const {
    write(fd[1], pedido.c_str(), pedido.size() + 1);
}

// Executado APENAS pelo FILHO
void Atendimento::iniciar() {
    pid = getpid(); // Filho pega seu próprio PID

    std::string nomeArquivo = "ChefeCozinha " + std::to_string(this->chefId) + ".txt";
    // Abre o log em MODO APPEND (anexar)
    std::ofstream logFile(nomeArquivo, std::ios::app);
    if (!logFile.is_open()) {
        _exit(1); // Sai se não conseguir abrir o log
    }

    close(fd[1]); // Filho só lê, fecha a ponta de escrita
    char buffer[256]; // buffer para ler as requisições

    while (true) {
        // Lê o pipe (bloqueante)
        const ssize_t n = read(fd[0], buffer, sizeof(buffer) - 1);
        if (n <= 0) break; // Se n <= 0, o pai fechou o pipe (chamou ~Atendimento)
        
        buffer[n] = '\0'; // Adiciona o terminador nulo
        std::string msg(buffer); // Converte para string

        // Escreve no arquivo de log
        logFile << msg << std::endl;
        logFile.flush(); // Garante que foi escrito
    }

    logFile.close(); 
    close(fd[0]); 
}

// --- CLASSE CHEF ---

Chef::Chef(const unsigned int id) : id(id), atendimento(nullptr) {
}

// Cria e armazena um novo objeto de Atendimento
void Chef::iniciarAtendimento(const unsigned int mesa) {
    atendimento = new Atendimento(id, mesa);
}

// Passa o pedido para o Atendimento (que envia pelo pipe)
void Chef::prepararPedido(const std::string &pedido) {
    if (atendimento == nullptr) {
        std::cerr << "Erro: Chef tentou preparar pedido sem atendimento ativo!" << std::endl;
        return;
    }
    atendimento->prepararPedido(pedido);
}

// Deleta o objeto Atendimento (acionando seu destrutor)
void Chef::encerrarAtendimento() {
    delete atendimento;
    atendimento = nullptr;
}