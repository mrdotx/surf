#!/bin/sh

# path:       ~/projects/surf/bookmarks.sh
# user:       klassiker [mrdotx]
# github:     https://github.com/mrdotx/surf
# date:       2019-12-26 19:29:40

# copy bookmarks from firefox developer edition
echo 'select url from moz_bookmarks, moz_places where moz_places.id=moz_bookmarks.fk;' \
    | sqlite3 ~/.mozilla/firefox/*.dev-edition-default/places.sqlite \
    | awk -F '//' '{print $2}' > ~/.config/surf/bookmarks