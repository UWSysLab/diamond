DEFAULT_THEME          = ' gtk-theme-name = "Default" '

STOCK_SEND_IM_FILE     = "send-im.png"
STOCK_SEND_IM          = _("Send Private Message")
STOCK_COPY_URL         = _('Copy Link Location')
STOCK_COPY_EMAIL       = _('Copy E-Mail Address')
STOCK_OPEN_URL         = _('Open Link in Browser')
STOCK_OFFLINE_MESSAGE  = _('Check Offline Messages')
STOCK_DEFINE           = _('Define')
STOCK_RESET_DEFAULT    = _('Reset to Default')
STOCK_SEND_MOVE        = _('Send Move')
STOCK_PASS             = _('Pass Move')
STOCK_TRADE_LETTERS    = _('Trade Letters')
STOCK_SHUFFLE          = _('Shuffle')
STOCK_REGISTER         = _("Register")
STOCK_ADD_HOSTNAME     = _('Add Host')

# TextTag Constants
TAG_TYPE     = "tag_type"
LINK_TAG     = "link_tag"
EMAIL_TAG    = "email_tag"
LINK_DATA    = "link_data"

MENU_DATA = """
    <menubar name="MainMenuBar">
        <menu action="File">
            <menuitem action="Exit"/>
        </menu>
        <menu action="Options">
            <menuitem action="Preferences"/>
        </menu>
        <menu action="Server">
            <menuitem action="Create Game"/>
            <separator/>
        </menu>
        <menu action="View">
            <menuitem action="Full Screen"/>
        </menu>
        <menu action="Help">
            <menuitem action="Online Help"/>
            <separator/>
            <menuitem action="About"/>
        </menu>
    </menubar>
"""

LOGIN_MENU_DATA = """
    <menubar name="MainMenuBar">
        <menu action="File">
            <menuitem action="Exit"/>
        </menu>
        <menu action="Options">
            <menuitem action="Preferences"/>
        </menu>
        <menu action="Help">
            <menuitem action="Online Help"/>
            <separator/>
            <menuitem action="About"/>
        </menu>
    </menubar>
"""        