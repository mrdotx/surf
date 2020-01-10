#!/bin/sh

# path:       ~/projects/surf/bookmarks.sh
# user:       klassiker [mrdotx]
# github:     https://github.com/mrdotx/surf
# date:       2020-01-10 19:25:06

# copy bookmarks from firefox
echo 'select url from moz_bookmarks, moz_places where moz_places.id=moz_bookmarks.fk;' \
    | sqlite3 ~/.mozilla/firefox/*.default-*/places.sqlite \
    | awk -F '//' '{print $2}' > ~/.config/surf/bookmarks
