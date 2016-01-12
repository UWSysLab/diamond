from pyscrabble.constants import *
from pyscrabble.lookup import *
from pyscrabble.exceptions import *
from pyscrabble import manager
from pyscrabble import util
from random import shuffle

import sys
sys.path.append("/home/nl35/research/diamond-src/backend/build/src/bindings/python")
sys.path.append("/home/nl35/research/diamond-src/backend/src/bindings/python")
from libpydiamond import *

class Letter(object):
    '''
    Letter class.  Represents a letter on the gameboard
    '''
    
    def __init__(self, letter="", score=0 ):
        '''
        Initialize the Letter
        
        @param letter: String representing the Letter
        '''
        self.score = score
        self.__isBlank = False
        self.setLetter( letter )
    
    def clone(self):
        '''
        Clone the letter
        
        @return: Clone of this Letter
        '''
        l = Letter(self.letter)
        l.__isBlank = self.isBlank()
        l.score = self.score
        return l
        
    
    def setLetter(self, letter):
        '''
        Set the Letter string
        
        @param letter:
        '''
        if (letter == ""):
            self.__isBlank = True
        #else:
        #    self.__isBlank = False
        self.letter = letter
    
    def setIsBlank(self, isBlank):
        '''
        Set isBlank property
        
        @param isBlank: True / False
        '''
        self.__isBlank = isBlank
        
            
    def getLetter(self):
        '''
        Get the Letter string
        
        @return: String representation of this Letter
        '''
        
        return self.letter
    
    def getCharacter(self):
        '''
        Get the Letter string
        
        @return: String representation of this Letter
        @see: L{Letter.getLetter}
        '''
        
        return self.getLetter()
    
    def getScore(self):
        '''
        Get the score of this letter
        
        @return: Score of this letter
        '''
        if self.isBlank():
            return 0
        else:
            return int(self.score)
    
    def setScore(self, score):
        '''
        Set score for letter
        
        @param score: Score
        '''
        self.score = score
        
    
    def __repr__(self):
        '''
        Return a String formatted as follows::
            LETTER_STRING: SCORE
        
        @return String representation of this Letter
        '''
        
        return self.getLetter() + ": "+str(self.getScore())
    
    def __eq__(self, other):
        '''
        Check if this Letter equals another Letter.
        
        @param other: Other letter
        @return: True if the Letter strings are the same
        '''
        
        if isinstance(other, Letter):
            a = util.getUnicode(self.getLetter())
            b = util.getUnicode(other.getLetter())

            if a == b:
                return True
            elif ((a != b) and (self.isBlank() == True and other.isBlank() == True)):
                return True
        return False
    
    def __neq__(self, other):
        '''
        Check if this Letter does not equal another Letter.
        
        @param other: Other letter
        @return: True if the LEtter strings are not the same.
        '''
        
        if isinstance(other, Letter):
            if self.getLetter() == other.getLetter():
                return False
        return True
    
    def isBlank(self):
        '''
        Check if the Letter is a Blank.
        
        @return: True if the Letter is a blank letter.
        '''
        
        return self.__isBlank

        
# Represents a tile on the gameboard
class Tile:
    '''
    Tile class.
    
    Represents a Tile on the gameboard.
    '''
    
    def __init__( self, style=TILE_NORMAL):
        '''
        Initialize a Tile.
        
        @param style: Tile Style
        @see: L{constants}
        '''
        
        self.__style = style
        self.letter = None
    
    def getStyle(self):
        '''
        Return Tile Style
        
        @return: Tile style
        @see: L{constants}
        '''
        
        return self.__style
    
    def setStyle(self, style):
        '''
        Set Tile Style
        
        @param style: Tile Stype
        @see: L{constants}
        '''
        
        self.__style = style
    
    def setLetter(self, letter):
        '''
        Set the Letter on this tile
        
        @param letter: Letter to put on the Tile
        @see: L{Letter}
        '''
        
        self.letter = letter
    
    def getLetter(self):
        '''
        Return Letter on this Tile or None
        
        @return: Letter on this Tile or None
        '''
        
        return self.letter
    
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


