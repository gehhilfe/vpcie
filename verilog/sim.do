vlib work
vlog tb.v
vlog vpcieHeader.v

vsim -pli ../cmake-build-debug/libvpi.so -t 1ps -lib work tb
view wave
add wave -position end  sim:/tb/*
run 100us
