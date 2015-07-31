from pyscrabble import constants
from pyscrabble import gtkconstants
from pyscrabble import manager
from pyscrabble import util
from pyscrabble import gui
from twisted.internet import reactor
import gtk
import pango

def fatalError(data):
    '''
    A Fatal error has occurred.  Usually it means the connection to the server has been lost.
    
    Show a dialog and then quit
    
    @param data: ErrorMessage data
    '''
    
    s = _("Error: A Fatal Error has occurred")
    s1 = _("The application will now close")
    dialog = gtk.MessageDialog(parent=None, type=gtk.MESSAGE_ERROR, buttons=gtk.BUTTONS_OK, message_format="")
    dialog.set_title(_("Error: Fatal Error"))
    dialog.set_markup("<big>%s: %s. %s.</big>" % (s,data.getErrorMessage(),s1))
    response = dialog.run()
    reactor.stop()

def setupStockItems():
    '''
    Setup stock images
    '''
    widget = gtk.DrawingArea()
    
    factory = gtk.IconFactory()
    
    r = manager.ResourceManager()
    
    buf = gtk.gdk.pixbuf_new_from_file( r["resources"]["images"][gtkconstants.STOCK_SEND_IM_FILE] )
    icon = gtk.IconSet(buf)
    factory.add( gtkconstants.STOCK_SEND_IM, icon )
    
    factory.add( gtkconstants.STOCK_COPY_URL, gtk.IconSet(widget.render_icon(gtk.STOCK_COPY, gtk.ICON_SIZE_MENU)) )
    factory.add( gtkconstants.STOCK_COPY_EMAIL, gtk.IconSet(widget.render_icon(gtk.STOCK_COPY, gtk.ICON_SIZE_MENU)) )
    factory.add( gtkconstants.STOCK_OPEN_URL, gtk.IconSet(widget.render_icon(gtk.STOCK_JUMP_TO, gtk.ICON_SIZE_MENU)) )
    factory.add( gtkconstants.STOCK_OFFLINE_MESSAGE, gtk.IconSet(widget.render_icon(gtk.STOCK_NETWORK, gtk.ICON_SIZE_MENU)) )
    factory.add( gtkconstants.STOCK_DEFINE, gtk.IconSet(widget.render_icon(gtk.STOCK_FIND, gtk.ICON_SIZE_MENU)) )
    factory.add( gtkconstants.STOCK_SEND_MOVE, gtk.IconSet(widget.render_icon(gtk.STOCK_YES, gtk.ICON_SIZE_MENU)) )
    factory.add( gtkconstants.STOCK_RESET_DEFAULT, gtk.IconSet(widget.render_icon(gtk.STOCK_UNDO, gtk.ICON_SIZE_MENU)) )
    factory.add( gtkconstants.STOCK_PASS, gtk.IconSet(widget.render_icon(gtk.STOCK_NO, gtk.ICON_SIZE_MENU)) )
    factory.add( gtkconstants.STOCK_TRADE_LETTERS, gtk.IconSet(widget.render_icon(gtk.STOCK_UNDO, gtk.ICON_SIZE_MENU)) )
    factory.add( gtkconstants.STOCK_SHUFFLE, gtk.IconSet(widget.render_icon(gtk.STOCK_REFRESH, gtk.ICON_SIZE_MENU)) )
    factory.add( gtkconstants.STOCK_REGISTER, gtk.IconSet(widget.render_icon(gtk.STOCK_ADD, gtk.ICON_SIZE_MENU)) )
    factory.add( gtkconstants.STOCK_ADD_HOSTNAME, gtk.IconSet(widget.render_icon(gtk.STOCK_COPY, gtk.ICON_SIZE_MENU)) )
    
    factory.add_default()
    
    widget.destroy()
    widget = None

def showAbout(menu):
    '''
    Show about dialog
    
    @param menu:
    '''
    r = manager.ResourceManager()
    dialog = gtk.AboutDialog()
    dialog.set_name( "PyScrabble" )
    dialog.set_authors( ["Kevin Conaway", "Dennis Harrison -- Testing", "Mark Lee -- Patches", "Jaderi -- Testing"] )
    dialog.set_comments( "For Erin" )
    dialog.set_website( constants.ONLINE_SITE )
    dialog.set_website_label( constants.ONLINE_SITE )
    dialog.set_logo( gtk.gdk.pixbuf_new_from_file(r["resources"]["images"]["py.ico"]) )
    dialog.set_translator_credits(_('translator-credits'))
    gtk.about_dialog_set_url_hook(util.showUrl, data=constants.ONLINE_SITE)
    dialog.set_version(constants.VERSION)
    dialog.show()

