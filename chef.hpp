#pragma once

#include <string>

// Esta classe representa um atendimento. O atendimento terá um subprocesso vinculado a ele
class Atendimento final {
public:
    // Construtor da classe. Para se criar um atendimento, é necessário informar o ID do chef e da mesa.
    // Ao se criar um objeto Atendimento, um subprocesso pronto com pipes para receber comunicação entre processos é gerado
    Atendimento(unsigned int chefId, unsigned int mesaId);

    // Destrutor da classe. Ao invocá-lo, o subprocesso vinculado a este atendimento deve ser encerrado.
    ~Atendimento();

    void prepararPedido(const std::string &pedido) const;

private:
    // ID do processo (pid)
    pid_t pid{-1};
    // Descritores de arquivo que guardam o "número dos canais" onde serão realizadas a comunicação entre processos.
    // São dois inteiros, inicializados previamente com o valor -1
    int fd[2]{-1, -1};
    // Isso não é necessário, apenas para fim didático
    std::string quemSou;

    // IDs para o processo filho saber quem é e qual arquivo de log usar
    unsigned int chefId;
    unsigned int mesaId;

    // Método chamado na inicialização do atendimento
    void iniciar();
};

class Chef {
public:
    explicit Chef(unsigned int id);

    void iniciarAtendimento(unsigned int mesa);

    void prepararPedido(const std::string &pedido);

    void encerrarAtendimento();

        // Retorna o ID deste chef
    unsigned int getId() const { return id; }

private:
    const unsigned int id;
    Atendimento *atendimento;
};