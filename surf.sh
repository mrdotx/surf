#!/bin/sh

# path:   /home/klassiker/.local/share/repos/surf/surf.sh
# author: klassiker [mrdotx]
# github: https://github.com/mrdotx/surf
# date:   2021-08-31T10:14:35+0200

xidfile="/tmp/surf/tabbed-surf.xid"

case "$#" in
    0)
        uris="about:blank"
        options="-h"
        ;;
    *)
        uris="$*"
        ;;
esac

mkdir -p "/tmp/surf"

for uri in $uris; do
    run() {
        surf -e "$xid" $options "$uri" >/dev/null 2>&1 &
    }

    runtabbed() {
        tabbed -cdn tabbed-surf -r 2 \
            surf -e "$xid" $options "$uri" >"$xidfile" 2>/dev/null &
        sleep .5
    }

    if [ -r "$xidfile" ]; then
        xid=$(cat "$xidfile")
        if xprop -id "$xid" >/dev/null 2>&1; then
            run
        else
            runtabbed
        fi
    else
        runtabbed
    fi
done
