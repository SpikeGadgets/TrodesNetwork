from setuptools import setup, find_packages

setup(name='SpikeGadgets',
      version='0.1',
      description='SpikeGadgets Python toolbox',
      author='SpikeGadgets',
      license='GPLv3',
      packages=['spikegadgets'],
      install_requires=[
          'numpy',
      ],
      package_data={'spikegadgets':['trodesnetwork.so']},
      zip_safe=False)
