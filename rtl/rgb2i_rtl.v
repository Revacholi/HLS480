


module rgb2i_rtl (
    input wire ap_clk,
    input wire ap_rst,
    input wire ap_ce,

    input wire [23:0] rgb,
    output wire [7:0] i
);

    wire [16:0] r = rgb[7:0];
    wire [17:0] g = rgb[15:8];
    wire [14:0] b = rgb[23:16];
    assign i = (17'd306*r + 18'd601*g + 15'd117*b) >> 10;

endmodule

