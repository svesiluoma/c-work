/*
/*
 * Simple daytime client -- modified from Stevens' example at
 * intro/daytimetcpcli.c */

#include <sys/socket.h>  // defines socket, connect, ...
#include <netinet/in.h>  // defines sockaddr_in
#include <string.h>      // defines memset
#include <stdio.h>       // defines printf, perror, ...
#include <arpa/inet.h>   // inet_pton, ...
#include <unistd.h>      // read, ...

#define MAXLINE 80

int main(int argc, char **argv)
{
    int sockfd, n;
    char recvline[MAXLINE + 1];  // merkkipuskuri, johon luetaan tietoa
    struct sockaddr_in servaddr;  // tietorakenne, joka esittää osoitetta

    // Määritellään merkkijono: osoite (ASCII-muodossa), johon otetaan yhteyttä
    const char *address = "130.233.154.208";

    // Luodaan pistoke, joka käyttää IPv4 - protokollaa (AF_INET)
    // ja TCP-protokollaa (SOCK_STREAM)
    // Paluuarvo on pistokkeen tunniste, tai -1 jos luominen ei onnistunut
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        return 1;
    }

    // Alustetaan osoitetta esittävä tietorakenne nollilla.
    // Sen jälkeen kerrotaan että osoiteperhe on IPv4,
    // ja määritellään palvelimen portti johon tullaan ottamaan yhteyttä
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(13); /* daytime server = 13 */

    // Seuraava funktio muuntaa ASCII-muotoisen IP-osoitteen binääriseksi.
    // Se talletetaan servaddr - rakenteeseen.
    if (inet_pton(AF_INET, address, &servaddr.sin_addr) <= 0) {
        fprintf(stderr, "inet_pton error for %s\n", address);
        return 1;
    }

    // Avataan TCP-yhteys käyttäen edellä määriteltyä servaddr - rakennetta.
    // Jos yhteydenotto onnistui, palautetaan 0. Muuten negatiivinen arvo.
    if (connect(sockfd,
                (struct sockaddr *) &servaddr,
                sizeof(servaddr)) < 0) {
        perror("connect error");
        return 1;
    }

    // Luetaan tavuja pistokkeesta, enintään 80 kappaletta (MAXLINE-vakio).
    // Tavut kopioidaan recvline - puskuriin, joka varattiin aiemmin.
    // Muuttujan n arvoksi tulee luettujen tavujen lukumäärä,
    // tai 0 jos yhteys suljettiin, tai negatiivinen jos tapahtui virhe.
    while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0; // lisätään loppunolla, jotta tulostus onnistuu

        //  tulostetaan stdout-virtaan (eli käyttäjän ruudulle)
        if (fputs(recvline, stdout) == EOF) {
            fprintf(stderr, "fputs error\n");
            return 1;
        }
    }

    // If read return value was 0, loop terminates, without error
    if (n < 0) {
        perror("read error");
        return 1;
    }
    return 0;
}
