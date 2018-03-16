
import sys
import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

server_addr = ('127.0.0.1', 12049)

sock.bind(server_addr)

sock.listen(1)

while True:
	conn_sock, _ = sock.accept()
	data = conn_sock.recv(1024)
	print data
	conn_sock.close()
