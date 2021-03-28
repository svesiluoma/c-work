/*
 * server.c
 * 5-server - Harjoitellaan TCP-palvelinohjelman toimintaa yksi asiakas kerrallaan
 * Sari Vesiluoma */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#define MAXLINE 80
#define LISTENQ 5

int main(int argc, char **argv)
{
    int sockfd, n, connfd, listenfd;
    socklen_t len;
    char recvline[MAXLINE + 1];  // Merkkipuskuri, johon luetaan tietoa
    struct sockaddr_in servaddr; //tietorakenne, joka esittää osoitetta
    struct sockaddr_in6 lisaddr, cliaddr;

    // Ottaa TCP-yhteyden IP-osoitteeseen 130.233.154.208, porttiin 5000.
    const char *address = "130.233.154.208";
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        return 1;
    } 
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5000);
    if (inet_pton(AF_INET, address, &servaddr.sin_addr) <= 0) {
        fprintf(stderr, "inet_pton error for %s/n", address);
        return 1;
    }
    if (connect(sockfd,
                (struct sockaddr *) &servaddr,
                sizeof(servaddr)) < 0) {
                    perror("connect error");
                    return 1;
    }

    // Lähettää pistokkeeseen oman opiskelijanumerosi ASCII-muodossa ja sen perään rivinvaihtomerkin
    char *opnro = "763088\n";
    n = write(sockfd, opnro, strlen(opnro));
    if (n < 0) {
        perror("write1 error\n");
        return 1;
    }

    // Lähettää pistokkeeseen tehtävän tunnisteen ja perään rivinvaihdon
    char *viesti = "5-server\n";
    n = write(sockfd, viesti, strlen(viesti));
    if (n < 0) {
        perror("write2 error\n");
        return 1;
    }

    // Sinun tulee hyväksyä satunnainen määrä tulevia yhteyksiä. 
    // Aina kun tehtäväpalvelin lähettää ensimmäiseen pistokkeeseen “MORE” + rivinvaihto, 
    char *returndata = malloc(5 * sizeof(char));
    char buff[80];
    int i = 0;
    while (1) {
        n = read(sockfd, returndata, sizeof(char)*5);
        if (strstr(returndata, "OK") != NULL) {
            break;
        }
        if (strstr(returndata, "MORE") != NULL) {
            // luo palvelinpistoke joka kuuntelee määräämääsi porttia (ja osoitetta).
            // luodaan kuunteleva pistoke
            if ((listenfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
                perror("socket");
                return -1;
            }
            // Valitaan portti ja sidotaan pistoke siihen
            // Hyväksytään sisääntulevat yhteydet mistä tahansa osoitteesta
            memset(&lisaddr, 0, sizeof(lisaddr));
            lisaddr.sin6_family = AF_INET6;
            lisaddr.sin6_addr = in6addr_any;
            lisaddr.sin6_port = htons(6272);

            if (bind(listenfd, (struct sockaddr *) &lisaddr, sizeof(lisaddr)) < 0) {
                perror("bind");
                return -1;
            }

            // Sitten lähetä tehtäväpalvelimelle takaisin viesti “SERV <IP-osoite> <portti>” + rivinvaihto. 
            // Nyt tehtäväpalvelin ottaa yhteyttä antamaasi osoitteeseen.
            char *data = "SERV  130.233.154.208 6272\n";
            n = write(sockfd, data, strlen(data));
            if (n < 0) {
                perror("write1 error\n");
                return 1;
            }

            // Kuunnellaan pistoketta, määrätään ruuhkajonon kooksi 5.
           if (listen(listenfd, LISTENQ) < 0) {
               perror("listen");
                return -1;
            }
            len = sizeof(cliaddr);

            // Uusi yhteys, connfd on uusi pistoke josta dataa voidaan
            // siirtää palvelimen ja asiakkaan välillä
            if ((connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &len)) < 0) {
                perror("accept");
                return -1;
            }

            // Nyt tehtäväpalvelin ottaa yhteyttä antamaasi osoitteeseen.           
            while (1) {
                // se lähettää sinulle 32-bittisen luvun. 
                uint32_t item32 = 0;
                n = read(connfd, &item32, sizeof(uint32_t));
                if (n < 0) {
                perror("read bb error\n");
                    return 1;
                }    
                int luku = ntohl(item32);

                if (luku == 0) {
                    close(connfd);
                    close(listenfd);
                    break;
                }

                // Seuraavaksi tehtäväpalvelin odottaa, että lähetät tuon luvun verran 
                // tavuja takaisin palvelinpistokkeen kautta. 
                char *sendstring;
                sendstring = malloc((luku+1)*sizeof(char));
                memset(sendstring, 'X', luku);
                n = write(connfd, sendstring, strlen(sendstring));
                if (n < 0) {
                 perror("Tavujen palauttaminen takaisin ei onnistu\n");
                    return 1;
                }
            }            
        }
    }
    free(returndata);

    // Testi päättyy kun palvelin lähettää ensimmäiseen pistokkeeseen 
    // “OK” tai “FAIL” riippuen siitä, onnistuiko testi.
    while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;
        if (fputs(recvline, stdout) == EOF) {
            fprintf(stderr, "fputs error\n");
            return 1;
        }
    }
    if (n < 0) {
        perror("read error");
        return 1;
    }
    return 0;
   
    // Sulkee pistokkeen
    close(sockfd);
}    
