from socket import *

messageSize = 1024
serverName = 'localhost'
serverPort = 12000
clientSocket = socket(AF_INET, SOCK_DGRAM)
message = input('Input lowercase sentence: ')

clientSocket.sendto(message.encode(), (serverName, serverPort))
modifiedMessage, serverAddress = clientSocket.recvfrom(messageSize)
print(f"Received modified message: {modifiedMessage.decode()}")
clientSocket.close()
