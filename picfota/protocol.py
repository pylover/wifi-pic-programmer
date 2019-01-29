import time
import struct
import socket

from .exceptions import ProgrammerNotDetectedError


SP_CMD_ECHO = 1
SP_CMD_PROGRAMMER_VERSION = 2
SP_CMD_DEVICE = 3

# Protocol Errors
SP_OK = 0
SP_ERR_INVALID_COMMAND = 1
SP_ERR_REQ_LEN = 2
SP_ERR_DEVICE_NOT_DETECTED = 3


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

        status, length = struct.unpack(cls.header_format, head)
        if length:
            body = s.recv(length)
        else:
            body = None

        return cls(status, body)

    @property
    def ok(self):
        return self.status == SP_OK

    def __str__(self):
        text = self.body.decode()

        if self.status != SP_OK:
            return f'Programmer Error: {text}'

        return text

    def __repr__(self):
        return f'<Packet status={self.status}>{str(self)}</Packet>'


class WifiProgrammer:
    version = None
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def __enter__(self):
        self._socket.__enter__()
        self._socket.connect((self.host, self.port))
        version_request = Packet(SP_CMD_PROGRAMMER_VERSION)
        response = version_request.send(self._socket)
        if response.status != 0:
            raise ProgrammerNotDetectedError(response)

        self.version = str(response)
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        return self._socket.__exit__()

    def get_device_info(self):
        info = Packet(SP_CMD_DEVICE)
        response = info.send(self._socket)
        return response

