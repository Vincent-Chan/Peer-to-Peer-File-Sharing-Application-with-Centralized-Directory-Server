from socket import *

messageSize = 1024
serverName = "localhost"
serverPort = 12000
clientSocket = socket(AF_INET, SOCK_STREAM)
clientSocket.connect((serverName,serverPort))

sentence = input('Input lowercase sentence: ')
clientSocket.send(sentence.encode())
modifiedSentence = clientSocket.recv(messageSize)

print(f"From Server: {modifiedSentence.decode()}")

clientSocket.close()
