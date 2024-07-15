/* modifier 0 means no modifier */
static int surfuseragent    = 0;  /* Append Surf version to default WebKit user agent */
static char *fulluseragent  = ""; /* Or override the whole user agent string */
static char *scriptfile     = "~/.config/surf/script.js";
static char *styledir       = "~/.config/surf/styles/";
static char *certdir        = "~/.config/surf/certificates/";
static char *cachedir       = "/tmp/surf/cache/";
static char *cookiefile     = "~/.local/state/surf/cookies.txt";

/* enable to open GO prompt on startup */
static int startgo = 0;

/* Webkit default features */
/* Highest priority value will be used.
 * Default parameters are priority 0
 * Per-uri parameters are priority 1
 * Command parameters are priority 2
 */
static Parameter defconfig[ParameterLast] = {
    /* parameter                    Arg value       priority */
    [AccessMicrophone]    =       { { .i = 0 },     },
    [AccessWebcam]        =       { { .i = 0 },     },
    [Certificate]         =       { { .i = 1 },     },
    [CaretBrowsing]       =       { { .i = 0 },     },
    [CookiePolicies]      =       { { .v = "@Aa" }, },
    [DarkMode]            =       { { .i = 1 },     },
    [DefaultCharset]      =       { { .v = "UTF-8" }, },
    [DiskCache]           =       { { .i = 1 },     },
    [DNSPrefetch]         =       { { .i = 0 },     },
    [Ephemeral]           =       { { .i = 0 },     },
    [FileURLsCrossAccess] =       { { .i = 0 },     },
    [FontSize]            =       { { .i = 12 },    },
    [Geolocation]         =       { { .i = 0 },     },
    [HideBackground]      =       { { .i = 0 },     },
    [Inspector]           =       { { .i = 0 },     },
    [JavaScript]          =       { { .i = 1 },     },
    [KioskMode]           =       { { .i = 0 },     },
    [LoadImages]          =       { { .i = 1 },     },
    [MediaManualPlay]     =       { { .i = 1 },     },
    [PreferredLanguages]  =       { { .v = (char *[]){ NULL } }, },
    [RunInFullscreen]     =       { { .i = 0 },     },
    [ScrollBars]          =       { { .i = 1 },     },
    [ShowIndicators]      =       { { .i = 1 },     },
    [SiteQuirks]          =       { { .i = 1 },     },
    [SmoothScrolling]     =       { { .i = 1 },     },
    [SpellChecking]       =       { { .i = 0 },     },
    [SpellLanguages]      =       { { .v = ((char *[]){ "en_US", NULL }) }, },
    [StrictTLS]           =       { { .i = 1 },     },
    [Style]               =       { { .i = 1 },     },
    [WebGL]               =       { { .i = 0 },     },
    [ZoomLevel]           =       { { .f = 1.0 },   },
    [ClipboardNotPrimary] =       { { .i = 1 },     },
};

static UriParameters uriparams[] = {
    { "(://|\\.)suckless\\.org(/|$)", {
      [JavaScript] = { { .i = 0 }, 1 },
    }, },
};

/* default window size: width, height */
static int winsize[] = { 800, 600 };

static WebKitFindOptions findopts = WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE |
                                    WEBKIT_FIND_OPTIONS_WRAP_AROUND;

#define PROMPT_GO   "Go:"
#define PROMPT_BM   "Bookmark:"
#define PROMPT_FIND "Find:"

/* SETPROP(readprop, setprop, prompt)*/
#define SETPROP(r, s, p) { \
        .v = (const char *[]){ "/bin/sh", "-c", \
            "prop=\"$(printf '%b' \"$(xprop -id $1 "r" " \
            "| sed -e 's/^"r"(UTF8_STRING) = \"\\(.*\\)\"/\\1/' " \
            "      -e 's/\\\\\\(.\\)/\\1/g')\" " \
            "| dmenu -b -l 1 -p '"p"' -w $1)\" " \
            "&& xprop -id $1 -f "s" 8u -set "s" \"$prop\"", \
            "surf-setprop", winid, NULL \
        } \
}

/* DOWNLOAD(URI, referer) */
/* urxvtc.sh can be replaced by another terminal emulator like st
 */
