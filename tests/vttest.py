import moo

DEFAULT_SPEED = 9600
LOG_ENABLED = False

BEL = '\007'
SO  = '\016'
SI  = '\017'
ESC = '\033'
CSI = '\233'
SS3 = '\217'
DCS = '\220'
ST  = '\234'

GATM    =  1    ## guarded area transfer (disabled)
KAM     =  2    ## keyboard action
CRM     =  3    ## control representation (setup)
IRM     =  4    ## insert/replace
SRTM    =  5    ## status reporting transfer (disabled)
ERM     =  6    ## erasure mode (non-DEC)
VEM     =  7    ## vertical editing (disabled)
BDSM    =  8    ## bi-directional support mode (non-DEC)
DCSM    =  9    ## device component select mode (non-DEC)
HEM     = 10    ## horizontal editing (disabled)
PUM     = 11    ## positioning unit (disabled)
SRM     = 12    ## send/receive
FEAM    = 13    ## format effector action (disabled)
FETM    = 14    ## format effector transfer (disabled)
MATM    = 15    ## multiple area transfer (disabled)
TTM     = 16    ## transfer termination (disabled)
SATM    = 17    ## selected area transfer (disabled)
TSM     = 18    ## tabulation stop (disabled)
EBM     = 19    ## editing boundary (disabled)
LNM     = 20    ## line feed/new line


DECCKM     = 1    ## cursor keys
DECANM     = 2    ## ANSI
DECCOLM    = 3    ## column
DECSCLM    = 4    ## scrolling
DECSCNM    = 5    ## screen
DECOM      = 6    ## origin
DECAWM     = 7    ## autowrap
DECARM     = 8    ## autorepeat
DECEDM    = 10    ## edit
DECLTM    = 11    ## line transmit
DECSCFDM  = 13    ## space compression field delimiter
DECTEM    = 14    ## transmission execution
DECEKEM   = 16    ## edit key execution
DECPFF    = 18    ## print form feed
DECPEX    = 19    ## printer extent
DECTCEM   = 25    ## text cursor enable
DECRLM    = 34    ## left-to-right
DECTEK    = 35    ## 4010/4014 emulation
DECHEM    = 36    ## Hebrew encoding
DECNRCM   = 42    ## national replacement character set
DECGEPM   = 43    ## graphics expanded print
DECGPCM   = 44    ## graphics print color
DECGPCS   = 45    ## graphics print color syntax
DECGPBM   = 46    ## graphics print background
DECGRPM   = 47    ## graphics rotated print
DEC131TM  = 53    ## VT131 transmit
DECNAKB   = 57    ## Greek/N-A Keyboard Mapping
DECHCCM   = 60    ## horizontal cursor coupling (disabled)
DECVCCM   = 61    ## vertical cursor coupling
DECPCCM   = 64    ## page cursor coupling
DECNKM    = 66    ## numeric keypad
DECBKM    = 67    ## backarrow key
DECKBUM   = 68    ## keyboard usage
DECVSSM   = 69    ## vertical split
DECXRLM   = 73    ## transmit rate linking
DECKPM    = 81    ## keyboard positioning
DECNCSM   = 95    ## no clearing screen on column change
DECRLCM   = 96    ## right-to-left copy
DECCRTSM  = 97    ## CRT save
DECARSM   = 98    ## auto resize
DECMCM    = 99    ## modem control
DECAAM    = 100   ## auto answerback
DECCANSM  = 101   ## conceal answerback
DECNULM   = 102   ## null
DECHDPXM  = 103   ## half duplex
DECESKM   = 104   ## enable secondary keyboard language
DECOSCNM  = 106   ## overscan
DECFWM    = 111   ## framed windows
DECRPL    = 112   ## review previous lines
DECHWUM   = 113   ## host wake-up mode (CRT and energy saver)
DECATCUM  = 114   ## alternate text color underline
DECATCBM  = 115   ## alternate text color blink
DECBBSM   = 116   ## bold and blink style
DECECM    = 117   ## erase color


soft_scroll = 0

#/******************************************************************************/

csi_7 = ESC + '['
csi_8 = chr(0x9b)
dcs_7 = ESC + 'P'
dcs_8 = chr(0x90)
osc_7 = ESC + ']'
osc_8 = chr(0x9d)
ss3_7 = ESC + 'O'
ss3_8 = chr(0x8f)
st_7  = ESC + '\\'
st_8  = chr(0x9c)


