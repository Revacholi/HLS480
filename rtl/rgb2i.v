


module rgb2i (
    input wire ap_clk,
    input wire ap_rst,
    input wire ap_ce,

    input wire rgb_empty_n,
    output wire rgb_read,
    input wire [23:0] rgb_dout,

    input wire i_full_n,
    output wire i_write,
    output wire [9:0] i_din
);
    // The FIFO interfaces used by HLS are FWFT(First Word Fall-Through) so can be accessed as follows:
    // the data on x_dout is valid as long as x_empty_n is 1, and the FIFO is then told that that value was read by setting x_read high for one clock cycle.
    // On the output side, you are allowed to write as long as x_full_n is high, and you do so by writing a value to x_din and simultaneously setting x_write for one clock cycle. 

    //In a purely combinatorial circuit, passing value from one FIFO to another looks as follows:

    wire [16:0] r = rgb_dout[7:0];
    wire [17:0] g = rgb_dout[15:8];
    wire [14:0] b = rgb_dout[23:16];
    assign i_din = (17'd306*r + 18'd601*g + 15'd117) >> 10;

    wire transaction = rgb_empty_n && i_full_n && ap_ce && !ap_rst;
    assign rgb_read = transaction;
    assign i_write = transaction;

endmodule

