#!/bin/bash

while read table_name columns
do
	if [ -n "$table_name" ] ; then
		echo "table_name:	"$table_name" columns:	"$columns
	fi
done < table_info
