#!/bin/sh

if [ "$1" = "latest_changelog_version" ]; then
	dpkg-parsechangelog | awk '$1 ~ /Version/ {print $2}'
fi
