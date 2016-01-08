from twisted.internet import reactor
from pyscrabble.net.client import *
from pyscrabble import gtkconstants
from pyscrabble import gtkutil
import pygtk
import gtk

class OfflineMessageWindow(gtk.Window):
    '''
    Offline Messages window
    '''
    
    def __init__(self, win, messages):
        '''
        Constructor
        
        @param messages: List of PrivateMessages
        '''
        
        gtk.Window.__init__(self, gtk.WINDOW_TOPLEVEL)
        self.connect("destroy", self.onDestroy )
        self.connect("delete_event", self.onDelete_event )
        
        s = _('Offline Messages')
        
        self.set_size_request( 800, 600 )
        self.set_title(s)
        
        self.messages = messages
        self.chatwin = win
        
        box = gtk.VBox(False)
        
        label = gtk.Label()
        label.set_markup("<b>%s</b>" % s)
        label.set_justify(gtk.JUSTIFY_CENTER)
        box.pack_start(label, False, False, 1)
        
        box.pack_start(self.getDataView(), True, True, 10)
        
        box.pack_start(self.getButtons(), False, False, 10)
        
        
        self.add( box )
        self.set_border_width( 5 )
        self.show_all()
    
    def close_cb(self, widget):
        '''
        Close callback
        
        @param widget: Widget that activated this callback
        '''
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
        self.chatwin.messageWindowClosed()
        pass
    
    def getDataView(self):
        '''
        Show messages in a TreeView
        
        @return: ScrolledWindow containing TreeView
        '''
        
        win = gtk.ScrolledWindow()
        win.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        
        list = gtk.ListStore(str,str,str,str)
        view = gtk.TreeView( gtk.TreeModelSort(list) )
        view.set_headers_visible( True )
        view.set_headers_clickable( True )
        view.connect("button-release-event", self.listClicked_cb)
        
        cell = gtk.CellRendererText()
        col = gtk.TreeViewColumn(_('Sender'))
        col.set_sort_column_id(0)
        col.pack_start(cell, True)
        col.add_attribute(cell, 'text', 0)
        view.append_column( col )
        
        cell = gtk.CellRendererText()
        col = gtk.TreeViewColumn(_('Date'))
        col.set_sort_column_id(1)
        col.pack_start(cell, True)
        col.add_attribute(cell, 'text', 1)
        view.append_column( col )
        
        cell = gtk.CellRendererText()
        col = gtk.TreeViewColumn(_('Message'))
        col.set_sort_column_id(2)
        col.pack_start(cell, True)
        col.add_attribute(cell, 'text', 2)
        view.append_column( col )
        
        cell = gtk.CellRendererText()
        col = gtk.TreeViewColumn()
        col.set_sort_column_id(3)
        col.set_visible(False)
        col.pack_start(cell, True)
        col.add_attribute(cell, 'text', 3)
        view.append_column( col )
        
        for message in self.messages:
            list.append( (message.sender, message.date, message.data, message.id) )
        
        
        self.view = view
        win.add( self.view )
        return win
    
    def getButtons(self):
        '''
        Get action buttons
        
        @return: gtk.ButtonBox
        '''
        
        box = gtk.HButtonBox()
        
        b = gtk.Button(_('Reply'))
        b.connect("clicked", self.reply_cb)
        box.add(b)
        
        b = gtk.Button(_('Delete'))
        b.connect("clicked", self.delete_cb, gtkutil.getSelectedItem(self.view, 3))
        box.add(b)
        
        b = gtk.Button(_('Close'))
        b.connect("clicked", self.close_cb)
        box.add(b)
        
        return box
    
    def error(self, data):
        '''
        Show Error dialog
        
        @param data: Error text
        '''
        
        s = _("Error")
        self.dialog = gtk.MessageDialog(parent=None, type=gtk.MESSAGE_ERROR, buttons=gtk.BUTTONS_OK, message_format="")
        self.dialog.set_title("%s" % s)
        self.dialog.set_markup("<big>%s: %s</big>" % (s,data))
        self.dialog.connect("response", lambda w,e: self.dialog.destroy())
        self.dialog.show()
        self.dialog.run()
    
    def listClicked_cb(self, widget, event):
        '''
        List clicked
        
        @param widget: Widget that activated this callback
        @param event: Event info
        '''
        
        sel = widget.get_selection()
        model, iter = sel.get_selected()
        
        if event.button == 3 and iter is not None:
            message_menu = gtk.Menu()
            
            item = gtk.ImageMenuItem(stock_id=gtk.STOCK_DIALOG_INFO)
            item.connect("activate", self.chatwin.requestUserInfo, gtkutil.getSelectedItem(widget, 0))
            message_menu.add(item)
            
            message_menu.add( gtk.SeparatorMenuItem() )
            
            item = gtk.ImageMenuItem(stock_id=gtkconstants.STOCK_SEND_IM)
            item.connect("activate", self.reply_cb, gtkutil.getSelectedItem(widget, 0))
            message_menu.add(item)
            
            message_menu.add( gtk.SeparatorMenuItem() )
            
            item = gtk.ImageMenuItem(stock_id=gtk.STOCK_DELETE)
            item.connect("activate", self.delete_cb, gtkutil.getSelectedItem(widget, 3))
            message_menu.add(item)
            
            message_menu.show_all()
            message_menu.popup(None, None, None, event.button, event.time)
        
    
    def reply_cb(self, widget, data=None):
        '''
        Reply callback
        
        @param widget: Widget that activated this callback
        @param data: Data
        '''
        if data is None:
            data = gtkutil.getSelectedItem(self.view, 0)
            if data is None:
                self.error(_('Please select a message'))
                return
        
        self.iconify()    
        self.chatwin.sendPrivateMessage(username=data, data=None)
    
    def delete_cb(self, widget, data=None):
        '''
        Delete callback
        
        @param widget: Widget that activated this callback
        @param data: Data
        '''
        if data is None:
            data = gtkutil.getSelectedItem(self.view, 3)
            if data is None:
                self.error(_('Please select a message'))
                return
            
        model = self.view.get_model().get_model() # TreeViewSort -> ListStore
        i = model.get_iter_first()
        while i:
            num = model.get_value(i, 3)
            if num == data:
                model.remove(i)
                i = None
            else:
                i = model.iter_next(i)
        
        self.chatwin.deleteMessage(data)
            
    
    def close_cb(self, widget):
        '''
        Close callback
        
        @param widget: Widget that activated this callback
        '''
        self.destroy()
    
    
        
        



