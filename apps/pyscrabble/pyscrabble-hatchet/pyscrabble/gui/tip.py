import gtk
from pyscrabble import constants
from pyscrabble import gtkutil
from pyscrabble import manager

WELCOME_TIP = _("Welcome to PyScrabble!\n\nIn the next screen, please register on a public server if you haven't already.\n\nHave fun!")

class TipWindow(gtk.Window):
    '''
    Window for viewing tips
    '''
    
    def __init__(self, tip, callback):
        '''
        Constructor
        
        @param tip: Tip text
        @param callback: Callback when tip is closed
        '''
        
        gtk.Window.__init__(self, gtk.WINDOW_TOPLEVEL)
        self.connect("destroy", self.onDestroy )
        self.connect("delete_event", self.onDelete_event )
        self.set_size_request( constants.REGISTER_WINDOW_WIDTH, constants.REGISTER_WINDOW_HEIGHT )
        self.set_resizable( False )
        self.set_border_width( 10 )
        self.set_title( _('Help') )
        
        self.callback = callback
        
        box = gtk.VBox(False, 10)
        
        text = gtkutil.TaggableTextView(buffer=None)
        text.set_editable( False )
        text.set_cursor_visible( False )
        text.set_wrap_mode( gtk.WRAP_WORD )
        text.set_left_margin(10)
        text.set_right_margin(10)
        
        buf = text.get_buffer()
        t = buf.create_tag(justification=gtk.JUSTIFY_LEFT)
        buf.insert_with_tags(buf.get_end_iter(), tip, t)
        
        window = gtk.ScrolledWindow()
        window.add( text )
        window.set_policy( gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC )
        
        box.pack_start( window, True, True, 0 )
        box.pack_start( self.getButtons(), False, False, 0 )
        
        self.add(box)
        
        self.show_all()
    
    def onDelete_event(self, widget, event, data=None):
        '''
        Callback when the widget is deleted
        
        @param widget:
        @param event:
        @param data:
        '''
        self.closeWindow_cb()

    def onDestroy(self, widget, data=None):
        '''
        Callback when the widget is destroyed
        
        @param widget:
        @param data:
        '''
        pass
    
    def getButtons(self):
        '''
        Return buttons
        
        @return: gtk.VButtonBox
        '''
        box = gtk.VButtonBox()
        
        button = gtk.Button(stock=gtk.STOCK_OK)
        button.connect("clicked", self.closeWindow_cb)
        
        box.add(button)
        
        o = manager.OptionManager()
        
        button = gtk.CheckButton(_('Show help at startup'))
        button.set_active( o.get_default_bool_option(constants.OPTION_SHOW_TIPS, True) )
        button.connect("toggled", self.toggleOption_cb, constants.OPTION_SHOW_TIPS, o)
        
        box.add(button)
        
        return box
    
    def closeWindow_cb(self, widget=None):
        '''
        Close the window
        
        @param widget:
        '''
        self.callback()
        self.destroy()
    
    def toggleOption_cb(self, widget, option, om):
        '''
        Preference toggled.
        
        Set the option name to the value of widget.get_active()
        
        @param widget: Widget that activated this callback
        @param option: Option name
        @param om: OptionManager
        '''
        om.set_option(option, int(widget.get_active()))
        
        
        
        
        