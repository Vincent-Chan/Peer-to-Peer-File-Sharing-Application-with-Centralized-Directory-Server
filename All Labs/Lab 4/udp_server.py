from socket import *

messageSize = 1024
serverPort = 12000
serverSocket = socket(AF_INET, SOCK_DGRAM)
serverSocket.bind(('', serverPort))
print ("The server is ready to receive")

while True:
    message, clientAddress = serverSocket.recvfrom(messageSize)
    print(f"From {clientAddress}: {message.decode()}")
    modifiedMessage = message.decode().upper()
    serverSocket.sendto(modifiedMessage.encode(), clientAddress)
