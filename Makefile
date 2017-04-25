#CC = gcc -g -DWITH_NONAMESPACES 
CC = gcc -g -DWITH_OPENSSL -DSOAP_DEBUG
#CC = gcc -g
BIN=onvif_test
CBIN=client_onvif_test
#INCLUDE= -I../gsoap/gsoap-2.8/gsoap -I../gsoap/gsoap-2.8/gsoap/import -I../gsoap/gsoap-2.8/gsoap/plugin
INCLUDE=
SERVER_OBJS=soapC.o soapStub.o stdsoap2.o soapServer.o discoveryserver.o
CLIENT_OBJS=soapC.o soapStub.o duration.o dom.o stdsoap2.o soapClient.o wsseapi.o smdevp.o mecevp.o threads.o wsaapi.o discoveryclient.o 
#CLIENT_OBJS=soapC.o soapStub.o duration.o dom.o stdsoap2.o soapClient.o wsseapi.o smdevp.o mecevp.o threads.o wsaapi.o test2.o 

all: client

onvif_test: $(SERVER_OBJS)
	$(CC) $(INCLUDE) $(SERVER_OBJS) -o $(BIN)
client: $(CLIENT_OBJS)
	$(CC) $(INCLUDE) $(CLIENT_OBJS) -o $(CBIN) -luuid -lcrypto -lssl
clean:
	rm *.o
	rm $(BIN)
	rm $(CBIN)
	
