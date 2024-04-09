# 自用板子没无线网卡,用以太网接电脑,电脑转发数据,本地简易的NAT转换

import socket
import threading
from time import sleep

# 自动获取本机无线网卡ip
def get_ip_address():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(("8.8.8.8", 80))
    ip = s.getsockname()[0]
    s.close()
    return ip


STM_addr = ('192.168.0.4', 8088)    # 单片机ip端口
eth = ('192.168.0.1', 8088)         # 以太网卡ip端口
server = ('123.249.8.75', 8088)     # 服务器ip端口
Nat = (get_ip_address(), 8087)      # 无线网卡ip端口

# 创建套接字
eth_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
Nat_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# 以太网卡,无线网卡的绑定
eth_sock.bind(eth)
Nat_sock.bind(Nat)

# 单片机数据转发到服务器线程
def handle_OUT():
    print('out,work')
    while True:
        data, addr = eth_sock.recvfrom(1024)
        print(data)
        sleep(1)
        ret = Nat_sock.sendto(data, server)
    eth_sock.close()

# 服务器包转发到单片机线程
def handle_in():
    print('in,work')
    while True:
        data, addr = Nat_sock.recvfrom(1024)
        print(data)
        eth_sock.sendto(data, STM_addr)
    Nat_sock.close()

# 线程创建
out_thread = threading.Thread(target=handle_OUT)
out_thread.start()
in_thread = threading.Thread(target=handle_in)
in_thread.start()