def getSelectedItem(view, col):
    '''
    Get selected column from a TreeView
    
    @param view: TreeView
    @param col: Column number
    '''
    
    sel = view.get_selection()
    model, iter = sel.get_selected()
    
    if (iter == None):
        return None
    
    return model.get_value(iter, col)

def copyToClipboard(widget, data):
    '''
    Copy to clipboard
    
    @param widget: Widget that activated this callback
    @param data: Data to add to clipboard
    '''
    clipboard = gtk.clipboard_get()
    clipboard.set_text(data)

def colorReset_cb(widget, key, val, button, enable):
    '''
    Reset a color option to the default option
    
    @param widget:
    @param key:
    @param val:
    @param button:
    @param enable:
    '''
    o = manager.OptionManager()
    enable.set_active(True)
    o.set_option(key, val)
    button.set_color( gtk.gdk.color_parse(val) )
    button.show()
        
def colorSet_cb(widget, key, enable):
    '''
    Set color preference
    
    @param widget: Color button
    @param key: Preference key
    @param enable: Enable checkbox
    '''
    o = manager.OptionManager()
    enable.set_active(True)
    c = widget.get_color()
    color = util.colorToHexString(c.red,c.green,c.blue)
    o.set_option(key, color)

def toggleOption_cb(widget, option):
    '''
    Notification preference toggled.
    
    Set the option name to the value of widget.get_active()
    
    @param widget: Widget that activated this callback
    @param option: Option name
    '''
    o = manager.OptionManager()
    o.set_option(option, int(widget.get_active()))

def createColorPreference(optionVal, defaultOptionVal, enableOptionVal, label, tips, tipText):
    '''
    Create color preference box
    
    @param optionVal:
    @param defaultOptionVal:
    @param enableOptionVal:
    @param label:
    @param tips:
    @param tipText:
    '''
    o = manager.OptionManager()
    opt = o.get_default_option(optionVal, defaultOptionVal) #option,default
    
    enable = gtk.CheckButton(_('Enable'))
    enable.connect("toggled", toggleOption_cb, enableOptionVal) #enable option
    enable.set_active(o.get_default_bool_option(enableOptionVal, True)) #enable option
    cb = gtk.ColorButton(gtk.gdk.color_parse(opt))
    cb.connect("color-set", colorSet_cb, optionVal, enable) #option
    b = gtk.Button(stock=gtkconstants.STOCK_RESET_DEFAULT)
    b.connect("clicked", colorReset_cb, optionVal, defaultOptionVal, cb, enable)
    bbox = gtk.HButtonBox()
    bbox.add( cb )
    bbox.add( b )
    bbox.add( enable )
    hbox = gtk.HBox(False, 5)
    l = createLeftJustifiedLabel( label ) # label
    hbox.pack_start( l, True, True, 5 )
    hbox.pack_start( bbox, False, False, 5)
    tips.set_tip(cb,tipText) #tips
    return hbox

def createLeftJustifiedLabel(text):
    '''
    Create a left justified label
    
    @param text: Label text
    '''
    l = gtk.Label(text)
    l.set_alignment(0., 0.4)
    return l
    

def getIterByColumn(model, col, value):
    '''
    Get a TreeIter by col num and value
    
    @param model: TreeModel
    @param col: Column Number
    @param value: Column Value
    @return: TreeIter
    '''
    iter = model.get_iter_root()
    
    while iter is not None:
        val = model.get_value(iter, col)
        if val == value:
            return iter
        iter = model.iter_next(iter)
    

def createToolButton(stock_id, label):
    '''
    Create a toolbar button
    
    @param stock_id: Stock_id
    @param label: Label text
    '''
    
    if label is not None:
        button = gtk.ToolButton(gtk.image_new_from_stock(stock_id, gtk.ICON_SIZE_MENU), label=label)
    else:
        button = gtk.ToolButton(stock_id=stock_id)
    button.set_is_important(True)
    return button

