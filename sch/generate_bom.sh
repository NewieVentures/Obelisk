#!/bin/bash

ARTIFACTS="FP1 FP2 FA1 FA2"

for i in $ARTIFACTS; do
    echo "Generating BOM for "$i
    kifield -x $i/$i.sch -i $i-BOM.xlsx --group --fields Description Manufacturer "Part Number" Supplier "Order Number"
done
