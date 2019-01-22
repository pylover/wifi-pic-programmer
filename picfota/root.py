from easycli import Root, Argument

from .programmer import Programmer


class PicFota(Root):
    __help__ = 'WIFI PIC Programmer'

    __arguments__ = [
        Argument('-V', '--version', action='store_true', help='Show version'),
        Programmer,
    ]

    def __call__(self, args):
        if args.version:
            import picfota
            print(picfota.__version__)
            return

        return super().__call__(args)

