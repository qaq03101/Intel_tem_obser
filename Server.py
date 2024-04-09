import socket
import threading

# 与单片机通信线程
def handle_udp(sock):
    global udp_address
    while True:
        data, address = sock.recvfrom(4096)
        udp_address = address
        if data:
            print(f"Received UDP data: {data.decode()} from {address}")
            if tcp_connection: # 如与app链接了,则转发数据到app
                data=(data.decode()+"\n").encode()
                try:
                    tcp_connection.sendall(data)
                    print('tcp发送')
                except BrokenPipeError:
                    print('tcp断开,未发送')


# 与app通信线程
def handle_tcp(connection, client_address):
    global tcp_connection
    tcp_connection = connection
    try:
        while True:
            data = connection.recv(4096)
            if data:
                print(f"Received TCP data: {data.decode()} from {client_address}")
                if udp_address: # 与单片机相连后,转发到单片机
                    print(data)
                    udp_sock.sendto(data, udp_address)

    finally:
        tcp_connection = None
        connection.close()

tcp_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcp_sock.setsockopt(socket.IPPROTO_TCP,socket.TCP_NODELAY,1)
tcp_server_address = ('', 8089)
tcp_sock.bind(tcp_server_address)
tcp_sock.listen(1)

udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
udp_server_address = ('', 8088)
udp_sock.bind(udp_server_address)

udp_address = None
tcp_connection = None

udp_thread = threading.Thread(target=handle_udp, args=(udp_sock,))
udp_thread.start()
# tcp请求监听
while True:
    connection, client_address = tcp_sock.accept()
    client_thread = threading.Thread(target=handle_tcp, args=(connection, client_address))
    client_thread.start()
