#!/bin/sh

# path:       ~/projects/surf/bookmarks.sh
# user:       klassiker [mrdotx]
# github:     https://github.com/mrdotx/surf
# date:       2020-01-13T12:10:18+0100

# copy bookmarks from firefox
echo 'select url from moz_bookmarks, moz_places where moz_places.id=moz_bookmarks.fk;' \
    | sqlite3 ~/.mozilla/firefox/*.default-*/places.sqlite \
    | awk -F '//' '{print $2}' \
    | sed '/^$/d' \
    | sort > ~/.config/surf/bookmarks
