

class ProgrammerError(Exception):
    def __init__(self, response: 'Packet'):
        self._response = response
        super().__init__(str(response))


class ProgrammerNotDetectedError(ProgrammerError):
    pass
