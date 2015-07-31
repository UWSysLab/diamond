import gtk
from pyscrabble.game.pieces import *
from pyscrabble.constants import *
from pyscrabble import lookup
from pyscrabble.gtkconstants import *
from pyscrabble import manager
from pyscrabble import gtkutil
from pyscrabble import util

import sys
sys.path.append("/Users/Niel/systems/diamond-src/backend/build/src/bindings/python")
from libpydiamond import *

# Class representing a Tile on the Gameboard            
class GameTile(gtk.Button):
    '''
    GameTiles represent Tiles on the Game board.
    '''
    
    def __init__(self, x, y, parent):
        '''
        Constructor.
        
        @param x: X Position
        @param y: Y Position
        @param parent: GameFrame class
        '''
        self.board = parent #callback to the parent widget
        
        gtk.Button.__init__(self)
                
        self.letterStr = DString()
        DString.Map(self.letterStr, "tileletter" + repr(y * BOARD_WIDTH + x))
        self.letterStr.Set("")
        
        self.letterScore = DLong()
        DLong.Map(self.letterScore, "tilescore" + repr(y * BOARD_WIDTH + x))
        self.letterScore.Set(0)
                
        self.__style = TILE_NORMAL
        
        self.set_size_request(TILE_WIDTH, TILE_HEIGHT)
        self.findStyle(x,y)
        
        self.x = x
        self.y = y
        
        self.handler_id = 0
        self.source_handler_id = 0
        self.active = False
        self.deactivate()
    
    def activate(self):
        '''
        Activate this tile.
        
        Allow letters to be dragged onto it.
        '''
        self.deactivate()
        
        if (self.getLetterStr() == ""):
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
        pass
        #print 'dragging tile gameletter %s' % widget.getLetter().getLetter()
        #s = widget.getLetter().getLetter()
        #selection.set(selection.target, 8, '%s:%s:%s' % (s, str(widget.getLetter().getScore()), str(int(widget.getLetter().isBlank()))))
    
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
        print 'letterDragged called in tile %d,%d' % (self.x,self.y)      
        
        c = context.get_source_widget()
        if c == self: # Ignore if we drag onto ourselves
            return
        
        self.setLetterNew(c, c.getLetterStr(), c.getLetterScore())
    
    #TODO: replace old setLetter with this one 
    def setLetterNew(self, widget, letter, score):
        #set up drag handler
        self.source_handler_id = self.connect("drag_data_get", self.dragLetter)
        self.drag_source_set(gtk.gdk.BUTTON1_MASK, [( "image/x-xpixmap", 0, 81 )], gtk.gdk.ACTION_COPY)
        
        print "Debug: beginning setLetterNew()"
        print self.board.onBoard
        print letter
        print score
        
        
        if self.getLetterStr() == "":
            if isinstance(widget, GameTile):
                self.board.removeMoveNew(widget, widget.x, widget.y)
                widget.clear()
            elif isinstance(widget, GameLetter):
                self.board.removeLetterNew(widget)
        else:
            if isinstance(widget, GameTile):
                myOldLetterStr = self.getLetterStr()
                myOldLetterScore = self.getLetterScore()
                self.board.removeMoveNew(self, self.x, self.y)
                self.board.removeMoveNew(widget, widget.x, widget.y)
                widget.putLetterNew(myOldLetterStr, myOldLetterScore)
                self.board.registerMoveNew(widget, widget.x, widget.y)
            elif isinstance(widget, GameLetter):
                self.board.removeMoveNew(self, self.x, self.y)
                widget.copyLetter(self.getLetterStr(), self.getLetterScore()) #TODO: is this working?
        
        self.putLetterNew(letter, score)
        self.board.registerMoveNew(self, self.x, self.y)
        
        print "Debug: ending setLetterNew()"
        print self.board.onBoard

    def putLetterNew(self, letter, score):
        self.setLetterStr(letter)
        self.setLetterScore(score)
        self.update_label()
    
    def clear(self):
        self.setLetterStr("")
        self.setLetterScore(0)
        self.update_label()
        self.activate()
    
    def setLetter(self, widget, letter, score, isBlank, showBlank=True):
        '''
        Set letter on this Tile
        
        @param widget: Widget that was dragged
        @param letter: Letter value
        @param score: Score value
        '''
        refresh = True
        
        # If we have a letter on here there are two cases
        # 1.) A GameTile is being dragged.  If so, we need to remove the old game tile (widget) and put its letter here
        # 2.) A Letter is being dragged.  If so, we need to remove the old letter and put it back in the rack and put the letter here
        #print 'SetLetter called on %s %s %d,%d with %s %s' % ( str(id(self)), str(self),self.x,self.y, str(widget.__class__), str(widget) )
        #print 'SetLetter %s %s %s %s' % ( str(letter), str(score), str(isBlank), str(showBlank) )
        if self.getLetter() is not None and widget is not None:
            if isinstance(widget, GameTile):
                refresh = False
                self.board.removeMove(widget, widget.x, widget.y, refresh=False)
                self.board.removeMove(self, self.x, self.y, refresh=False)
                letter,score = widget.getLetter().getLetter(), widget.getLetter().getScore()
                widget.setLetter( self.getLetter(), self.getLetter().getLetter(), self.getLetter().getScore(), self.getLetter().isBlank() )
            if isinstance(widget, GameLetter):
                self.board.removeMove(self, self.x, self.y, refresh=False)
                widget.copyLetter( self.getLetter() )
                refresh = False
        
        else:
            if isinstance(widget, GameTile):
                self.board.removeMove(widget,widget.x,widget.y)
                refresh = False
        
        l = Letter(letter,score)    
        l.setIsBlank( isBlank )
        if l.isBlank() and showBlank:
            l.setLetter("")
        self.putLetter( l, showBlank )
        #print '%s %s %s put on %s %d,%d' % (str(l.getLetter()), str(l.getScore()), str(l.isBlank()), str(id(self)), self.x,self.y)
        self.board.registerMove(self, self.x, self.y, refresh, widget)
        
        self.source_handler_id = self.connect("drag_data_get", self.dragLetter)
        self.drag_source_set(gtk.gdk.BUTTON1_MASK, [( "image/x-xpixmap", 0, 81 )], gtk.gdk.ACTION_COPY)
        
        o = manager.OptionManager()
        if o.get_default_bool_option(USE_COLOR_NEW_TILE, True):
            self.setBackground(o.get_default_option(COLOR_NEW_TILE, DEFAULT_NEW_TILE))
        
    def putLetter(self, letter, showBlank=False):
        '''
        Put a Letter on the tie.
        
        @param letter:
        @param showBlank: True to show blank value
        '''
        o = manager.OptionManager()
        color = None
        if o.get_default_bool_option(USE_COLOR_LETTER, True):
            color = o.get_default_option(COLOR_LETTER, TILE_COLORS[TILE_LETTER])
        else:
            color = TILE_COLORS[TILE_LETTER]
        
        self.setStyle( TILE_LETTER, color )
        self.set_label( letter, showBlank )
        
        self.letter = letter
    
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
        
        self.__style = style
        
        self.modify_bg(gtk.STATE_NORMAL, gtk.gdk.color_parse(color) )
        self.modify_bg(gtk.STATE_PRELIGHT, gtk.gdk.color_parse(color) )
    
    def update_label(self, showBlank=False):
        '''
        Set the Letter/score label on the Tile
        
        @param letter: Letter
        @see: L{pyscrabble.game.pieces.Letter}
        '''
        widget = self.get_child()
        if (widget is not None):
            self.remove(widget)
        widget = gtk.Label()
            
        l = self.getLetterStr()
        s = str(self.getLetterScore())
        o = manager.OptionManager()
        if l == "":
            s = ""
            #if showBlank:
            #    l = ""
            #if o.get_default_bool_option(USE_COLOR_BLANK_TILE, True):
            #    l = """<span foreground="%s">%s</span>""" % (o.get_default_option(COLOR_BLANK_TILE, DEFAULT_BLANK_TILE), l)
            #    s = """<span foreground="%s">%s</span>""" % (o.get_default_option(COLOR_BLANK_TILE, DEFAULT_BLANK_TILE), s)
        else:
            if o.get_default_bool_option(USE_COLOR_TEXT, True):
                l = """<span foreground="%s">%s</span>""" % (o.get_default_option(COLOR_TEXT, DEFAULT_COLOR_TEXT), l)
                s = """<span foreground="%s">%s</span>""" % (o.get_default_option(COLOR_TEXT, DEFAULT_COLOR_TEXT), s)
        
        if o.get_default_bool_option(OPTION_TEXT_BOLD, False):
            l = """<span weight="bold">%s</span>""" % l
            s = """<span weight="bold">%s</span>""" % s
            
        if len(str(self.getLetterScore())) == 2:
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
        o = manager.OptionManager()
        if self.getLetter() is not None:
            self.set_label( self.getLetter() )
            
            if self.handler_is_connected(self.handler_id):
                if ( o.get_default_bool_option(USE_COLOR_NEW_TILE, True) ):
                    self.setBackground( o.get_default_option(COLOR_NEW_TILE, DEFAULT_NEW_TILE) )
                    return
                    
            if o.get_default_bool_option(USE_COLOR_LETTER, True):
                self.setBackground( o.get_default_option(COLOR_LETTER, TILE_COLORS[TILE_LETTER]) )
            else:
                self.setBackground( TILE_COLORS[TILE_LETTER] )
        else:
            if self.getStyle() is TILE_NORMAL:
                if o.get_default_bool_option(USE_COLOR_NORMAL_TILE, True):
                    self.setBackground( o.get_default_option(COLOR_NORMAL_TILE, TILE_COLORS[TILE_NORMAL]) )
                else:
                    self.setBackground( TILE_COLORS[TILE_NORMAL] )       
    
    def getStyle(self):
        '''
        Return Tile Style
        
        @return: Tile style
        @see: L{constants}
        '''
        
        return self.__style
        
    
    def getLetterStr(self):
        return self.letterStr.Value()
    
    def getLetterScore(self):
        return self.letterScore.Value()
    
    def setLetterStr(self, inStr):
        print "Hello: " + repr(type(inStr))
        self.letterStr.Set(inStr)
    
    def setLetterScore(self, score):
        self.letterScore.Set(score)
    
    def getLetter(self):
        return Letter(self.getLetterStr(), self.getLetterScore())
    
    def getTileScore(self):
        '''
        Get the score of this Tile.
        
        The score is the score of the Letter on the Tile * TILE_STYLE if TILE_STYLE is a Letter Modifier
        
        @return: Tile score
        @see: L{constants}
        '''
        
        
        # If this Tile is a letter modifier, use the stle
        # If its a word modifier, return the letter score
        if (self.__style in LETTER_MODIFIERS):
            return self.letter.getScore() * self.__style
        else:
            return self.letter.getScore() * TILE_NORMAL
    
    def getWordModifier(self):
        '''
        Return the word modifier on this tile if it has one.  If it doesn't, return TILE_NORMAL
        
        @return Word Modifier on this tile
        @see: L{constants}
        '''
        
        if (self.style in WORD_MODIFIERS):
            # Word modifiers are stored as twice the necessary amount so as not to conflict with letter modifiers
            return self.style / 2 
        else:
            return TILE_NORMAL
    
    def __repr__(self):
        '''
        Format Tile as a string::
            Letter: LETTER_ON_TILE Modifier: TILE_MODIFIER
        
        @return: Formatted Tile string
        '''
        
        return "Letter: "+self.letter+" Modifier: "+str(self.style)
        
        




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
        
        self.letterScore = letter.getScore()
        self.letterStr = letter.getLetter()
        
        self.__isBlank = False
        
        self.set_size_request(TILE_WIDTH, TILE_HEIGHT)
        
        self.letterBox = letterBox
        
        self.setBackground()
        
        self.set_label(self.letterStr, self.letterScore)
        
        self.handlerId = 0
        self.destHandlerId = 0
        
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
    
    def getLetterStr(self):
        return self.letterStr.encode('utf-8')
    
    def getLetterScore(self):
        return self.letterScore
    
    def getLetter(self):
        return Letter(self.getLetterStr(), self.getLetterScore())
    
