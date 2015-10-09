#!/usr/bin/env python
from twisted.internet import gtk2reactor
gtk2reactor.install()
import warnings
warnings.filterwarnings("ignore")

from pyscrabble import constants
import gettext
gettext.install(constants.LOCALE_DOMAIN, constants.LOCALE_DIR)

from pyscrabble import manager
l = manager.LocaleManager()

from pyscrabble import dist
from pyscrabble import gtkconstants
from pyscrabble import gtkutil
from twisted.internet import reactor
from pyscrabble.gui.login import LoginWindow
import gtk

import sys
sys.path.append("/Users/Niel/systems/diamond-src/backend/build/src/bindings/python")
sys.path.append("/home/nl35/research/diamond-src/backend/build/src/bindings/python")
from libpydiamond import *

if __name__ == '__main__':
   
    DiamondInit("coldwater.cs.washington.edu");
   
    dist.ensure_config_dir( dist.CONFIG_DIR )
    
    gtk.rc_parse_string( gtkconstants.DEFAULT_THEME )
    gtkutil.setupStockItems()
    
    o = manager.OptionManager()
    def callback(): LoginWindow()
    
    if o.get_default_bool_option(constants.OPTION_SHOW_TIPS, True):
        from pyscrabble.gui import tip
        tip.TipWindow(tip=tip.WELCOME_TIP, callback=callback)
    else:
        callback()
    
    reactor.run()
