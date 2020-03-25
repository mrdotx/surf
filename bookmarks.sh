#!/bin/sh

# path:       ~/.local/share/repos/surf/bookmarks.sh
# author:     klassiker [mrdotx]
# github:     https://github.com/mrdotx/surf
# date:       2020-03-25T23:35:16+0100

# copy bookmarks from firefox
#printf 'select url from moz_bookmarks, moz_places where moz_places.id=moz_bookmarks.fk;\n' \
#    | sqlite3 ~/.mozilla/firefox/*.default-*/places.sqlite \
#    | awk -F '//' '{print $2}' \
#    | sed '/^$/d' \
#    | sort > ~/.config/surf/bookmarks

# copy bookmarks from brave
grep \"url\": ~/.config/BraveSoftware/Brave-Browser/Default/Bookmarks \
    | awk -F '"' '{print $4}' \
    | awk -F '//' '{print $2}' \
    | sort > ~/.config/surf/bookmarks
