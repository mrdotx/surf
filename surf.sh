#!/bin/sh

# path:   /home/klassiker/.local/share/repos/surf/surf.sh
# author: klassiker [mrdotx]
# github: https://github.com/mrdotx/surf
# date:   2024-05-04T13:02:47+0200

xidfile="/tmp/surf/tabbed-surf.xid"
mkdir -p "/tmp/surf"

[ "$#" -eq 0 ] && options="-h"

for uri in "${@:-"about:blank"}"; do
    xid=$(cat "$xidfile" 2>/dev/null)
    # shellcheck disable=SC2086
    case $? in
        0)
            if xprop -id "$xid" >/dev/null 2>&1; then
                surf -e "$xid" $options "$uri" >/dev/null 2>&1 &
            else
                tabbed -cdn tabbed-surf -r 2 \
                    surf -e "$xid" $options "$uri" >/dev/null 2>&1 &
            fi
            ;;
        *)
            tabbed -cdn tabbed-surf -r 2 \
                surf -e "$xid" $options "$uri" >"$xidfile" 2>/dev/null &
            ;;
    esac
    sleep 1
done
