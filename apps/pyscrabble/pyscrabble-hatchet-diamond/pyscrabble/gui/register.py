import ConfigParser
import gtk
import urllib
import xmlrpclib
import socket
from pyscrabble.gui import options
from pyscrabble import constants
from pyscrabble import gtkconstants
from pyscrabble import gtkutil
from pyscrabble import lookup
from pyscrabble import manager
from pyscrabble import serialize
from pyscrabble import util
from twisted.internet import reactor

import sys
sys.path.append("../../../platform/build/bindings/python/")
sys.path.append("../../../platform/bindings/python/")
from libpydiamond import *
import threading
import gobject
from pyscrabble.lookup import *

def upper(data):
    return data.upper()

class RegisterWindow(gtk.Window):
    '''
    Window for Viewing the Public Server listing
    '''
    
    def __init__(self, lw):
        '''
        Constructor
        
        @param lw: Login Window instance
        '''
        
        gtk.Window.__init__(self, gtk.WINDOW_TOPLEVEL)
        self.connect("destroy", self.onDestroy )
        self.connect("delete_event", self.onDelete_event )
        self.set_size_request( constants.REGISTER_WINDOW_WIDTH, constants.REGISTER_WINDOW_HEIGHT )
        self.set_resizable( False )
        self.set_border_width( 10 )
        self.set_title(_('Find a Server'))
        
        self.loginWindow = lw
        
        box = gtk.VBox(False, 5)
        
        s = _("Server Listing")
        label = gtk.Label()
        label.set_markup("<b>%s</b>" % s)
        label.set_justify(gtk.JUSTIFY_CENTER)
        box.pack_start(label)
        
        box.pack_start(self.getView())
        box.pack_start(self.getToolbar())
        
        self.add(box)
        
        self.show_all()
        reactor.callLater(0.5, self.loadServers)
    
    def loadServers(self):
        '''
        Load servers
        '''
        self.servers = self.readServers()
        
        if not self.servers:
            self.destroy()
            return
        
        self.serverList.clear()
        
        for host,g_port,w_port,location in self.servers:
            
            try:
                loc = 'http://%s:%s/xmlrpc' % (host,w_port)
                s = xmlrpclib.Server(loc)
                numUsers = s.getNumUsers()
            except:
                numUsers="N/A"
            
            self.serverList.append( (host,location, numUsers) )
        
        self.toolBar.set_sensitive(True)
        
    
    def onDelete_event(self, widget, event, data=None):
        '''
        Callback when the widget is deleted
        
        @param widget:
        @param event:
        @param data:
        '''
        self.destroy()

    def onDestroy(self, widget, data=None):
        '''
        Callback when the widget is destroyed
        
        @param widget:
        @param data:
        '''
        pass
    
    def readServers(self):
        '''
        Read the server config file
        
        @return: Server config parser
        '''
        #try:
        #    file, headers = urllib.urlretrieve(constants.SERVER_LISTING_LOCATION)
        #except:
        #    self.error(util.ErrorMessage(_("Could not read server listing. Please try another time.")))
        #    return None
            
        #parser = ConfigParser.ConfigParser()
        #parser.read(file)
        
        servers = []
        #for server in parser.sections():
        #    servers.append( (server, parser.get(server, "g_port"), parser.get(server, "w_port"), parser.get(server, "location")) )
        
        servers.extend( util.getAdditionalHosts() )
        
        return servers
    
    def getView(self):
        '''
        Show treeview of servers
        
        @return gtk.ScrolledWindow
        '''
        
        self.serverList = gtk.ListStore(str, str, str)
        self.serverView = gtk.TreeView( self.serverList )
        
        col1 = gtk.TreeViewColumn(_('Name'))
        cell1 = gtk.CellRendererText()
        col1.pack_start(cell1, True)
        col1.add_attribute(cell1, 'text', 0)
        
        col2 = gtk.TreeViewColumn(_('Location'))
        cell2 = gtk.CellRendererText()
        col2.pack_start(cell2, True)
        col2.add_attribute(cell2, 'text', 1)
        
        col3 = gtk.TreeViewColumn(_('Registered Users'))
        cell3 = gtk.CellRendererText()
        col3.pack_start(cell3, True)
        col3.add_attribute(cell3, 'text', 2)
        
        self.serverView.append_column( col1 )
        self.serverView.append_column( col2 )
        self.serverView.append_column( col3 )
        
        self.serverList.append( (_('Loading...'), '', '') )
        
        win = gtk.ScrolledWindow()
        win.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        win.add(self.serverView)
        
        return win
    
    def getToolbar(self):
        '''
        Get toolbar
        
        @return: gtk.ButtonBox
        '''
        
        registerButton = gtkutil.createToolButton(gtkconstants.STOCK_REGISTER, gtkconstants.STOCK_REGISTER)
        registerButton.connect("clicked", self.showCreateUserDialog_cb)
        
        hostButton = gtkutil.createToolButton(gtkconstants.STOCK_ADD_HOSTNAME, gtkconstants.STOCK_ADD_HOSTNAME)
        hostButton.connect("clicked", self.addHost_cb)
        
        self.toolBar = gtk.Toolbar()
        self.toolBar.set_style(gtk.TOOLBAR_BOTH)
        self.toolBar.set_show_arrow(False)
        self.toolBar.set_icon_size(gtk.ICON_SIZE_SMALL_TOOLBAR)
        self.toolBar.insert(registerButton, 0)
        self.toolBar.insert(gtk.SeparatorToolItem(), 1)
        self.toolBar.insert(hostButton, 2)
        self.toolBar.insert(gtk.SeparatorToolItem(), 3)
        self.toolBar.set_sensitive(False)
        
        vbox = gtk.VBox(False, 5)
        vbox.pack_start(self.toolBar, False, False, 0)
        
        o = manager.OptionManager()
        
        button = gtk.CheckButton(_('Show public servers at startup'))
        button.set_active( o.get_default_bool_option(constants.OPTION_SHOW_PS, True) )
        button.connect("toggled", self.toggleOption_cb, constants.OPTION_SHOW_PS, o)
        vbox.pack_start(button, False, False, 0)
        
        return vbox
    
    def closeWindow_cb(self, widget):
        '''
        Close the window
        
        @param widget: Widget that was clicked to activate this callback
        '''
        self.destroy()
    
    def showCreateUserDialog_cb(self, button = None):
        '''
        Show dialog to create new user
        
        @param button: Widget that was clicked to activate this handler.
        '''
        
        s = _("Register New User")
        dialog = gtk.Dialog(title=s, flags=gtk.DIALOG_MODAL)
        dialog.vbox.set_border_width( 5 )
        
        header = gtk.Label()
        header.set_markup("<b><big>%s:</big></b>" % s)
        dialog.vbox.pack_start(header)
        
        s = _("Username")
        username = gtkutil.EntryWithLabel( label="%s:" % s)
        dialog.vbox.pack_start( username )
        
        s = _("Password")
        password1 = gtkutil.EntryWithLabel(label="%s: " %s, visibility=False)
        dialog.vbox.pack_start( password1 )
        
        s = _("Confirm Password")
        password2 = gtkutil.EntryWithLabel(label="%s: " %s, visibility=False)
        dialog.vbox.pack_start( password2 )
        
        okbutton = gtk.Button(_("Create"))
        cancelbutton = gtk.Button(_("Cancel"))
        
        dialog.action_area.pack_start(okbutton)
        dialog.action_area.pack_start(cancelbutton)
        
        okbutton.connect("clicked", self.createNewUser_cb, username, password1, password2, dialog)
        cancelbutton.connect("clicked", lambda b: dialog.destroy() )
        
        dialog.show_all()
    
    def createNewUser_cb(self, widget, username, password1, password2, dialog):
        '''
        
        @param widget:
        @param username:
        @param password1:
        @param password2:
        @param dialog:
        '''
        
        uname = username.get_text()
        pw1 = password1.get_text()
        pw2 = password2.get_text()
        
        if len(uname) == 0:
            self.error(util.ErrorMessage(_("You must enter a username")),dialog)
            return
        
        if len(pw1) == 0:
            self.error(util.ErrorMessage(_("You must enter a password")),dialog)
            return
        
        if pw1 != pw2:
            self.error(util.ErrorMessage(_("Passwords do not match")),dialog)
            return
        
        threading.Thread(target=self.createNewUserHelper, args=(uname, pw1)).start()
        
    def createNewUserHelper(self, username, password):
        hashedPassword = util.hashPassword(password)
        
        users = DStringList();
        DStringList.Map(users, "global:users")
        
        DObject.TransactionBegin()
        if username.upper() in map(upper, users.Members()):
            self.error(util.ErrorMessage(ServerMessage([USER_ALREADY_EXISTS])))
        
        if username in constants.RESERVED_NAMES:
            self.error(util.ErrorMessage(ServerMessage([USERNAME_NOT_ALLOWED])))
        
        if (len(username) > constants.MAX_NAME_LENGTH):
            self.error(util.ErrorMessage(ServerMessage([USERNAME_MUST_BE_LESS_THAN])))
        
        users.Append(username)
        
        pwString = DString()
        DString.Map(pwString, "user:" + username + ":hashedpw")
        pwString.Set(hashedPassword)
        DObject.TransactionCommit()
        
        gobject.idle_add(self.loginWindow.populateFields_cb, username, password, "DEBUG")
        gobject.idle_add(self.destroy)
    
    def addHost_cb(self, widget):
        '''
        Add the selectedserver to the host
        
        @param widget:
        '''
        self.toolBar.set_sensitive(False)
        options.OptionWindow(self, options.CONNECTION)
    
    def optionWindowClosed(self):
        '''
        Option window closed
        '''
        self.serverList.clear()
        self.serverList.append( (_('Loading...'), '', '') )
        reactor.callLater(0.5, self.loadServers)
    
    def error(self, data, parent=None):
        '''
        Show error dialog.
        
        @param data: ErrorMessage data
        @see: L{util.ErrorMessage}
        '''
        s = _("Error")
        if not parent:
            parent = self    
        self.dialog = gtk.MessageDialog(parent=parent, type=gtk.MESSAGE_ERROR, buttons=gtk.BUTTONS_OK, message_format="")
        self.dialog.set_markup("<big>%s: %s</big>" % (s, data.getErrorMessage()))
        self.dialog.connect("response", lambda w,e: self.dialog.destroy())
        self.dialog.show()
        self.dialog.run()
    
    def toggleOption_cb(self, widget, option, om):
        '''
        Preference toggled.
        
        Set the option name to the value of widget.get_active()
        
        @param widget: Widget that activated this callback
        @param option: Option name
        @param om: OptionManager
        '''
        om.set_option(option, int(widget.get_active()))
        
        
        
