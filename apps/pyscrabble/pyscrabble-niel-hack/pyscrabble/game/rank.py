import ConfigParser
import os

class Rankings:
    '''
    Rankings
    '''
    
    def __init__(self, config):
        '''
        Constructor
        
        Parse rankings file
        
        @param config: Confile file location
        '''
        
        if not os.path.exists(config):
            raise IOError, "%s must exist in %s" % (os.path.basename(config), os.path.dirname(config))
        
        parser = ConfigParser.ConfigParser()
        parser.read( config )
        
        self.ranks = []
        for section in parser.sections():
            p_wins = parser.get(section, "wins")
            rank_num = parser.get(section, "rank")
            self.ranks.append( Rank( section, rank_num, p_wins ) )
    
        self.ranks.sort()
        
    def getRankByWins(self, wins):
        '''
        Get the Rank corresponding to C{wins}
        
        @param wins: Number of wins
        @return: Rank corresponding to C{wins}
        '''
        
        wins = int(wins)
        
        r = None
        for rank in self.ranks:
            if wins >= int(rank.wins):
                r = rank
        
        return r
    
    def getTopRank(self):
        '''
        Get top rank
        
        @return: Top Rank
        '''
        
        return max(self.ranks)
    
    def getMinRank(self):
        '''
        Get lowest rank
        
        @return: Lowest rank
        '''
        
        return min(self.ranks)
    
    def getRankInfo(self):
        '''
        Information about the ranks
        
        @return: List of tuples (Name, Wins Required) for all the Ranks
        '''
        
        l = []
        for rank in self.ranks:
            l.append( (rank.name, rank.wins) )
        return l

class Rank:
    '''
    Rank
    '''
    
    def __init__(self, name, rank_number, wins):
        '''
        Constructor
        
        @param name: Rank Name
        @param rank_number: Rank order
        @param wins: Number of wins required
        '''
        
        self.name = name
        self.rank_number = rank_number
        self.wins = wins
    
    def __eq__(self, other):
        return self.rank_number == other.rank_number
    
    def __lt__(self, other):
        return self.rank_number < other.rank_number
    
    def __gt__(self, other):
        return self.rank_number > other.rank_number
    
    def __repr__(self):
        return self.name