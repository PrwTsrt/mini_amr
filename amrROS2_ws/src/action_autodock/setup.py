from setuptools import find_packages, setup
import os
from glob import glob

package_name = 'action_autodock'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
	(os.path.join('share', package_name, 'launch'), glob(os.path.join('launch', '*launch.[pxy][yma]*'))),
    ((os.path.join('share', package_name, 'config'), glob('config/*.yaml'))),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='poo',
    maintainer_email='poo@todo.todo',
    description='TODO: Package description',
    license='Apache-2.0',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'autodock_action_server = action_autodock.autodock_action_server:main'
        ],
    },
)
