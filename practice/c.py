import sys
import struct
import socket

FMT = '!BI'


def main():
    s = socket.socket()
    s.connect(('192.168.1.231', 8585))

    body = b'abcdefghijklmnopqrstuvwxyz' \
        b'abcdefghijklmnopqrstuvwxyz' \
        b'abcdefghijklmnopqrstuvwxyz'

    d = struct.pack(FMT, 1, len(body)) + body
    s.sendall(d)

    status, length = struct.unpack(FMT, s.recv(5))
    body = s.recv(length)
    print(status, length, body.decode())
    s.close()


if __name__ == '__main__':
    sys.exit(main());
