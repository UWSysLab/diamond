# -*- coding: utf-8 -*-
import codecs
import os
import re
import ConfigParser
from pyscrabble import constants
from pyscrabble import dist
from pyscrabble import lookup
import __builtin__
if not hasattr(__builtin__, '_'):
    from gettext import gettext as _
import gettext


class ResourceManager(object):
    '''
    Manager for filesystem resources
    
    Implements the Borg paradigm for shared state
    '''
    __shared_state = {}
    
    def __init__(self):
        '''
        Constructor
        '''
        self.__dict__ = self.__shared_state
        
        if not hasattr(self, 'loaded'):
            self.data = {}
            self.loadDefault()
    
    def loadDefault(self):
        '''
        Load default resources
        '''
        self.loaded = True
        
        self["config"] = dist.Resource( dist.CONFIG_DIR )
        self["resources"] = dist.Resource( dist.RESOURCE_PREFIX )
    
    def __getitem__(self, item):
        '''
        Get item
        
        @param item: Item
        @return: Resource item
        '''
        return self.data[item]
    
    def __setitem__(self, item, value):
        '''
        Set item
        
        @param item:
        @param value:
        '''
        self.data[item] = value
        
        
        
        
    

class SoundManager:
    '''
    Manager Class for Sounds
    
    Implements the Borg paradigm for shared state
    '''
    __shared_state = {}
    HAS_PYMEDIA = False

    max_ms = 300000
    
    def __init__(self):
        '''
        Constructor
        
        Initialize sounds on the first initialization
        '''
        
        self.__dict__ = self.__shared_state
        
        if not hasattr(self, 'sounds'):
            self.loadSounds()
    
    def loadSounds(self):
        '''
        Load sound files from preferences
        '''
        
        try:
            import wave
            import pymedia.audio.sound as sound
            SoundManager.HAS_PYMEDIA = True
        except ImportError:
            SoundManager.HAS_PYMEDIA = False
            try:
                import pygame.mixer
                from pygame.mixer import Sound
                pygame.mixer.init()
            except ImportError:
                import sys
                sys.stderr.write('Pymedia and pygame are not available, one of which is needed for sound support.')
                sys.exit(1)
        
        self.sounds = {}
        
        o = OptionManager()
        r = ResourceManager()
        
        for s in constants.SOUNDS:
            
            opt = o.get_default_option(s, r["resources"]["sounds"][constants.DEFAULT_SOUND] )
            
            if SoundManager.HAS_PYMEDIA:
                f = wave.open(opt, 'rb')
                if f.getsampwidth() >1:
                    snd = sound.Output(f.getframerate(), f.getnchannels(), sound.AFMT_S16_LE)
                else:
                    snd = sound.Output(f.getframerate(), f.getnchannels(), sound.AFMT_U8)
                self.sounds[s] = { "frames" : f.readframes( self.max_ms ), "snd"  : snd }
                f.close()
            else:
                self.sounds[s] = Sound(opt)
    def play(self, sound):
        '''
        Play sound
        
        @param sound: Sound key name
        '''
        
        d = self.sounds[sound]
        if SoundManager.HAS_PYMEDIA:
            d["snd"].play( d["frames"] )
        else:
            d.play(0, self.max_ms)



class OptionManager:
    '''
    Option Manager
    
    Implements the Borg paradigm for shared state
    '''
    __options = None
    
    def __init__(self, section=constants.OPTIONS_SECTION):
        '''
        Constructor
        
        Load options on the first initialization
        '''
        
        self.options = OptionManager.__options
        self._section = section
        
        if self.options is None:
            self.loadOptions()
    
    def loadOptions(self):
        '''
        Load options from config file
        '''
        
        r = ResourceManager()
        
        dist.ensure_config_dir( r["config"].path )
        
        OptionManager.__options = ConfigParser.ConfigParser()
        OptionManager.__options.read( r["config"][constants.OPTION_CONFIG] )
        self.options = OptionManager.__options
    
    def set_option(self, option, value):
        '''
        Set option
        
        @param option: Option Name
        @param value: Option Value
        '''
        
        r = ResourceManager()
        config = r["config"][constants.OPTION_CONFIG]
        
        if not self.options.has_section(self._section):
            self.options.add_section(self._section)
        
        self.options.set(self._section, option, value)
        f = open( config, 'w' )
        self.options.write(f )
        f.close()
        self.options.read( config )
        
    def get_default_option(self, option, default):
        '''
        Get option value returning a default value if the option is empty
        
        @param option:
        @param default: Default value if option is empty
        '''
        
        if not self.options.has_section(self._section):
            self.options.add_section(self._section)
        
        if not self.options.has_option(self._section, option):
            return default
        
        val = self.options.get(self._section, option)
        
        if val is None or val == '':
            return default
        else:
            return val
    
    def get_default_bool_option(self, option, default):
        '''
        Get boolean option
        
        @param option: Option Name
        @param default: Default value
        @return: Option value as a boolean
        '''
        
        return bool(int(self.get_default_option(option, default)))
        
        
