# VLDBJ_SUBMISSION

## Input File Format
The graph file format for data graphs and query graphs is a text format to store an undirected graph.
* The first line of the file should be "#vertices #edges" indicating the total number of distinct vertices and edges.
- Following #edges lines of "src_vertex_id dst_vertex_id". Each line refers to an edge.

* The edges should be written in the file in ascending order of ID of their src vertex and a vertex ID should  [0, #vertices - 1].

For example:

```
6 7
0 1
1 2
1 3
1 4
1 5
2 3
2 4
```

## Compiling the FPGA processing units and run

The repo contains the C++-based project export from Vitis.

To compile the binary container of the FPGA processing units, first you should import the project into Vitis.

Afterwards, project configuration should be loaded and you can immediately launch the HLS and Vivado compiler.

The following Xilinx runtime and platforms are required (Avaliable at https://www.xilinx.com/products/boards-and-kits/alveo/u200.html#gettingStarted):

Xilinx runtime : xrt_202210.2.13.466_7.8.2003

Deployment target platform : xilinx-u200-gen3x16-xdma_2022.1_2022_0415_2123

Development target Platform : xilinx-u200-gen3x16-xdma-2-202110-1-dev-1-3514848