class PrivateMessageFrame(gtk.Frame):
    '''
    ChatFrame.
    
    This class displays the Chat window where all users on a server congregate.
    '''
    
    def __init__(self, client, main, username):
        '''
        Initialize the ChatFrame
        
        @param client: ScrabbleClient instance
        @param main: MainWindow instance
        @see: L{pyscrabble.net.client.ScrabbleClient}
        @see: L{pyscrabble.gui.main.MainWindow}
        '''
        
        gtk.Frame.__init__(self)
        self.client = client
        self.mainwindow = main
        
        self.connect("realize", self.onRealize_cb)
        
        main = gtk.VBox( False, 10)
        
        self.username = username
        
        main.pack_start( self.createChatWindow(), True, True, 0 )
        main.pack_start( self.createEntryWindow(), False, False, 0 )
        
        self.set_border_width( 10 )
        self.add( main )
        self.show_all()
    
    def onRealize_cb(self, widget):
        '''
        Realize the window
        
        @param: widget
        '''
        self.entry.get_parent().set_focus_child(self.entry)
        self.entry.grab_focus()
    
    def createChatWindow(self):
        '''
        Create the chat TextView and User view
        
        @return: gtk.HBox containg main chat window and user treeview
        '''
        
        self.chat = gtkutil.TaggableTextView(buffer=None)
        self.chat.set_editable( False )
        self.chat.set_cursor_visible( False )
        self.chat.set_wrap_mode( gtk.WRAP_WORD )
        
        window = gtk.ScrolledWindow()
        window.add( self.chat )
        window.set_policy( gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC )
        return window
    
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
        
        return sizer
    
    
    def error(self, data):
        '''
        Show error dialog.
        
        @param data: ErrorMessage data
        @see: L{util.ErrorMessage}
        '''
        
        s = _("Error")
        self.dialog = gtk.MessageDialog(parent=self.mainwindow, type=gtk.MESSAGE_ERROR, buttons=gtk.BUTTONS_OK, message_format="")
        self.dialog.set_markup("<big>%s: %s</big>" % (s, data.getErrorMessage()))
        self.dialog.connect("response", lambda w,e: self.dialog.destroy())
        self.dialog.show()
        
    def submitChat(self, widget, event, data=None):
        '''
        Submit chat message to server
        
        @param widget:
        @param event:
        @param data:
        '''
        
        
        if (event.keyval == gtk.keysyms.Return):
            if (self.entry.get_text() != None and len(self.entry.get_text()) > 0):
                self.client.privateMessage( self.username, self.entry.get_text() )
                self.entry.set_text( '' )
                return True
        
        return False
    
  
    
    def receiveChatMessage(self, msg):
        '''
        Callback from ScrabbleClient when a chat message is posted.
        
        @param msg: Chat text to post in buffer
        '''
        self.chat.insert_text(msg)
        self.mainwindow.notifyPrivateMessage(self.username)
    
    def hasFocus(self):
        '''
        Callback that the window has focus
        
        Give focus to the chat entry
        '''
        self.entry.grab_focus()
        self.set_focus_chain([self.entry])
    