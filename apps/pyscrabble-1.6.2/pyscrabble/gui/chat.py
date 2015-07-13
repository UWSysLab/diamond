from twisted.internet import reactor
from pyscrabble import gtkutil
from pyscrabble.gui.info import InfoWindow
from pyscrabble.gui import message
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
        
        self.connect("realize", self.onRealize_cb)
        
        main = gtk.VBox( False, 10)
        
        self.messageWindowOpen = False
        self.serverInfoWindow = None
        
        main.pack_start( self.createChatWindow(), True, True, 0 )
        main.pack_start( self.createEntryWindow(), False, False, 0 )
        
        self.tips = gtk.Tooltips()
        
        self.set_border_width( 10 )
        self.add( main )
        self.show_all()
        
        self.client.checkMessages()
    
    def onRealize_cb(self, widget):
        '''
        Realize the window
        
        @param: widget
        '''
        self.entry.grab_focus()
        self.set_focus_chain([self.entry])
        
    
    def createChatWindow(self):
        '''
        Create the chat TextView and User view
        
        @return: gtk.HBox containg main chat window and user treeview
        '''
        
        sizer = gtk.HBox( False, 10 )
        
        self.chat = gtkutil.TaggableTextView(buffer=None)
        self.chat.set_editable( False )
        self.chat.set_cursor_visible( False )
        self.chat.set_wrap_mode( gtk.WRAP_WORD )
        
        window = gtk.ScrolledWindow()
        window.add( self.chat )
        window.set_policy( gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC )
        sizer.pack_start( window, True, True, 0 )
        sizer.pack_start( self.createUsersWindow(), False, False, 0 )
        sizer.pack_start(self.createGamesWindow(), False, False, 0)
        return sizer
    
    def createEntryWindow(self):
        '''
        Create the chat entry window
        
        @return: gtk.HBox with chat entry window.
        '''
        
        sizer = gtk.HBox( False, 10 )
        
        self.entry = gtk.Entry()
        self.entry.connect("key-press-event", self.submitChat)
        self.entry.set_flags ( gtk.CAN_FOCUS )
        self.entry.grab_focus()
        
        sizer.pack_start(self.entry, True, True, 0)
        sizer.set_focus_child(self.entry)
        
        return sizer
    
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
        self.spectateButton = gtk.Button(label=_("Watch Game"))
        
        self.spectateButton.connect("clicked", self.spectateGame)
        self.joinButton.connect("clicked", self.joinGame_cb)
        
        bbox.add(self.joinButton)
        bbox.add(self.spectateButton)
        
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
        
    def submitChat(self, widget, event, data=None):
        '''
        Submit chat message to server
        
        @param widget:
        @param event:
        @param data:
        '''
        
        
        if (event.keyval == gtk.keysyms.Return):
            if (self.entry.get_text() != None and len(self.entry.get_text()) > 0):
                self.client.postChatMessage( self.entry.get_text() )
                self.entry.set_text( '' )
                return True
        
        return False
    
    def refreshUserList(self, users):
        '''
        Callback from ScrabbleClient.
        
        Refresh the list of users in the user TreeView
        
        @param users: List of users currently on the server
        '''
        
        self.userList.clear()
        for player in users:
            self.userList.append( [player.getUsername()] )
    
    def userJoinChat(self, user):
        '''
        Callback from ScrabbleClient when another user joins the chat room
        
        @param user: User joining chat
        '''
        o = manager.OptionManager()
        if not self.mainwindow.is_active():
            #if o.get_default_bool_option(constants.OPTION_SOUND_NEW_USER, True):
            #    self.mainwindow.soundmanager.play(constants.SOUND_MSG_OPTION)
            if o.get_default_bool_option(constants.OPTION_POPUP_NEW_USER, True):
                p = gtkutil.Popup( title=user, text='%s %s' % (user, lookup.SERVER_MESSAGE_LOOKUP[lookup.LOGGED_IN]))
        
        self.userList.append( [user] )
    
    def receiveChatMessage(self, msg):
        '''
        Callback from ScrabbleClient when a chat message is posted.
        
        @param msg: Chat text to post in buffer
        '''
        txt,server = msg # Server will be true if its a Server message
        if server:
            buf = self.chat.get_buffer()
            t = buf.create_tag(weight=pango.WEIGHT_BOLD)
            self.chat.insert_text_with_tags(txt, t)
        else:
            self.chat.insert_text(txt)
        self.mainwindow.notifyChatMessage()
    
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
            for key,value in game.options:
                if not isinstance(value,str):
                    value = str(value)
                self.gameList.append(iter, (str(key), value, ""))
    
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
    
    # Change password dialog
    def changePasswordDialog(self, button = None):
        '''
        Show dialog to change password
        
        @param button: Widget that was clicked to activate this handler.
        '''
        
        s = _("Change Password")
        changepasswordDialog = gtk.Dialog(title="%s" % s, flags=gtk.DIALOG_MODAL)
        changepasswordDialog.vbox.set_border_width( 5 )
        
        header = gtk.Label()
        header.set_markup("<b><big>%s:</big></b>" % s)
        changepasswordDialog.vbox.pack_start(header)
        
        s = _("Old Password")
        oldpassword = gtkutil.EntryWithLabel(label="%s: " % s, visibility=False)
        changepasswordDialog.vbox.pack_start( oldpassword )
        
        s = _("Password")
        password1 = gtkutil.EntryWithLabel(label="%s: " % s, visibility=False)
        changepasswordDialog.vbox.pack_start( password1 )
        
        s = _("Confirm Password")
        password2 = gtkutil.EntryWithLabel(label="%s: " % s, visibility=False)
        changepasswordDialog.vbox.pack_start( password2 )
        
        okbutton = gtk.Button(_("Change"))
        cancelbutton = gtk.Button(_("Cancel"))
        
        changepasswordDialog.action_area.pack_start(okbutton)
        changepasswordDialog.action_area.pack_start(cancelbutton)
        
        okbutton.connect("clicked", self.changePassword, oldpassword, password1, password2, changepasswordDialog)
        cancelbutton.connect("clicked", lambda b: changepasswordDialog.destroy() )
        
        changepasswordDialog.show_all()
        
    
    # Change password
    def changePassword(self, button, oldpassword, password1, password2, dialog):
        '''
        Ask server to change password
        
        @param button: Widget that was clicked to activate this handler
        @param oldpassword: Old password widget
        @param password1: New Password widget
        @param password2: New Password Confirmation widget
        @param dialog: Change Password dialog widget
        '''
        
        if password1.get_text() != password2.get_text():
            self.error(util.ErrorMessage(_("Passwords don't match.")))
            return
        
        self.client.changePassword(util.hashPassword(oldpassword.get_text()), util.hashPassword(password1.get_text()))
        dialog.destroy()
    
    def sendPrivateMessage(self, username, data):
        self.mainwindow.sendPrivateMessage(widget=None, username=username, data=data)
    
    
    def spectateGame(self, button):
        '''
        Callback when 'Watch Game' button is clicked.  Send request to the server to watch the game.
        
        @param button: Button that was clicked to call this handler.
        '''
        
        self.setGameButtonsState(False)
        
        sel = self.gameView.get_selection()
        model, iter = sel.get_selected()
        if (iter == None):
            self.error(util.ErrorMessage(_("Please select a game to join.")),True)
            return
        
        gameName = model.get(iter, 0)[0]
        
        if (self.mainwindow.hasJoinedGame(gameName)):
            self.error(util.ErrorMessage(_("You have already joined that game.")),True)
            return
        else:
            self.client.spectateGame(gameName)
    
    def hasFocus(self, widget=None, event=None):
        '''
        Callback that the window has focus
        
        Give focus to the chat entry
        '''
        self.entry.grab_focus()

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
    
    def requestUserInfo(self, widget, username):
        '''
        Request user information
        
        @param widget: Widget that activated this callback
        @param username: Username
        '''
        self.client.requestUserInfo(util.getUnicode(username))
    
    def showUserInfo(self, user):
        '''
        Show user information
        
        @param user: User object
        '''
        data = []
        data.append( (_("Wins"), str(user.getNumericStat(constants.STAT_WINS))) )
        data.append( (_("Losses"), str(user.getNumericStat(constants.STAT_LOSSES))) )
        data.append( (_("Ties"), str(user.getNumericStat(constants.STAT_TIES))) )
        data.append( (_("Rank"), user.rankName) )
        data.append( (_("Member since"), str(user.getCreatedDate())) )
        data.append( (_('Last login'), str(user.getLastLoginDate())) )
        data.append( (_('Status'), str(user.status)) )
        s = _('User Information')
        x = InfoWindow(title='%s - %s' % (s, user.getUsername()), header=s, main=None)
        x.initialize()
        
        x.appendPage(gtk.Label(user.getUsername()), [_('Name'),_('Value')],data,visible=False)
        
        cols = [_('Name'),_('Wins'),_('Losses'),_('Ties')]
        data = []
        for name,record in user.record.iteritems():
            data.append( (name, int(record["w"]), int(record["l"]), int(record["t"])) )
        
        x.appendPage(gtk.Label(_("Record")), cols, data, visible=True, sortable=True, signals={ "button-release-event" : self.mainwindow.userListClicked_cb, "button-press-event" : self.mainwindow.userListClicked_cb })
        
        
        data = []
        for action in reversed(user.audit):
            data.append( (str(action), str(action.date)) )
        x.appendPage(gtk.Label(_('History')), [_('Name'),_('Value')],data,visible=False)
        
        x.show_all()
    
    def requestServerStats(self, widget):
        '''
        Request server stats
        
        @param widget: Widget that activated this callback
        '''
        if self.serverInfoWindow is None:
            s = _('Server Information')
            self.serverInfoWindow = InfoWindow(title=s, header=s, main=self)
            reactor.callLater(0.5, self.client.requestServerStats)
            
    def showServerStats(self, stats):
        '''
        Show server stats
        
        @param stats: Stats
        '''
        stats,users,ranks = stats
        
        cols = [_('Name'), _('Value')]
        
        stts = []
        for key,val in stats:
            stts.append( (repr(key), val) ) #key is a ServerMessage, call repr to display val
        
        self.serverInfoWindow.initialize()
        
        self.serverInfoWindow.appendPage(gtk.Label(_("Statistics")), cols, stts, visible=False)
        
        cols = [_('Name'),_('Wins'),_('Losses'),_('Ties'),_('Rank')]
        
        self.serverInfoWindow.appendPage(gtk.Label(_("Users")), cols, users, visible=True, sortable=True, signals={ "button-release-event" : self.mainwindow.userListClicked_cb, "button-press-event" : self.mainwindow.userListClicked_cb })
        self.serverInfoWindow.appendPage(gtk.Label(_("Rankings")), [_('Rank'),_('Wins Required')], ranks, visible=True)
        self.serverInfoWindow.show_all()
    
    def serverInfoClosed_cb(self):
        '''
        Callback that the server info window has been closed
        '''
        self.serverInfoWindow = None
        
    
    def showOfflineMessages(self, messages):
        '''
        Show offline messages
        
        @param messages: List of PrivateMessages
        '''
        
        if len(messages) > 0:
            self.messageWindowOpen = True
            x = message.OfflineMessageWindow(self,messages)
        else:
            s = _('You do not have any messages')
            self.infoWindow(s)
    
    def messageWindowClosed(self):
        '''
        Notify this widget that the message window is closed
        '''
        self.messageWindowOpen = False
    
    def getMessages_cb(self, widget):
        '''
        Check offline messages
        
        @param widget: Widget that activated this callback
        '''
        if not self.messageWindowOpen:
            self.client.getMessages()
    
    def deleteMessage(self, id):
        '''
        Delete message
        
        @param id: Message ID
        '''
        self.client.deleteMessage(id)
    
    def setGameButtonsState(self, flag):
        '''
        Set game buttons to be sensitive
        
        @param flag: Sensitive flag
        '''
        self.joinButton.set_sensitive(flag)
        self.spectateButton.set_sensitive(flag)
    
    def gameListClicked_cb(self, widget, event):
        '''
        Callback for when the gamelist is clicked.  If its a double click, attempt to join the selected game
        
        @param widget:
        @param event:
        '''
        if event.type == gtk.gdk._2BUTTON_PRESS:
            self.joinGame_cb(None)
        
        
        
        
        
        
        
