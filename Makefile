.PHONY: clean all

all: net_val_bw net_val_lat net_val_lat_mul net_val_bw_mul

all_mul: net_val_lat_mul net_val_bw_mul

net_val_bw_mul: net_val_bw_mul.c
	mpicc -o net_val_bw_mul.out net_val_bw_mul.c bk_micro_util.c

net_val_lat_mul: net_val_lat_mul.c
	mpicc -o net_val_lat_mul.out net_val_lat_mul.c bk_micro_util.c

net_val_lat: net_val_lat.c
	mpicc -o net_val_lat.out net_val_lat.c bk_micro_util.c

net_val_bw: net_val_bw.c
	mpicc -o net_val_bw.out net_val_bw.c bk_micro_util.c

clean:
	rm *.out