"""

Custom serializer class to replace pickle.  This does not use eval() when (de)serializing objects and thus 
is safe from arbitrary code execution (hopefully)

Adapted from:
    
    http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/415791 
"""

from cStringIO import StringIO
from struct import pack, unpack
from types import IntType, TupleType, StringType, FloatType, LongType, ListType, DictType, NoneType, BooleanType, UnicodeType
from pyscrabble.game.player import User, Player, PlayerInfo
from pyscrabble.game.pieces import Letter, Move
from pyscrabble.game.game import ScrabbleGameInfo
from pyscrabble.lookup import ServerMessage
from pyscrabble.util import Time, TimeDeltaWrapper, ServerBulletin, PrivateMessage, RingList
from pyscrabble.command import helper
from time import struct_time
from datetime import timedelta
import zlib

class EncodeError(Exception): pass
class DecodeError(Exception): pass
class ProtocolError(Exception): pass

HEADER = "SRW3"

protocol = {
    ServerBulletin     : "a",
    BooleanType        : "b",
    PlayerInfo         : "c",
    timedelta          : "d",
    Time               : "e",
    TimeDeltaWrapper   : "f",
    ScrabbleGameInfo   : "g",
    RingList           : "i",
    helper.Command     : "j",
    Letter             : "l",
    Move               : "m",
    Player             : "p",
    PrivateMessage     : "r",
    ServerMessage      : "s",
    struct_time        : "t",
    User               : "u",
    LongType           : "B",
    DictType           : "D",
    FloatType          : "F",
    IntType            : "I",
    ListType           : "L",
    NoneType           : "N",
    StringType         : "S",
    TupleType          : "T",
    UnicodeType        : "U"
}

def x_for(type, data, error):
    result = None
    if type in data:
        result = data[type]
    else:
        for d in data:
            if issubclass(type, d):
                result = data[d]
                break
    if result is not None:
        return result
    else:
        raise error, "%s not found in %s" % (str(type), str(data))

def protocol_for(objType):
    return x_for(objType, protocol, ProtocolError)

def encoder_for(objType):
    return x_for(objType, encoder, EncodeError)

def decoder_for(objType):
    return x_for(objType, decoder, DecodeError)

encoder = {}
class register_encoder_for_type(object):
    """Registers an encoder function, for a type, in the global encoder dictionary."""
    def __init__(self, t):
        self.type = t
    def __call__(self, func):
        encoder[self.type] = func
        return func

#contains dictionary of decoding functions, where the dictionary key is the type prefix used.
decoder = {}
class register_decoder_for_type(object):
    """Registers a decoder function, for a prefix, in the global decoder dictionary."""
    def __init__(self, t):
        self.prefix = protocol_for(t)
    def __call__(self, func):
        decoder[self.prefix] = func
        return func

## <encoding functions> ##
@register_encoder_for_type(DictType)
def enc_dict_type(obj):
    data = "".join([encoder_for(type(i))(i) for i in obj.items()])
    return "%s%s%s" % ("D", pack("!L", len(data)), data)

@register_encoder_for_type(struct_time)
def enc_struct_time_type(obj):
    data = "".join([encoder_for(type(i))(i) for i in obj])
    return "%s%s%s" % (protocol[struct_time], pack("!L", len(data)), data)
    
@register_encoder_for_type(TupleType)
@register_encoder_for_type(ListType)
def enc_list_type(obj):
    data = "".join([encoder_for(type(i))(i) for i in obj])
    return "%s%s%s" % (protocol_for(type(obj)), pack("!L", len(data)), data)

@register_encoder_for_type(IntType)
def enc_int_type(obj):
    return "%s%s" % (protocol[IntType], pack("!i", obj))

@register_encoder_for_type(FloatType)
def enc_float_type(obj):
    return "%s%s" % (protocol[FloatType], pack("!d", obj))

@register_encoder_for_type(LongType)
def enc_long_type(obj):
    obj = hex(obj)[2:-1]
    return "%s%s%s" % (protocol[LongType], pack("!L", len(obj)), obj)

@register_encoder_for_type(UnicodeType)
def enc_unicode_type(obj):
    obj = obj.encode('utf-8')
    return "%s%s%s" % (protocol[UnicodeType], pack("!L", len(obj)), obj)


@register_encoder_for_type(StringType)
def enc_string_type(obj):
    return "%s%s%s" % (protocol[StringType], pack("!L", len(obj)), obj)

@register_encoder_for_type(NoneType)
def enc_none_type(obj):
    return protocol[NoneType]

@register_encoder_for_type(BooleanType)
def enc_bool_type(obj):
    return protocol[BooleanType] + str(int(obj))

@register_encoder_for_type(User)
@register_encoder_for_type(Player)
@register_encoder_for_type(Move)
@register_encoder_for_type(Letter)
@register_encoder_for_type(ScrabbleGameInfo)
@register_encoder_for_type(ServerMessage)
@register_encoder_for_type(PrivateMessage)
@register_encoder_for_type(ServerBulletin)
@register_encoder_for_type(PlayerInfo)
@register_encoder_for_type(Time)
@register_encoder_for_type(TimeDeltaWrapper)
@register_encoder_for_type(RingList)
@register_encoder_for_type(helper.Command)
def enc_obj_type(obj):
    data = "".join([encoder_for(type(i))(i) for i in obj.__dict__.items()])
    return "%s%s%s" % (protocol_for(type(obj)), pack("!L", len(data)), data)

