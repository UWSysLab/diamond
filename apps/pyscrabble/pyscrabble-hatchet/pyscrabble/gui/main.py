from twisted.internet import reactor
from pyscrabble.gui.chat import ChatFrame
from pyscrabble.gui.game import GameFrame
from pyscrabble.constants import *
from pyscrabble.gtkconstants import *
from pyscrabble.gui.options import OptionWindow
from pyscrabble import manager
from pyscrabble import gtkutil
from pyscrabble import util
import gtk
import ConfigParser


class MainWindow(gtk.Window):
    '''
    MainWindow is the top level window of the application.  It holds the menubar, status bar and the notebook
    which contains the chat room and game windows.
    '''
    
    def __init__(self, client, username=''):
        '''
        
        Initialize the main window.
        
        @param client: ScrabbleClient reference
        @param username: Username
        '''
        
        gtk.Window.__init__( self, gtk.WINDOW_TOPLEVEL )
        
        self.connect("destroy", self.onDestroy )
        self.connect("delete_event", self.onDelete_event )
        self.connect("focus-in-event", self.focus_cb)
        
        self.add_events(gtk.gdk.KEY_PRESS_MASK)
        self.connect_after("key-press-event", self.keyPress_cb)
        
        self.isFullScreen = False
        self.username = username
        self.set_title( "PyScrabble - %s" % username )
        self.set_default_size( DEFAULT_WIDTH, DEFAULT_HEIGHT )
        
        # Reference to client socket
        self.client = client
        self.client.setMainWindow( self )
        
        self.loggingOut = False
        
        # List of game names
        self.games = {}
        
        # List of messages
        self.messages = {}
        
        # Read options
        self.optionmanager = manager.OptionManager()
        
        #self.soundmanager = manager.SoundManager()
        
        self.optionWindowShown = False
        vbox = gtk.VBox(False, 1)
        notebook = self.createNotebook()
        self.menubar = self.createMenuBar()
        vbox.pack_start(self.menubar, False, False, 0)
        vbox.pack_start(notebook, True, True, 0)
        
        self.add( vbox )
        self.maximize()
        self.show_all()
    
    def onDelete_event(self, widget, event=None, data=None):
        '''
        Callback when the widget is deleted
        
        @param widget:
        @param event:
        @param data:
        '''
        
        if len(self.games) != 0:
            s = _("Are you sure you want to quit")
            dialog = gtk.MessageDialog(parent=None, type=gtk.MESSAGE_QUESTION, buttons=gtk.BUTTONS_YES_NO, message_format="")
            dialog.set_title(s)
            dialog.set_markup("<big>%s?</big>" % s)
            dialog.show()
            response = dialog.run()
            
            dialog.destroy()
            
            if (response != gtk.RESPONSE_YES):
                return True
                
        self.loggingOut = True
        self.client.logout()
        return True
        #self.stopReactor()

    def onDestroy(self, widget, data=None):
        '''
        Callback when the widget is destroyed
        
        @param widget:
        @param data:
        '''
        pass
    
    def stopReactor(self, data=None):
        '''
        Stop the reactor. This will kill the program.
        
        @param data:
        '''
        reactor.stop()
    
    def createNotebook(self):
        '''
        Create the notebook with a ChatFrame
        
        @return gtk.Notebook
        '''
        self.notebook = gtk.Notebook()
        self.notebook.set_tab_pos( gtk.POS_TOP )
        
        self.chatwin = ChatFrame(self.client, self)
        
        # Chat frame
        self.notebook.append_page( self.chatwin, gtk.Label(_("Chat")) )
        
        self.notebook.show_all()
        
        self.notebook.connect_after("switch-page", self.switchPage)
        
        return self.notebook
    
    def createMenuBar(self):
        '''
        Create the main menu bar
        
        
        @return: gtk.Menubar
        '''
        ui = gtk.UIManager()
        self.add_accel_group( ui.get_accel_group() )
        ui.add_ui_from_string(MENU_DATA)
        
        group = gtk.ActionGroup('MainMenuBarGroup')
        group.add_actions([('Exit', gtk.STOCK_QUIT, _('_Exit'), None, None, self.onDelete_event),
                           ('Create Game', None, _('_Create Game'), '<control>G', None, self.chatwin.createGameWindow),
                           ('Preferences', gtk.STOCK_PREFERENCES, _('_Preferences'), '<control>P', None, self.showOptions),
                           ('Online Help', gtk.STOCK_HELP, _('_Online Help'), None, None, util.showHelp),
                           ('About', gtk.STOCK_ABOUT, _('_About'), None, None, gtkutil.showAbout),
                           ('Help', None, _('_Help')),
                           ('Server', None, _('_Server')),
                           ('View', None, _('_View')),
                           ('Options', None, _('_Tools')),
                           ('File', None, _('_File'))])
        
        group.add_toggle_actions([('Full Screen', None, _('_Full Screen'), 'F11', 'Toggle Full Screen Mode', self.fullScreen_cb)])
        
        ui.insert_action_group(group, 0)
        return ui.get_widget('/MainMenuBar')
    
    ### Menu Item callbacks ###
    
    def newGame(self, gameId, spectating, options):
        '''
        Add a new game window to the notebook
        
        @param gameId: Game ID to add
        @param spectating: True if the user is spectating
        @param options: Game options dict
        '''
        frame = GameFrame(self.client, self, gameId, spectating, options)
        window = gtk.ScrolledWindow()
        window.add_with_viewport( frame )
        window.set_policy( gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC )
        window.get_child().set_shadow_type( gtk.SHADOW_NONE )
        window.show()
        
        self.games[gameId] = window
        
        box = gtk.HBox(False, 0)
        button = gtk.Button(label=None)
        button.connect("clicked", frame.askLeaveGame_cb)
        button.add( gtk.image_new_from_stock( gtk.STOCK_CLOSE, gtk.ICON_SIZE_MENU ) )
        button.set_relief( gtk.RELIEF_NONE )
        
        focus_pad = button.style_get_property('focus-padding')
        focus_wid = button.style_get_property('focus-line-width')
        wid, height = gtk.icon_size_lookup(gtk.ICON_SIZE_MENU)
        wid += (focus_pad + focus_wid) * 2
        height += (focus_pad + focus_wid) * 2
        button.set_size_request( wid, height )
        
        box.pack_start(gtk.Label("%s " % gameId), False, False, 0)
        box.pack_start(button, False, False, 0)
        box.show_all()
        
        page_num = self.notebook.append_page( window , box )
        self.notebook.set_current_page(page_num)
    
    def hasJoinedGame(self, gameId):
        '''
        Check if the player has joined a game
        
        @param gameId: Game ID to check.
        @return: True if the player has joined the game.
        '''
        return gameId in self.games
    
    def removeGame(self, gameId):
        '''
        Destroy a game window
        
        
        @param gameId: Game ID to destroy
        '''
        self.games[gameId].destroy()
        del self.games[gameId]
    
    def setCurrentTurn(self, gameId, isCurrentTurn):
        '''
        Mark the label of the game indicating whether it is the players turn.
        
        
        @param gameId: Game ID of which this player has control
        @param isCurrentTurn: If True, mark the label bold.
        '''
    
        widget = self.games[gameId]
        
        box = self.notebook.get_tab_label(widget)
        label = box.get_children().pop(0)
        
        if isCurrentTurn:
            label.set_markup("<b>%s</b>" % label.get_text())
            
            #if self.optionmanager.get_default_bool_option(OPTION_SOUND_TURN, True):
            #    self.soundmanager.play(SOUND_GAME_OPTION)
            if self.optionmanager.get_default_bool_option(OPTION_POPUP_TURN, True):
                p = gtkutil.Popup( title=gameId, text=_("Its your turn"))
            
        else:
            label.set_markup("%s" % label.get_text())
        
        #button.set_label(label)
        #self.notebook.set_tab_label(widget, button)

    def switchPage(self, notebook, page, page_num):
        '''
        Callback when the notebook page is switched.
        
        If we are switching to a chat window, set the label to normal text.
        
        
        @param notebook: Notebook
        @param page: Current page
        @param page_num: Current page number
        '''
        if self.notebook.page_num(self.chatwin) == page_num:
            label = self.notebook.get_tab_label(self.chatwin)
            label.set_markup("%s" % label.get_text())
            self.chatwin.hasFocus()
        
        frame = self.notebook.get_nth_page(page_num)
            
        if isinstance(frame, gtk.ScrolledWindow):
            frame = frame.get_child().get_child()
            frame.notifyFocus()
    
    def fatalError(self, msg):
        '''
        Call the fatal error handler
        
        @param msg: ErrorMessage
        '''
        
        if not self.loggingOut:
            gtkutil.fatalError(msg)
    
    def userListClicked_cb(self, widget, event):
        '''
        Callback when the userlist is clicked on.
        
        If its a right click, show the private message list
        
        @param widget:
        @param event:
        '''
        
        user = self.getSelectedItem(widget)
        
        if (event.button == 3 and user is not None):
            message_menu = gtk.Menu()
            
            item = gtk.ImageMenuItem(stock_id=gtk.STOCK_DIALOG_INFO)
            item.connect("activate", self.chatwin.requestUserInfo, user)
            message_menu.add(item)
            
            message_menu.add( gtk.SeparatorMenuItem() )
            
            item = gtk.ImageMenuItem(stock_id=STOCK_SEND_IM)
            item.connect("activate", self.sendPrivateMessage, user)
            message_menu.add(item)
            
            
            message_menu.show_all()
            message_menu.popup(None, None, None, event.button, event.time)
        
        if event.type == gtk.gdk._2BUTTON_PRESS and user is not None:
            self.chatwin.requestUserInfo(widget, user)
    
    def getSelectedItem(self, view):
        sel = view.get_selection()
        model, iter = sel.get_selected()
        
        if (iter == None):
            return None
        
        return model.get_value(iter, 0)
    
    def showOptions(self, widget):
        if not self.optionWindowShown:
            self.optionWindowShown = True
            x = OptionWindow(self)
        
    def optionWindowClosed(self):
        '''
        Mark the option window as closed and refresh gameboards with colors
        '''
        self.optionWindowShown = False
        for window in self.games.itervalues():
            window.get_child().get_child().refreshOptions()
    
    def showImDialog_cb(self, widget):
        '''
        Show dialog to send new Private message
        
        @param widget: widget that activated this callback
        '''
        s = _("Send Private Message")
        dialog = gtk.Dialog(title="%s" % s, parent=None, buttons=(gtk.STOCK_OK, gtk.RESPONSE_OK, gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL))
        dialog.set_default_response(gtk.RESPONSE_OK)
        dialog.vbox.set_spacing( 10 )
        
        s = _("Player's Name")
        header = gtk.Label()
        header.set_markup("<b><big>%s:</big></b>" % s)
        dialog.vbox.pack_start( header )
        
        entry = gtk.Entry()
        entry.set_width_chars(5)
        entry.connect("key-press-event", self.privateMessageDialogKeypress_cb, dialog)
        dialog.vbox.pack_start( entry )
        
        dialog.show_all()
        response = dialog.run()
        
        name = None
        if response == gtk.RESPONSE_OK:
            name = entry.get_text()
        
        dialog.destroy()
        
        if name:
            self.sendPrivateMessage(None, name, data=None)       
    
    def focus_cb(self, widget, event):
        if not event.in_:
            return
        
        page = self.notebook.get_nth_page( self.notebook.get_current_page() )
        if isinstance(page, gtk.ScrolledWindow):
            frame = page.get_child().get_child()
            frame.notifyFocus()
    
    def gameFrameHasFocus(self, frame):
        '''
        Return True if the Widget has focus in the notebook
        
        @param frame: GameFrame
        @return: True if the GameFrame is focused in the notebook
        '''
        page = self.notebook.get_nth_page( self.notebook.get_current_page() )
        if isinstance(page, gtk.ScrolledWindow):
            f = page.get_child().get_child()
            if f == frame:
                return True
        
        return False
        
            
    def fullScreen_cb(self, widget):
        '''
        Toggle fullscreen
        
        @param widget: Widget that activated this callback
        '''
        if widget.get_active():
            self.isFullScreen = True
            self.fullscreen()
            self.notebook.set_show_tabs(False)
            self.menubar.hide()
        else:
            self.isFullScreen = False
            self.unfullscreen()
            self.notebook.set_show_tabs(True)
            self.menubar.show_all()
    
    def keyPress_cb(self, widget, event):
        '''
        
        @param widget:
        @param event:
        '''
        if event.state & gtk.gdk.CONTROL_MASK:
            if event.keyval == gtk.keysyms.a:
                page = self.notebook.get_nth_page( self.notebook.get_current_page() )
                if isinstance(page, gtk.ScrolledWindow):
                    f = page.get_child().get_child()
                    if isinstance(f, GameFrame):
                        f.selectAllLetters()