#define DOWNLOAD(u, r) { \
        .v = (const char *[]){ "urxvtc.sh", "-e", "/bin/sh", "-c", \
            "aria2c --load-cookies \"$0\" --save-cookies \"$0\" " \
            "--referer \"$1\" --user-agent \"$2\" \"$3\"; " \
            "printf \"%s\" \"The command exited with status $?. \" && " \
            "printf \"%s\" \"Press ENTER to continue.\"; " \
            "read -r \"select\"", \
            cookiefile, r, useragent, u, NULL \
        } \
}

/* PLUMB(URI) */
/* This called when some URI which does not begin with "about:",
 * "http://" or "https://" should be opened.
 */
#define PLUMB(u) {\
        .v = (const char *[]){ "/bin/sh", "-c", \
            "xdg-open \"$0\"", u, NULL \
        } \
}

/* VIDEOPLAY(URI) */
#define VIDEOPLAY(u) {\
        .v = (const char *[]){ "/bin/sh", "-c", \
             "mpv --really-quiet \"$0\"", u, NULL \
        } \
}

/* BM_SET(readprop, setprop, prompt)*/
#define BM_SET(r, s, p) { \
        .v = (const char *[]){ "/bin/sh", "-c", \
            "prop=\"$(printf '%b' \"$(xprop -id $1 "r" " \
            "| sed -e 's/^"r"(UTF8_STRING) = \"\\(.*\\)\"/\\1/' " \
            "      -e 's/\\\\\\(.\\)/\\1/g' " \
            "&& cat ~/.config/surf/bookmarks)\" " \
            "| dmenu -b -l 10 -p '"p"' -w $1)\" " \
            "&& xprop -id $1 -f "s" 8u -set "s" \"$prop\"", \
            "surf-setprop", winid, NULL \
        } \
}

/* BM_ADD(readprop) */
#define BM_ADD(r) {\
        .v = (const char *[]){ "/bin/sh", "-c", \
            "(echo $(xprop -id $0 $1) | cut -d '\"' -f2 " \
            "| sed 's/.*https*:\\/\\/\\(www\\.\\)\\?//' " \
            "&& cat ~/.config/surf/bookmarks) " \
            "| awk '!seen[$0]++' > ~/.config/surf/bookmarks.tmp && " \
            "mv ~/.config/surf/bookmarks.tmp ~/.config/surf/bookmarks", \
            winid, r, NULL \
        } \
}

/* styles */
/*
 * The iteration will stop at the first match, beginning at the beginning of
 * the list.
 */
static SiteSpecific styles[] = {
    /* regexp               file in $styledir */
    { ".*",                 "default.css" },
};

/* certificates */
/*
 * Provide custom certificate for urls
 */
static SiteSpecific certs[] = {
    /* regexp               file in $certdir */
    /*{ "://suckless\\.org/", "suckless.org.crt" }, */
    { "://m625q/", "fritz.box.crt" },
};

/* search engines */
static SearchEngine searchengines[] = {
       { "e", "https://www.ecosia.org/search?q=%s" },
       { "d", "https://duckduckgo.com/?q=%s"       },
       { "g", "https://www.google.com/search?q=%s" },
};

#define MODKEY GDK_CONTROL_MASK

/* hotkeys */
/*
 * If you use anything else but MODKEY and GDK_SHIFT_MASK, don't forget to
 * edit the CLEANMASK() macro.
 */
