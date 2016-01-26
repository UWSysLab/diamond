from twisted.internet import reactor
import pygtk
import gtk
from pyscrabble.net.client import ScrabbleClient
from pyscrabble.gui.main import MainWindow
from pyscrabble.gtkconstants import *
from pyscrabble.constants import *
from pyscrabble.gui.register import *
from pyscrabble.gui import options
from pyscrabble import manager
from pyscrabble import util
from pyscrabble import gtkutil

import sys
sys.path.append("../../../platform/build/bindings/python/")
sys.path.append("../../../platform/bindings/python/")
from libpydiamond import *
import threading
import gobject
from pyscrabble.lookup import *

class LoginWindow(gtk.Window):
    '''
    The LoginWindow class presents a Window for the user to log in to the server
    '''
    
    def __init__(self):
        '''
        Constructor.
        
        Show the LoginWindow.
        '''
        
        gtk.Window.__init__( self, gtk.WINDOW_TOPLEVEL )
        self.connect("delete_event", self.delete_event )
        
        self.set_title( "PyScrabble" )
        self.set_default_size( 300, 200 )
        
        self.optionWindowShown = False
        
        left_right = gtk.HBox( False, 20 )
        main = gtk.VBox( False, 20)
        
        left_right.pack_start( self.getLabels(), False, False, 0 )
        left_right.pack_start( self.getEntries(), False, False, 0 )
        left_right.set_border_width( 10 )
        
        main.pack_start( self.createMenuBar(), False, False, 0)
        main.pack_start( self.getHeaderLabel(), False, False, 0 )
        main.pack_start( left_right, False, False, 0 )
        main.pack_start( self.getButtons(), False, False, 0 )
        
        r = manager.ResourceManager()
        
        history_file = file(r["config"][SERVER_HISTORY], "ab+")
        self.history = []
        for server in reversed(history_file.read().split()):
            self.hostname.append_text(server)
            self.history.append(server)
        history_file.close()
        
        self.restoreCredentials()
        
        #self.set_border_width( 10 )
        self.add( main )
        self.show_all()
        
        self.loggingOut = False
        
        self.client = None
        
        o = manager.OptionManager()
        if o.get_default_bool_option(OPTION_SHOW_PS, True):
            self.findServer_cb(None)
    
    def clickLogin(self, widget, data=None):
        '''
        Callback when the user clicks the login button.
        
        @param widget: Button that was clicked
        @param data:
        '''
        
        username = self.username.get_text()
        password = util.hashPassword( self.password.get_text() )
        
        threading.Thread(target=self.clickLoginHelper, args=(username, password)).start()
    
    def clickLoginHelper(self, username, password):
        pwString = DString()
        DString.Map(pwString, "user:" + username + ":hashedpw")
        
        DObject.TransactionBegin()
        if pwString.Value() == password:
            gobject.idle_add(self.loginOK)
        else:
            gobject.idle_add(self.error, util.ErrorMessage(ServerMessage([INVALID_USERNAME_PASSWORD])))
        DObject.TransactionCommit()
    
    def loginOK(self):
        '''
        Callback from server if the user is authenticated.
        
        Start the MainWindow
        '''
        
        o = manager.OptionManager()
        if o.get_default_bool_option(OPTION_SAVE_LOGIN, True):
            o.set_option(OPTION_SAVE_UNAME, self.username.get_text())
            o.set_option(OPTION_SAVE_PWORD, self.password.get_text())
        
        MainWindow(self.username.get_text())
        self.destroy()
    
    def error(self, data):
        '''
        Callback from the server when an error happens
        
        @param data: ErrorMessage data
        '''
        s = _("Error")
        self.dialog = gtk.MessageDialog(parent=self, type=gtk.MESSAGE_ERROR, buttons=gtk.BUTTONS_OK, message_format="")
        self.dialog.set_markup("<big>%s: %s</big>" % (s, data.getErrorMessage()))
        self.dialog.connect("response", lambda w,e: self.dialog.destroy())
        self.dialog.show()
        self.loginButton.set_sensitive(True)
    
    def getLabels(self):
        '''
        Get screen labels
        
        @return: gtk.VBox
        '''
        
        left = gtk.VBox( False, 10 )
        
        # Add Username label
        s = _("Username")
        username_t = gtk.Label("%s: " %s)
        username_t.set_justify( gtk.JUSTIFY_LEFT )
        
        # Add Password label
        s = _("Password")
        password_t = gtk.Label("%s: " % s)
        password_t.set_justify( gtk.JUSTIFY_LEFT )
        
        # Add Host label
        s = _("Hostname")
        hostname_t = gtk.Label("%s: " % s)
        hostname_t.set_justify( gtk.JUSTIFY_LEFT )
        
        left.pack_start(username_t, False, False, 5)
        left.pack_start(password_t, False, False, 3)
        left.pack_start(hostname_t, False, False, 0)
        
        return left

    def getEntries(self):
        '''
        Get entries
        
        @return: gtk.VBox containg gtk.Entry's
        '''
        
        
        right = gtk.VBox( False, 10 )
        
        self.username = gtk.Entry()
        
        self.password = gtk.Entry()
        self.password.set_visibility( False )
        
        #self.hostname = gtk.Entry()
        self.hostname = gtk.combo_box_entry_new_text()
        
        self.username.connect("key-press-event", self.entryKeyPress_cb)
        self.password.connect("key-press-event", self.entryKeyPress_cb)
        self.hostname.get_child().connect("key-press-event", self.entryKeyPress_cb)
        
        right.pack_start(self.username, False, False, 0)
        right.pack_start(self.password, False, False, 0)
        right.pack_start(self.hostname, False, False, 0)
        
        return right

    def getHeaderLabel(self):
        '''
        Get the header label
        
        @return: gtk.Label
        '''
        
        s = _("Welcome to PyScrabble")
        header = gtk.Label ()
        header.set_justify( gtk.JUSTIFY_LEFT )
        header.set_markup("""<b><big>%s</big></b>""" % s)
        
        return header

    def getButtons(self):
        '''
        Get the Buttons
        
        @return: ButtonBox
        '''
        
        box = gtk.VButtonBox()
        box.set_border_width(5)
        box.set_spacing(0)
        
        button = gtk.Button(label=_("Login"))
        button.set_image( gtk.image_new_from_stock(gtk.STOCK_APPLY, gtk.ICON_SIZE_MENU))
        button.connect("clicked", self.clickLogin)
        button.set_relief( gtk.RELIEF_NONE )
        box.add(button)
        self.loginButton = button
        
        button = gtk.Button(label=_("Find Public Servers"))
        button.set_image( gtk.image_new_from_stock(gtk.STOCK_FIND, gtk.ICON_SIZE_MENU) )
        button.connect("clicked", self.findServer_cb)
        button.set_relief( gtk.RELIEF_NONE )
        box.add(button)
        
        return box


    def delete_event(self, widget=None, event=None, data=None):
        '''
        Delete event callback
        
        @param widget:
        @param event:
        @param data:
        '''
        self.stopReactor()
    
    def stopReactor(self):
        '''
        Stop the reactor
        '''
        reactor.stop()
    
    def createMenuBar(self):
        '''
        Create the main menu bar
        
        @return gtk.MenuBar
        '''
        ui = gtk.UIManager()
        self.add_accel_group( ui.get_accel_group() )
        ui.add_ui_from_string(LOGIN_MENU_DATA)
        
        group = gtk.ActionGroup('MainMenuBarGroup')
        group.add_actions([('Exit', gtk.STOCK_QUIT, _('_Exit'), None, None, self.delete_event),
                           ('Preferences', gtk.STOCK_PREFERENCES, _('_Preferences'), '<control>P', None, self.showOptions),
                           ('Online Help', gtk.STOCK_HELP, _('_Online Help'), None, None, util.showHelp),
                           ('About', gtk.STOCK_ABOUT, _('_About'), None, None, gtkutil.showAbout),
                           ('Help', None, _('_Help')),
                           ('Options', None, _('_Tools')),
                           ('File', None, _('_File'))])
        
        ui.insert_action_group(group, 0)
        return ui.get_widget('/MainMenuBar')
    
    def fatalError(self, msg):
        '''
        Call the fatal error handler
        
        @param msg: ErrorMessage
        '''
        if not self.loggingOut:
            gtkutil.fatalError(msg)
    
    def findServer_cb(self, widget):
        '''
        Show the find server window
        
        @param widget: Widget that activated this callback
        '''
        x = RegisterWindow(self)
    
    
    def restoreCredentials(self):
        '''
        If the user has a preference to save their username/password/host, restore it here
        '''
        o = manager.OptionManager()
        
        if o.get_default_bool_option(OPTION_SAVE_LOGIN, True):
            uname = o.get_default_option(OPTION_SAVE_UNAME, '')
            pword = o.get_default_option(OPTION_SAVE_PWORD, '')
            host = o.get_default_option(OPTION_SAVE_HOST, '')
            self.populateFields_cb(uname,pword,host)
            
    def populateFields_cb(self, username, password, host):
        '''
        Populate the login fields
        
        @param username: Username
        @param password: Password
        @param host: Hostname
        '''
        if username is not None:
            self.username.set_text( username )
        
        if password is not None:
            self.password.set_text( password )
        
        if host not in self.history and host is not '':
            self.hostname.append_text(host)
        
        model = self.hostname.get_model()
        iter = model.get_iter_first()
        
        while iter:
            val = model.get_value(iter, 0)
            if val == host:
                self.hostname.set_active_iter(iter)
            iter = model.iter_next(iter)
    
    def showOptions(self, widget):
        if not self.optionWindowShown:
            self.optionWindowShown = True
            x = options.OptionWindow(self)
        
    def optionWindowClosed(self):
        self.optionWindowShown = False
    
    def entryKeyPress_cb(self, widget, event):
        '''
        Key press event on one of the entries
        
        If its a return, send the login command
        
        @param widget:
        @param event:
        '''
        
        if event.keyval == gtk.keysyms.Return:
            self.clickLogin(self.loginButton, None)