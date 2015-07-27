import gtk
from pyscrabble.constants import *

class InfoWindow(gtk.Window):
    '''
    Information window
    '''
    
    def __init__(self, title, header, main):
        '''
        Constructor
        
        @param title: Window title
        @param header: Header text
        '''
        
        gtk.Window.__init__(self, gtk.WINDOW_TOPLEVEL)
        self.connect("destroy", self.onDestroy )
        self.connect("delete_event", self.onDelete_event )
        
        self.set_size_request( 500, 300 )
        self.set_title(title)
        
        self.main = main
        
        box = gtk.VBox(False)
        
        label = gtk.Label()
        label.set_markup("<b>%s</b>" % header)
        label.set_justify(gtk.JUSTIFY_CENTER)
        box.pack_start(label, False, False, 1)
        
        self.notebook = gtk.Notebook()
        box.pack_start( self.notebook, True, True, 5)
        
        s = _('Loading...')
        self.appendPage( gtk.Label(s), [s], [' '], visible=False )
        
        bbox = gtk.HButtonBox()
        button = gtk.Button(_("Close"))
        button.connect("clicked", self.close_cb)
        bbox.add(button)
        
        box.pack_start(bbox, False, False, 0)
        
        self.add( box )
        self.set_border_width( 5 )
        self.show_all()
    
    def close_cb(self, widget):
        '''
        Close callback
        
        @param widget: Widget that activated this callback
        '''
        if self.main is not None:
            self.main.serverInfoClosed_cb()
        self.destroy()
        
    
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
    
    def initialize(self):
        '''
        Initialize the notebook by removing the first page
        '''
        self.notebook.remove_page(0)
    
    def appendPage(self, label, cols, data, visible=True, sortable=False, signals=None):
        '''
        Append a TreeView to the ntoebook
        
        @param label: Heade label
        @param cols: Column names
        @param data: Column data
        @param sortable: Sortable flag
        @param signals: Optional signals to connect to the treeview. Should be a dict {name : callback}
        @return: ScrolledWindow containing TreeView
        '''
        
        if data is None or len(data) == 0:
            return
        
        win = gtk.ScrolledWindow()
        win.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        
        c = []
        
        for item in data[0]:
            if type(item) == unicode:
                c.append(str)
            else:
                c.append(type(item))
                
        list = gtk.ListStore(*c)
        view = gtk.TreeView( gtk.TreeModelSort(list) )
        view.set_headers_visible( visible )
        view.set_headers_clickable(sortable)
        
        if signals is not None:
            for name,callback in signals.iteritems():
                view.connect(name, callback)
        
        i = 0
        for c in cols:
            cell = gtk.CellRendererText()
            col = gtk.TreeViewColumn(c)
            if sortable:
                col.set_sort_column_id(i)
            col.pack_start(cell, True)
            col.add_attribute(cell, 'text', i)
            view.append_column( col )
            i = i + 1
        
        for item in data:
            list.append( item )
        
        win.add( view )
        self.notebook.append_page(win, label)
        self.notebook.show_all()