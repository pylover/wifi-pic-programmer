import sys
import argparse
import traceback
from os import path


class Argument:
    def __init__(self, *a, **kw):
        self._args = a
        self._kwargs = kw

    def register(self, parser):
        return parser.add_argument(*self._args, **self._kwargs)


class Command:
    __arguments__ = []
    __command__ = None
    __help__ = None

    def __init__(self):
        self._parser = self._create_parser()

        _subcommands = []
        for a in self.__arguments__:
            if isinstance(a, Argument):
                a.register(self._parser)
            else:
                _subcommands.append(a)

        if _subcommands:
            self._subparsers = self._parser.add_subparsers(
                title='Sub commands',
                dest='command'
            )

            for p in _subcommands:
                p(self._subparsers)

    def _create_parser(self):
        raise NotImplementedError()

    def __call__(self, args):
        if self._parser:
            self._parser.print_help()


class SubCommand(Command):
    def __init__(self, subparsers):
        self._parent_subparsers = subparsers
        super().__init__()

    def _create_parser(self):
        assert self.__command__, 'Please provide a name for your command' \
            ' using __command__ class attribute'


        parser = self._parent_subparsers.add_parser(
            self.__command__,
            help=self.__help__
        )
        parser.set_defaults(func=self)
        return parser


class Root(Command):

    def __init__(self, argv=None):
        super().__init__()
        self.launch(argv)

    def launch(self, argv):
        args = self._parser.parse_args(argv)
        try:
            if hasattr(args, 'func'):
                exitcode = args.func(args)
            else:
                exitcode = self(args)
        except:
            traceback.print_exc()
            exitcode = 1

        sys.exit(exitcode or 0)

    @classmethod
    def _create_parser(self):
        return argparse.ArgumentParser(
            prog=path.basename(sys.argv[0]),
            description=self.__help__
        )
