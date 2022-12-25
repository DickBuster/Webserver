#!/bin/bash

for run in {1..50}; do
  curl http://localhost:8080/multiply.cgi?4 &

done
wait