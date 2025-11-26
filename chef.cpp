#include <fstream>
#include "chef.hpp"

#include <iostream>
#include <csignal>
#include <unistd.h>

// --- CLASSE ATENDIMENTO ---

// Construtor: Cria o pipe e faz o fork()
Atendimento::Atendimento(unsigned int chefId, unsigned int mesaId) 
    : chefId(chefId), mesaId(mesaId) // recebe e salva os IDs
{
    // Define "quemSou" como "Pai" por padrão.
    this->quemSou = "Pai";

    // 1. Cria o Pipe
    if (pipe(fd) < 0) {//checagem de erro
        perror("pipe");
        exit(EXIT_FAILURE); //se der erro ele fecha
    }

    // 2. Cria o Processo Filho (Fork)
    pid_t fork_pid = fork(); // faz o fork
    this->pid = fork_pid; // Salva o PID na classe

    // 3. Checa Erro
    if (fork_pid < 0) {//se for <0 da erra e fecha
        perror("fork");
        exit(EXIT_FAILURE);
    }
    // 4. Lógica do Filho
    else if (fork_pid == 0) { //se = 0 é filho
        this->quemSou = "Filho"; //guarda a identificação dele
        iniciar(); // o filho entra no loop de escuta
        _exit(0);  // se por acaso o filho sair da escuta ele mata o filho
    } 
    // 5. Lógica do Pai
    else { //se não for = 0, é = 1, então é pai
        this->quemSou = "Pai"; //guarda a identificação dele
        close(fd[0]); // O pai só escreve, então fecha a ponta de leitura 0
    }
}

// Destrutor: Executado pelo PAI
Atendimento::~Atendimento() {
    if (quemSou == "Pai") { //garante q só pai execute
        close(fd[1]); // Fecha o funil do pipe, o filho sai do loop de escuta
        kill(pid, SIGKILL); // Mata o subprocesso filho na hora
    }
}

// Executado pelo PAI para enviar um pedido ao FILHO
void Atendimento::prepararPedido(const std::string &pedido) const {
    write(fd[1], pedido.c_str(), pedido.size()); //escreve no funil
}

// Executado APENAS pelo FILHO
void Atendimento::iniciar() {
    pid = getpid(); // Filho pega seu próprio PID

    std::string nomeArquivo = "ChefeCozinha " + std::to_string(this->chefId) + ".txt"; //monta o nome do arquivo
    // Abre o log em modo append, q só adiciona
    std::ofstream logFile(nomeArquivo, std::ios::app); //adiciona ao final do arquivo que o pai criou
    if (!logFile.is_open()) { //se não abriu de boas o arquivo
        _exit(1); // mata o filho
    }

    close(fd[1]); // Filho só lê, então fecha a ponta de escrita
    char buffer[256]; // buffer para ler as requisições

    while (true) { //loop onde o filho vive
        const ssize_t n = read(fd[0], buffer, sizeof(buffer) - 1);  // Lê o pipe, guarda no buffer
        if (n <= 0) break; // Se n <= 0, pode ser um erro, encerra
        
        logFile.write(buffer, n); //escreve
        logFile.flush(); // Garante que foi escrito, tipo "salvar agora!"
    }

    logFile.close(); //fecha o arquivo
    close(fd[0]); //fecha a ponta de leitura
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
    if (atendimento == nullptr) { //esse chef tem um atendimento ativo?
        std::cerr << "Erro: Chef tentou preparar pedido sem atendimento ativo!" << std::endl; //imprime erro
        return;
    }
    atendimento->prepararPedido(pedido); //solicita a preparação do pedido
}

// Deleta o objeto Atendimento (acionando seu destrutor)
void Chef::encerrarAtendimento() {
    delete atendimento; //mata o processo
    atendimento = nullptr; //ponteiro do chefe volta a ficar nulo
}