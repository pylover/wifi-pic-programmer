from os import path, makedirs

from appdirs import user_config_dir

from . import mdns


def get_cache_directory():
    import picfota
    version = picfota.__version__.split('.')[0]
    return user_config_dir(appname='picfota', version=version)


class Hosts(dict):
    _filename = None

    @property
    def filename(self):
        if self._filename is None:
            directory = get_cache_directory()
            self._filename = path.join(directory, 'hosts')
            if not path.exists(directory):
                makedirs(directory, exist_ok=True)

        return self._filename

    def load(self):
        if not path.exists(self.filename):
            return

        with open(self.filename) as f:
            for l in f.readlines():
                l = l.strip()
                if not l:
                    continue

                k, v = l.split(' ')
                self[k] = v

    def save(self):
        with open(self.filename, 'w') as f:
            for k, v in self.items():
                f.write(f'{k} {v}\n')

    def clear(self):
        super().clear()
        self.save()

    def __getitem__(self, hostname):
        try:
            result = super().__getitem__(hostname)
        except KeyError:
            host, port = mdns.resolve(f'{hostname}.')
            self[hostname] = f'{host}:{port}'
        else:
            host, port = result.split(':')
            port = int(port)

        return host, port

    def __enter__(self):
        self.load()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.save()

