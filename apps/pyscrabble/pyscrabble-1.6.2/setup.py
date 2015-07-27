# setup.py for pyscrabble
from distutils.core import setup
try:
    import py2exe
    HAS_PY2EXE = True
except ImportError:
    HAS_PY2EXE = False
import glob
import os
import pkg_resources
import sys
from pyscrabble.constants import VERSION
from pyscrabble import util
from pyscrabble import dist

def fix_path(item):
    if type(item) in (list, tuple):
        if 'config' in item[0]:
            return (item[0].replace('config', dist.get_app_data_dir()), item[1])
        else:
            return (item[0].replace('resources/', 'share/pyscrabble/'), item[1])
    else:
        return item

kwargs = {
    'name': 'pyscrabble',
    'version': VERSION,
    'author': 'Kevin Conaway',
    'author_email': 'kevin.a.conaway@gmail.com',
    'url': 'http://pyscrabble.sourceforge.net',
    'data_files': dist.getDataFiles(),
    'packages': ['pyscrabble', 'pyscrabble.command', 'pyscrabble.game', 'pyscrabble.gui', 'pyscrabble.net']
}

if HAS_PY2EXE and 'py2exe' in sys.argv:
    #eggpacks = pkg_resources.require("nevow")
    #for egg in eggpacks:
    #   if os.path.isdir(egg.location):
    #       sys.path.insert(0, egg.location)
    try:
        import modulefinder
        import win32com
        for p in win32com.__path__[1:]:
            modulefinder.AddPackagePath("win32com",p)
        for extra in ["win32com.shell"]:
            __import__(extra)
            m = sys.modules[extra]
            for p in m.__path__[1:]:
                modulefinder.addPackagePath(extra, p)
    except ImportError:
        print 'import error'
    kwargs['py_modules'] = ['pyscrabble-main', 'server_console', 'db_upgrade']
    kwargs['options'] = {
         "py2exe": {
             "packages": "encodings, nevow",
             "includes": "pango,atk,gobject,decimal,dumbdbm,dbhash,xml.sax.expatreader",
             "dll_excludes": ["iconv.dll","intl.dll","libatk-1.0-0.dll", 
                                 "libgdk_pixbuf-2.0-0.dll","libgdk-win32-2.0-0.dll",
                     "libglib-2.0-0.dll","libgmodule-2.0-0.dll",
                     "libgobject-2.0-0.dll","libgthread-2.0-0.dll",
                     "libgtk-win32-2.0-0.dll","libpango-1.0-0.dll",
                     "libpangowin32-1.0-0.dll"],
         }
    }
    kwargs['windows'] = [{ 
        "script": "pyscrabble-main.py",
        "icon_resources" : [(1, "resources/images/py.ico")]
    }]
    kwargs['console'] = [{
        "script": "server_service.py",
        "icon_resources" : [(1, "resources/images/py.ico")]
    }, {
        "script": "server_console.py",
        "icon_resources" : [(1, "resources/images/py.ico")]
    }]
    kwargs['service'] = ['server_service']
    kwargs['data_files'] += [('.', ['CHANGELOG.txt'])]
    kwargs['data_files'] += [('.', ['LICENSE.txt'])]
    #for egg in eggpacks:
    #    kwargs['data_files'] += dist.getResourceDirs(egg.location, ensureLower=False, basePath=None, outdir='extra')
else:
    kwargs['scripts'] = ['pyscrabble-main.py', 'server_console.py', 'db_upgrade.py']
    kwargs['data_files'] = [fix_path(x) for x in kwargs['data_files']]
    kwargs['cmdclass'] = {'install_lib': dist.InstallLib, 'install_scripts' : dist.InstallScripts}

setup(**kwargs)