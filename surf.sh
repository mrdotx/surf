#!/bin/sh

# path:       /home/klassiker/.local/share/repos/surf/surf.sh
# author:     klassiker [mrdotx]
# github:     https://github.com/mrdotx/surf
# date:       2020-09-14T10:00:59+0200

xidfile="/tmp/surf/tabbed-surf.xid"
uri="$1"

mkdir -p "/tmp/surf"

runtabbed() {
    tabbed -cdn tabbed-surf -r 2 surf -e '' "$uri" >"$xidfile" 2>/dev/null &
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
