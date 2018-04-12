vlib work
vlib msim

vlib msim/xil_defaultlib
vlib msim/xpm

vmap xil_defaultlib msim/xil_defaultlib
vmap xpm msim/xpm

set VIVADO_ROOT "/opt/remote/rsstudent/tools/xilinx/vivado/2017.2/Vivado/2017.2"

vlog -64 -incr -sv -work xil_defaultlib  "+incdir+imports" \
"$VIVADO_ROOT/data/ip/xpm/xpm_cdc/hdl/xpm_cdc.sv" \
"$VIVADO_ROOT/data/ip/xpm/xpm_memory/hdl/xpm_memory.sv" \

vcom -64 -93 -work xpm  \
"$VIVADO_ROOT/data/ip/xpm/xpm_VCOMP.vhd" \

vlog -64 -incr -work xil_defaultlib vpcieHeader.v

vlog -64 -incr -work xil_defaultlib  "+incdir+imports" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_pipe_eq.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_pipe_drp.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_pipe_rate.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_pipe_reset.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_pipe_sync.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_gtp_pipe_rate.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_gtp_pipe_drp.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_gtp_pipe_reset.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_pipe_user.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_pipe_wrapper.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_qpll_drp.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_qpll_reset.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_qpll_wrapper.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_rxeq_scan.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_pcie_top.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_core_top.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_axi_basic_rx_null_gen.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_axi_basic_rx_pipeline.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_axi_basic_rx.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_axi_basic_top.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_axi_basic_tx_pipeline.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_axi_basic_tx_thrtl_ctl.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_axi_basic_tx.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_pcie_7x.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_pcie_bram_7x.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_pcie_bram_top_7x.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_pcie_brams_7x.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_pcie_pipe_lane.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_pcie_pipe_misc.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_pcie_pipe_pipeline.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_gt_top.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_gt_common.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_gtp_cpllpd_ovrd.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_gtx_cpllpd_ovrd.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_gt_rx_valid_filter_7x.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_gt_wrapper.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/source/pcie_7x_0_pcie2_top.v" \
"pcie_7x_0_ex.srcs/sources_1/ip/pcie_7x_0/sim/pcie_7x_0.v" \
"imports/EP_MEM.v" \
"imports/PIO.v" \
"imports/PIO_EP.v" \
"imports/PIO_EP_MEM_ACCESS.v" \
"imports/PIO_RX_ENGINE.v" \
"imports/PIO_TO_CTRL.v" \
"imports/PIO_TX_ENGINE.v" \
"imports/pci_exp_usrapp_cfg.v" \
"imports/pci_exp_usrapp_com.v" \
"imports/pci_exp_usrapp_pl.v" \
"imports/pci_exp_usrapp_rx.v" \
"imports/pci_exp_usrapp_tx.v" \
"imports/pcie_2_1_rport_7x.v" \
"imports/pcie_7x_0_gt_top_pipe_mode.v" \
"imports/pcie_7x_0_pipe_clock.v" \
"imports/pcie_7x_0_support.v" \
"imports/pcie_app_7x.v" \
"imports/pcie_axi_trn_bridge.v" \
"imports/sys_clk_gen.v" \
"imports/sys_clk_gen_ds.v" \
"imports/xilinx_pcie_2_1_ep_7x.v" \
"imports/xilinx_pcie_2_1_rport_7x.v" \
"imports/board.v" \

# compile glbl module
vlog -work xil_defaultlib "glbl.v"


vsim -pli ../cmake-build-debug/libvpi.so -t 1ps -voptargs="+acc" -L xil_defaultlib -L xpm -L unisims_ver -L unimacro_ver -L secureip -lib xil_defaultlib xil_defaultlib.board xil_defaultlib.glbl

view wave

add wave -position end -group VPCIE-Status sim:/board/RP/vpcie_status_inst/*
add wave -position end -group VPCIE-Header sim:/board/RP/vpcie_header_inst/*
add wave -position end -group tx_usrapp sim:/board/RP/tx_usrapp/*
add wave -position end -group PIO-s sim:/board/EP/app/PIO/s*
add wave -position end -group PIO-m sim:/board/EP/app/PIO/m*
run -all
