#!/usr/bin/env python

try:
    from setuptools import setup
except ImportError:
    import distribute_setup
    distribute_setup.use_setuptools()
    from setuptools import setup


setup(
    name='ap1302py',
    version='0.1.0',
    license='GPLv2',
    packages=['ap1302py'],
    package_dir={'ap1302py': 'ap1302py'},
    entry_points={
        'console_scripts': [
            'vdlg_ap1302 = ap1302py.__main__:main',
        ]
    },
    install_requires=[
        'python_periphery>=2.1.0',
        'pytimerfd>=1.2',
    ],
    classifiers=[
        'Development Status :: 4 - Beta',
        'Environment :: Console',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: GNU General Public License v2 (GPLv2)',
        'Operating System :: Linux',
        'Programming Language :: Python :: 3',
        'Topic :: Utilities',
        'Topic :: Multimedia :: Video',
    ],
)