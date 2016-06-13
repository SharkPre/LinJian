#/bin/bash
awk -F "|" '{ print $2 }' test | tr '\n' ','|sed 's/ //g' > 2