static Key keys[] = {
    /* modifier              keyval          function    arg */
    { 0,                     GDK_KEY_g,      spawn,      SETPROP("_SURF_URI", "_SURF_GO", PROMPT_GO) },
    { 0,                     GDK_KEY_f,      spawn,      SETPROP("_SURF_FIND", "_SURF_FIND", PROMPT_FIND) },
    { 0,                     GDK_KEY_slash,  spawn,      SETPROP("_SURF_FIND", "_SURF_FIND", PROMPT_FIND) },
    { 0,                     GDK_KEY_b,      spawn,      BM_SET("_SURF_URI", "_SURF_GO", PROMPT_BM) },
    { 0,                     GDK_KEY_m,      spawn,      BM_ADD("_SURF_URI") },

    { 0,                     GDK_KEY_i,      insert,     { .i = 1 } },
    { 0,                     GDK_KEY_Escape, insert,     { .i = 0 } },

    { 0,                     GDK_KEY_c,      stop,       { 0 } },

    { 0,                     GDK_KEY_q,      quit,       { 0 } },

    { MODKEY,                GDK_KEY_r,      reload,     { .i = 1 } },
    { 0,                     GDK_KEY_r,      reload,     { .i = 0 } },

    { 0,                     GDK_KEY_l,      navigate,   { .i = +1 } },
    { 0,                     GDK_KEY_h,      navigate,   { .i = -1 } },

    /* vertical and horizontal scrolling, in viewport percentage */
    { 0,                     GDK_KEY_j,      scrollv,    { .i = +10 } },
    { 0,                     GDK_KEY_k,      scrollv,    { .i = -10 } },
    { 0,                     GDK_KEY_space,  scrollv,    { .i = +50 } },
    { 0,                     GDK_KEY_o,      scrollv,    { .i = -50 } },
    { 0,                     GDK_KEY_i,      scrollh,    { .i = +10 } },
    { 0,                     GDK_KEY_u,      scrollh,    { .i = -10 } },


    { 0|GDK_SHIFT_MASK,      GDK_KEY_j,      zoom,       { .i = -1 } },
    { 0|GDK_SHIFT_MASK,      GDK_KEY_k,      zoom,       { .i = +1 } },
    { 0|GDK_SHIFT_MASK,      GDK_KEY_q,      zoom,       { .i = 0  } },
    { 0,                     GDK_KEY_minus,  zoom,       { .i = -1 } },
    { 0|GDK_SHIFT_MASK,      GDK_KEY_plus,   zoom,       { .i = +1 } },
    { 0,                     GDK_KEY_equal,  zoom,       { .i = 0  } },

    { 0,                     GDK_KEY_p,      clipboard,  { .i = 1 } },
    { 0,                     GDK_KEY_y,      clipboard,  { .i = 0 } },

    { 0,                     GDK_KEY_n,      find,       { .i = +1 } },
    { 0|GDK_SHIFT_MASK,      GDK_KEY_n,      find,       { .i = -1 } },

    { MODKEY,                GDK_KEY_p,      print,      { 0 } },
    { MODKEY,                GDK_KEY_t,      showcert,   { 0 } },

    { MODKEY|GDK_SHIFT_MASK, GDK_KEY_a,      togglecookiepolicy, { 0 } },
    { 0,                     GDK_KEY_F11,    togglefullscreen, { 0 } },
    { MODKEY|GDK_SHIFT_MASK, GDK_KEY_o,      toggleinspector, { 0 } },

    { MODKEY|GDK_SHIFT_MASK, GDK_KEY_c,      toggle,     { .i = CaretBrowsing } },
    { MODKEY|GDK_SHIFT_MASK, GDK_KEY_g,      toggle,     { .i = Geolocation } },
    { MODKEY|GDK_SHIFT_MASK, GDK_KEY_s,      toggle,     { .i = JavaScript } },
    { MODKEY|GDK_SHIFT_MASK, GDK_KEY_i,      toggle,     { .i = LoadImages } },
    { MODKEY|GDK_SHIFT_MASK, GDK_KEY_b,      toggle,     { .i = ScrollBars } },
    { MODKEY|GDK_SHIFT_MASK, GDK_KEY_t,      toggle,     { .i = StrictTLS } },
    { MODKEY|GDK_SHIFT_MASK, GDK_KEY_m,      toggle,     { .i = Style } },
    { MODKEY|GDK_SHIFT_MASK, GDK_KEY_d,      toggle,     { .i = DarkMode } },
};

/* button definitions */
/* target can be OnDoc, OnLink, OnImg, OnMedia, OnEdit, OnBar, OnSel, OnAny */
static Button buttons[] = {
    /* target       event mask      button  function        argument        stop event */
    { OnLink,       0,              2,      clicknewwindow, { .i = 0 },     1 },
    { OnLink,       MODKEY,         2,      clicknewwindow, { .i = 1 },     1 },
    { OnLink,       MODKEY,         1,      clicknewwindow, { .i = 1 },     1 },
    { OnAny,        0,              8,      clicknavigate,  { .i = -1 },    1 },
    { OnAny,        0,              9,      clicknavigate,  { .i = +1 },    1 },
    { OnMedia,      MODKEY,         1,      clickexternplayer, { 0 },       1 },
};
