#include <Client.h>

class Network {
private:
    Client* client;
public:    
    void setClient(Client * client);
    int connect(char* hostname, int port);
    int read(unsigned char* buffer, int len, int timeout);
    int write(unsigned char* buffer, int len, int timeout);
    boolean connected();
    int disconnect();
};