class VtTest(moo.term.Term):
    def __init__(self):
        moo.term.Term.__init__(self)

        self.brkrd = 0
        self.input_8bits = False
        self.log_disabled = False
        self.max_cols = 132
        self.max_lines = 24
        self.min_cols = 80
        self.output_8bits = False
        self.reading = 0
        self.tty_speed = DEFAULT_SPEED
        self.use_padding = False

    def csi_input(self):
        if self.input_8bits: return csi_8
        else: return csi_7
    def csi_output(self):
        if self.output_8bits: return csi_8
        else: return csi_7
    def dcs_input(self):
        if self.input_8bits: return dcs_8
        else: return dcs_7
    def dcs_output(self):
        if self.output_8bits: return dcs_8
        else: return dcs_7
    def osc_input(self):
        if self.input_8bits: return osc_8
        else: return osc_7
    def osc_output(self):
        if self.output_8bits: return osc_8
        else: return osc_7
    def ss3_input(self):
        if self.input_8bits: return ss3_8
        else: return ss3_7
    def ss3_output(self):
        if self.output_8bits: return ss3_8
        else: return ss3_7
    def st_input(self):
        if self.input_8bits: return st_8
        else: return st_7
    def st_output(self):
        if self.output_8bits: return st_8
        else: return st_7

    def padding(self, msecs):
        if self.use_padding:
            count = (3 * msecs * tty_speed + DEFAULT_SPEED - 1) / DEFAULT_SPEED
            while count > 0:
                count -= 1
                self.feed('\0')

    def extra_padding(self, msecs):
        if self.use_padding:
            if self.soft_scroll:
                self.padding(msecs * 4)
            else:
                self.padding(msecs)

    def println(self, s):
        self.feed(s + "\r\n")
        return 1

    def do_csi(self, fmt, *args):
        self.feed(self.csi_output())
        self.feed(fmt % args)

    def do_dcs(self, fmt, *args):
        self.feed(self.dcs_output())
        self.feed(fmt % args)

    def do_osc(self, fmt, *args):
        self.feed(self.osc_output())
        self.feed(fmt % args)

    def send_char(self, c):
        self.feed(chr(c))

    def esc(self, s):
        self.feed(ESC + s)

    def brc(self, pn, c):
        self.do_csi("%d%s", pn, c)

    def brc2(self, pn1, pn2, c):
        self.do_csi("%d;%d%s", pn1, pn2, c)

    def brc3(self, pn1, pn2, pn3, c):
        self.do_csi("%d;%d;%d%s", pn1, pn2, pn3, c)

    def cbt(self, pn):
        self.brc(pn, 'Z')

    ## Cursor Character Absolute
    def cha(self, pn):
        self.brc(pn, 'G')

    ## Cursor Forward Tabulation
    def cht(self, pn):
        self.brc(pn, 'I')

    # Cursor Next Line
    def cnl(self, pn):
        self.brc(pn, 'E')

    # Cursor Previous Line
    def cpl(self, pn):
        brc(pn, 'F')

    # Cursor Backward
    def cub(self, pn):
        self.brc(pn, 'D')
        self.padding(2)

    # Cursor Down
    def cud(self, pn):
        self.brc(pn, 'B')
        self.extra_padding(2)

    # Cursor Forward
    def cuf(self, pn):
        self.brc(pn, 'C')
        self.padding(2)

    def cup(self, pn1, pn2):           # Cursor Position
      self.brc2(pn1, pn2, 'H')
      self.padding(5)   # 10 for vt220
      return 1     # used for indenting

    def cuu(self, pn):                     # Cursor Up
      self.brc(pn, 'A')
      self.extra_padding(2)

    def da(self):                        # Device Attributes
      self.brc(0, 'c')

    def decaln(self):                    # Screen Alignment Display
      self.esc("#8")

    def decarm(self, flag):                # DECARM autorepeat
      if flag:
        self.sm("?8")   # autorepeat
      else:
        self.rm("?8")   # no autorepeat

    def decawm(self, flag):                # DECAWM autowrap
      if flag:
        self.sm("?7")   # autowrap
      else:
        self.rm("?7")   # no autowrap

    def decbi(self):                     # VT400: Back Index
      self.esc("6")
      self.padding(40)

    def decbkm(self, flag):                # VT400: Backarrow key
      if flag:
        self.sm("?67")  # backspace
      else:
        self.rm("?67")  # delete

    def deccara(self, top, left, bottom, right, attr):
      self.do_csi("%d%d%d%d%d$r", top, left, bottom, right, attr)

    def decckm(self, flag):                # DECCKM Cursor Keys
      if flag:
        self.sm("?1")   # application
      else:
        self.rm("?1")   # normal

    def deccolm(self, flag):               # DECCOLM 80/132 Columns
      if flag:
        self.sm("?3")   # 132 columns
      else:
        self.rm("?3")   # 80 columns

    def deccra(self, Pts, Pl, Pbs, Prs, Pps, Ptd, Pld, Ppd):
      self.do_csi("%d%d%d%d%d%d%d%d$v",
             Pts,   # top-line border
             Pl,    # left-line border
             Pbs,   # bottom-line border
             Prs,   # right-line border
             Pps,   # source page number
             Ptd,   # destination top-line border
             Pld,   # destination left-line border
             Ppd)  # destination page number

    def decdc(self, pn):                   # VT400 Delete Column
      self.do_csi("%d'~", pn)
      self.padding(10 * pn)
      return 1

    def decefr(self, top, left, bottom, right):  # DECterm Enable filter rectangle
      self.do_csi("%d%d%d%d'w", top, left, bottom, right)

    def decelr(self, all_or_one, pixels_or_cells):   # DECterm Enable Locator Reporting
      self.do_csi("%d%d'z", all_or_one, pixels_or_cells)

    def decera(self, top, left, bottom, right):  # VT400 Erase Rectangular area
      self.do_csi("%d%d%d%d$z", top, left, bottom, right)

    def decdhl(self, lower):               # Double Height Line (also double width)
      if lower:
        self.esc("#4")
      else:
        self.esc("#3")

    def decdwl(self):                    # Double Wide Line
      self.esc("#6")

    def decfi(self):                     # VT400: Forward Index
      self.esc("9")
      self.padding(40)

    def decfra(self, c, top, left, bottom, right):   # VT400 Fill Rectangular area
      self.do_csi("%d%d%d%d%d$x", c, top, left, bottom, right)

    def decid(self):                     # required for VT52, not recommended above VT100
      self.esc("Z")     # Identify

    def decic(self, pn):                   # VT400 Insert Column
      self.do_csi("%d'}", pn)
      self.padding(10 * pn)
      return 1

    def deckbum(self, flag):               # VT400: Keyboard Usage
      if flag:
        self.sm("?68")  # data processing
      else:
        self.rm("?68")  # typewriter

    def deckpam(self):                   # Keypad Application Mode
      self.esc("=")

    def deckpm(self, flag):                # VT400: Keyboard Position
      if flag:
        self.sm("?81")  # position reports
      else:
        self.rm("?81")  # character codes

    def deckpnm(self):                   # Keypad Numeric Mode
      self.esc(">")

    def decll(self, ps):                 # Load LEDs
      self.do_csi("%sq", ps)

    def decnkm(self, flag):                # VT400: Numeric Keypad
      if flag:
        self.sm("?66")  # application
      else:
        self.rm("?66")  # numeric

    def decom(self, flag):                 # DECOM Origin
      if flag:
        self.sm("?6")   # relative
      else:
        self.rm("?6")   # absolute

    def decpex(self, flag):                # VT220: printer extent mode
      if flag:
        self.sm("?19")  # full screen (page)
      else:
        self.rm("?19")  # scrolling region

    def decpff(self, flag):                # VT220: print form feed mode
      if flag:
        self.sm("?18")  # form feed
      else:
        self.rm("?18")  # no form feed

    def decnrcm(self, flag):               # VT220: National replacement character set
      if flag:
        self.sm("?42")  # national
      else:
        self.rm("?42")  # multinational

    def decrara(self, top, left, bottom, right, attr):
      self.do_csi("%d%d%d%d%d$t", top, left, bottom, right, attr)

    def decrc(self):                     # Restore Cursor
      self.esc("8")

    def decreqtparm(self, pn):             # Request Terminal Parameters
      self.brc(pn, 'x')

    def decrqlp(self, mode):               # DECterm Request Locator Position
      self.do_csi("%d'|", mode)

    def decrqss(self, pn):               # VT200 Request Status-String
      self.do_dcs("$q%s", pn)

    def decsace(self, flag):               # VT400 Select attribute change extent
      if flag:
        self.do_csi("%d*x", 2)
      else:
        self.do_csi("%d*x", 0)

    def decsasd(self, pn):                 # VT200 Select active status display
      self.do_csi("%d$}", pn)

    def decsc(self):                     # Save Cursor
      self.esc("7")

    def decsca(self, pn1):                 # VT200 select character attribute (protect)
      self.do_csi("%d\"q", pn1)

    def decsclm(self, flag):               # Scrolling mode (smooth/jump)
      if flag:
        self.sm("?4")   # smooth scrolling
      else:
        self.rm("?4")   # jump-scrolling scrolling
      self.soft_scroll = flag

    def decscnm(self, flag):               # Screen mode (inverse-video)
      if flag:
        self.sm("?5")   # inverse video
      else:
        self.rm("?5")   # normal video
      self.padding(200)

    def decsed(self, pn1):                 # VT200 selective erase in display
      self.do_csi("?%dJ", pn1)

    def decsel(self, pn1):                 # VT200 selective erase in line
      self.do_csi("?%dK", pn1)

    def decsera(self, top, left, bottom, right):   # VT400 Selective erase rectangular area
      self.do_csi("%d%d%d%d${", top, left, bottom, right)

    def decsle(self, mode):                # DECterm Select Locator Events
      self.do_csi("%d'{", mode)

    def decsnls(self, pn):                 # VT400 Select number of lines per screen
      self.do_csi("%d*|", pn)

    def decssdt(self, pn):                 # VT200 Select status line type
      self.do_csi("%d$~", pn)

    def decstbm(self, pn1, pn2):       # Set Top and Bottom Margins
      if pn1 or pn2:
        self.brc2(pn1, pn2, 'r')
      else:
        self.esc("[r")
      # Good for >24-line terminals

    def decstr(self):                    # VT200 Soft terminal reset
      self.do_csi("!p")

    def decswl(self):                    # Single Width Line
      self.esc("#5")

    def dectst(self, pn):                  # Invoke Confidence Test
      self.brc2(2, pn, 'y')

    def dsr(self, pn):                     # Device Status Report
      self.brc(pn, 'n')

    def ed(self, pn):                      # Erase in Display
      self.brc(pn, 'J')
      self.padding(50)

    def el(self, pn):                      # Erase in Line
      self.brc(pn, 'K')
      self.padding(3)   # 4 for vt400

    def ech(self, pn):                     # Erase character(s)
      self.brc(pn, 'X')

    def hpa(self, pn):                     # Character Position Absolute
      self.brc(pn, '`')

    def hts(self):                       # Horizontal Tabulation Set
      self.esc("H")

    def hvp(self, pn1, pn2):           # Horizontal and Vertical Position
      self.brc2(pn1, pn2, 'f')

    def ind(self):                       # Index
      self.esc("D")
      self.padding(20)  # vt220

    # The functions beginning "mc_" are variations of Media Copy (MC)

    def mc_autoprint(self, flag):          # VT220: auto print mode
      if flag:
        self.do_csi("?%di", 5)
      else:
        self.do_csi("?%di", 4)

    def mc_printer_controller(self, flag): # VT220: printer controller mode
      if flag:
        self.do_csi("%di", 5)
      else:
        self.do_csi("%di", 4)

    def mc_print_page(self):             # VT220: print page
      self.do_csi("i")

    def mc_print_composed(self):         # VT300: print composed main display
      self.do_csi("?10i")

    def mc_print_all_pages(self):        # VT300: print composed all pages
      self.do_csi("?11i")

    def mc_print_cursor_line(self):      # VT220: print cursor line
      self.do_csi("?1i")

    def mc_printer_start(self, flag):      # VT300: start/stop printer-to-host session
      if flag:
        self.do_csi("?%di", 9)
      else:
        self.do_csi("?%di", 8)

    def mc_printer_assign(self, flag):     # VT300: assign/release printer to active session
      if flag:
        self.do_csi("?%di", 18)
      else:
        self.do_csi("?%di", 19)

    def nel(self):                       # Next Line
      self.esc("E")

    def rep(self, pn):                     # Repeat
      self.do_csi("%db", pn)

    def ri(self):                        # Reverse Index
      self.esc("M")
      self.extra_padding(5)   # 14 on vt220

    def ris(self):                       #  Reset to Initial State
      self.esc("c")

    def rm(self, ps):                    # Reset Mode
      self.do_csi("%sl", ps)

    def s8c1t(self, flag):                 # Tell terminal to respond with 7-bit or 8-bit controls
      self.input_8bits = flag
      if flag:
        self.esc(" G")  # select 8-bit controls
      else:
        self.esc(" F")  # select 7-bit controls
      self.zleep(300)

    # /*
    #  * If g is zero,
    #  *    designate G0 as character set c
    #  *    designate G1 as character set B (ASCII)
    #  *    shift-in (select G0 into GL).
    #  * If g is nonzero
    #  *    designate G0 as character set B (ASCII)
    #  *    designate G1 as character set c
    #  *    shift-out (select G1 into GL).
    #  * See also scs_normal() and scs_graphics().
    #  */
    def scs(self, g, c):               # Select character Set
      if g:
        self.esc("%c%c" % (')', c))
      else:
        self.esc("%c%c" % ('(', c))

      if g:
        self.esc("%c%c" % (')', 'B'))
      else:
        self.esc("%c%c" % ('(', 'B'))

      if g: self.feed(SO)
      else: self.feed(SI)

      self.padding(4)

    def sd(self, pn):                      # Scroll Down
      self.brc(pn, 'T')

    def sgr(self, ps):                   # Select Graphic Rendition
      self.do_csi("%sm", ps)
      self.padding(2)

    def sl(self, pn):                      # Scroll Left
      self.do_csi("%d @", pn)

    def sm(self, ps):                    # Set Mode
      self.do_csi("%sh", ps)

    def sr(self, pn):                      # Scroll Right
      self.do_csi("%d A", pn)

    def srm(self, flag):                   # VT400: Send/Receive mode
      if flag:
        self.sm("12")   # local echo off
      else:
        self.rm("12")   # local echo on

    def su(self, pn):                      # Scroll Up
      self.brc(pn, 'S')
      self.extra_padding(5)

    def tbc(self, pn):                     # Tabulation Clear
      self.brc(pn, 'g')

    def dch(self, pn):                     # Delete character
      self.brc(pn, 'P')

    def ich(self, pn):                     # Insert character -- not in VT102
      self.brc(pn, '@')

    def dl(self, pn):                      # Delete line
      self.brc(pn, 'M')

    def il(self, pn):                      # Insert line
      self.brc(pn, 'L')

    def vpa(self, pn):                     # Line Position Absolute
      self.brc(pn, 'd')

    def vt52cub1(self):                  # cursor left
      self.esc("D")
      self.padding(5)

    def vt52cud1(self):                  # cursor down
      self.esc("B")
      self.padding(5)

    def vt52cuf1(self):                  # cursor right
      self.esc("C")
      self.padding(5)

    def vt52cup(self, l, c):           # direct cursor address
      self.esc("Y%c%c" % (l + 31, c + 31))
      self.padding(5)

    def vt52cuu1(self):                  # cursor up
      self.esc("A")
      self.padding(5)

    def vt52ed(self):                    # erase to end of screen
      self.esc("J")
      self.padding(5)

    def vt52el(self):                    # erase to end of line
      self.esc("K")
      self.padding(5)

    def vt52home(self):                  # cursor to home
        self.esc("H")
        self.padding(5)

    def vt52ri(self):                    # reverse line feed
        self.esc("I")
        self.padding(5)

    def tst_movements(self, N=26):
        ctext = "This is a correct sentence";

        self.deccolm(False)
        width = 80

        # Compute left/right columns for a 60-column box centered in 'width'
        inner_l = (width - 60) / 2;
        inner_r = 61 + inner_l;
        hlfxtra = (width - 80) / 2;

        self.decaln();
        self.cup( 9,inner_l); self.ed(1);
        self.cup(18,60+hlfxtra); self.ed(0); self.el(1);
        self.cup( 9,inner_r); self.el(0);
        # 132: 36..97 */
        #  80: 10..71 */
        for row in range(10,17):
            self.cup(row, inner_l); self.el(1);
            self.cup(row, inner_r); self.el(0);

        self.cup(17,30); self.el(2);
        for col in range(1,width+1):
            self.hvp(self.max_lines, col); self.feed("*");
            self.hvp( 1, col); self.feed("*");

        self.cup(2,2);
        for row in range(2,self.max_lines):
            self.feed("+");
            self.cub(1);
            self.ind();

        self.cup(self.max_lines-1,width-1);
        l = range(2, self.max_lines)
        l.reverse
        for row in l:
            self.feed("+");
            self.cub(1); self.ri();

        self.cup(2,1);
        for row in range(2, self.max_lines):
            self.feed("*"); self.cup(row, width);
            self.feed("*");
            self.cub(10);
            if (row < 10): self.nel();
            else:          self.feed("\n");

        self.cup(2,10);
        self.cub(42+hlfxtra); self.cuf(2);
        for col in range(3, width-1):
            self.feed("+");
            self.cuf(0); self.cub(2); self.cuf(1);

        self.cup(self.max_lines-1,inner_r-1);
        self.cuf(42+hlfxtra); self.cub(2);

        l = range(3, width-1)
        l.reverse()
        for col in l:
            self.feed("+");
            self.cub(1); self.cuf(1); self.cub(0); self.feed("%c" % (8,));

        self.cup( 1, 1); self.cuu(10); self.cuu(1); self.cuu(0);
        self.cup(self.max_lines,width); self.cud(10); self.cud(1); self.cud(0);

        self.cup(10,2+inner_l);

        for row in range(10, 16):
            for col in range(2+inner_l, inner_r-1): self.feed(" ");
            self.cud(1); self.cub(58);

        self.cuu(5); self.cuf(1);
        self.feed("The screen should be cleared,  and have an unbroken bor-");
        self.cup(12,inner_l+3);
        self.feed("der of *'s and +'s around the edge,   and exactly in the");
        self.cup(13,inner_l+3);
        self.feed("middle  there should be a frame of E's around this  text");
        self.cup(14,inner_l+3);
        self.feed("with  one (1) free position around it.    ");

        for i in range(1):
            on_left = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
            on_right = "abcdefghijklmnopqrstuvwxyz";
            height = len(on_left) - 1;
            region = self.max_lines - 6;

            height = 10;

            self.deccolm(False); width = self.min_cols

            self.println("Test of autowrap, mixing control and print characters.");
            self.println("The left/right margins should have letters in order:");

            self.decstbm(3, region + 3);
            self.decom(True)

            for i in range(N):
                if i % 4 == 0:
                    self.cup(region + 1, 1)
                    self.feed(on_left[i]);
                    self.cup(region + 1, width)
                    self.feed(on_right[i]);
                    self.feed("\n");
                elif i % 4 == 1:
                    self.cup(region, width)
                    self.feed(on_right[i - 1] + on_left[i])
                    self.cup(region + 1, width)
                    self.feed(on_left[i] + "\010 " + on_right[i]);
                    self.feed("\n");
                elif i % 4 == 2:
                    self.cup(region + 1, width)
                    self.feed(on_left[i] + "\010\010\011\011" + on_right[i]);
                    self.cup(region + 1, 2)
                    self.feed("\010" + on_left[i] + "\n");
                else:
                    self.cup(region + 1, width)
                    self.feed("\n");
                    self.cup(region, 1)
                    self.feed(on_left[i]);
                    self.cup(region, width)
                    self.feed(on_right[i]);

