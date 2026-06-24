import os
import platform
import sys
import subprocess

here = os.path.dirname(__file__)


def r_out(script, args):
    return subprocess.run([sys.executable, os.path.join(here, script)] + args, check=True, stdout=subprocess.PIPE).stdout.decode('utf-8').strip()


if len(sys.argv) > 1 and sys.argv[1] == 'artifact_name':
    artifact_name = ''
    if platform.system() == "Linux":
        artifact_name = r_out('appimage.py', ['artifact_name'])
    if platform.system() == "Windows":
        artifact_name = r_out('windeploy.py', ['artifact_name'])
    if platform.system() == "Darwin":
        artifact_name = r_out('macdeploy.py', ['artifact_name'])
    print(artifact_name)
    exit(0)


def r(script):
    return subprocess.run([sys.executable, os.path.join(here, script)], check=True)


r('get_qt.py')
r('get_qt_ssl.py')

r('get_hunspell.py')
r('test.py')
r('build.py')

if platform.system() == "Linux":
    r('appimage.py')
if platform.system() == "Windows":
    r('windeploy.py')
if platform.system() == "Darwin":
    r('macdeploy.py')

github_env = os.environ.get('GITHUB_ENV')
if github_env:
    artifact_name = ''
    if platform.system() == "Linux":
        artifact_name = r_out('appimage.py', ['artifact_name'])
    elif platform.system() == "Windows":
        artifact_name = r_out('windeploy.py', ['artifact_name'])
    elif platform.system() == "Darwin":
        artifact_name = r_out('macdeploy.py', ['artifact_name'])
    
    if artifact_name:
        with open(github_env, 'a') as f:
            f.write(f"artifact={artifact_name}\n")

