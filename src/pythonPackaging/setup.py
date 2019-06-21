from setuptools import setup, find_packages
import sys

setup(name='SpikeGadgets',
      version='0.1',
      description='SpikeGadgets Python toolbox',
      author='SpikeGadgets',
      license='GPLv3',
      packages=['spikegadgets'],
      install_requires=[
          'numpy',
      ],
      package_data= {'spikegadgets': ['trodesnetwork.pyd', 'czmq.dll', 'libmlm.dll']} if sys.platform=='win32' else {'spikegadgets':['trodesnetwork.so']},
      zip_safe=False)