class LocaleManager:
    '''
    Manager Class for Locales
    
    Implements the Borg paradigm for shared state
    '''
    __shared_state = {}
    
    iso639_languageDict = { 
                        'aa'    : 'Afar. ',
                        'ab'    : 'Abkhazian. ',
                        'ae'    : 'Avestan. ',
                        'af'    : 'Afrikaans. ',
                        'am'    : 'Amharic. ',
                        'ar'    : 'Arabic. ',
                        'as'    : 'Assamese. ',
                        'ay'    : 'Aymara. ',
                        'az'    : 'Azerbaijani. ',
                        'ba'    : 'Bashkir. ',
                        'be'    : 'Byelorussian; Belarusian. ',
                        'bg'    : 'Bulgarian. ',
                        'bh'    : 'Bihari. ',
                        'bi'    : 'Bislama. ',
                        'bn'    : 'Bengali; Bangla. ',
                        'bo'    : 'Tibetan. ',
                        'br'    : 'Breton. ',
                        'bs'    : 'Bosnian. ',
                        'ca'    : 'Catalan. ',
                        'ce'    : 'Chechen. ',
                        'ch'    : 'Chamorro. ',
                        'co'    : 'Corsican. ',
                        'cs'    : 'Czech. ',
                        'cu'    : 'Church Slavic. ',
                        'cv'    : 'Chuvash. ',
                        'cy'    : 'Welsh. ',
                        'da'    : 'Danish. ',
                        'de'    : _('German'),
                        'dz'    : 'Dzongkha; Bhutani. ',
                        'el'    : 'Greek. ',
                        'en'    : _('English'),
                        'eo'    : 'Esperanto. ',
                        'es'    : 'Spanish. ',
                        'et'    : 'Estonian. ',
                        'eu'    : 'Basque. ',
                        'fa'    : 'Persian. ',
                        'fi'    : _('Finnish'),
                        'fj'    : 'Fijian; Fiji. ',
                        'fo'    : 'Faroese. ',
                        'fr'    : _('French'),
                        'fy'    : 'Frisian. ',
                        'ga'    : 'Irish. ',
                        'gd'    : 'Scots; Gaelic. ',
                        'gl'    : 'Gallegan; Galician. ',
                        'gn'    : 'Guarani. ',
                        'gu'    : 'Gujarati. ',
                        'gv'    : 'Manx. ',
                        'ha'    : 'Hausa (?). ',
                        'he'    : 'Hebrew (formerly iw). ',
                        'hi'    : 'Hindi. ',
                        'ho'    : 'Hiri Motu. ',
                        'hr'    : 'Croatian. ',
                        'hu'    : 'Hungarian. ',
                        'hy'    : 'Armenian. ',
                        'hz'    : 'Herero. ',
                        'ia'    : 'Interlingua. ',
                        'id'    : 'Indonesian (formerly in). ',
                        'ie'    : 'Interlingue. ',
                        'ik'    : 'Inupiak. ',
                        'io'    : 'Ido. ',
                        'is'    : 'Icelandic. ',
                        'it'    : 'Italian. ',
                        'iu'    : 'Inuktitut. ',
                        'ja'    : 'Japanese. ',
                        'jv'    : 'Javanese. ',
                        'ka'    : 'Georgian. ',
                        'ki'    : 'Kikuyu. ',
                        'kj'    : 'Kuanyama. ',
                        'kk'    : 'Kazakh. ',
                        'kl'    : 'Kalaallisut; Greenlandic. ',
                        'km'    : 'Khmer; Cambodian. ',
                        'kn'    : 'Kannada. ',
                        'ko'    : 'Korean. ',
                        'ks'    : 'Kashmiri. ',
                        'ku'    : 'Kurdish. ',
                        'kv'    : 'Komi. ',
                        'kw'    : 'Cornish. ',
                        'ky'    : 'Kirghiz. ',
                        'la'    : 'Latin. ',
                        'lb'    : 'Letzeburgesch. ',
                        'ln'    : 'Lingala. ',
                        'lo'    : 'Lao; Laotian. ',
                        'lt'    : 'Lithuanian. ',
                        'lv'    : 'Latvian; Lettish. ',
                        'mg'    : 'Malagasy. ',
                        'mh'    : 'Marshall. ',
                        'mi'    : 'Maori. ',
                        'mk'    : 'Macedonian. ',
                        'ml'    : 'Malayalam. ',
                        'mn'    : 'Mongolian. ',
                        'mo'    : 'Moldavian. ',
                        'mr'    : 'Marathi. ',
                        'ms'    : 'Malay. ',
                        'mt'    : 'Maltese. ',
                        'my'    : 'Burmese. ',
                        'na'    : 'Nauru. ',
                        'nb'    : 'Norwegian Bokmål. ',
                        'nd'    : 'Ndebele, North. ',
                        'ne'    : 'Nepali. ',
                        'ng'    : 'Ndonga. ',
                        'nl'    : 'Dutch. ',
                        'nn'    : 'Norwegian Nynorsk. ',
                        'no'    : 'Norwegian. ',
                        'nr'    : 'Ndebele, South. ',
                        'nv'    : 'Navajo. ',
                        'ny'    : 'Chichewa; Nyanja. ',
                        'oc'    : 'Occitan; Provençal. ',
                        'om'    : '(Afan) Oromo. ',
                        'or'    : 'Oriya. ',
                        'os'    : 'Ossetian; Ossetic. ',
                        'pa'    : 'Panjabi; Punjabi. ',
                        'pi'    : 'Pali. ',
                        'pl'    : 'Polish. ',
                        'ps'    : 'Pashto, Pushto. ',
                        'pt'    : 'Portuguese. ',
                        'qu'    : 'Quechua. ',
                        'rm'    : 'Rhaeto-Romance. ',
                        'rn'    : 'Rundi; Kirundi. ',
                        'ro'    : 'Romanian. ',
                        'ru'    : 'Russian. ',
                        'rw'    : 'Kinyarwanda. ',
                        'sa'    : 'Sanskrit. ',
                        'sc'    : 'Sardinian. ',
                        'sd'    : 'Sindhi. ',
                        'se'    : 'Northern Sami. ',
                        'sg'    : 'Sango; Sangro. ',
                        'si'    : 'Sinhalese. ',
                        'sk'    : 'Slovak. ',
                        'sl'    : 'Slovenian. ',
                        'sm'    : 'Samoan. ',
                        'sn'    : 'Shona. ',
                        'so'    : 'Somali. ',
                        'sq'    : 'Albanian. ',
                        'sr'    : _('Serbian'),
                        'ss'    : 'Swati; Siswati. ',
                        'st'    : 'Sesotho; Sotho, Southern. ',
                        'su'    : 'Sundanese. ',
                        'sv'    : 'Swedish. ',
                        'sw'    : 'Swahili. ',
                        'ta'    : 'Tamil. ',
                        'te'    : 'Telugu. ',
                        'tg'    : 'Tajik. ',
                        'th'    : 'Thai. ',
                        'ti'    : 'Tigrinya. ',
                        'tk'    : 'Turkmen. ',
                        'tl'    : 'Tagalog. ',
                        'tn'    : 'Tswana; Setswana. ',
                        'to'    : 'Tonga (?). ',
                        'tr'    : 'Turkish. ',
                        'ts'    : 'Tsonga. ',
                        'tt'    : 'Tatar. ',
                        'tw'    : 'Twi. ',
                        'ty'    : 'Tahitian. ',
                        'ug'    : 'Uighur. ',
                        'uk'    : 'Ukrainian. ',
                        'ur'    : 'Urdu. ',
                        'uz'    : 'Uzbek. ',
                        'vi'    : 'Vietnamese. ',
                        'vo'    : 'Volapük; Volapuk. ',
                        'wa'    : 'Walloon. ',
                        'wo'    : 'Wolof. ',
                        'xh'    : 'Xhosa. ',
                        'yi'    : 'Yiddish (formerly ji). ',
                        'yo'    : 'Yoruba. ',
                        'za'    : 'Zhuang. ',
                        'zh'    : 'Chinese. ',
                        'zu'    : 'Zulu.'
                     }
    
    def __init__(self):
        '''
        Constructor
        
        Initialize locales on the first initialization
        '''
        
        self.locale = None
        self.__dict__ = self.__shared_state
        
        if not hasattr(self, 'locales'):
            self.loadLocales()
    
    def loadLocales(self):
        '''
        Load locales into memory and set option locale
        '''
        r = ResourceManager()
        self.locales = {}
        
        dir = r["resources"][constants.LOCALE_DIR].path
        langs = os.listdir( dir )
        
        self.locales['en'] = gettext.NullTranslations()
        
        for lang in langs:
            self.locales[lang] = gettext.translation(constants.LOCALE_DOMAIN, dir, languages=[lang])
        
        self.setLocale()

    def setLocale(self):
        '''
        Set locale based on option
        '''
        x = OptionManager()
        lang = x.get_default_option(constants.LOCALE_OPTION, None)
        
        if lang:
            self.locales[lang].install()
            self.locale = lang
        else:
            x = gettext.NullTranslations()
            x.install()
            self.locale = 'en'
        
        reload(constants)
        reload(lookup)
    
    def getAvailableLocales(self):
        '''
        Return list of available Locales
        
        @return: List of available locales
        '''
        keys = self.locales.keys()
        keys.sort(lambda x, y: cmp(_(self.getLocaleDescription(x)),_(self.getLocaleDescription(y))) )
        return keys
    
    def getLocaleDescription(self, locale):
        '''
        Get locale description
        
        @param locale: Locale code
        @return: Locale description
        '''
        return self.iso639_languageDict[locale]
    
    def setDefault(self):
        '''
        Use default translation
        '''
        x = gettext.NullTranslations()
        x.install()


