import glob
import os
import sys
from distutils.command.install_lib import install_lib
from distutils.command.install_scripts import install_scripts

APP_NAME = 'pyscrabble'

def get_app_data_dir():
    '''
    Get application specific data dir
    
    @return: Application data directory
    '''
    if sys.platform == 'win32':
        try:
            from win32com.shell.shell import SHGetFolderPath
            from win32com.shell.shellcon import CSIDL_APPDATA, CSIDL_COMMON_APPDATA
            app_data_dir = SHGetFolderPath(0, CSIDL_APPDATA, 0, 0)
            if not os.path.exists(app_data_dir):
                app_data_dir = SHGetFolderPath(0, CSIDL_COMMON_APPDATA, 0, 0)
        except ImportError, inst:
            app_data_dir = 'C:\\'
        return os.path.join(app_data_dir, APP_NAME)
    else:
        return os.path.join(os.path.expanduser('~'), '.config',  APP_NAME)

try:
    from __installed__ import RESOURCE_PREFIX
except ImportError:
    RESOURCE_PREFIX = 'resources'

CONFIG_DIR = get_app_data_dir()
if not os.path.exists(CONFIG_DIR):
    CONFIG_DIR = os.path.join(RESOURCE_PREFIX, 'config')

def getLocaleDirs(dir, domain):
    l = []
    
    langs = os.listdir( dir )
    for lang in langs:
        d = os.path.join(dir,lang,"LC_MESSAGES")
        path = os.path.join(d, '%s.mo' % domain)
        l.append( (d,[path]) )
    
    return l

def getResourceDirs(dir, ensureLower=True, basePath = None, outdir=None):
    result = []
    absolute = os.path.abspath(dir)
    
    base = basePath
    if base is None:
        base = os.path.dirname(absolute)
    else:
        base = os.path.dirname( os.path.abspath(base) )
    
    for root, dirs, files in os.walk(absolute):
        if ensureLower and not os.path.basename(root).islower(): continue
        if len(files) > 0:
            f = []
            d = root[len(base)+1:]
            if outdir is not None:
                d = os.path.join(outdir, d)
            for file in files:
                f.append( os.path.join(root, file) )
            result.append( (d, f) )
    return result

def getDataFiles():
    return getLocaleDirs('resources/locale',APP_NAME) + \
           [('resources/images', glob.glob('resources/images/*.*')), \
            ('resources/sounds', glob.glob('resources/sounds/*.*')), \
            ('config', glob.glob('resources/config/*.cfg')), \
            ('resources/web', glob.glob('resources/web/*.*'))] + \
           getResourceDirs('resources/dict', True, 'resources') + \
           getResourceDirs('resources/letters', True, 'resources')

def ensure_config_dir(dir):
    '''
    Ensure config directory exists
    
    @param dir: Dir
    '''
    if not os.path.exists(dir):
        os.makedirs(dir)


class InstallScripts(install_scripts):
    '''
    install_scripts handler to strip any possible ^M's from files so they will run properly on unix
    '''
    
    def run(self):
        install_scripts.run(self)
        for file in self.get_outputs():
            self.fix(file)
    
    def fix(self, path):
        f = open(path, "rb")
        data = f.read().replace("\r\n", "\n")
        f.close()
        f = open(path, "w")
        f.write(data)
        f.close()
    

class InstallLib(install_lib):
    
    def generate_template(self):
        filename = os.path.join(self.build_dir, APP_NAME, '__installed__.py')
        self.mkpath(os.path.dirname(filename))
        
        install = self.distribution.get_command_obj('install')
        datadir = os.path.join(install.prefix, 'share', APP_NAME)
        
        fp = open(filename, 'w')
        fp.write('# Generated by setup.py do not modify\n')
        fp.write("RESOURCE_PREFIX = '%s'\n" % datadir)
        fp.close()

        return filename
    
    def install(self):
        template = self.generate_template()
        return install_lib.install(self) + [template]

class Resource(object):
    '''
    Filesystem resource
    '''
    
    def __init__(self, path=None):
        '''
        Constructor
        
        @param path: Resource path
        '''
        
        self.data = {}
        if path is not None:
            self.path = os.path.abspath(path)
            
            if os.path.isdir( self.path ):
                self.addSubResources()
    
    def addSubResources(self):
        '''
        Add sub resources
        '''
        for item in os.listdir( self.path ):
            r = os.path.join(self.path, item)
            if os.path.isdir(r):
                r = Resource(r)
            self[item] = r
    
    def __repr__(self):
        '''
        @return: Path
        '''
        return self.path
        
    def __getitem__(self, item):
        '''
        Get item
        
        @param item: Item
        @return: Resource item
        '''
        try:
            return self.data[item]
        except KeyError:
            return os.path.join(self.path, item)
    
    def __setitem__(self, item, value):
        '''
        Set item
        
        @param item:
        @param value:
        '''
        self.data[item] = value
        