# Bag is the "bag" of letters
class Bag:
    '''
    Bag represents a "Bag" of Letters
    '''
    
    def __init__(self, rules):
        '''
        Initialize the Letter bag
        
        @see: L{Letter}
        '''
        self.letterStrs = DStringList()
        DStringList.Map(self.letterStrs, "testgame:bag:letterstrs")
        self.letterScores = DList()
        DList.Map(self.letterScores, "testgame:bag:letterscores")
        self.rules = DString()
        DString.Map(self.rules, "testgame:bag:rules")
    
    def reset(self, rules):
        self.rules.Set(rules)
        self.letterStrs.Clear()
        self.letterScores.Clear()
        l = manager.LettersManager()
        for letter,count,score in l.getLetters(self.rules.Value()):
            for x in range(count):
                self.letterStrs.Append(letter.encode("utf-8"))
                self.letterScores.Append(score)
        
        # Shuffle the letters in the bag
        self.shuffleLetters()
    
    def shuffleLetters(self):
        indices = range(0, self.letterStrs.Size())
        shuffle(indices)
        tempLetterStrs = []
        tempLetterScores = []
        for i in range(0, self.letterStrs.Size()):
            tempLetterStrs.append(self.letterStrs.Value(i))
            tempLetterScores.append(self.letterScores.Value(i))
        self.letterStrs.Clear()
        self.letterScores.Clear()
        for index in indices:
            self.letterStrs.Append(tempLetterStrs[index])
            self.letterScores.Append(tempLetterScores[index])
    
    def getDistribution(self):
        '''
        Get Letter distribution
        
        @return: dict(Letter,Count)
        '''
        
        result = {}
        
        for letter in self.letterStrs.Members():
            key = letter
            if result.has_key(key):
                result[key] = result[key] + 1
            else:
                result[key] = 1
        
        return result

    
    def getLetters(self, count = 7):
        '''
        Get C{count} number of letters from the bag. If C{count} is greater than the number of Letters
        left in the Bag, the remaining number of Letters are returned
        
        @param count: Number of letters to get
        @raise BagEmptyException: If the Bag is empty.
        @see: L{Letter}
        '''
        
        if self.isEmpty():
            raise BagEmptyException()
        
        # Take "count" number letters from bag, or remaining number of letters
        if (self.getCount() < count):
            count = self.getCount()
        
        result = []
        for x in range(count):
            result.append(Letter(self.letterStrs.Value(0), self.letterScores.Value(0)))
            self.letterStrs.Erase(0)
            self.letterScores.Erase(0)
        print "DEBUG BAG::GETLETTERS " + repr(result)
        return result
    
    def isEmpty(self):
        '''
        Check to see if the Bag is empty
        
        @return: True if the Bag is empty
        '''
        
        return (self.getCount() == 0)
    
    def getCount(self):
        '''
        Get the number of Letters in the Bag
        
        @return: Number of letters in the Bag
        '''
        
        return self.letterStrs.Size()
    
    def returnLetters(self, letters):
        '''
        Return a list of Letters to the Bag.
        
        @param letters: List of Letters to return to the Bag
        @see: L{Letter}
        '''
        
        for letter in letters:
            self.letterStrs.Append( letter.getLetter() )
            self.letterScores.Append( letter.getScores() )
        
        self.shuffleLetters()