class LetterPlaceHolder(gtk.Fixed):
    '''
    Class to be used when a letter is dragged out of the rack
    
    This empty placeholder will take the spot of the Letter
    '''
    
    def __init__(self, letterBox, game_frame):
        '''
        Constructor
        
        @param letterBox: Box that holds the GameLetters
        @param game_frame: GameFrame instance
        '''
        
        gtk.Fixed.__init__(self)
        self.set_size_request( constants.TILE_WIDTH, constants.TILE_HEIGHT )
        
        self.letterBox = letterBox
        self.game_frame = game_frame
        
        self.handler_id = self.connect("drag_data_received", self.swapLetter);
        self.drag_dest_set(gtk.DEST_DEFAULT_MOTION | gtk.DEST_DEFAULT_HIGHLIGHT | gtk.DEST_DEFAULT_DROP, [( "image/x-xpixmap", 0, 81 )], gtk.gdk.ACTION_COPY)
    
    def swapLetter(self, widget, context, x, y, selection, targetType, eventType):
        '''
        
        @param widget: Placeholder
        @param context: Drag context
        @param x:
        @param y:
        @param selection:
        @param targetType:
        @param eventType:
        '''
        letter = context.get_source_widget()
        
        if isinstance(letter, gui.pieces.GameTile):
            self.game_frame.putTileOnPlaceholder(letter)
#             newLetter = gui.pieces.GameLetter(letter.getLetter(), self.letterBox)
#             self.game_frame.removeMove(letter, letter.x, letter.y)
#             self.game_frame.addLetter(letter.getLetter())
#             letter.clear();
#             #self.game_frame.removeMove(letter, letter.x, letter.y)
#             if letter.getLetter().isBlank():
#                 letter.getLetter().setLetter("")
#             letter = newLetter
#         
#         if not isinstance(letter, gui.pieces.GameLetter):
#             return
#         
#         letters = self.letterBox.get_children()
#         self.letterBox.foreach(lambda w: self.letterBox.remove(w))
#         
#         for l in letters:
#             if id(l) == id(widget):
#                 self.letterBox.pack_start(letter, False, False, 0)
#             elif id(l) == id(letter):
#                 self.letterBox.pack_start(widget, False, False, 0)
#             else:
#                 self.letterBox.pack_start(l, False, False, 0)
#         self.letterBox.show_all()
        
    def activate(self):
        '''
        Do nothing
        '''
        return True
    
    def deactivate(self):
        '''
        Do nothing
        '''
        return True
        
        
        
        
