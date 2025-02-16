//
//  main.c
//  RaspConnect
//
//  Created by Emran Sediq Sultani on 16.02.25.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define HTML_FILE "/Users/swfd/Desktop/Rasp/RaspConnect/RaspConnect/index.html"  // Der Pfad zur HTML-Datei

// Function to send HTTP response
void send_http_response(int client_socket) {
    // Öffne die HTML-Datei
    FILE *html_file = fopen(HTML_FILE, "r");
    if (html_file == NULL) {
        perror("Error opening HTML file");
        return;
    }

    // Berechne die Größe der Datei
    fseek(html_file, 0, SEEK_END);
    long html_size = ftell(html_file);
    fseek(html_file, 0, SEEK_SET);

    // Lese die Datei in einen Puffer
    char *html_content = (char *)malloc(html_size + 1);
    if (html_content == NULL) {
        perror("Error allocating memory for HTML content");
        fclose(html_file);
        return;
    }
    fread(html_content, 1, html_size, html_file);
    html_content[html_size] = '\0'; // Nullterminierung

    // HTTP-Header
    const char *http_header =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n\r\n";

    // Sende den HTTP-Header
    write(client_socket, http_header, strlen(http_header));
    
    // Sende den Inhalt der HTML-Datei
    write(client_socket, html_content, html_size);

    // Freigabe des Puffers und Schließen der Datei
    free(html_content);
    fclose(html_file);
}

int main(int argc, char *argv[]) {
    char client_ip[INET_ADDRSTRLEN];
    int times = 1;
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    /* {} Start: Modifizierter Bereich */
    int port = 80;  // Standardport

    // Überprüfe, ob der Benutzer einen Port als Argument angegeben hat
    if (argc > 1) {
        port = atoi(argv[1]);  // Wandle das Argument in eine Ganzzahl um
        if (port <= 0 || port > 65535) {
            printf("Invalid port number. Using default port 80.\n");
            port = 80;  // Setze den Standardport, falls die Eingabe ungültig ist
        }
    }
    /* {} End: Modifizierter Bereich */

    // Erstelle den Server-Socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error opening socket");
        exit(1);
    }

    // Initialisiere die Serveradresse
    memset(&server_addr, 0, sizeof(server_addr));

    // Setup der Serveradresse
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Binde an alle verfügbaren Interfaces
    server_addr.sin_port = htons(port);       // Setze den benutzerdefinierten Port

    // Binde den Socket an die Adresse und den Port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        close(server_socket);
        exit(1);
    }

    // Beginne, auf eingehende Verbindungen zu hören
    if (listen(server_socket, 5) < 0) {
        perror("Error listening on socket");
        close(server_socket);
        exit(1);
    }

    printf("HTTP Server running on port %d\n", port);

    // Hauptschleife, um eingehende Verbindungen zu akzeptieren und zu bearbeiten
    while (1) {
        // Akzeptiere eine eingehende Client-Verbindung
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Error accepting client connection");
            continue;
        }
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        printf("%s",client_ip);
        printf("\nData sent No.%d\n",times);
        times++;
        
        // Sende die HTTP-Antwort an den Client
        send_http_response(client_socket);

        // Schließe den Client-Socket nach dem Senden der Antwort
        close(client_socket);
    }

    // Schließe den Server-Socket, wenn er nicht mehr benötigt wird
    close(server_socket);
    return 0;
}