#             self.decom(False);
#             self.decstbm(0, 0);
#             self.cup(self.max_lines - 2, 1);

#         self.deccolm(False)

#         vt_clear(2);
#         vt_move(1,1);
#         println("Test of cursor-control characters inside ESC sequences.");
#         println("Below should be four identical lines:");
#         println("");
#         println("A B C D E F G H I");
#         for (i = 1; i < 10; i++) {
#             printf("%c", '@' + i);
#             do_csi("2\010C");   /* Two forward, one backspace */
#         }
#         println("");
#         /* Now put CR in CUF sequence. */
#         printf("A ");
#         for (i = 2; i < 10; i++)
#             printf("%s\015%dC%c", csi_output(), 2 * i - 2, '@' + i);
#         println("");
#         /* Now put VT in CUU sequence. */
#         rm("20");
#         for (i = 1; i < 10; i++) {
#             printf("%c ", '@' + i);
#             do_csi("1\013A");
#         }
#         println("");
#         println("");
#         holdit();
#
#         if (LOG_ENABLED)
#             fprintf(log_fp, "tst_movements leading zeros in ESC sequences\n");
#
#         vt_clear(2);
#         vt_move(1,1);
#         println("Test of leading zeros in ESC sequences.");
#         printf("Two lines below you should see the sentence \"%s\".",ctext);
#         for (col = 1; *ctext; col++)
#         printf("%s00000000004;00000000%dH%c", csi_output(), col, *ctext++);
#         cup(20,1);
#
#         restore_ttymodes();
#         return MENU_HOLD;
#         }
