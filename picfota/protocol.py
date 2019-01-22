import struct
import socket


SP_CMD_ECHO = 1
SP_CMD_PROGRAMMER_VERSION = 2



class Packet:
    header_format = '!BI'

    def __init__(self, status, body=None):
        self.status = status
        self.body = body

    @property
    def length(self):
        return len(self.body) if self.body else 0

    def dump(self):
        data = struct.pack(self.header_format, self.status, self.length)
        if self.body:
            data += body

        return data

    def send(self, s):
        s.send(self.dump())
        time.sleep(.2)
        return self.receive(s)

    @classmethod
    def receive(cls, s):
        head = s.recv(5)
        if len(head) < 5:
            raise InvalidHeaderLengthError()

        status, length = struct.unpack(self.header_format, head)
        if length:
            body = s.recv(length)
        else:
            body = None

        return cls(status, body)


class ClientProtocol:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def __enter__(self):
        self._socket.__enter__()
        self._socket.connect((self.host, self.port))
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self._socket.__exit__()

    def get_version(self):
        version = Packet(SP_CMD_PROGRAMMER_VERSION)
        response = version.send(self._socket)
        return response.body.decode()

