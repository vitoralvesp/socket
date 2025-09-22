/**
 * @brief Cliente que ira atuar no envio das operacoes matematicas para o servidor processar e ira receber o resultado utilizando um socket 
 * @author Vitor Alves Pereira - 10410862
 * @date 2025-09-21
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>

#define BUFFER_SIZE 1024

/**
 * @brief Funcao para tratar erros
 * @param msg Mensagem de erro
 * @return void
 */
void die(const char *msg) {
    perror(msg);
    exit(1);
}

/**
 * @brief Funcao principal do cliente
 * @param argc Numero de argumentos
 * @param argv Argumentos
 * @return int Codigo de retorno (0 se sucesso, 1 se erro)
 */
int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char operation[BUFFER_SIZE];

    if (argc != 3) {
        fprintf(stderr, "[ USO ] %s <endereco_ip> <porta>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);

    // Cria o socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        die("[ ERRO ] falha ao criar socket");
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    // Converte o endereco IP do servidor
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
        die("[ ERRO ] endereco invalido ou nao suportado");

    // Conecta ao servidor
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        die("[ ERRO ] falha ao conectar");
    
    printf("[ SUCESSO ] conectado ao servidor %s:%d\n", server_ip, port);

    while (1) {
        printf("[ INSERIR ] digite a operacao (+, -, *, /) ou 'sair' para encerrar: ");
        
        if (fgets(operation, sizeof(operation), stdin) == NULL) {
            printf("[ ERRO ] falha ao ler a operacao\n");
            continue;
        }

        // Remove o caractere de nova linha
        operation[strcspn(operation, "\n")] = 0;

        if (strcmp(operation, "sair") == 0) {
            printf("[ SAIR ] encerrando o cliente.\n");
            break;
        }

        // Envia a operacao para o servidor
        if (send(sock, operation, strlen(operation), 0) < 0) {
            perror("[ ERRO ] falha ao enviar operacao");
            continue;
        }

        // Recebe a resposta do servidor
        int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received < 0) {
            perror("[ ERRO ] falha ao receber resposta");
            continue;
        } else if (bytes_received == 0) {
            printf("[ ERRO ] conexao encerrada pelo servidor.\n");
            break;
        }

        buffer[bytes_received] = '\0'; // Null-terminate the received string
        printf("[ RESULTADO ] o resultado da operacao inserida e %s\n", buffer);
    }

    close(sock);
    return 0;
}