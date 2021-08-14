#!/bin/csh -f
date %FT%T >> /tmp/json
printenv >> /tmp/json
cat >> /tmp/json
echo "" >> /tmp/json
echo "Status: 204"
echo ""