class Popup(gtk.Window):
    
    DELAY = 0.02
    WIDTH_BUFFER = 0
    COUNT = 0
    COUNT_BUFFER = 30
    HEIGHT_BUFFER = 0
    MAX_INTERVALS = 20
    TIMEOUT = 3
    
    def __init__(self, title, text):
        '''
        Toaster popup window
        
        @param title:
        @param text:
        '''
        gtk.Window.__init__(self, gtk.WINDOW_POPUP)
        
        self.set_default_size(150,100)
        self.move(gtk.gdk.screen_width(),gtk.gdk.screen_height()) # Start the window off to the side somewhere
        self.connect("map-event", self.onMapEvent_cb)
        self.connect("destroy", self.onDestroy_cb)
        self.connect("delete-event", self.close_cb)
        
        o = manager.OptionManager()
        self.timeout = int( o.get_default_option(constants.OPTION_POPUP_TIMEOUT, Popup.TIMEOUT) )
        self.intervals = 0
        
        l = gtk.Label()
        l.set_markup("""<span background="#FFFFFF" weight="bold">%s</span>""" % text)
        hbox = gtk.HBox(False, 5)
        hbox.pack_start( gtk.image_new_from_stock(gtk.STOCK_DIALOG_INFO, gtk.ICON_SIZE_BUTTON), False, False, 5)
        hbox.pack_start( l, False, False, 3)
        
        box = gtk.EventBox()
        box.add( hbox )
        box.modify_bg(gtk.STATE_NORMAL, gtk.gdk.color_parse("#FFFFFF"))
        box.connect("button-press-event", self.close_cb)
        
        l = gtk.Label()
        l.set_markup("""<span weight="bold" foreground="#FFFFFF">%s</span>""" % title)
        header = gtk.EventBox()
        header.modify_bg(gtk.STATE_NORMAL, gtk.gdk.color_parse("#6699ff"))
        header.add( l )
        header.connect("button-press-event", self.close_cb)
        
        vbox = gtk.VBox(False, 0)
        vbox.pack_start(header, False, False, 0)
        vbox.pack_start(gtk.HSeparator(), False, False, 0)
        vbox.pack_start(box, True, True, 0)
        
        frame = gtk.Frame()
        frame.set_shadow_type( gtk.SHADOW_ETCHED_IN )
        frame.add(vbox)
        
        self.add( frame )
        
        self.show_all()
    
    def close_cb(self, widget=None, event=None):
        '''
        Close callback
        
        @param widget:
        @param event:
        '''
        width, height = self.get_size()
        Popup.HEIGHT_BUFFER -= height
        if Popup.HEIGHT_BUFFER < 0:
            Popup.HEIGHT_BUFFER = 0
        Popup.COUNT = Popup.COUNT - 1
        self.destroy()
    
    def onDestroy_cb(self, widget):
        '''
        Destroy callback
        
        @param widget:
        '''
        pass
    
    def onMapEvent_cb(self, widget, event):
        '''
        Callback when widget is mapped to the display
        
        Show the window and set the move timer
        
        @param widget:
        @param event:
        '''
        self.set_gravity(gtk.gdk.GRAVITY_SOUTH_EAST)
        width, height = self.get_size()
        
        self.count = Popup.COUNT
        Popup.COUNT = Popup.COUNT + 1
        
        self.max_height = Popup.HEIGHT_BUFFER
        Popup.HEIGHT_BUFFER += height
        
        self.move(gtk.gdk.screen_width() - (width + Popup.WIDTH_BUFFER), gtk.gdk.screen_height())
        reactor.callLater(Popup.DELAY, self.moveWindow, up=True)
        return True
    
    def moveWindow(self, up=True):
        '''
        Move the window
        
        @param up: True to move the window up, False down
        '''
        if up:
            self.intervals = self.intervals + 1
        else:
            self.intervals = self.intervals - 1
        
        width, height = self.get_size()
        
        w = gtk.gdk.screen_width() - (width + Popup.WIDTH_BUFFER)
        h = gtk.gdk.screen_height() - (self.max_height * self.intervals / Popup.MAX_INTERVALS) - ((height+self.getCountBuffer()) * self.intervals / Popup.MAX_INTERVALS)
        
        self.move(w, h)
        
        if self.intervals == Popup.MAX_INTERVALS:
            reactor.callLater(self.timeout, self.moveWindow, False)
        elif self.intervals == 0:
            self.close_cb()
        else:
            reactor.callLater(Popup.DELAY, self.moveWindow, up)
    
    def getCountBuffer(self):
        '''
        If there are other windows shown, we need to adjust the height so the windows are stacked
        '''
        return util.ternary(Popup.COUNT == 1, Popup.COUNT_BUFFER, Popup.COUNT_BUFFER - Popup.COUNT)