@register_encoder_for_type(timedelta)
def enc_time_delta_type(obj):
    data = (obj.days, obj.seconds)
    data = "".join([encoder_for(type(i))(i) for i in data])
    return "%s%s%s" % (protocol_for(type(obj)), pack("!L", len(data)), data)
    

def dumps(obj, compress=False):
    """Encode simple Python types into a binary string."""
    option = "N"
    if compress: option = "Z"
    
    data = encoder_for(type(obj))(obj)
        
    if compress: 
        data = zlib.compress(data)
    x = "%s%s%s" % (HEADER, option, data)
    return x
    
        
## </encoding functions> ##

## <decoding functions> ##
def build_sequence(data, cast=list):
    size = unpack('!L', data.read(4))[0]
    items = []
    data_tell = data.tell
    data_read = data.read
    items_append = items.append
    start_position = data.tell()
    while (data_tell() - start_position) < size:
        T = data_read(1)
        value = decoder_for(T)(data)
        items_append(value)
    return cast(items)

@register_decoder_for_type(struct_time)
@register_decoder_for_type(TupleType)
def dec_tuple_type(data):
    return build_sequence(data, cast=tuple)

@register_decoder_for_type(ListType)
def dec_list_type(data):
    return build_sequence(data, cast=list)

@register_decoder_for_type(DictType)
def dec_dict_type(data):
    return build_sequence(data, cast=dict)

@register_decoder_for_type(LongType)
def dec_long_type(data):
    size = unpack('!L', data.read(4))[0]
    value = long(data.read(size),16)
    return value

@register_decoder_for_type(StringType)
def dec_string_type(data):
    size = unpack('!L', data.read(4))[0]
    value = str(data.read(size))
    return value

@register_decoder_for_type(FloatType)
def dec_float_type(data):
    value = unpack('!d', data.read(8))[0]
    return value

@register_decoder_for_type(IntType)
def dec_int_type(data):
    value = unpack('!i', data.read(4))[0]
    return value

@register_decoder_for_type(NoneType)
def dec_none_type(data):
    return None

@register_decoder_for_type(BooleanType)
def dec_bool_type(data):
    value = int(data.read(1))
    return bool(value)

@register_decoder_for_type(UnicodeType)
def dec_unicode_type(data):
    size = unpack('!L', data.read(4))[0]
    value = data.read(size).decode('utf-8')
    return value

@register_decoder_for_type(User)
def dec_user_type(data):
    u = User()
    u.__dict__ = dec_dict_type(data)
    return u

@register_decoder_for_type(struct_time)
def dec_struct_time_type(data):
    return build_sequence(data, cast=tuple)

@register_decoder_for_type(Move)
def dec_move_type(data):
    obj = Move()
    obj.__dict__ = dec_dict_type(data)
    return obj

@register_decoder_for_type(Letter)
def dec_letter_type(data):
    obj = Letter()
    obj.__dict__ = dec_dict_type(data)
    return obj

@register_decoder_for_type(ScrabbleGameInfo)
def dec_sg_type(data):
    obj = ScrabbleGameInfo()
    obj.__dict__ = dec_dict_type(data)
    return obj

@register_decoder_for_type(Player)
def dec_player_type(data):
    obj = Player()
    obj.__dict__ = dec_dict_type(data)
    return obj

@register_decoder_for_type(ServerMessage)
def dec_server_message_type(data):
    obj = ServerMessage()
    obj.__dict__ = dec_dict_type(data)
    return obj

@register_decoder_for_type(PrivateMessage)
def dec_private_message_type(data):
    obj = PrivateMessage()
    obj.__dict__ = dec_dict_type(data)
    return obj

@register_decoder_for_type(ServerBulletin)
def dec_bulletin_type(data):
    obj = ServerBulletin()
    obj.__dict__ = dec_dict_type(data)
    return obj

@register_decoder_for_type(PlayerInfo)
def dec_player_info_type(data):
    obj = PlayerInfo()
    obj.__dict__ = dec_dict_type(data)
    return obj

@register_decoder_for_type(timedelta)
def dec_time_delta_type(data):
    days,seconds = dec_tuple_type(data)
    obj = timedelta(days=days, seconds=seconds)
    return obj

@register_decoder_for_type(Time)
def dec_time_type(data):
    obj = Time()
    obj.__dict__ = dec_dict_type(data)
    return obj

@register_decoder_for_type(TimeDeltaWrapper)
def dec_time_delta_wrapper_type(data):
    obj = TimeDeltaWrapper()
    obj.__dict__ = dec_dict_type(data)
    return obj

@register_decoder_for_type(RingList)
def dec_ringlist_type(data):
    obj = RingList()
    obj.__dict__ = dec_dict_type(data)
    return obj

@register_decoder_for_type(helper.Command)
def dec_command_type(data):
    d = dec_dict_type(data)
    obj = helper.fromType( d["type"] )
    obj.__dict__ = d
    return obj


def loads(data):
    """
    Decode a binary string into the original Python types.
    """
    buffer = StringIO(data)
    header = buffer.read(len(HEADER))
    try:
        assert header == HEADER
    except AssertionError:
        print 'Header is',header
        raise
    option = buffer.read(1)
    if option == "Z":
        buffer = StringIO(zlib.decompress(buffer.read()))
    
    t = buffer.read(1)
    
    value = decoder_for(t)(buffer)
        
    return value
## </decoding functions> ##
