import gtk
from pyscrabble.game.pieces import *
from pyscrabble.constants import *
from pyscrabble import lookup
from pyscrabble.gtkconstants import *
from pyscrabble import manager
from pyscrabble import gtkutil
from pyscrabble import util

# Class representing a Tile on the Gameboard            
class GameTile(gtk.Button):
    '''
    GameTiles represent Tiles on the Game board.
    '''
    
    DIR_NONE = 0
    DIR_HORIZ = 1
    DIR_VERT = 2
    
    def __init__(self, x, y, parent):
        '''
        Constructor.
        
        @param x: X Position
        @param y: Y Position
        @param parent: GameFrame class
        '''
        self.board = parent #callback to the parent widget
        
        self.letter = None
        
        gtk.Button.__init__(self)
        self.set_size_request(TILE_WIDTH, TILE_HEIGHT)
        self.findStyle(x,y)
        
        self.x = x
        self.y = y
        self.direction = GameTile.DIR_NONE
        self.fixed = False
        self.start = False
        
        self.handler_id = 0
        self.source_handler_id = 0
        self.key_press_handler = 0
        self.active = False
        self.deactivate()
        
        self.connect("button-release-event", self.buttonRelease_cb)
        self.connect("button-press-event", self.buttonPress_cb)
    
    def clone(self):
        '''
        Clone this Tile
        
        @return: Cloned Tile
        '''
        
        g = GameTile(self.x,self.y,self.parent)
        g.putLetter( self.getLetter() )
        return g
    
    def activate(self):
        '''
        Activate this tile.
        
        Allow letters to be dragged onto it.
        '''
        self.deactivate()
            
        if (self.getLetter() == None):
            self.handler_id = self.connect("drag_data_received", self.letterDragged);
        self.drag_dest_set(gtk.DEST_DEFAULT_MOTION | gtk.DEST_DEFAULT_HIGHLIGHT | gtk.DEST_DEFAULT_DROP, [( "image/x-xpixmap", 0, 81 )], gtk.gdk.ACTION_COPY)
        self.active = True
    
    def deactivate(self):
        '''
        Deactivate this tile.
        
        Disallow letters to be dragged onto it.
        '''
        if (self.handler_is_connected(self.handler_id)):
            self.disconnect(self.handler_id)
        
        if (self.handler_is_connected(self.source_handler_id)):
            self.disconnect(self.source_handler_id)
            
        self.drag_dest_unset()
        self.drag_source_unset()
        self.active = False
    
    def dragLetter(self, widget, context, selection, targetType, eventTime):
        '''
        Drag data callback.
        
        @param widget:
        @param context:
        @param selection:
        @param targetType:
        @param eventTime:
        '''
        #print 'dragging tile gameletter %s' % widget.getLetter().getLetter()
        s = widget.getLetter().getLetter()
        selection.set(selection.target, 8, '%s:%s:%s' % (s, str(widget.getLetter().getScore()), str(int(widget.getLetter().isBlank()))))
    
    # Callback when a GameLetter is dragged onto this GameTile
    def letterDragged(self, widget, context, x, y, selection, targetType, eventType):
        '''
        Callback when a GameLetter is dragged onto this Tile.
        
        Callback to the GameFrame to let it know that a letter has been dropped
        
        @param widget:
        @param context:
        @param x:
        @param y:
        @param selection:
        @param targetType:
        @param eventType:
        '''
        #print 'set letter called in tile %d,%d' % (self.x,self.y)
        
        # Kill typing
        self.get_parent().clearArrows()
        
        
        c = context.get_source_widget()
        if c == self: # Ignore if we drag onto ourselves
            return
        
        self.setLetter(c, c.getLetter().getLetter(), c.getLetter().getScore(), c.getLetter().isBlank())
    
    def setLetter(self, widget, letter, score, isBlank, showBlank=True):
        '''
        Set letter on this Tile
        
        @param widget: Widget that was dragged
        @param letter: Letter value
        @param score: Score value
        '''     
        self.source_handler_id = self.connect("drag_data_get", self.dragLetter)
        self.drag_source_set(gtk.gdk.BUTTON1_MASK, [( "image/x-xpixmap", 0, 81 )], gtk.gdk.ACTION_COPY)
        
        if isinstance(widget, GameTile):
            self.board.swapTiles(self, widget)
        elif isinstance(widget, GameLetter):
            self.board.swapTileAndLetter(self, widget)
        
    def putLetter(self, letter, showBlank=False):
        '''
        Put a Letter on the tie.
        
        @param letter:
        @param showBlank: True to show blank value
        '''
        self.set_label( letter, showBlank )
        self.letter = letter
        self.refresh()

    def update_label(self):
        self.set_label(self.letter, False)
    
    def clear(self):
        self.letter = None
        self.refresh()
        self.activate()
        
    def getLetter(self):
        return self.letter

        
    
    def findStyle(self, x, y):
        '''
        Get the style for this Tile based on the x,y position
        
        @param x:
        @param y:
        '''
        
        
        pos = (x+1, y+1)
        if ( pos in DOUBLE_LETTERS ):
            self.setStyle(TILE_DOUBLE_LETTER, TILE_COLORS[TILE_DOUBLE_LETTER])
            self.board.tileTips.set_tip(self, _('Double Letter Score'))
        elif ( pos in TRIPLE_LETTERS ):
            self.setStyle(TILE_TRIPLE_LETTER, TILE_COLORS[TILE_TRIPLE_LETTER])
            self.board.tileTips.set_tip(self, _('Triple Letter Score'))
        elif ( pos in DOUBLE_WORDS ):
            self.setStyle(TILE_DOUBLE_WORD, TILE_COLORS[TILE_DOUBLE_WORD])
            self.board.tileTips.set_tip(self, _('Double Word Score'))
        elif ( pos in TRIPLE_WORDS ):
            self.setStyle(TILE_TRIPLE_WORD, TILE_COLORS[TILE_TRIPLE_WORD])
            self.board.tileTips.set_tip(self, _('Triple Word Score'))
        elif ( pos in CENTER ):
            if self.board.gameOptions[OPTION_CENTER_TILE]:
                self.setStyle(TILE_DOUBLE_WORD, TILE_COLORS[TILE_DOUBLE_WORD])
                self.board.tileTips.set_tip(self, _('Double Word Score'))
            else:
                self.setStyle(TILE_CENTER, TILE_COLORS[TILE_CENTER])
        else:
            o = manager.OptionManager()
            if o.get_default_bool_option(USE_COLOR_NORMAL_TILE, True):
                self.setStyle(TILE_NORMAL, o.get_default_option(COLOR_NORMAL_TILE, TILE_COLORS[TILE_NORMAL]))
            else:
                self.setStyle(TILE_NORMAL, TILE_COLORS[TILE_NORMAL])
        
    def setBackground(self, color):
        '''
        Set backround color
        
        @param color 
        '''
        self.modify_bg(gtk.STATE_NORMAL, gtk.gdk.color_parse(color))
        self.modify_bg(gtk.STATE_PRELIGHT, gtk.gdk.color_parse(color))
        
    
    def setStyle(self, style, color):
        '''
        Set the style on this Tile
        
        @param style:
        @param color
        '''
        
        self.modify_bg(gtk.STATE_NORMAL, gtk.gdk.color_parse(color) )
        self.modify_bg(gtk.STATE_PRELIGHT, gtk.gdk.color_parse(color) )

    
    def __repr__(self):
        '''
        Print the Letter on the Tile
        '''
        
        return self.getLetter()
    
    def set_label(self, letter, showBlank=False):
        '''
        Set the Letter/score label on the Tile
        
        @param letter: Letter
        @see: L{pyscrabble.game.pieces.Letter}
        '''
        widget = self.get_child()
        if (widget is not None):
            self.remove(widget)
        widget = gtk.Label()
            
        if letter == None:
            l = ""
            s = ""
            widget.set_markup("""%s <sub><span size="x-small">%s</span></sub>""" % (l, s))
            self.add(widget)
            self.show_all()
        else:
            l = letter.getLetter()
            s = str(letter.getScore())
            o = manager.OptionManager()
            if letter.isBlank():
                if showBlank:
                    l = ""
                if o.get_default_bool_option(USE_COLOR_BLANK_TILE, True):
                    l = """<span foreground="%s">%s</span>""" % (o.get_default_option(COLOR_BLANK_TILE, DEFAULT_BLANK_TILE), l)
                    s = """<span foreground="%s">%s</span>""" % (o.get_default_option(COLOR_BLANK_TILE, DEFAULT_BLANK_TILE), s)
            else:
                if o.get_default_bool_option(USE_COLOR_TEXT, True):
                    l = """<span foreground="%s">%s</span>""" % (o.get_default_option(COLOR_TEXT, DEFAULT_COLOR_TEXT), l)
                    s = """<span foreground="%s">%s</span>""" % (o.get_default_option(COLOR_TEXT, DEFAULT_COLOR_TEXT), s)
            
            if o.get_default_bool_option(OPTION_TEXT_BOLD, False):
                l = """<span weight="bold">%s</span>""" % l
                s = """<span weight="bold">%s</span>""" % s
                
            if len(str(letter.getScore())) == 2:
                widget.set_markup("""%s <sub><span size="xx-small">%s</span></sub>""" % (l, s))
            else:
                widget.set_markup("""%s <sub><span size="x-small">%s</span></sub>""" % (l, s))
            self.add(widget)
            self.show_all()
    
    def refresh(self):
        '''
        Refresh tile color
        
        @return: True if the tile is active
        '''
        self.set_label( self.getLetter() )
        
        o = manager.OptionManager()
        if self.getLetter() is not None:
            if o.get_default_bool_option(USE_COLOR_LETTER, True):
                self.setBackground( o.get_default_option(COLOR_LETTER, TILE_COLORS[TILE_LETTER]) )
            else:
                self.setBackground( TILE_COLORS[TILE_LETTER] )
        else:
            self.findStyle(self.x, self.y)
        
    
    def buttonPress_cb(self, widget, event):
        '''
        Button press event
        
        @param widget: widget
        @param event: event info
        '''
        if event.type == gtk.gdk._2BUTTON_PRESS:
            if self.getLetter() is not None:
                if self.handler_is_connected(self.handler_id):
                    self.board.removeMove(self, self.x, self.y)
                    self.board.refreshLetterBox()
            return True
            
        if event.button == 1 and self.getLetter() is None and event.type not in (gtk.gdk._2BUTTON_PRESS, gtk.gdk._3BUTTON_PRESS):
            if self.board.isCurrentTurn() == True:
                
                if self.fixed:
                    return False
                
                self.direction = self.direction + 1
                if self.direction > GameTile.DIR_VERT:
                    self.direction = GameTile.DIR_NONE
                    
                self.handleArrow(self.getArrow(self.direction))
                
        return False
        
    
    def getArrow(self, direction):
        '''
        Get arrow from direction
        
        @param direction:
        '''
        if direction == GameTile.DIR_HORIZ:
            return gtk.Arrow(gtk.ARROW_RIGHT, gtk.SHADOW_OUT)
        if direction == GameTile.DIR_VERT:
            return gtk.Arrow(gtk.ARROW_DOWN, gtk.SHADOW_OUT)
            
        return None
    
    def handleArrow(self, arrow):
        '''
        Place an arrow on the board
        
        @param arrow:
        '''
        dir = self.direction
        self.get_parent().clearArrows()
        self.direction = dir
        
        if arrow is not None:
            box = gtk.HBox(False, 0)
            arrow.set_size_request(10,10)
            arrow.set_padding(0, 0)
            box.pack_start(arrow, False, False, 0)
            self.add(box)
            self.key_press_handler = self.connect("key-press-event", self.keyPress_cb, self.direction)
        self.show_all()
        
    
    def buttonRelease_cb(self, widget, event):
        '''
        Button release event
        
        @param widget: Widget
        @param event: Event info
        '''
        
        if event.button == 3:
            
            menu = gtk.Menu()
            
            if widget.getLetter() is not None:
            
                board = widget.get_parent()
                moves = board.getMovesAtXY(widget.x,widget.y)
                
                if len(moves) > 0:
                    item = gtk.ImageMenuItem(stock_id=STOCK_DEFINE)
                    menu.append(item)
                    
                    l = manager.LettersManager()
                    
                    meta = l.getMeta( self.board.getGameOption(OPTION_RULES) )
                    site = meta['lookup']
                    submenu = gtk.Menu()
                    for move in moves:
                        word = move.getWord()
                        i = gtk.MenuItem(word)
                        i.connect("activate", util.showUrl, '%s%s' % (site,word.lower()))
                        submenu.append(i)
                        
                    item.set_submenu(submenu)
                 
            if len(menu.get_children()) > 0:
                menu.show_all()
                menu.popup(None, None, None, 1, event.time)
    
    def removeArrow(self):
        '''
        Remove arrow
        '''
        if self.getLetter() is None:
            x = self.get_child()
            if x is not None:
                self.remove(x)
                self.direction = GameTile.DIR_NONE
                self.fixed = False
        if self.handler_is_connected(self.key_press_handler):
            self.disconnect(self.key_press_handler)     
        
    def keyPress_cb(self, widget, event, direction):
        '''
        Key press
        
        @param widget:
        @param event:
        @param direction:
        '''
        val = event.keyval
        board = self.get_parent()
        
        if val == gtk.keysyms.Escape:
            self.removeArrow()
            self.direction = GameTile.DIR_NONE
            self.fixed = False
            return True
        
        if val in (gtk.keysyms.BackSpace, gtk.keysyms.Delete):
            
            # Handle edges
            if direction == GameTile.DIR_HORIZ:
                if self.x == 14 and self.getLetter() is not None:
                    self.board.removeLetter(self.x,self.y)
                    tile = board.get(self.x,self.y)
                    board.set_focus_child(tile)
                    tile.grab_focus()
                    tile.direction = direction
                    tile.fixed = True
                    tile.handleArrow( self.getArrow(direction) )
                    return True
            if direction == GameTile.DIR_VERT and self.getLetter() is not None:
                if self.y == 14:
                    self.board.removeLetter(self.x,self.y)
                    tile = board.get(self.x,self.y)
                    board.set_focus_child(tile)
                    tile.grab_focus()
                    tile.direction = direction
                    tile.fixed = True
                    tile.handleArrow( self.getArrow(direction) )
                    return True
            
            tile = None
            if direction == GameTile.DIR_HORIZ:
                x = self.x - 1
                while x >= 0:
                    if self.board.hasOnboardMove(x, self.y):
                        tile = board.get(x, self.y)
                        break
                    x = x - 1
            if direction == GameTile.DIR_VERT:
                y = self.y -1
                while y >= 0:
                    if self.board.hasOnboardMove(self.x, y):
                        tile = board.get(self.x, y)
                        break
                    y = y - 1
            if tile is not None:
                self.board.removeLetter(tile.x,tile.y)
                tile = board.get(tile.x,tile.y)
                board.set_focus_child(tile)
                tile.grab_focus()
                tile.direction = direction
                if self.board.getNumOnBoardMoves() == 0:
                    tile.fixed = False
                else:
                    tile.fixed = True
                tile.handleArrow( self.getArrow(direction) )
                self.removeArrow()
            return True
        
        if val == gtk.keysyms.Return:
            self.board.sendCurrentMove()
            self.get_parent().clearArrows()
            return True
        
        if val in range(gtk.keysyms.a, gtk.keysyms.z+1):
            val = val - 32
                
        if val in range(gtk.keysyms.A, gtk.keysyms.Z+1):
            ch = chr(val)
            
            if direction == GameTile.DIR_HORIZ:
                if self.x == 14 and self.getLetter() is not None:
                    gtk.gdk.beep()
            if direction == GameTile.DIR_VERT and self.getLetter() is not None:
                if self.y == 14:
                    gtk.gdk.beep()
            
            tile = None
            if direction == GameTile.DIR_HORIZ:
                num = min(14 - self.x, 6)
                for x in range(1,num+1):
                    t = board.get(self.x + x, self.y)
                    if t.getLetter() is None:
                        tile = t
                        break
            if direction == GameTile.DIR_VERT:
                num = min(14 - self.y, 6)
                for x in range(1,num+1):
                    t = board.get(self.x, self.y+x)
                    if t.getLetter() is None:
                        tile = t
                        break
            
            placed = self.board.placeLetter(str(ch), self.x, self.y)
            if tile is not None:
                if placed:
                    board.set_focus_child(t)
                    t.grab_focus()
                    t.direction = direction
                    t.handleArrow( self.getArrow(direction) )
                    tile.fixed = True  
                    self.removeArrow()
            
            return True
        return False
            
        
        




