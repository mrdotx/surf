#!/bin/sh

# path:       /home/klassiker/.local/share/repos/surf/surf.sh
# author:     klassiker [mrdotx]
# github:     https://github.com/mrdotx/surf
# date:       2020-12-26T11:49:26+0100

xidfile="/tmp/surf/tabbed-surf.xid"
mkdir -p "/tmp/surf"

for uri in "$@"; do

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