#     def getLetter(self):
#         '''
#         Return the Letter object that this object represents
#         
#         @return: Letter
#         '''
#         
#         return self.getLetterObject()
#     
#     def getLetterObject(self):
#         l = Letter(self.letter)
#         l.__isBlank = self.isBlank()
#         l.score = self.score
#         return l
    
#     def copyLetter(self, letter):
#         '''
#         Copy a letter onto this Letter
#         
#         @param letter: Letter
#         '''
#         
#         s = letter.getLetter()
#         if letter.isBlank():
#             s = ""
#         
#         self.setLetter(s)
#         self.setScore( letter.getScore() )
#         self.setIsBlank(letter.isBlank())
#         self.set_label(letter)

    def copyLetter(self, letterStr, letterScore):
        self.letterStr = letterStr
        self.letterScore = letterScore
        self.set_label(letterStr, letterScore)
    
    def dragLetter(self, widget, context, selection, targetType, eventTime):
        '''
        Drag data callback.
        
        @param widget:
        @param context:
        @param selection:
        @param targetType:
        @param eventTime:
        '''
        print 'dragging GameLetter %s' % widget.getLetterStr()
        #selection.set(selection.target, 8, '%s:%s:%s' % (widget.getLetter().getLetter(), str(widget.getLetter().getScore()), str(int(widget.getLetter().isBlank()))))
        #selection.set(selection.target, 8, '%s:%s:%s' % (widget.getLetterStr(), str(widget.getLetterScore()), "False"))

    def letterDragged(self, widget, context, x, y, selection, targetType, eventType):
        sourceWidget = context.get_source_widget()
        
        if isinstance(sourceWidget, GameTile): # Swap from Board to Tile
            sourceWidget.board.removeMoveNew(sourceWidget, sourceWidget.x, sourceWidget.y)
            tmpLetterStr = self.getLetterStr()
            tmpLetterScore = self.getLetterScore()
            self.copyLetter(sourceWidget.getLetterStr(), sourceWidget.getLetterScore())
            sourceWidget.putLetterNew(tmpLetterStr, tmpLetterScore)
            sourceWidget.board.registerMoveNew(sourceWidget, sourceWidget.x, sourceWidget.y)
            sourceWidget.update_label()
            return
        
        if isinstance(sourceWidget, GameLetter):
            if id(sourceWidget) == id(self):
                return
            
            o = manager.OptionManager()
            opt = o.get_default_option(OPTION_SWAP, OPTION_LETTER_SWAP)
            
            if opt == OPTION_LETTER_INSERT:
                print "INSERT"
                letters = self.letterBox.get_children()
                self.letterBox.foreach(lambda w: self.letterBox.remove(w))
                letters = [ l for l in letters if id(l) != id(sourceWidget) ]
                for l in letters:
                    if id(l) == id(widget):
                        self.letterBox.pack_start(sourceWidget, False, False, 0)
                    self.letterBox.pack_start(l, False, False, 0)
             
            if opt == OPTION_LETTER_SWAP:
                print "SWAP"
                tmpLetterStr = self.getLetterStr()
                tmpLetterScore = self.getLetterScore()
                self.copyLetter(sourceWidget.getLetterStr(), sourceWidget.getLetterScore())
                sourceWidget.copyLetter(tmpLetterStr, tmpLetterScore)
            
    
    def letterDraggedOld(self, widget, context, x, y, selection, targetType, eventType):
        '''
        Callback when a widget is dragged onto this letter.
        
        @param widget:
        @param context:
        @param x:
        @param y:
        @param selection:
        @param targetType:
        @param eventType:
        '''
        letter = context.get_source_widget()
        
        if isinstance(letter, GameTile): # Swap from Board to Tile
            tile = letter
            tmp = self.clone()
            tile.board.removeMove(tile,tile.x,tile.y, refresh=False)
            self.copyLetter( tile.getLetter() )
            tile.setLetter( None, tmp.getLetter(), tmp.getScore(), tmp.isBlank() )
            return
         
        if isinstance(letter, GameLetter):
             
            if id(letter) == id(self): # ignore if widget is dragged onto itself
                return
             
            o = manager.OptionManager()
            opt = o.get_default_option(OPTION_SWAP, OPTION_LETTER_SWAP)
             
            if opt == OPTION_LETTER_INSERT:
                letters = self.letterBox.get_children()
                self.letterBox.foreach(lambda w: self.letterBox.remove(w))
                letters = [ l for l in letters if id(l) != id(letter) ]
                for l in letters:
                    if id(l) == id(widget):
                        self.letterBox.pack_start(letter, False, False, 0)
                    self.letterBox.pack_start(l, False, False, 0)
             
            if opt == OPTION_LETTER_SWAP:
                l = self.getLetter().clone()
                self.copyLetter(letter.getLetter())
                letter.copyLetter(l)
    
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
        
        self.set_label( self.letterStr, self.letterScore )
        self.setBackground()
        
    
    def set_label(self, letterStr, letterScore):
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
        
        l = letterStr
        s = str(letterScore)
        o = manager.OptionManager()
        if o.get_default_bool_option(USE_COLOR_TEXT, True):
            l = """<span foreground="%s">%s</span>""" % (o.get_default_option(COLOR_TEXT, DEFAULT_COLOR_TEXT), l)
            s = """<span foreground="%s">%s</span>""" % (o.get_default_option(COLOR_TEXT, DEFAULT_COLOR_TEXT), s)
        
        if o.get_default_bool_option(OPTION_TEXT_BOLD, False):
            l = """<span weight="bold">%s</span>""" % l
            s = """<span weight="bold">%s</span>""" % s
        
        if len(str(letterScore)) == 2:
            widget.set_markup("""%s <sub><span size="xx-small">%s</span></sub>""" % (l, s))
        else:
            widget.set_markup("""%s <sub><span size="x-small">%s</span></sub>""" % (l, s))
        
        self.add(widget)
        self.show_all()
        
    def setLetter(self, letter):
        '''
        Set the Letter string
        
        @param letter:
        '''
        if (letter == ""):
            self.__isBlank = True
        #else:
        #    self.__isBlank = False
        self.letterStr = letter

    #def clone(self):
    #    '''
    #    Clone the letter
    #    
    #    @return: Clone of this Letter
    #    '''
    #    l = Letter(self.letter)
    #    l.__isBlank = self.isBlank()
    #    l.score = self.score
    #    return l
    
    def isBlank(self):
        '''
        Check if the Letter is a Blank.
        
        @return: True if the Letter is a blank letter.
        '''
        
        return self.__isBlank
        
        
       
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
                if (self.tiles[(x,y)].getLetterStr() != l):
                    return False
            elif not self.tiles.has_key( (x,y) ):
                return False
        
        return True
        
        
    #TODO: figure out if this method is still used
    def putLetter(self, letter, x, y, set_bg):
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
        
        if set_bg:
            o = manager.OptionManager()
            self.tiles[(x,y)].setBackground(o.get_default_option(COLOR_RECENT_TILE, DEFAULT_RECENT_TILE))
    
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
                    if self.tiles[(x, y)].getLetterStr() != None:
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
                    if self.tiles[(x, y)].getLetterStr() != None:
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
            if self.tiles[(x+1, y)].getLetterStr() != "":
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
            if self.tiles[(x, y+1)].getLetterStr() != "":
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
            if self.tiles[(x-1, y)].getLetterStr() != "":
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
            if self.tiles[(x, y-1)].getLetterStr() != "":
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
    
    #def isEmpty(self):
    #    '''
    #    Check whether the board has any moves on it
    #    
    #    @return True if the board has no moves
    #    '''
    #    return self.empty
    
    def getMovesAtXY(self, x, y):
        '''
        Get moves at X and Y positions
        
        @param x:
        @param y:
        '''
        moves = []
        
        h = Move()
        v = Move()
        h.addMove( Letter(self.get(x,y).getLetterStr(), self.get(x,y).getLetterScore()), x, y)
        v.addMove( Letter(self.get(x,y).getLetterStr(), self.get(x,y).getLetterScore()), x, y)
        
        _x = x
        _y = y
        while self.hasNextHorizontalLetter(_x, _y):
            item, _x, _y = self.getNextHorizontalLetter(_x, _y)
            h.addMove(Letter(item.getLetterStr(), item.getLetterScore()), _x, _y)
        
        _x = x
        _y = y
        while self.hasPreviousHorizontalLetter(_x, _y):
            item, _x, _y = self.getPreviousHorizontalLetter(_x, _y)
            h.addMove(Letter(item.getLetterStr(), item.getLetterScore()), _x, _y)
        
        h.sort()
        if h.length() > 1:
            moves.append(h)
        
        
        _x = x
        _y = y
        while self.hasNextVerticalLetter(_x, _y):
            item, _x, _y = self.getNextVerticalLetter(_x, _y)
            v.addMove(Letter(item.getLetterStr(), item.getLetterScore()), _x, _y)
        
        _x = x
        _y = y
        while self.hasPreviousVerticalLetter(_x, _y):
            item, _x, _y = self.getPreviousVerticalLetter(_x, _y)
            v.addMove(Letter(item.getLetterStr(), item.getLetterScore()), _x, _y)
        
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
            pass
            #tile.removeArrow()
        
        
        
        