# Class representing a Letter tile
class GameLetter(gtk.ToggleButton):
    '''
    The GameLetter represents the Letters that are used to make words.
    
    They are dragged onto specific GameTiles on the GameBoard.
    '''
    
    def __init__(self, letter, letterBox):
        '''
        Constructor.
        
        Initialize the letter
        
        @param letter: Letter
        '''
        gtk.ToggleButton.__init__(self)
        self.set_size_request(TILE_WIDTH, TILE_HEIGHT)
        
        
        self.letterBox = letterBox
        
        self.setBackground()
        
        self.set_label(letter)
        
        self.handlerId = 0
        self.destHandlerId = 0
        
        self.letter = letter
        
        self.activate()

    def setBackground(self):
        '''
        Set background color according to preference
        '''
        o = manager.OptionManager()
        color = None
        if o.get_default_bool_option(USE_COLOR_LETTER, True):
            color = o.get_default_option(COLOR_LETTER, TILE_COLORS[TILE_LETTER])
        else:
            color = TILE_COLORS[TILE_LETTER]
        
        color = gtk.gdk.color_parse(color)
        self.modify_bg(gtk.STATE_NORMAL, color )
        self.modify_bg(gtk.STATE_ACTIVE, color )
        self.modify_bg(gtk.STATE_PRELIGHT, color )
    
    def getLetter(self):
        '''
        Return the Letter object that this object represents
        
        @return: Letter
        '''
        
        return self.letter
    
    def copyLetter(self, letter):
        '''
        Copy a letter onto this Letter
        
        @param letter: Letter
        '''
        
        s = letter.getLetter()
        if letter.isBlank():
            s = ""
        
        self.setLetter(s)
        self.setScore( letter.getScore() )
        self.setIsBlank(letter.isBlank())
        self.set_label(letter)
    
    def dragLetter(self, widget, context, selection, targetType, eventTime):
        '''
        Drag data callback.
        
        @param widget:
        @param context:
        @param selection:
        @param targetType:
        @param eventTime:
        '''
        #print 'dragging gameletter %s' % widget.getLetter().getLetter()
        selection.set(selection.target, 8, '%s:%s:%s' % (widget.getLetter().getLetter(), str(widget.getLetter().getScore()), str(int(widget.getLetter().isBlank()))))
    
    def letterDragged(self, widget, context, x, y, selection, targetType, eventType):
        sourceWidget = context.get_source_widget()
        if isinstance(sourceWidget, GameTile):
            sourceWidget.board.swapTileAndLetter(sourceWidget, self)
    
    def activate(self):
        '''
        Activate this letter.
        
        Allow the Letter to be dragged.
        '''
        
        self.deactivate()
        
        self.handlerId = self.connect("drag_data_get", self.dragLetter)
        self.drag_source_set(gtk.gdk.BUTTON1_MASK, [( "image/x-xpixmap", 0, 81 )], gtk.gdk.ACTION_COPY)
        
        self.destHandlerId = self.connect("drag_data_received", self.letterDragged);
        self.drag_dest_set(gtk.DEST_DEFAULT_MOTION | gtk.DEST_DEFAULT_HIGHLIGHT | gtk.DEST_DEFAULT_DROP, [( "image/x-xpixmap", 0, 81 )], gtk.gdk.ACTION_COPY)
        return True
    
    def deactivate(self):
        '''
        Deactivate this letter.
        
        Disallow the Letter to be dragged.
        '''
        
        if (self.handler_is_connected(self.handlerId)):
            self.disconnect(self.handlerId)
        if (self.handler_is_connected(self.destHandlerId)):
            self.disconnect(self.destHandlerId)
        self.drag_source_unset()
        self.drag_dest_unset()
        return True
    
    def refresh(self):
        '''
        Refresh colors on this tile
        '''
        
        self.set_label( self.getLetter() )
        self.setBackground()
        
    
    def set_label(self, letter):
        '''
        Set the Letter/score label on the Tile
        
        @param letter: Letter
        @see: L{pyscrabble.game.pieces.Letter}
        '''
        widget = self.get_child()
        if (widget == None):
            widget = gtk.Label()
        else:
            self.remove(widget)
        
        l = letter.getLetter()
        s = str(letter.getScore())
        o = manager.OptionManager()
        if letter.isBlank():
            l = ""
            if o.get_default_bool_option(USE_COLOR_BLANK_TILE, True):
                l = """<span foreground="%s">%s</span>""" % (o.get_default_option(COLOR_BLANK_TILE, DEFAULT_BLANK_TILE), l)
                s = """<span foreground="%s">%s</span>""" % (o.get_default_option(COLOR_BLANK_TILE, DEFAULT_BLANK_TILE), s)
        else:
            if o.get_default_bool_option(USE_COLOR_TEXT, True):
                l = """<span foreground="%s">%s</span>""" % (o.get_default_option(COLOR_TEXT, DEFAULT_COLOR_TEXT), l)
                s = """<span foreground="%s">%s</span>""" % (o.get_default_option(COLOR_TEXT, DEFAULT_COLOR_TEXT), s)
        
        if o.get_default_bool_option(OPTION_TEXT_BOLD, False):
            l = """<span weight="bold">%s</span>""" % l
            s = """<span weight="bold">%s</span>""" % s
        
        if len(str(letter.getScore())) == 2:
            widget.set_markup("""%s <sub><span size="xx-small">%s</span></sub>""" % (l, s))
        else:
            widget.set_markup("""%s <sub><span size="x-small">%s</span></sub>""" % (l, s))
        
        self.add(widget)
        self.show_all()

        
        
       