class Move(object):
    '''
    Move represents a move that is made on the board.
    
    The Move is a list of tuples containing (letter, x-position, y-position) for all Letters in the Move.
    '''
    
    def __init__(self, move = None):
        '''
        Initialize the Move.
        
        @param move: Optional Move to clone
        '''
        
        self.score = 0
        self.__hasBlank = False
        self.move = []
        
        if (move != None): # The move is a list of (letter, x-position, y-position) tuples
            for letter,x,y in move:
                self.addMove(letter, x, y)
    
    def setScore(self, score):
        '''
        Set the score
        
        @param score: Score
        '''
        self.score = score
    
    def getScore(self):
        '''
        Get the score for this move
        
        @return: Score for the move
        '''
        return self.score
        
        
                
    
    def addMove(self, letter, x, y):
        '''
        Add a Letter to the Move
        
        @param letter: Letter
        @param x: X position
        @param y: Y Position
        @see: L{Letter}
        '''
        
        if not self.hasBlank():
            self.__hasBlank = letter.isBlank()
            
        self.move.append( (letter, x, y) )
    
    def removeMove(self, letter, x, y):
        '''
        Remove move
        
        @param letter:
        @param x:
        @param y:
        '''
        self.move.remove( (letter,x,y) )
        
    
    def prependMove(self, letter, x, y):
        '''
        Insert a Letter at the beginning of the Move
        
        @param letter: Letter
        @param x: X position
        @param y: Y position
        @see: L{Letter}
        '''
        
        if not self.hasBlank():
            self.__hasBlank = letter.isBlank()

        self.move.insert(0, (letter,x,y) )
    
    def isEmpty(self):
        '''
        Check whether the Move is empty
        
        @return: True if the Move has no Letters in it.
        '''
        
        return len(self.move) == 0
    
    def isContinuous(self):
        '''
        Check to see if the move is continous, meaning all the tiles are connected
        
        @return: True if all the tiles are connected
        '''
        self.sort()
        
        if self.isVertical():
            prevy = -1
            for letter, x, y in self.move:
                if prevy == -1:
                    prevy = y
                    continue
                if y != (prevy +1):
                    return False
                prevy = y
            return True
        
        if self.isHorizontal():
            prevx = -1
            for letter, x, y in self.move:
                if prevx == -1:
                    prevx = x
                    continue
                if x != (prevx +1):
                    return False
                prevx = x
            return True
                
        
    
    def isVertical(self):
        '''
        Check whether the Move is vertical.
        
        @return: True if all Letters in the move are arranged vertically.
        '''
        
        if (self.isEmpty()):
            return False
        
        letter,x,y = self.move[0]
        for _letter,_x,_y in self.move:
            if x != _x:
                return False
        
        return True
    
    def isHorizontal(self):
        '''
        Check whether the Move is horizontal.
        
        @return: True if all Letters in the move are arranged horizontally.
        '''
        
        if (self.isEmpty()):
            return False
        
        letter,x,y = self.move[0]
        for _letter, _x, _y in self.move:
            if y != _y:
                return False
        
        return True
    
    def isValid(self):
        '''
        Check whether the move is valid.
        
        A Move is valid if it is::
            - Not empty
            - Horitzontal or
            - Veritcal
        
        @return: True if the Move is valid.
        '''
        
        return (not self.isEmpty()) and (self.isHorizontal() or self.isVertical())
    
    def clear(self):
        '''
        Clear the move
        '''
        
        self.move = []
    
    def getTiles(self):
        '''
        Get the list of tuples in the move.
        
        @return: List of (letter, x-position, y-position) tuples.
        '''
        
        return self.move
    
    def getFirstMove(self):
        '''
        Return the first tuple in the Move
        
        @return: The first tuple in the Move or None if the Move is empty
        '''
        
        if (self.isEmpty()):
            return None
        else:
            return self.move[0]
    
    def hasCommonTile(self, move):
        '''
        Check to see if this move has a common tile with C{move}
        
        @param move: Move to check
        @return: True if this Move has a letter,x,y tile in common with C{move}
        '''
        for letter, x, y in move.getTiles():
            if (self.contains(letter,x,y)):
                return True
        return False
        
    
    def containsMove(self, move):
        '''
        Check to see if this Move contains the tiles in C{move}
        
        @param move: Move
        '''
        for letter, x, y in move.getTiles():
            if not self.contains(letter, x, y):
                return False
        return True
        
    
    def contains(self,letter,x,y):
        '''
        Check to see if the Move contains C{letter} at C{x,y}
        
        @param letter:
        @param x:
        @param y:
        @return: True if the Move contains C{letter} at C{x,y}
        @see: L{Letter}
        '''
        
        return (letter,x,y) in self.move
    
    def hasLetterAt(self, x, y):
        '''
        Checks to see if the move has a letter at C{x,y}
        
        @param x: X position
        @param y: Y position
        @return: True if the Move has a Letter at C{x,y}
        '''
        for letter, _x, _y in self.move:
            if _x == x and _y == y:
                return True
        return False
        
    
    def clone(self):
        '''
        Clone this move
        
        @return: Cloned Move.
        '''
        
        return Move(self.move[:])
    
    def appendMove(self, move):
        '''
        Append a Move
        
        @param move: Move to append
        '''
        
        for letter, x, y in move.getTiles():
            self.addMove(letter,x,y)
    
    def length(self):
        '''
        Get number of tuples in the Move.
        
        @return: Number of tuples in the move.
        '''
        
        return len(self.move)
    
    def __repr__(self):
        '''
        String format.
        
        @return: String format of the Move.  Concatenate each letter in the Move.
        '''
        
        buf = ''
        for letter,x,y in self.move:
            buf += str(letter)
            buf += ' | '
        
        return buf
    
    def getWord(self):
        '''
        Get the word that this Move spells
        
        @return: Word that this Move spells
        '''
        
        self.sort()
        
        word = unicode('', 'utf-8')
        for letter, x, y in self.move:
            word += letter.getLetter()
        
        return word
    
    def sort(self):
        '''
        Sort the move.
        
        If Horizontal, arrange the Letters by the x-position
        If Vertical, arrange the Letters by their y-position
        '''
        
        
        if (self.isHorizontal()):
            self.move.sort( lambda (t1, x1, y1), (t2, x2, y2):  x1 - x2 )
        
        elif (self.isVertical()):
            self.move.sort( lambda (t1, x1, y1), (t2, x2, y2):  y1 - y2 )
    
    def hasBlank(self):
        '''
        Check whether this Move contains a blank letter
        
        @return: True if this Move contains a blank Letter
        @see: L{Letter}
        '''
        
        return self.__hasBlank
            
        
        