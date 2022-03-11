#include "helpers.h"
#include "requests.h"
#include "buffer.h"
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

#define HOST_IP "34.118.48.238"
#define HOST_PORT 8080
#define PARAMLEN 128
#define MAXLEN 512
#define JSON_PAYLOAD "application/json"
#define REGISTER_URL "/api/v1/tema/auth/register"
#define LOGIN_URL "/api/v1/tema/auth/login"
#define ENTER_LIB_URL "/api/v1/tema/library/access"
#define GET_BOOK_URL "/api/v1/tema/library/books/"
#define GET_BOOKS_URL "/api/v1/tema/library/books"
#define ADD_BOOK_URL "/api/v1/tema/library/books"
#define DELETE_BOOK_URL "/api/v1/tema/library/books/"
#define LOGOUT_URL "/api/v1/tema/auth/logout"


// ./client
int main(void) {

    int sockfd;
    char *message;
    char *response;
    int login = 0;
    char *session_cookie = NULL;
    char *token = NULL;
    char input[MAXLEN];

    while(1) {
        memset(input, 0, sizeof(input));
        fgets(input, MAXLEN - 1, stdin);

        // DONE
        if(strcmp(input, "exit\n") == 0) {
            break;
        }

        // DONE
        if(strcmp(input, "register\n") == 0) {

            char username[PARAMLEN];
            char password[PARAMLEN];
            char *tok;
            // Citire date
            printf("username=");
            fgets(username, PARAMLEN - 1, stdin);
            printf("password=");
            fgets(password, PARAMLEN - 1, stdin);

            // Eliminare "\n" de la sfarsitul parametrilor
            strtok(username, "\n");
            strtok(password, "\n");

            // Iesire comanda
            if(strcmp(username, ".") == 0 || strcmp(password, ".") == 0) {
                printf("Iesire comanda!\n");
                continue;
            }

            // Construire obiect JSON
            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            json_object_set_string(object, "username", username);
            json_object_set_string(object, "password", password);

            char* serial = NULL;
            serial = json_serialize_to_string_pretty(value);

            // Construire header
            char **body = (char **) malloc(1 * sizeof(char*));
            body[0] = serial;

            // Deschidere conexiune
            sockfd = open_connection(HOST_IP, HOST_PORT, AF_INET, SOCK_STREAM, 0);

            // Comunicare client-server
            message = compute_post_request(HOST_IP, REGISTER_URL, NULL, JSON_PAYLOAD, body, 1, NULL, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // Cod raspuns
            tok = strtok(response, " ");
            if(tok != NULL) {
                tok = strtok(NULL, " ");
            }
            if(strcmp(tok, "201") == 0) {
                printf("Inregistrarea s-a facut cu succes!\n");
            } else {
                printf("Numele de utilizator este deja folosit!\n");
            }

            free(body);
            json_free_serialized_string(serial);
            json_value_free(value);
            close_connection(sockfd);
        }

        //DONE
        if(strcmp(input, "login\n") == 0) {
            // Se verifica daca este vreun cont conectat deja
            if(login != 0) {
                printf("Exista deja un cont autentificat!\n");
                continue;
            }

            char username[PARAMLEN];
            char password[PARAMLEN];
            char *tok;
            // Citire date
            printf("username=");
            fgets(username, PARAMLEN - 1, stdin);
            printf("password=");
            fgets(password, PARAMLEN - 1, stdin);

            // Eliminare "\n" de la sfarit
            strtok(username, "\n");
            strtok(password, "\n");

            // Iesire comanda
            if(strcmp(username, ".") == 0 || strcmp(password, ".") == 0) {
                printf("Iesire comanda!\n");
                continue;
            }

            // Construire obiect JSON
            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            json_object_set_string(object, "username", username);
            json_object_set_string(object, "password", password);

            char* serial = NULL;
            serial = json_serialize_to_string_pretty(value);

            // Construire header
            char **body = (char **) malloc(1 * sizeof(char*));
            body[0] = serial;

            // Deschidere conexiune
            sockfd = open_connection(HOST_IP, HOST_PORT, AF_INET, SOCK_STREAM, 0);

            // Comunicare client-server
            message = compute_post_request(HOST_IP, LOGIN_URL, NULL, JSON_PAYLOAD, body, 1, NULL, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // Cod raspuns
            tok = strtok(response, " ");
            if(tok != NULL) {
                tok = strtok(NULL, " ");
            }
            if(strcmp(tok, "200") == 0) {
                printf("Autentificare s-a facut cu succes!\n");
                login = 1;

                // obtinere cookie
                while(strcmp(tok, "Set-Cookie") != 0) {
                    tok = strtok(NULL, ":\n");
                }
                tok = strtok(NULL, " ;\n");
                session_cookie = malloc(strlen(tok) + 1);
                strcpy(session_cookie, tok);
            } else {
                printf("Datele de autentificare sunt incorecte!\n");
            }

            free(body);
            json_free_serialized_string(serial);
            json_value_free(value);
            close_connection(sockfd);
        }

        // DONE
        if(strcmp(input, "enter_library\n") == 0) {
            char *tok;

            // Deschidere conexiune
            sockfd = open_connection(HOST_IP, HOST_PORT, AF_INET, SOCK_STREAM, 0);

            // Construire header / mesaj
            if(session_cookie != NULL) {
                char **cookies = (char **) malloc(1 * sizeof(char *));
                cookies[0] = session_cookie;
                message = compute_get_request(HOST_IP, ENTER_LIB_URL, NULL, NULL, cookies, 1);
            } else {
                message = compute_get_request(HOST_IP, ENTER_LIB_URL, NULL, NULL, NULL, 0);
            }

            // Comnuicare client-server
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // Cod raspuns
            tok = strtok(response, " ");
            if(tok != NULL) {
                tok = strtok(NULL, " ");
            }
            if(strcmp(tok, "200") == 0) {
                strtok(NULL, "{");
                strtok(NULL, "\"");
                tok = strtok(NULL, ":\"");
                token = malloc(strlen(tok) + 1);
                strcpy(token, tok);
                printf("Accesul la biblioteca s-a realizat cu succes\n");
            } else {
                printf("Nu sunteti autentificat!\n");
            }
            close_connection(sockfd);
        }

        // DONE
        if(strcmp(input, "get_books\n") == 0) {

            char *tok;

            //Deschidere conexiune
            sockfd = open_connection(HOST_IP, HOST_PORT, AF_INET, SOCK_STREAM, 0);

            // Comnuicare client-server
            message = compute_get_request(HOST_IP, GET_BOOKS_URL, token, NULL, NULL, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // Cod raspuns
            tok = strtok(response, " ");
            if(tok != NULL) {
                tok = strtok(NULL, " ");
            }
            if(strcmp(tok, "200") == 0) {
                printf("Cartile din biblioeteca:\n");
                while(tok != NULL) {
                    strtok(NULL, "{");
                    tok = strtok(NULL, "}");
                    if(tok != NULL) {
                        printf("%s\n", tok);
                    }
                }
            } else {
                printf("Nu aveti acces la biblioteca!\n");
            }
            close_connection(sockfd);
        }

        //DONE
        if(strcmp(input, "get_book\n") == 0) {

            char *tok;
            char id[PARAMLEN];
            char path[MAXLEN];

            // Citire date
            printf("id=");
            fgets(id, PARAMLEN - 1, stdin);
            strtok(id, "\n");

            if(strcmp(id, ".") == 0) {
                printf("Iesire Comanda!\n");
                continue;
            }

            // Concatenare URL
            sprintf(path, GET_BOOK_URL "%s",id);

            // Deschidere conexiune
            sockfd = open_connection(HOST_IP, HOST_PORT, AF_INET, SOCK_STREAM, 0);

            // Comnuicare client-server
            message = compute_get_request(HOST_IP, path, token, NULL, NULL, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // Cod raspuns
            tok = strtok(response, " ");
            if(tok != NULL) {
                tok = strtok(NULL, " ");
            }
            if(strcmp(tok, "200") == 0) {
                printf("Informatiile despre cartea cautata:\n");
                strtok(NULL, "{");
                while(tok != NULL) {
                    tok = strtok(NULL, ",}]");
                    if(tok != NULL) {
                        printf("%s\n", tok);
                    }
                }
            } else if(strcmp(tok, "400") == 0) {
                printf("ID-ul introdus trebuie sa fie un numar!\n");
            } else if(strcmp(tok, "404") == 0) {
                printf("Nu a fost gasita cartea cu ID-ul introdus!\n");
            } else {
                printf("Nu aveti acces la biblioteca!\n");
            }

            close_connection(sockfd);
        }

        // DONE
        if(strcmp(input, "add_book\n") == 0) {
            // cod add_book
            char *tok;
            char title[PARAMLEN];
            char author[PARAMLEN];
            char genre[PARAMLEN];
            char publisher[PARAMLEN];
            uint32_t page_count;

            // Citire date
            printf("title=");
            fgets(title, PARAMLEN - 1, stdin);
            strtok(title, "\n");

            printf("author=");
            fgets(author, PARAMLEN - 1, stdin);
            strtok(author, "\n");

            printf("genre=");
            fgets(genre, PARAMLEN - 1, stdin);
            strtok(genre, "\n");

            printf("publisher=");
            fgets(publisher, PARAMLEN - 1, stdin);
            strtok(publisher, "\n");

            printf("page_count=");
            if(scanf("%d", &page_count) != 1) {
                printf("Input invalid!\n");
                continue;
            }

            if(strcmp(title, ".") == 0 || strcmp(author, ".") == 0 ||
               strcmp(genre, ".") == 0 || strcmp(publisher, ".") == 0) {
                printf("Iesire comanda!\n");
                continue;
            }

            // Consturire obiect JSON
            JSON_Value *value = json_value_init_object();
            JSON_Object *object = json_value_get_object(value);
            json_object_set_string(object, "title", title);
            json_object_set_string(object, "author", author);
            json_object_set_string(object, "genre", genre);
            json_object_set_number(object, "page_count", page_count);
            json_object_set_string(object, "publisher", publisher);

            char *serial = NULL;
            serial = json_serialize_to_string_pretty(value);

            // Construire header
            char **body = (char **) malloc(1 * sizeof(char*));
            body[0] = serial;

            // Deschidere conexiune
            sockfd = open_connection(HOST_IP, HOST_PORT, AF_INET, SOCK_STREAM, 0);

            // Comunicare client-server
            message = compute_post_request(HOST_IP, ADD_BOOK_URL, token, JSON_PAYLOAD, body, 1, NULL, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // Cod raspuns
            tok = strtok(response, " ");
            if(tok != NULL) {
                tok = strtok(NULL, " ");
            }

            if(strcmp(tok, "200") == 0) {
                printf("Cartea a fost adaugata in biblioteca cu succes!\n");
            } else {
                printf("Nu aveti acces la bibilioteca!\n");
            }

            free(body);
            json_free_serialized_string(serial);
            json_value_free(value);
            close_connection(sockfd);
        }

        // DONE
        if(strcmp(input, "delete_book\n") == 0) {

            char *tok;
            char id[PARAMLEN];
            char path[MAXLEN];

            //Citire date
            printf("id=");
            fgets(id, PARAMLEN - 1, stdin);
            strtok(id, "\n");

            if(strcmp(id, ".") == 0) {
                printf("Iesire Comanda!\n");
                continue;
            }

            // Concatenare URL
            sprintf(path, DELETE_BOOK_URL "%s",id);

            // Deschidere conexiune
            sockfd = open_connection(HOST_IP, HOST_PORT, AF_INET, SOCK_STREAM, 0);

            // Comunicare client-server
            message = compute_delete_request(HOST_IP, path, token, NULL, NULL, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // Cod raspuns
            tok = strtok(response, " ");
            if(tok != NULL) {
                tok = strtok(NULL, " ");
            }
            if(strcmp(tok, "200") == 0) {
                printf("Cartea a fost stearsa cu succes!\n");
            } else if(strcmp(tok, "404") == 0) {
                printf("Nu a fost gasita cartea cu ID-ul introdus!\n");
            } else if(strcmp(tok, "403") == 0) {
                printf("Nu aveti acces la biblioteca!\n");
            } else {
                printf("ID-ul introdus este invalid!\n");
            }

            close_connection(sockfd);
        }

        if(strcmp(input, "logout\n") == 0) {

            char *tok;

            // Deschidere conexiune
            sockfd = open_connection(HOST_IP, HOST_PORT, AF_INET, SOCK_STREAM, 0);

            // Construire header / mesaj
            if(session_cookie != NULL) {
                char **cookies = (char **) malloc(1 * sizeof(char *));
                cookies[0] = session_cookie;
                message = compute_get_request(HOST_IP, LOGOUT_URL, NULL, NULL, cookies, 1);
            } else {
                message = compute_get_request(HOST_IP, LOGOUT_URL, NULL, NULL, NULL, 0);
            }

            // Comunicare client-server
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // Cod raspuns
            tok = strtok(response, " ");
            if(tok != NULL) {
                tok = strtok(NULL, " ");
            }
            if(strcmp(tok, "200") == 0) {
                printf("Deconectarea s-a efectuat cu succes!\n");
                login = 0;
                token = NULL;
                session_cookie = NULL;
            } else if(strcmp(tok, "400") == 0) {
                printf("Nu sunteti conectat!\n");
            }

            close_connection(sockfd);
        }

    }
}