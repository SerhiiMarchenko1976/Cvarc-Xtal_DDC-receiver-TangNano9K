//------------------------------------------------------------------------------
// Аппаратная память Dual Port RAM 256x25 бит для упаковки в Gowin BSRAM
// берем разрядность 25 бит, так как в оригинальном проекте shift_reg один 
// бит используется как флаг start (флаг нового сэмпла)
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Аппаратная упаковка в Gowin Block RAM (BSRAM) без использования defparam
//------------------------------------------------------------------------------
module dpram_256x25 (
    input clk,
    input we,                          // Разрешение записи (clken)
    input [7:0] waddr,                 // Адрес записи
    input [7:0] raddr,                 // Адрес чтения
    input [24:0] din,                  // Входные данные (25 бит)
    output [24:0] dout                 // Выходные данные (25 бит)
);

    wire [31:0] doa_unused;
    wire [31:0] dob_val;
    wire gw_gnd = 1'b0;
    wire gw_vcc = 1'b1;

    // Аппаратный примитив истинной двухпортовой памяти Gowin
    DPB dpb_inst (
        // Порт A (Только ЗАПИСЬ данных)
        .DOA(doa_unused),
        .CLKA(clk),
        .CEA(we),
        .RESETA(gw_gnd),
        .WREA(gw_vcc), 
        .BLKSELA({gw_gnd, gw_gnd, gw_gnd}),
        // Для режима x32 бита адрес смещается на 5 бит (в соответствии с документацией Gowin)
        .ADA({waddr[7:0], gw_gnd, gw_gnd, gw_gnd, gw_gnd, gw_gnd}), 
        .DIA({7'b0, din[24:0]}), 

        // Порт B (Только ЧТЕНИЕ данных)
        .DOB(dob_val),
        .CLKB(clk),
        .CEB(gw_vcc), 
        .RESETB(gw_gnd),
        .WREB(gw_gnd), 
        .BLKSELB({gw_gnd, gw_gnd, gw_gnd}),
        .ADB({raddr[7:0], gw_gnd, gw_gnd, gw_gnd, gw_gnd, gw_gnd}),
        .DIB(32'b0)
    );

    // Выделяем наши 25 бит данных из аппаратной 32-битной ячейки
    assign dout = dob_val[24:0];

endmodule


/*
module dpram_256x25 (
    input clk,
    input we,                          // Разрешение записи (clken)
    input [7:0] waddr,                 // Адрес записи
    input [7:0] raddr,                 // Адрес чтения
    input [24:0] din,                  // Входные данные (25 бит)
    output reg [24:0] dout             // Выходные данные (25 бит)
);

    // Память, которая будет автоматически упакована в Block RAM (BSRAM)
    reg [24:0] mem [0:255];

    // Синхронная запись и чтение - обязательное условие для BSRAM
    always @(posedge clk) begin
        if (we) begin
            mem[waddr] <= din;
        end
        dout <= mem[raddr];
    end

endmodule
*/