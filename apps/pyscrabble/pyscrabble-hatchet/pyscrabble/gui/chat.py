from twisted.internet import reactor
from pyscrabble import gtkutil
from pyscrabble.gui.info import InfoWindow
from pyscrabble import constants
from pyscrabble import lookup
from pyscrabble import manager
from pyscrabble import util
import gtk
import pango

class ChatFrame(gtk.Frame):
    '''
    ChatFrame.
    
    This class displays the Chat window where all users on a server congregate.
    '''
    
    def __init__(self, client, main):
        '''
        Initialize the ChatFrame
        
        @param client: ScrabbleClient instance
        @param main: MainWindow instance
        @see: L{pyscrabble.net.client.ScrabbleClient}
        @see: L{pyscrabble.gui.main.MainWindow}
        '''
        
        gtk.Frame.__init__(self)
        self.client = client
        self.client.setChatWindow( self )
        self.mainwindow = main
        
        main = gtk.VBox( False, 10)
        
        self.messageWindowOpen = False
        self.serverInfoWindow = None
        
        main.pack_start( self.createGamesWindow(), True, True, 0 )
        
        self.tips = gtk.Tooltips()
        
        self.set_border_width( 10 )
        self.add( main )
        self.show_all()
            
    def createUsersWindow(self):
        '''
        Create TreeView of all users on the server
        
        @return: gtk.VBox containing TreeView of all users on the server.
        '''
        
        vbox = gtk.VBox(False, 1)
        
        self.userList = gtk.ListStore(str)
        
        self.client.getUserList()
        
        self.userView = gtk.TreeView( gtk.TreeModelSort(self.userList) )
        self.userView.connect("button-release-event", self.mainwindow.userListClicked_cb)
        self.userView.connect("button-press-event", self.mainwindow.userListClicked_cb)
        self.userView.set_headers_clickable(True)
        
        col = gtk.TreeViewColumn(_('Users'))
        cell = gtk.CellRendererText()
        col.pack_start(cell, True)
        col.add_attribute(cell, 'text', 0)
        col.set_sizing(gtk.TREE_VIEW_COLUMN_AUTOSIZE)
        col.set_sort_column_id(0)
        
        header = gtk.Label()
        s = _('All Users')
        header.set_markup("<b><big>%s:</big></b>" % s)
        header.set_justify( gtk.JUSTIFY_LEFT )
        
        self.userView.append_column( col )
        
        window = gtk.ScrolledWindow()
        window.set_policy( gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC )
        window.add( self.userView )
        
        vbox.pack_start(header, False, False, 1)
        vbox.pack_start(window, True, True, 0)
        
        return vbox
    
    def createGamesWindow(self):
        '''
        Create TreeView of all Games on the server.
        
        @return: gtk.VBox containing a TreeView of all Games on the server.
        '''
        
        vbox = gtk.VBox(False, 1)
        
        self.gameList = gtk.TreeStore(str, str, str)
        
        self.client.getGameList()
        
        self.gameView = gtk.TreeView( gtk.TreeModelSort(self.gameList) )
        self.gameView.set_headers_clickable(True)
        self.gameView.connect("button-press-event", self.gameListClicked_cb)
        
        title = _('Game')
        col1 = gtk.TreeViewColumn('%s' % title)
        col1.set_sizing(gtk.TREE_VIEW_COLUMN_AUTOSIZE)
        
        cell1 = gtk.CellRendererText()
        col1.pack_start(cell1, True)
        col1.add_attribute(cell1, 'text', 0)
        col1.set_sort_column_id(0)
        
        title = _('Players')
        col2 = gtk.TreeViewColumn('# %s' % title)
        
        cell2 = gtk.CellRendererText()
        col2.pack_start(cell2, True)
        col2.add_attribute(cell2, 'text', 1)
        col2.set_sort_column_id(1)
        
        title = _('Status')
        col3 = gtk.TreeViewColumn('%s' % title)
        cell3 = gtk.CellRendererText()
        col3.pack_start(cell3, True)
        col3.add_attribute(cell3, 'text', 2)
        col3.set_sort_column_id(2)
        
        self.gameView.append_column( col1 )
        self.gameView.append_column( col2 )
        self.gameView.append_column( col3 )
        
        header = gtk.Label()
        title = _('Game Listing')
        header.set_markup("<b><big>%s:</big></b>" % title)
        header.set_justify( gtk.JUSTIFY_LEFT )
        
        bbox = gtk.HButtonBox()
        self.joinButton = gtk.Button(label=_("Join Game"))
        
        self.joinButton.connect("clicked", self.joinGame_cb)
        
        bbox.add(self.joinButton)
        
        window = gtk.ScrolledWindow()
        window.set_policy( gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC )
        window.add( self.gameView )
        
        vbox.pack_start(header, False, False, 1)
        vbox.pack_start(window, True, True, 0)
        vbox.pack_start(bbox, False, False, 0)
        
        return vbox
    
    def error(self, data, enableButtons=False):
        '''
        Show error dialog.
        
        @param data: ErrorMessage data
        @see: L{util.ErrorMessage}
        '''
        
        if enableButtons:
            self.setGameButtonsState(True)
        
        e = _("Error")
        self.dialog = gtk.MessageDialog(parent=self.mainwindow, type=gtk.MESSAGE_ERROR, buttons=gtk.BUTTONS_OK, message_format="")
        self.dialog.set_markup("<big>%s: %s</big>" % (e, data.getErrorMessage()))
        self.dialog.connect("response", lambda w,e: self.dialog.destroy())
        self.dialog.show()
        self.dialog.run()
    
    def refreshUserList(self, users):
        '''
        Callback from ScrabbleClient.
        
        Refresh the list of users in the user TreeView
        
        @param users: List of users currently on the server
        '''
        
        self.userList.clear()
        for player in users:
            self.userList.append( [player.getUsername()] )
    
    # Join a game
    def joinGame_cb(self, button):
        '''
        Callback when 'Join Game' button is clicked.  Send request to the server to join the game.
        
        @param button: Button that was clicked to call this handler.
        '''
        self.setGameButtonsState(False)
        
        sel = self.gameView.get_selection()
        model, iter = sel.get_selected()
        if (iter == None):
            self.error(util.ErrorMessage(_("Please select a game to join.")), True)
            return
        
        # If the user clicks on one of the sub items, we need to get the root iter, which is the gameId
        parent = model.iter_parent(iter)
        if parent is not None:
            iter = parent
            
        gameName = model.get(iter, 0)[0]
        self.joinGame( gameName )
        
    
    def joinGame(self, gameName):
        '''
        Attempt to join a game
        
        @param gameName: Game name
        '''
        if (self.mainwindow.hasJoinedGame(gameName)):
            self.error(util.ErrorMessage(_("You have already joined that game.")), True)
            return
        else:
            self.client.joinGame(gameName)
        
    
    # Show dialog to create a new game
    def createGameWindow(self, button = None):
        '''
        Show the dialog to create a new game
        
        @param button: Widget that was clicked to activate this handler.
        '''
        
        s = _("Create New Game")
        self.gamedialog = gtk.Dialog(title="%s" %s , flags=gtk.DIALOG_MODAL)
        self.gamedialog.vbox.set_spacing( 10 )
        self.gamedialog.vbox.set_border_width( 5 )
        self.gamedialog.vbox.set_homogeneous( False )
        main = gtk.VBox(False, 5)
        
        header = gtk.Label()
        header.set_markup("<b><big>%s:</big></b>" %s)
        main.pack_start( header )
        
        s = _("Game Name")
        self.createGameEntry = gtkutil.EntryWithLabel(label="%s: " % s, maxLength=constants.MAX_NAME_LENGTH)
        main.pack_start( self.createGameEntry )
        
        s = _("Options")
        main.pack_start( gtk.Label("%s:" % s) )
        
        centerOption = gtk.CheckButton(_("Center square is double word score"))
        centerOption.set_active(True)
        rankedOption = gtk.CheckButton(_("Official Game"))
        rankedOption.set_active(True)
        showCountOption = gtk.CheckButton(_("Show letter distribution"))
        showCountOption.set_active(True)
        
        timeBox = gtk.HBox(False, 2)
        limitBox = gtk.HBox(False, 2)
        moveTimeBox = gtk.HBox(False, 2)
        limitOption = gtk.CheckButton( _('Optional Overtime Limit') )
        untimedOption = gtk.RadioButton(None, _('Untimed') )
        timedOption = gtk.RadioButton(untimedOption, _('Timed Game') )
        moveTimeOption = gtk.RadioButton(untimedOption, _('Timed Moves') )
        
        a = gtk.Adjustment(value=25, lower=1, upper=100, step_incr=1, page_incr=1, page_size=1)
        timeControl = gtk.SpinButton(a, climb_rate=1, digits=0)
        timeBox.pack_start(timedOption, False, False, 0)
        timeBox.pack_start(timeControl, False, False, 5)
        timedOption.set_active(False)
        timedOption.connect("toggled", lambda w, a, b: [x.set_sensitive(w.get_active()) for x in (a,b)], timeControl, limitBox)
        timedOption.connect("toggled", lambda w, a: a.set_active(w.get_active()), limitOption)
        timeControl.set_sensitive(False)
        
        a = gtk.Adjustment(value=1, lower=1, upper=100, step_incr=1, page_incr=1, page_size=1)
        limitControl = gtk.SpinButton(a, climb_rate=1, digits=0)
        limitBox.pack_start(limitOption, False, False, 20)
        limitBox.pack_start(limitControl, False, False, 5)
        limitOption.connect("toggled", lambda w, a: a.set_sensitive(w.get_active()), limitControl)
        limitBox.set_sensitive(False)
        
        self.tips.set_tip(rankedOption, _('If this game is Official, the outcome will be marked in your statistics'))
        self.tips.set_tip(limitOption, _('If you set the optional overtime limit, a player will be allowed to go over the normal game time, but the player will be penalized 10 points for every overtime minute'))
        self.tips.set_tip(timedOption, _('Select this option if you wish to limit the overall time each player has for the entire game'))
        self.tips.set_tip(moveTimeOption, _('Select this option if you wish to limit the time a player has for each move'))
        self.tips.set_tip(showCountOption, _('Select this option if you wish to show a count of the letters remaining in the bag during the game'))
        
        a = gtk.Adjustment(value=3, lower=1, upper=100, step_incr=1, page_incr=1, page_size=1)
        moveTimeControl = gtk.SpinButton(a, climb_rate=1, digits=0)
        moveTimeBox.pack_start(moveTimeOption, False, False, 0)
        moveTimeBox.pack_start(moveTimeControl, False, False, 5)
        moveTimeOption.connect("toggled", lambda w, a: a.set_sensitive(w.get_active()), moveTimeControl)
        moveTimeControl.set_sensitive(False)
        
        box = gtk.VBox(False, 5)
        box.pack_start(untimedOption, False, False, 0)
        box.pack_start(timeBox, False, False, 0)
        box.pack_start(limitBox, False, False, 0)
        box.pack_start(moveTimeBox, False, False, 0)
        
        exp = gtk.Expander( _('Timing Options') )
        exp.add(box)
        exp.set_spacing(10)
        
        main.pack_start( centerOption, False, False, 3 )
        main.pack_start( rankedOption, False, False, 3 )
        main.pack_start( showCountOption, False, False, 3 )

        box = gtk.HBox(False, 3)
        box.pack_start(gtk.Label(_('Rules:')), False, False, 3)
        
        model = gtk.ListStore(str,str)
        
        l = manager.LocaleManager()
        locales = l.getAvailableLocales()
        for locale in locales:
            model.append( [ _(l.getLocaleDescription(locale)), locale] )
        
        cell = gtk.CellRendererText()
        combo = gtk.ComboBox(model)
        combo.pack_start(cell)
        combo.add_attribute(cell, 'text', 0)
        
        cur = None
        for locale in locales:
            if l.locale == locale:
                cur = locale
        
        if cur is not None:
            i = model.get_iter_first()
            while i:
                if model.get_value(i, 1) == cur:
                    combo.set_active_iter(i)
                    break
                i = model.iter_next(i)
        else:
            combo.set_active(0)
        
        box.pack_start(combo, False, False, 0)
        main.pack_start( box, False, False, 3 )
        
        okbutton = gtk.Button(_("Create"))
        cancelbutton = gtk.Button(_("Cancel"))
        
        self.gamedialog.vbox.pack_start(main, False, False, 0)
        self.gamedialog.vbox.pack_start(exp, False, False, 0)
        
        self.gamedialog.action_area.pack_start(okbutton)
        self.gamedialog.action_area.pack_start(cancelbutton)
        
        okbutton.connect("clicked", self.createGame, centerOption, rankedOption, showCountOption, combo, timedOption, timeControl, limitOption, limitControl, moveTimeOption, moveTimeControl)
        cancelbutton.connect("clicked", lambda b: self.gamedialog.destroy() )
        
        self.gamedialog.show_all()
        self.gamedialog.run()
    
    # Create a game
    def createGame(self, button, centerOption, rankedOption, showCountOption, combo, timedOption, timeControl, limitOption, limitControl, moveTimeOption, moveTimeControl):
        '''
        Create a game
        
        @param button: Widget that was clicked to activate this handler.
        @param centerOption: Option widget
        @param rankedOption: Option widget
        param showCountOption: Option widget
        @param combo: Rules widget
        @param timedOption: Time option
        @param timeControl: Time control widget
        @param limitOption: Time limit option
        @param limitControl: Time limit control
        @param moveTimeOption: Move time option
        @param moveTimeControl: Move time control
        '''
        
        gameId = self.createGameEntry.get_text()
        if len(gameId) == 0:
            self.error(util.ErrorMessage(_("Please enter a Game ID of at least one character.")))
            return
        
        model = combo.get_model()
        iter = combo.get_active_iter()
        opt = model.get_value(iter, 1)
        
        options = {}
        options[lookup.OPTION_CENTER_TILE] = centerOption.get_active()
        options[lookup.OPTION_SHOW_COUNT] = showCountOption.get_active()
        options[lookup.OPTION_RANKED] = rankedOption.get_active()
        options[lookup.OPTION_RULES] = opt
        if timedOption.get_active():
            options[lookup.OPTION_TIMED_GAME] = long(timeControl.get_value_as_int())
        if limitOption.get_active():
            options[lookup.OPTION_TIMED_LIMIT] = long(limitControl.get_value_as_int())
        if moveTimeOption.get_active():
            options[lookup.OPTION_MOVE_TIME] = long(moveTimeControl.get_value_as_int())
        
        self.gamedialog.destroy()
        self.client.createGame( gameId, options )
    
    
    # Show/Refresh game list
    def showGameList(self, games):
        '''
        Callback from Scrabble Client when a game is added or removed from the Game Listing
        
        @param games: List of ScrabbleGameInfo
        @see: L{pyscrabble.game.game.ScrabbleGameInfo}
        '''
        
        self.gameList.clear()
        for game in games:
            iter = self.gameList.append(None, (game.getName(), str(game.getNumberOfPlayers()), game.getStatus()) )
    
    # Set the current game ID
    def newGame(self, gameId, spectating, options):
        '''
        Notify the main window to create a new game tab
        
        @param gameId: Game ID
        @param spectating: True if the user is watching the game
        @param options: Game options dict
        '''
        
        self.setGameButtonsState(True)
        
        self.mainwindow.newGame( gameId, spectating, options )
        
    
    def hasFocus(self, widget=None, event=None):
        '''
        Callback that the window has focus
        
        Give focus to the chat entry
        '''
        pass

    def infoWindow(self, data):
        '''
        Show Information dialog
        
        @param data: Information text
        '''
        
        s = _("Info")
        self.dialog = gtk.MessageDialog(parent=None, type=gtk.MESSAGE_INFO, buttons=gtk.BUTTONS_OK, message_format="")
        self.dialog.set_title("%s" % s)
        self.dialog.set_markup("<big>%s: %s</big>" % (s,data))
        self.dialog.connect("response", lambda w,e: self.dialog.destroy())
        self.dialog.show()
        self.dialog.run()
    
    def setGameButtonsState(self, flag):
        '''
        Set game buttons to be sensitive
        
        @param flag: Sensitive flag
        '''
        self.joinButton.set_sensitive(flag)
    
    def gameListClicked_cb(self, widget, event):
        '''
        Callback for when the gamelist is clicked.  If its a double click, attempt to join the selected game
        
        @param widget:
        @param event:
        '''
        if event.type == gtk.gdk._2BUTTON_PRESS:
            self.joinGame_cb(None)
        
        
        
        
        
        
        