# Class representing the GUI Game Board
class GameBoard(gtk.Table):
    '''
    The GameBoard class is the widget responsible for holding the tiles on the Board.  It is basically a
    gtk.Table on steroids that allows access to elements based on their x,y position.
    '''
    
    
    def __init__(self):
        '''
        Constructor
        '''
        
        gtk.Table.__init__(self,rows=15, columns=15, homogeneous = True)
        self.tiles = {} # Hold a reference to all tiles
        self.empty = True
    
    def activate(self):
        '''
        Activate each tile on the board
        
        @see: {pyscrabble.gui.pieces.GameTile}
        '''
        
        for tile in self.tiles.values():
            tile.activate()
    
    def deactivate(self):
        '''
        Deactivate each tile on the board
        
        @see: {pyscrabble.gui.pieces.GameTile}
        '''
        
        for tile in self.tiles.values():
            tile.deactivate()
    
    def put(self, widget, x, y):
        '''
        Add a widget to the board
        
        @param widget: Widget
        @param x: X position
        @param y: Y position
        '''
        
        # If there is a tile in the space, destroy it
        if (self.tiles.has_key((x,y))):
            self.tiles[(x,y)].destroy()

        self.attach(widget, x, x+1, y, y+1)
        self.tiles[(x,y)] = widget
    
    def hasMove(self, move):
        '''
        Check to see if the board has this move.
        
        @param move: Move
        '''
        
        for l, x, y in move.getTiles():
            if self.tiles.has_key( (x,y) ):
                if (self.tiles[(x,y)].getLetter() != l):
                    return False
            elif not self.tiles.has_key( (x,y) ):
                return False
        
        return True
        
    
    def putLetter(self, letter, x, y):
        '''
        Put a Letter on a Tile on the board
        
        This is different from adding a new Widget to the board.
        
        This assumes that a GameTile already exists at (x,y) and that
        we are going to change the Letter on that Tile
        
        @param letter: Letter
        @param x: x position
        @param y: y position
        @param set_bg: True to set COLOR_RECENT_TILE as background color
        @see: L{pyscrabble.gui.pieces.GameTile}
        @see: L{pyscrabble.game.pieces.Letter}
        '''
        
        self.empty = False
        self.tiles[(x,y)].putLetter(letter)
        self.tiles[(x,y)].deactivate()
    
    def moveTouching(self, move, onBoard):
        '''
        Check if this move is touching another on the board
        
        @param move: Move
        @param onBoard: Tiles on board
        @return: True if this move is touching another move, or if there are no other moves on the board
        '''
        
        #if self.isEmpty():
        #    return True
        
        for letter, x, y in move.getTiles():
            
            _l, _x, _y = self.getNextHorizontalLetter(x, y)
            if self.hasNextHorizontalLetter(x,y) and not onBoard.hasLetterAt(_x,_y):
                return True
            
            _l, _x, _y = self.getNextVerticalLetter(x, y)
            if self.hasNextVerticalLetter(x,y) and not onBoard.hasLetterAt(_x,_y):
                return True
            
            _l, _x, _y = self.getPreviousHorizontalLetter(x, y)
            if self.hasPreviousHorizontalLetter(x,y) and not onBoard.hasLetterAt(_x,_y):
                return True
            
            _l, _x, _y = self.getPreviousVerticalLetter(x, y)
            if self.hasPreviousVerticalLetter(x,y) and not onBoard.hasLetterAt(_x,_y):
                return True
        
        return False
        
    
    def isEmpty(self):
        '''
        Check if the board is empty
        
        @return: True if the board is empty
        '''
        return self.empty
    
    def getTilesAtX(self, _x):
        '''
        Return all tiles that have C{_x} in common
        
        @param _x: X position
        @return: A List of all tiles that have C{_x} in common
        '''
        
        list = []
        for x,y in self.tiles.keys():
            if x == _x:
                if (self.tiles.has_key((x, y))):
                    if self.tiles[(x, y)].getLetter() != None:
                        list.append( (self.tiles[x,y], x, y) )
        
        return list
    
    def getTilesAtY(self, _y):
        '''
        Return all tiles that have C{_y} in common
        
        @param _y: Y position
        @return: A List of all tiles that have C{_y} in common
        '''
        
        list = []
        for x,y in self.tiles.keys():
            if y == _y:
                if (self.tiles.has_key((x, y))):
                    if self.tiles[(x, y)].getLetter() != None:
                        list.append( (self.tiles[x,y], x, y) )
        
        return list
        
    
    def get(self, x, y):
        '''
        Get a Tile at this (x,y) position or None if a Tile is not there
        
        @param x: X position
        @param y: Y position
        @return: GameTile at this (x,y) position or None
        '''
        
        if (self.tiles.has_key((x,y))):
            return self.tiles[(x,y)]
        else:
            return None
    
    def hasNextHorizontalLetter(self, x, y):
        '''
        Check if there is a Letter on the Tile at (x+1,y)
        
        @param x: X position
        @param y: Y position
        @return: True if there is a Letter on the Tile at (x+1, y)
        '''
        
        if (self.tiles.has_key((x+1, y))):
            if self.tiles[(x+1, y)].getLetter() != None:
                return True
        return False
    
    def hasNextVerticalLetter(self, x, y):
        '''
        Check if there is a Letter on the Tile at (x,y+1)
        
        @param x: X position
        @param y: Y position
        @return: True if there is a Letter on the Tile at (x, y+1)
        '''
        
        if (self.tiles.has_key((x, y+1))):
            if self.tiles[(x, y+1)].getLetter() != None:
                return True
        return False
    
    def getNextVerticalLetter(self, x, y):
        '''
        Return the Letter on the Tile at (x,y+1)
        
        @param x: X position
        @param y: Y position
        @return: True if there is a Letter on the Tile at (x, y+1) or None
        '''
        
        if (self.tiles.has_key((x, y+1))):
            return (self.tiles[(x, y+1)], x, y+1)
        else:
            return None,None,None
    
    def getNextHorizontalLetter(self, x, y):
        '''
        Return the Letter on the Tile at (x+1,y)
        
        @param x: X position
        @param y: Y position
        @return: True if there is a Letter on the Tile at (x+1,y) or None
        '''
        
        if (self.tiles.has_key((x+1, y))):
            return (self.tiles[(x+1, y)], x+1, y)
        else:
            return None,None,None
    
    def hasPreviousHorizontalLetter(self, x, y):
        '''
        Check if there is a Letter on the Tile at (x-1,y)
        
        @param x: X position
        @param y: Y position
        @return: True if there is a Letter on the Tile at (x-1, y)
        '''
        
        if (self.tiles.has_key((x-1, y))):
            if self.tiles[(x-1, y)].getLetter() != None:
                return True
        return False
    
    def hasPreviousVerticalLetter(self, x, y):
        '''
        Check if there is a Letter on the Tile at (x,y-1)
        
        @param x: X position
        @param y: Y position
        @return: True if there is a Letter on the Tile at (x, y-1)
        '''
        
        if (self.tiles.has_key((x, y-1))):
            if self.tiles[(x, y-1)].getLetter() != None:
                return True
        return False
    
    def getPreviousVerticalLetter(self, x, y):
        '''
        Return the Letter on the Tile at (x,y-1)
        
        @param x: X position
        @param y: Y position
        @return: True if there is a Letter on the Tile at (x, y-1) or None
        '''
        
        if (self.tiles.has_key((x, y-1))):
            return (self.tiles[(x, y-1)], x, y-1)
        else:
            return None,None,None
    
    def getPreviousHorizontalLetter(self, x, y):
        '''
        Return the Letter on the Tile at (x-1,y)
        
        @param x: X position
        @param y: Y position
        @return: True if there is a Letter on the Tile at (x-1,y) or None
        '''
        
        if (self.tiles.has_key((x-1, y))):
            return (self.tiles[(x-1, y)], x-1, y)
        else:
            return None,None,None
    
    def isEmpty(self):
        '''
        Check whether the board has any moves on it
        
        @return True if the board has no moves
        '''
        return self.empty
    
    def getMovesAtXY(self, x, y):
        '''
        Get moves at X and Y positions
        
        @param x:
        @param y:
        '''
        moves = []
        
        h = Move()
        v = Move()
        h.addMove( self.get(x,y).getLetter(), x, y)
        v.addMove( self.get(x,y).getLetter(), x, y)
        
        _x = x
        _y = y
        while self.hasNextHorizontalLetter(_x, _y):
            item, _x, _y = self.getNextHorizontalLetter(_x, _y)
            h.addMove(item.getLetter(), _x, _y)
        
        _x = x
        _y = y
        while self.hasPreviousHorizontalLetter(_x, _y):
            item, _x, _y = self.getPreviousHorizontalLetter(_x, _y)
            h.addMove(item.getLetter(), _x, _y)
        
        h.sort()
        if h.length() > 1:
            moves.append(h)
        
        
        _x = x
        _y = y
        while self.hasNextVerticalLetter(_x, _y):
            item, _x, _y = self.getNextVerticalLetter(_x, _y)
            v.addMove(item.getLetter(), _x, _y)
        
        _x = x
        _y = y
        while self.hasPreviousVerticalLetter(_x, _y):
            item, _x, _y = self.getPreviousVerticalLetter(_x, _y)
            v.addMove(item.getLetter(), _x, _y)
        
        v.sort()
        if v.length() > 1:
            moves.append(v)
        
        return moves
    
    def clearRecentMove(self, move):
        '''
        Set recent move tiles to normal
        
        @param move: Move tiles
        '''
        for letter,x,y in move.getTiles():
            self.tiles[(x,y)].refresh()
    
    def refresh(self):
        '''
        Refresh the board
        '''

        for tile in self.tiles.itervalues():
            tile.refresh()
    
    def clearArrows(self):
        '''
        Remove arrows
        '''
        for tile in self.tiles.itervalues():
            tile.removeArrow()
        
        
        
        