class LettersManager:
    '''
    Manager Class for Letters
    
    Implements the Borg paradigm for shared state
    '''
    __shared_state = {}
    
    def __init__(self):
        '''
        Constructor
        
        Initialize letters on the first initialization
        '''
        
        self.locale = None
        self.__dict__ = self.__shared_state
        
        if not hasattr(self, 'letters'):
            self.loadLetters()
    
    def loadLetters(self):
        '''
        Load letter configurations into memory
        '''
        self.letters = {}
        self.meta = {}
        r = ResourceManager()
        
        dir = r["resources"][constants.LETTERS_DIR].path
        langs = os.listdir( dir )
        
        pattern = re.compile("\((\d+),(\d+)\)")
        
        for lang in langs:
            if not lang.islower(): continue # Avoids CVS directories
            p = UTF8ConfigParser();
            p.read( r["resources"][constants.LETTERS_DIR][lang]["letters.txt"] )
            letters = []
            for letter,info in p.items('letters'):
                l = letter.strip('"')
                m = pattern.match(info)
                count,score = int(m.group(1)), int(m.group(2))
                letters.append( (l,count,score) )
            self.letters[lang] = letters
            
            self.meta[lang] = {}
            for name,value in p.items('meta'):
                self.meta[lang][name] = value
    
    def getLetters(self, lang):
        '''
        Get Letter Configuration for lang
        
        @param lang: Lang code
        '''
        return self.letters[ lang ]
    
    def getMeta(self, lang):
        '''
        Get meta information about a character set
        
        @param lang:
        '''
        return self.meta[lang]
    
    def isValidLetter(self, lang, letter):
        '''
        Check if C{letter} is valid for C{lang}
        
        @param lang: Language code
        @param letter: Letter
        '''
        for l,count,score in self.getLetters(lang):
            if letter == l:
                return True
        return False


class UTF8ConfigParser(ConfigParser.ConfigParser):
    
    optionxform = unicode
    
    def read(self, filenames):
        """Read and parse a filename or a list of filenames.

        Files that cannot be opened are silently ignored; this is
        designed so that you can specify a list of potential
        configuration file locations (e.g. current directory, user's
        home directory, systemwide directory), and all existing
        configuration files in the list will be read.  A single
        filename may also be given.

        Return list of successfully read files.
        """
        if isinstance(filenames, basestring):
            filenames = [filenames]
        read_ok = []
        for filename in filenames:
            try:
                fp = codecs.open(filename, encoding='utf-8')
            except IOError:
                continue
            self._read(fp, filename)
            fp.close()
            read_ok.append(filename)
        return read_ok
        
        
        
        