class TaggableTextView(gtk.TextView):
    '''
    TextView that will markup text based on its content
    
    Currently handles
    - URLs
    - Email addresses
    '''
    
    def __init__(self, buffer=None):
        '''
        Constructor
        
        @param buffer: gtk.TextBuffer
        '''
        gtk.TextView.__init__(self, buffer)
        self.connect("motion-notify-event", self.motionNotify_cb)
    
    def insert_text(self, text):
        '''
        Insert text into buffer
        
        @param text: Text
        '''
        
        self.insert_text_with_tags(text, None)
    
    def insert_text_with_tags(self, text, tags):
        '''
        Insert text with tags
        
        @param text: Text
        @param tags: Tags
        '''
        
        buf = self.get_buffer()
        items = str(text).split()
        
        for item in items:
            t = None
            trailing = None
            if util.isURL(item):
                item, trailing = util.getURL(item)
                t = buf.create_tag(foreground="blue", underline=pango.UNDERLINE_SINGLE)
                t.set_data(gtkconstants.TAG_TYPE, gtkconstants.LINK_TAG)
                t.connect("event", self.tagEvent, item)
            elif util.isEmail(item):
                item, trailing = util.getEmail(item)
                t = buf.create_tag(foreground="blue", underline=pango.UNDERLINE_SINGLE)
                t.connect("event", self.tagEvent, '%s%s' % ('mailto:',item))
                t.set_data(gtkconstants.TAG_TYPE, gtkconstants.EMAIL_TAG)
            
            if t:    
                if tags:
                    buf.insert_with_tags(buf.get_end_iter(), '%s' % item, tags, t)
                else:
                    buf.insert_with_tags(buf.get_end_iter(), '%s' % item, t)
            else:
                if tags:
                    buf.insert_with_tags(buf.get_end_iter(), item, tags)
                else:
                    buf.insert(buf.get_end_iter(), item)
            
            if trailing is not None:
                for item in trailing:
                    buf.insert(buf.get_end_iter(), item)
            buf.insert(buf.get_end_iter(), ' ')
                
        buf.insert(buf.get_end_iter(), '\n')
        
        buf.create_mark("end", buf.get_end_iter())
        self.scroll_to_mark(buf.get_mark("end"), 0.0)
    
    def tagEvent(self, tag, widget, event, iter, data):
        '''
        Tag event
        
        @param tag: TextTag
        @param widget: self
        @param event: Event
        @param iter: TextIter
        @param data: URL
        '''
        
        if event.type == gtk.gdk.BUTTON_PRESS:
            if event.button in (1,2):
                util.showUrl(None, data, None)
                return True
            if event.button == 3:
                message_menu = gtk.Menu()
                
                type = tag.get_data(gtkconstants.TAG_TYPE)
                
                if type == gtkconstants.LINK_TAG:
                    item = gtk.ImageMenuItem(stock_id=gtkconstants.STOCK_COPY_URL)
                    item.connect("activate", copyToClipboard, data)
                    message_menu.add(item)
                    
                    message_menu.add( gtk.SeparatorMenuItem() )
                    
                    item = gtk.ImageMenuItem(stock_id=gtkconstants.STOCK_OPEN_URL)
                    item.connect("activate", util.showUrl, data, None)
                    message_menu.add(item)
                if type == gtkconstants.EMAIL_TAG:
                    item = gtk.ImageMenuItem(stock_id=gtkconstants.STOCK_COPY_EMAIL)
                    item.connect("activate", copyToClipboard, data.lstrip('mailto:'))
                    message_menu.add(item)
                
                
                message_menu.show_all()
                message_menu.popup(None, None, None, event.button, event.time)
                return True
                
    def motionNotify_cb(self, widget, event):
        '''
        Motion notify over the TextView.  If we hover over a link tag, change the pointer
        
        @param widget:
        @param event:
        '''
        
        x,y = widget.window_to_buffer_coords(gtk.TEXT_WINDOW_TEXT, event.x, event.y)
       
        iter = widget.get_iter_at_location(x,y)
        tags = iter.get_tags()
       
        t = None
        for tag in tags:
            type = tag.get_data(gtkconstants.TAG_TYPE)
            if type in (gtkconstants.LINK_TAG, gtkconstants.EMAIL_TAG):
                t = tag
                break
       
        if t:
            widget.get_window(gtk.TEXT_WINDOW_TEXT).set_cursor( gtk.gdk.Cursor(gtk.gdk.HAND2))
        else:
            widget.get_window(gtk.TEXT_WINDOW_TEXT).set_cursor( gtk.gdk.Cursor(gtk.gdk.XTERM))
       
        return False
        
class EntryWithLabel(gtk.HBox):
    
    def __init__(self, label="", visibility = True, maxLength = None):
        gtk.HBox.__init__(self, homogeneous = True, spacing = 20)
        self.__label = gtk.Label(label)
        self.__label.set_justify( gtk.JUSTIFY_LEFT )
        self.__entry = gtk.Entry()
        self.__entry.set_visibility( visibility )
        if maxLength is not None:
            self.__entry.set_max_length(maxLength)
        self.pack_start( self.__label )
        self.pack_start( self.__entry )
    
    # Get text value from entry
    def get_text(self):
        return self.__entry.get_text()
    
    # Set text value of entry
    def set_text(self, text):
        self.__entry.set_text( text )
    
    # Set length of textbox
    def setEntryWidth(self, num):
        self.__entry.set_width_chars( num )