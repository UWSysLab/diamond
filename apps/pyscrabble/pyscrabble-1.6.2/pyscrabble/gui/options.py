import gtk
import wave
from pyscrabble.constants import *
from pyscrabble.lookup import *
from pyscrabble import manager
from pyscrabble import gtkconstants
from pyscrabble import util
from pyscrabble import gtkutil

COLORS = _('Colors')
CONNECTION = _('Connection')
GAMEPLAY = _('Gameplay')
LOCALE = _("Locale")
NOTIFICATIONS =_("Notifications")
SOUNDS = _("Sounds")

class OptionWindow(gtk.Window):
    '''
    Options Window
    '''
    
    def __init__(self, mainwindow, section=None):
        '''
        Constructor
        
        @param mainwindow: Mainwindow instance
        '''
        
        gtk.Window.__init__(self, gtk.WINDOW_TOPLEVEL)
        self.connect("destroy", self.onDestroy )
        self.connect("delete_event", self.onDelete_event )
        self.set_title(_('Options'))
        
        self.mainwindow = mainwindow
        self.section = section
        
        self.optionmanager = manager.OptionManager()
        
        self.tips = gtk.Tooltips()
        
        self.set_default_size( OPTION_WINDOW_WIDTH, OPTION_WINDOW_HEIGHT )
        
        hbox = gtk.HBox(False, 10)
        hbox.pack_start( self.getOptionsMenu(), False, False, 0)
        hbox.pack_start( self.getOptionsFrame(), True, True, 0)
        
        self.add( hbox )
        
        #self.set_resizable( False )
        self.set_border_width( 10 )
        self.show_all()
    
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
        self.mainwindow.optionWindowClosed()
        pass
    
    def getOptionsMenu(self):
        '''
        Left nav option menu
        
        @return: Options Menu gtk.Frame
        '''
        
        frame = gtk.Frame()
        frame.set_shadow_type( gtk.SHADOW_OUT )
        
        self.optionsList = gtk.ListStore(str)
        self.optionsView = gtk.TreeView( self.optionsList )
        self.optionsView.set_headers_visible( False )
        self.optionsView.connect("button-release-event", self.optionsViewClicked_cb)
        
        col1 = gtk.TreeViewColumn(_('Name'))
        cell1 = gtk.CellRendererText()
        col1.pack_start(cell1, True)
        col1.add_attribute(cell1, 'text', 0)
        cell1.set_property('xpad', 5)
        cell1.set_property('ypad', 5)
                
        self.optionsView.append_column( col1 )
        
        self.optionsList.append( [COLORS] )
        self.optionsList.append( [CONNECTION] )
        self.optionsList.append( [GAMEPLAY] )
        self.optionsList.append( [LOCALE] )
        self.optionsList.append( [NOTIFICATIONS] )
        self.optionsList.append( [SOUNDS] )
        
        
        frame.add( self.optionsView )
        
        return frame
    
    def getOptionsFrame(self):
        '''
        Main options frame
        
        @return: Main Options gtk.Frame
        '''
        
        self.optionFrame = gtk.Frame()
        self.optionFrame.set_label(_("Preferences"))
        
        # Default to Colors frame
        box = gtk.HBox(False,20)    
        box.set_border_width( 20 )
        
        box.pack_start(self.getColorPrefs(), False, False, 0)
        self.optionFrame.add( box )
        
        if self.section is not None:
            self.showOptionFrame(self.section)
            i = self.optionsList.get_iter_first()
            while i:
                val = self.optionsList.get_value(i, 0)
                if val == self.section:
                    self.optionsView.get_selection().select_iter(i)
                i = self.optionsList.iter_next(i)
        
        return self.optionFrame
    
    def getColorPrefs(self):
        '''
        Show Color Pickers
        
        @return: gtk.Box
        '''
        self.optionFrame.set_label(_("Color Preferences"))
        box = gtk.VBox(False, 3)
        
        b = gtkutil.createColorPreference(COLOR_NEW_TILE, DEFAULT_NEW_TILE, USE_COLOR_NEW_TILE, _('Tile color for current moves: '), self.tips, _('This will be the color of tiles you are currently playing'))
        box.pack_start( b, False, False, 5 )
        
        b = gtkutil.createColorPreference(COLOR_RECENT_TILE, DEFAULT_RECENT_TILE, USE_COLOR_RECENT_TILE, _('Tile color for new moves: '), self.tips, _('This will be the color of tiles that were recently played'))
        box.pack_start( b, False, False, 5 )
        
        b = gtkutil.createColorPreference(COLOR_BLANK_TILE, DEFAULT_BLANK_TILE, USE_COLOR_BLANK_TILE, _('Text color for blank tiles: '), self.tips, _('This will be the color of text on Blank tiles'))
        box.pack_start( b, False, False, 5 )
        
        b = gtkutil.createColorPreference(COLOR_NORMAL_TILE, TILE_COLORS[TILE_NORMAL], USE_COLOR_NORMAL_TILE, _('Background color for normal tiles: '), self.tips, _('This will be the background color for normal tiles'))
        box.pack_start( b, False, False, 5 )
        
        b = gtkutil.createColorPreference(COLOR_TEXT, DEFAULT_COLOR_TEXT, USE_COLOR_TEXT, _('Text color for letter tiles: '), self.tips, _('This will be the text color for letters on the tiles'))
        box.pack_start( b, False, False, 5 )
        
        b = gtkutil.createColorPreference(COLOR_LETTER, TILE_COLORS[TILE_LETTER], USE_COLOR_LETTER, _('Background color for letters: '), self.tips, _('This will be the background color for letters'))
        box.pack_start( b, False, False, 5 )
        
        return box
        
    
    def getNotificationPrefs(self):
        '''
        Notification Options
        
        @return: gtk.Box
        '''
        self.optionFrame.set_label(_("Notification Preferences")) 
        box = gtk.VButtonBox()
        
        soundOnTurn = gtk.CheckButton(_("Play sounds on turn"))
        popupOnTurn = gtk.CheckButton(_("Popup on turn"))
        soundOnMessage = gtk.CheckButton(_("Play sound on new message"))
        popupOnMessage = gtk.CheckButton(_("Popup on new message"))
        soundOnNewUser = gtk.CheckButton(_("Play sound when a user signs on"))
        popupOnNewUser = gtk.CheckButton(_("Popup when a new user signs on"))
        
        soundOnTurn.set_active( self.optionmanager.get_default_bool_option(OPTION_SOUND_TURN, True) )
        popupOnTurn.set_active( self.optionmanager.get_default_bool_option(OPTION_POPUP_TURN, True) )
        soundOnMessage.set_active( self.optionmanager.get_default_bool_option(OPTION_SOUND_MSG, True) )
        popupOnMessage.set_active( self.optionmanager.get_default_bool_option(OPTION_POPUP_MSG, True) )
        soundOnNewUser.set_active( self.optionmanager.get_default_bool_option(OPTION_SOUND_NEW_USER, True) )
        popupOnNewUser.set_active( self.optionmanager.get_default_bool_option(OPTION_POPUP_NEW_USER, True) )
        
        soundOnTurn.connect("toggled", self.toggleOption_cb, OPTION_SOUND_TURN)
        popupOnTurn.connect("toggled", self.toggleOption_cb, OPTION_POPUP_TURN)
        soundOnMessage.connect("toggled", self.toggleOption_cb, OPTION_SOUND_MSG)
        popupOnMessage.connect("toggled", self.toggleOption_cb, OPTION_POPUP_MSG)
        soundOnNewUser.connect("toggled", self.toggleOption_cb, OPTION_SOUND_NEW_USER)
        popupOnNewUser.connect("toggled", self.toggleOption_cb, OPTION_POPUP_NEW_USER)
        
        popupOnMessage.set_alignment(0.0,0.0)
        
        box.add( soundOnTurn )
        box.add( popupOnTurn )
        box.add( soundOnMessage )
        box.add( popupOnMessage )
        box.add( soundOnNewUser )
        box.add( popupOnNewUser )
        
        vbox = gtk.VBox(False, 3)
        vbox.pack_start(box, False, False, 0)
        
        opt = int(self.optionmanager.get_default_option(OPTION_POPUP_TIMEOUT, gtkutil.Popup.TIMEOUT))
        
        hbox = gtk.HBox(False, 3)
        a = gtk.Adjustment(value=opt, lower=1, upper=100, step_incr=1, page_incr=1, page_size=1)
        spin = gtk.SpinButton(a, climb_rate=1, digits=0)
        spin.connect("value-changed", self.spinChanged_cb, OPTION_POPUP_TIMEOUT)
        hbox.pack_start(spin, False, False, 0)
        hbox.pack_start(gtk.Label(_('Popup Timeout')), False, False, 0)
        
        self.tips.set_tip(spin, _('Number of seconds a popup window is displayed'))
        
        vbox.pack_start(hbox, False, False, 0)
        
        return vbox
    
    def getLocalePrefs(self):
        '''
        Locale prefs
        
        @return: gtk.Box
        '''
        self.optionFrame.set_label(_("Locale Preferences"))
        box = gtk.VBox(False)
        box.set_border_width(3)
        
        box.pack_start( gtk.Label(_('Locale:')), False, False, 10 )
        
        l = manager.LocaleManager()
        model = gtk.ListStore(str,str)
        
        for key in l.getAvailableLocales():
            desc = l.getLocaleDescription(key)
            model.append( [_(desc), key] )
        
        cell = gtk.CellRendererText()
        combo = gtk.ComboBox(model)
        combo.pack_start(cell)
        combo.add_attribute(cell, 'text', 0)
        
        box.pack_start(combo, False, False, 0)
        
        button = gtk.Button( _("Save") )
        button.connect("clicked", self.saveLocalePrefs_cb, combo)
        
        bbox = gtk.HButtonBox()
        bbox.add(button)
        box.pack_start(bbox, False, False, 10)
        
        o = manager.OptionManager()
        opt = o.get_default_option(LOCALE_OPTION, None)
        
        if not opt:
            combo.set_active_iter(model.get_iter_first())
        else:
            i = model.get_iter_first()
            while i:
                val = model.get_value(i, 1)
                if val == opt:
                    combo.set_active_iter(i)
                i = model.iter_next(i)
        
        return box
        
    
    def getSoundPrefs(self):
        '''
        Sound prefs
        
        @return: gtk.Box
        '''
        self.optionFrame.set_label(_("Sound Preferences"))
        box = gtk.VBox(False, 10)
        box.set_border_width(3)
        
        s = _("Message Sound")
        c = _("Choose")
        d = _("Use Default")
        msgbox = gtk.VBox(False)
        msgentry = gtk.Entry()
        button = gtk.Button("%s.."%c)
        m_default = gtk.CheckButton("%s" % d)
        bbox = gtk.VButtonBox()
        bbox.add(button)
        bbox.add(m_default)
        msgbox_h = gtk.HBox(False, 1)
        msgbox_h.pack_start(gtk.Label("%s:" % s))
        msgbox_h.pack_start(msgentry, False, False, 10)
        msgbox_h.pack_start(bbox, False, False, 10)
        msgbox.pack_start(msgbox_h)
        button.connect("clicked", self.chooseFile_cb, msgentry, m_default)
        
        x = gtk.VBox(False, 0)
        x.pack_start(msgbox, False, False, 0)
        box.pack_start(x)
        
        s = _("Game Sound")
        gamebox = gtk.VBox(False)
        gameentry = gtk.Entry()
        button = gtk.Button("%s.." % c)
        g_default = gtk.CheckButton("%s" % d)
        bbox = gtk.VButtonBox()
        bbox.add(button)
        bbox.add(g_default)
        gamebox_h = gtk.HBox(False, 1)
        gamebox_h.pack_start(gtk.Label("%s:" % s))
        gamebox_h.pack_start(gameentry, False, False, 10)
        gamebox_h.pack_start(bbox, False, False, 10)
        gamebox.pack_start(gamebox_h)
        button.connect("clicked", self.chooseFile_cb, gameentry, g_default)
        
        x = gtk.VBox(False, 0)
        x.pack_start(gamebox, False, False, 0)
        box.pack_start(x)
        
        game_sound = self.optionmanager.get_default_option(SOUND_GAME_OPTION, None)
        msg_sound = self.optionmanager.get_default_option(SOUND_MSG_OPTION, None)
        
        if not game_sound:
            g_default.set_active( True )
        else:
            gameentry.set_text( game_sound )
        if not msg_sound:
            m_default.set_active( True )
        else:
            msgentry.set_text( msg_sound )
        
        bbox = gtk.HButtonBox()
        button = gtk.Button(_("Save"))
        button.connect("clicked", self.saveSoundPrefs_cb, msgentry, gameentry, m_default, g_default)
        bbox.add(button)
        
        box.pack_start(bbox)
        
        return box
    
    def getConnectionPrefs(self):
        '''
        Connection preferences
        '''
        self.optionFrame.set_label(_("Connection Preferences"))
        
        box = gtk.VBox(False, 3)
        left = gtk.VBox(False, 3)
        right = gtk.VBox(False, 3)
        lr = gtk.HBox(True, 3)
        proxyBox = self.getProxyInfo()
        
        left.pack_start( gtkutil.createLeftJustifiedLabel(_('Save current username/password/host')), False, False, 0 )
        left.pack_start( gtkutil.createLeftJustifiedLabel(_('Use Proxy')), False, False, 0 )
        left.pack_start( proxyBox, False, False, 0 )
        
        button = gtk.CheckButton()
        button.set_active( self.optionmanager.get_default_bool_option(OPTION_SAVE_LOGIN, True) )
        button.connect("toggled", self.toggleOption_cb, OPTION_SAVE_LOGIN)
        right.pack_start(button, False, False, 0)
        
        button = gtk.CheckButton()
        button.set_active( self.optionmanager.get_default_bool_option(OPTION_USE_PROXY, False) )
        button.connect("toggled", self.toggleUseProxy_cb, OPTION_USE_PROXY, proxyBox)
        right.pack_start(button, False, False, 0)
        
        lr.pack_start(left, False, False, 0)
        lr.pack_start(right, False, False, 0)
        
        box.pack_start(lr, False, False, 0)
        box.pack_start(self.getAdditionalHosts(), False, False, 0)
        
        return box
    
    def getProxyInfo(self):
        '''
        Return widgets for setting proxy information
        '''
        p_left = gtk.VBox(True, 3)
        p_right = gtk.VBox(True, 3)
        bbox = gtk.VButtonBox()
        proxyTop = gtk.HBox(False, 3)
        proxyBox = gtk.VBox(False, 3)
        
        selectedProxyType = self.optionmanager.get_default_option(OPTION_PROXY_TYPE, OPTION_PROXY_HTTP)
        
        p_left.pack_start( gtkutil.createLeftJustifiedLabel(_("Username")), False, False, 0 )
        p_left.pack_start( gtkutil.createLeftJustifiedLabel(_('Password')), False, False, 0 )
        p_left.pack_start( gtkutil.createLeftJustifiedLabel(_("Host")), False, False, 0 )
        
        for field,tip in ( (OPTION_PROXY_USER, None), (OPTION_PROXY_PASSWORD, None), (OPTION_PROXY_HOST, _('Value should be <hostname>:<port>')) ):
            e = gtk.Entry()
            e.set_text( self.optionmanager.get_default_option(field, '') )
            e.connect("changed", self.entryChanged_cb, field)
            if tip is not None:
                self.tips.set_tip(e, tip)
            p_right.pack_start( e, False, False, 0 )
        
        button = gtk.RadioButton(None, _('HTTP'))
        button.connect("toggled", self.toggleOption_cb, OPTION_PROXY_TYPE, OPTION_PROXY_HTTP)
        button.set_active( selectedProxyType == OPTION_PROXY_HTTP)
        bbox.pack_start(button)
        #button1 = gtk.RadioButton(button, _('SOCKS V4'))
        #button1.connect("toggled", self.toggleOption_cb, OPTION_PROXY_TYPE, OPTION_PROXY_SOCKS4)
        #button1.set_active( selectedProxyType == OPTION_PROXY_SOCKS4)
        #bbox.pack_start(button1)
        #button1 = gtk.RadioButton(button, _('SOCKS V5'))
        ##button1.connect("toggled", self.toggleOption_cb, OPTION_PROXY_TYPE, OPTION_PROXY_SOCKS5)
        #button1.set_active( selectedProxyType == OPTION_PROXY_SOCKS5)
        #bbox.pack_start(button1)
        
        proxyTop.pack_start(p_left, False, False, 0)
        proxyTop.pack_start(p_right, False, False, 0)
        
        proxyBox.pack_start(proxyTop, False, False, 0)
        proxyBox.pack_start(bbox, False, False, 0)
        proxyBox.set_sensitive(self.optionmanager.get_default_bool_option(OPTION_USE_PROXY, False))
        
        return proxyBox
    
    def getAdditionalHosts(self):
        '''
        Return additional hosts frame for the connection prefs
        '''
        hostbox = gtk.VBox(False, 3)
        frame = gtk.Frame()
        frame.set_shadow_type( gtk.SHADOW_OUT )
        
        optionsList = gtk.ListStore(str,str)
        optionsView = gtk.TreeView( optionsList )
        optionsView.set_headers_visible( False )
        
        col1 = gtk.TreeViewColumn(_('Name'))
        cell1 = gtk.CellRendererText()
        col1.pack_start(cell1, True)
        col1.add_attribute(cell1, 'text', 0)
        optionsView.append_column( col1 )
        
        col2 = gtk.TreeViewColumn(_('Location'))
        cell2 = gtk.CellRendererText()
        col2.pack_start(cell2, True)
        col2.add_attribute(cell2, 'text', 1)
        optionsView.append_column( col2 )
        
        for host,gport,wport,location in util.getAdditionalHosts():
            optionsList.append( (host,location) )
        
        frame.add( optionsView )
        
        hostbuttonbox = gtk.HButtonBox()
        
        button = gtk.Button( _('Add') )
        button.connect("clicked", self.addHost_cb, optionsList, optionsView)
        hostbuttonbox.add( button )
        button = gtk.Button( _('Remove') )
        button.connect("clicked", self.deleteHost_cb, optionsView)
        hostbuttonbox.add( button )
        
        hostbox.pack_start( gtkutil.createLeftJustifiedLabel(_('Additional Hosts')), False, False, 0)
        hostbox.pack_start(frame, False, False, 0)
        hostbox.pack_start(hostbuttonbox, False, False, 0)
        
        return hostbox
        
    def getGameplayPrefs(self):
        '''
        Gameplay Preferences
        '''
        self.optionFrame.set_label(_("Gameplay Preferences"))
        box = gtk.VBox(False, 3)
        
        left = gtk.VBox(True, 3)
        right = gtk.VBox(True, 3)
        
        left.pack_start( gtkutil.createLeftJustifiedLabel(_('Rack re-order method')), False, False, 0 )
        left.pack_start( gtkutil.createLeftJustifiedLabel(_('Clear move from board on game error')), False, False, 0 )
        left.pack_start( gtkutil.createLeftJustifiedLabel(_('Show tooltips for premium squares')), False, False, 0 )
        left.pack_start( gtkutil.createLeftJustifiedLabel(_('Bold letter text')), False, False, 0 )
        left.pack_start( gtkutil.createLeftJustifiedLabel(_('Show timestamps in 24-hour format')), False, False, 0 )
        left.pack_start( gtkutil.createLeftJustifiedLabel(_('Show help at startup')), False, False, 0 )
        left.pack_start( gtkutil.createLeftJustifiedLabel(_('Show public servers at startup')), False, False, 0 )
        
        opt = self.optionmanager.get_default_option(OPTION_SWAP, OPTION_LETTER_INSERT)
        model = gtk.ListStore(str,str)
        model.append( [_('Insert Letters'), OPTION_LETTER_INSERT] )
        model.append( [_('Swap Letters'), OPTION_LETTER_SWAP] )
        cell = gtk.CellRendererText()
        combo = gtk.ComboBox(model)
        combo.pack_start(cell)
        combo.add_attribute(cell, 'text', 0)
        combo.set_active(0)
        combo.connect("changed", self.setComboValue_cb, OPTION_SWAP)
        combo.set_active_iter( gtkutil.getIterByColumn(model, 1, opt))
        right.pack_start(combo, False, False, 0)
        
        button = gtk.CheckButton()
        button.set_active( self.optionmanager.get_default_bool_option(OPTION_CLEAR_ON_ERROR, True) )
        button.connect("toggled", self.toggleOption_cb, OPTION_CLEAR_ON_ERROR)
        right.pack_start(button, False, False, 0)
        
        button = gtk.CheckButton()
        button.set_active( self.optionmanager.get_default_bool_option(OPTION_ENABLE_T_TIPS, True) )
        button.connect("toggled", self.toggleOption_cb, OPTION_ENABLE_T_TIPS)
        right.pack_start(button, False, False, 0)
        
        button = gtk.CheckButton()
        button.set_active( self.optionmanager.get_default_bool_option(OPTION_TEXT_BOLD, False) )
        button.connect("toggled", self.toggleOption_cb, OPTION_TEXT_BOLD)
        right.pack_start(button, False, False, 0)
        
        button = gtk.CheckButton()
        button.set_active( self.optionmanager.get_default_bool_option(OPTION_24_HOUR, False) )
        button.connect("toggled", self.toggleOption_cb, OPTION_24_HOUR)
        right.pack_start(button, False, False, 0)
        
        button = gtk.CheckButton()
        button.set_active( self.optionmanager.get_default_bool_option(OPTION_SHOW_TIPS, True) )
        button.connect("toggled", self.toggleOption_cb, OPTION_SHOW_TIPS)
        right.pack_start(button, False, False, 0)
        
        button = gtk.CheckButton()
        button.set_active( self.optionmanager.get_default_bool_option(OPTION_SHOW_PS, True) )
        button.connect("toggled", self.toggleOption_cb, OPTION_SHOW_PS)
        right.pack_start(button, False, False, 0)
        
        lr = gtk.HBox(True, 3)
        lr.pack_start(left, False, False, 0)
        lr.pack_start(right, False, False, 0)
        
        box.pack_start(lr, False, False, 0)
        
        return box
        
    
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
    
    def setComboValue_cb(self, combo, key):
        '''
        Set combo value
        
        @param combo: Combo widget
        @param key: Option key
        '''
        model = combo.get_model()
        val = model.get_value(combo.get_active_iter(), 1)
        
        self.optionmanager.set_option(key, val)
        
    
    def entryChanged_cb(self, widget, option):
        '''
        
        @param widget: Widget that activated this callback
        @param option: Option name
        '''
        self.optionmanager.set_option(option, widget.get_text())
    
    def toggleOption_cb(self, widget, option, value=None):
        '''
        Preference toggled.
        
        Set the option name to the value of widget.get_active()
        
        @param widget: Widget that activated this callback
        @param option: Option name
        @param value: Option value
        '''
        
        optionValue = value
        if optionValue == None:
            optionValue = int(widget.get_active())
        
        self.optionmanager.set_option(option, optionValue)
    
    def toggleUseProxy_cb(self, widget, option, box):
        '''
        Use Proxy Preference toggled
        
        Set the option name to the value of widget.get_active() and show/hide the proxy info block
        
        @param widget: Widget that activated this callback
        @param option: Option name
        @param box: Proxy box
        '''
        
        box.set_sensitive( widget.get_active() )
        
        self.optionmanager.set_option(option, int(widget.get_active()))
    
    def optionsViewClicked_cb(self, widget, event):
        '''
        Option menu clicked callback.  Change the main option section
        
        @param widget: Widget that activated this callback
        @param event: Event information
        '''
        opt = gtkutil.getSelectedItem(widget, 0)
        if opt is not None and event.button == 1:
            self.showOptionFrame( opt )
    
    def showOptionFrame(self, opt):
        '''
        Show option frame
        
        @param opt: Option type
        '''
        if opt is not None:
            child = self.optionFrame.get_child()
            if child:
                self.optionFrame.remove( child )
            
            box = gtk.HBox(False,20)    
            box.set_border_width( 20 )
            if opt == NOTIFICATIONS:
                box.pack_start( self.getNotificationPrefs(), False, False, 0 )
                self.optionFrame.add( box )
            if opt == SOUNDS:
                box.pack_start( self.getSoundPrefs(), False, False, 0  )
                self.optionFrame.add( box )
            if opt == LOCALE:
                box.pack_start( self.getLocalePrefs(), False, False, 0  )
                self.optionFrame.add( box )
            if opt == COLORS:
                box.pack_start( self.getColorPrefs(), False, False, 0  )
                self.optionFrame.add( box )
            if opt == GAMEPLAY:
                box.pack_start( self.getGameplayPrefs(), False, False, 0)
                self.optionFrame.add( box )
            if opt == CONNECTION:
                box.pack_start( self.getConnectionPrefs(), False, False, 0)
                self.optionFrame.add( box )
        
        self.optionFrame.show_all()
    
    def chooseFile_cb(self, widget, popWidget, default):
        '''
        Show file chooser
        
        @param widget: Widget that activated this callback
        @param popWidget: Widget to populate answer in
        @param default: Default checkbox.  Uncheck this if the user selects a file
        '''
        s = _('Choose File')
        dialog = gtk.FileChooserDialog( s, None, gtk.FILE_CHOOSER_ACTION_OPEN, (gtk.STOCK_OPEN, gtk.RESPONSE_OK) )
        dialog.set_default_response(gtk.RESPONSE_OK)
        filter = gtk.FileFilter()
        filter.add_pattern('*.wav')
        filter.set_name(_('Wav Files'))
        dialog.add_filter(filter)
        
        response = dialog.run()
        
        if (response == gtk.RESPONSE_OK):
            str = dialog.get_filename()
            popWidget.set_text(str)
            default.set_active( False )
        
        dialog.destroy()
    
    
    def saveSoundPrefs_cb(self, widget, msgentry, gameentry, m_default, g_default):
        '''
        Save sound preferences
        
        If Game Entry Default is checked, ignore whats in the entry field.  Same for Message Entry
        
        @param widget: Widget that activated this callback
        @param msgentry: Message Sound Entry box
        @param gameentry: Game Sound Entry box
        @param m_default: Message Sound Default checkbox
        @param g_default: Game Sound Default checkbox
        '''
        
        if m_default.get_active():
            self.optionmanager.set_option(SOUND_MSG_OPTION, '')
        else:
            file = msgentry.get_text()
            try:
                f = wave.open(file, 'rb')
                f.close()
                self.optionmanager.set_option(SOUND_MSG_OPTION, file)
            except wave.Error:
                s = _("is not a valid .wav file")
                self.error(util.ErrorMessage('%s %s' % (s,file)))
                return
            except IOError:
                self.error(util.ErrorMessage(_('Please select a file or choose the default option')))
                return
        
        if g_default.get_active():
            self.optionmanager.set_option(SOUND_GAME_OPTION, '')
        else:
            file = gameentry.get_text()
            try:
                f = wave.open(file, 'rb')
                f.close()
                self.optionmanager.set_option(SOUND_GAME_OPTION, file)
            except wave.Error:
                s = _("is not a valid .wav file")
                self.error(util.ErrorMessage('%s %s' % (s,file)))
                return
            except IOError:
                self.error(util.ErrorMessage(_('Please select a file or choose the default option')))
                return
        
        s = manager.SoundManager()
        s.loadSounds()
        
        self.destroy()
    
    def saveLocalePrefs_cb(self, widget, combo):
        '''
        Save Locale Prefs
        
        @param widget: Widget that activated this callback
        @param combo: Combo box
        '''
        
        model = combo.get_model()
        iter = combo.get_active_iter()
        opt = model.get_value(iter, 1)
        
        o = manager.OptionManager()
        o.set_option(LOCALE_OPTION, opt)
        
        l = manager.LocaleManager()
        l.setLocale( )
        
        s = _('Please restart the application for the changes to take effect.')
        self.info(s)
    
    def spinChanged_cb(self, button, opt):
        '''
        Spin changed
        
        @param button:
        @param opt:
        '''
        self.optionmanager.set_option(opt, button.get_value_as_int())
        
    
    def info(self, data, parent=None):
        '''
        Show info dialog.
        
        @param data: ErrorMessage data
        @see: L{util.ErrorMessage}
        '''
        s = _("Info")
        if not parent:
            parent = self    
        self.dialog = gtk.MessageDialog(parent=parent, type=gtk.MESSAGE_INFO, buttons=gtk.BUTTONS_OK, message_format="")
        self.dialog.set_markup("<big>%s: %s</big>" % (s, data))
        self.dialog.connect("response", lambda w,e: self.dialog.destroy())
        self.dialog.show()
        self.dialog.run()
    
    def addHost_cb(self, button, model, view):
        '''
        Add an additional host
        
        @param button:
        @param model:
        @param view:
        '''
        self.showHostDialog(view)
    
    def deleteHost_cb(self, button, view):
        '''
        Delete an additional host
        
        @param button:
        @param view:
        '''
        
        sel = view.get_selection()
        model, iter = sel.get_selected()
        
        if (iter == None):
            return None
        
        host = model.get_value(iter, 0)
        model.remove(iter)
        
        o = manager.OptionManager(section=HOSTS_SECTION)
        data = o.get_default_option(OPTION_HOSTS, None)
        result = ''
        if data is not None:
            chunks = data.split('/')
            for chunk in chunks:
                if not chunk.startswith(host):
                    result = '%s/%s' % (result,chunk)
        o.set_option(OPTION_HOSTS, result)
    
    def showHostDialog(self, view):
        '''
        Show host information dialog
        
        @param view: 
        '''
        title = _("Host Information")
        dialog = gtk.Dialog(title=title, flags=gtk.DIALOG_MODAL)
        dialog.vbox.set_border_width( 5 )
        
        header = gtk.Label()
        header.set_markup("<b><big>%s:</big></b>" % title)
        dialog.vbox.pack_start(header)
        
        s = _("Host")
        host = gtkutil.EntryWithLabel(label="%s: " % s, visibility=True)
        dialog.vbox.pack_start( host )
        
        s = _("Game port")
        gport = gtkutil.EntryWithLabel(label="%s: " % s, visibility=True)
        dialog.vbox.pack_start( gport )
        
        s = _("Web port")
        wport = gtkutil.EntryWithLabel(label="%s: " % s, visibility=True)
        dialog.vbox.pack_start( wport )
        
        s = _("Location")
        location = gtkutil.EntryWithLabel(label="%s: " % s, visibility=True)
        dialog.vbox.pack_start( location )
        
        okbutton = gtk.Button(_("Ok"))
        cancelbutton = gtk.Button(_("Cancel"))
        
        dialog.action_area.pack_start(okbutton)
        dialog.action_area.pack_start(cancelbutton)
        
        okbutton.connect("clicked", self.setHost, host, gport, wport, location, dialog, view)
        cancelbutton.connect("clicked", lambda b: dialog.destroy() )
        
        dialog.show_all()
    
    def setHost(self, button, host, gport, wport, location, dialog, view):
        '''
        Set host info
        
        @param button:
        @param host:
        @param gport:
        @param wport:
        @param location:
        @param dialog:
        @param view
        '''
        
        try:
            int(gport.get_text())
            int(gport.get_text())
        except:
            self.error(util.ErrorMessage(_('Port must be a number')))
            return

        o = manager.OptionManager(section=HOSTS_SECTION)
        data = o.get_default_option(OPTION_HOSTS, '')
        data = '%s%s:%s:%s:%s/' % (data, host.get_text(), gport.get_text(), wport.get_text(), location.get_text())
        o.set_option(OPTION_HOSTS, data)

        dialog.destroy()
        
        model = view.get_model()
        model.clear()
        for host,gport,wport,location in util.getAdditionalHosts():
            model.append( (host,location) )
        
