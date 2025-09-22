/**
 * @brief Servidor utilizado para o processamento das operacoes matematicas recebidas via socket pelo cliente
 * @author Vitor Alves Pereira - 10410862
 * @date 2025-09-21
 **/  

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int setup_server_socket();
void process_request(int new_socket);

int main() {

    while (1) {
        int server_fd = setup_server_socket();

        printf("[ SUCESSO ] servidor iniciado na porta %d. Aguardando conexões...", PORT);

        struct sockaddr_in client_address;
        socklen_t client_addrlen = sizeof(client_address);

        int new_socket = accept(server_fd, (struct sockaddr *)&client_address, &client_addrlen);
        if (new_socket < 0) {
            perror("[ ERRO ] falha ao aceitar conexao!");
            continue;
        }

        printf("[ SUCESSO ] conexao aceita de %s:%d", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
        process_request(new_socket);

        close(server_fd);
    }
    return 0;

}

/**
 * @brief Configura o socket do servidor
 * @return int Socket file descriptor do servidor
 */
int setup_server_socket() {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    // Cria o socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("[ ERRO ] falha ao criar socket!");
        exit(EXIT_FAILURE);
    }

    // Configura o socket para reutilizar o endereço
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("[ ERRO ] falha ao configurar SO_REUSEADDR!");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Configura o socket para reutilizar a porta
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("[ ERRO ] falha ao configurar SO_REUSEPORT!");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Configura o endereco do servidor
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    memset(address.sin_zero, '\0', sizeof address.sin_zero);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("[ ERRO ] falha ao fazer bind!");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Coloca o socket em modo de escuta
    if (listen(server_fd, 3) < 0) {
        perror("[ ERRO ] falha ao escutar!");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

/**
 * @brief Processa a requisicao recebida do cliente
 * @param new_socket Socket file descriptor do cliente
 */
void process_request(int new_socket) {
    char buffer[BUFFER_SIZE];

    while (1) {
        int valread = read(new_socket, buffer, BUFFER_SIZE);

        if (valread < 0) {
            perror("[ ERRO ] falha ao ler do socket!");
            close(new_socket);
            return;
        }

        printf("[ RECEBIDO ] requisicao: %s", buffer);

        // Processa a operacao recebida
        char operation;
        double num1, num2, result;

        if (sscanf(buffer, "%lf %c %lf", &num1, &operation, &num2) != 3) {
            const char *error_msg = "[ ERRO ] formato de operacao invalido. Use: <numero1> <operacao> <numero2>!";
            send(new_socket, error_msg, strlen(error_msg), 0);
            close(new_socket);
            return;
        }

        printf("[ RECEBIDO ] requisicao: %s", buffer);

        switch (operation) {
            case '+':
                result = num1 + num2;
                break;
            case '-':
                result = num1 - num2;
                break;
            case '*':
                result = num1 * num2;
                break;
            case '/':
                if (num2 == 0) {
                    const char *error_msg = "[ ERRO ] divisao por zero nao e permitida!";
                    send(new_socket, error_msg, strlen(error_msg), 0);
                    close(new_socket);
                    return;
                }
                result = num1 / num2;
                break;
            default:
                {
                    const char *error_msg = "[ ERRO ] operacao invalida. Use: +, -, *, /!";
                    send(new_socket, error_msg, strlen(error_msg), 0);
                    close(new_socket);
                    return;
                }
        }

        snprintf(buffer, sizeof(buffer), "[ RESULTADO ] %.2lf %c %.2lf = %.2lf", num1, operation, num2, result);
        send(new_socket, buffer, strlen(buffer), 0);
        printf("[ ENVIADO ] resultado: %s", buffer);
    }
    close(new_socket);

}