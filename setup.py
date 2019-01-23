
from setuptools import setup, find_packages
import os.path
import re

# reading package's version (same way sqlalchemy does)
with open(
    os.path.join(os.path.dirname(__file__), 'picfota', '__init__.py')
) as v_file:
    package_version = \
        re.compile('.*__version__ = \'(.*?)\'', re.S)\
        .match(v_file.read())\
        .group(1)


dependencies = [
    'zeroconf',
    'easycli',
    'appdirs'
]


setup(
    name='picfota',
    version=package_version,
    author='Vahid Mardani',
    author_email='vahid.mardani@gmail.com',
    url='http://github.com/pylover/wifi-pic-programmer',
    description='WIFI PIC programmer',
    long_description=open('README.md').read(),
    long_description_content_type='text/markdown',  # This is important!
    install_requires=dependencies,
    packages=find_packages(),
    entry_points={
        'console_scripts': [
            'picfota = picfota.root:PicFota'
        ]
    },
    license='MIT',
    classifiers=[
        'Environment :: Console',
        'Intended Audience :: Developers',
        'Natural Language :: English',
        'Development Status :: 1 - Planning',
        #'Development Status :: 5 - Production/Stable',
        'License :: OSI Approved :: MIT License',
        'Operating System :: OS Independent',
        'Programming Language :: Python :: 3.6',
        'Topic :: Software Development',
        ]
    )
