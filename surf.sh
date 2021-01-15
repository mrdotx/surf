#!/bin/sh

# path:   /home/klassiker/.local/share/repos/surf/surf.sh
# author: klassiker [mrdotx]
# github: https://github.com/mrdotx/surf
# date:   2021-01-15T14:03:54+0100

default="about:blank"
xidfile="/tmp/surf/tabbed-surf.xid"

mkdir -p "/tmp/surf"

case "$#" in
    0)
        options="$default"
        ;;
    *)
        options="$*"
        ;;
esac

for uri in $options; do
    runtabbed() {
        tabbed -cdn tabbed-surf -r 2 surf -e '' "$uri" >"$xidfile" 2>/dev/null &
        sleep .5
    }

    if [ ! -r "$xidfile" ];
    then
        runtabbed
    else
        xid=$(cat "$xidfile")
        if xprop -id "$xid" >/dev/null 2>&1;
        then
            surf -e "$xid" "$uri" >/dev/null 2>&1 &
        else
            runtabbed
        fi
    fi
done
